"""
ZED 2i IMU 传感器驱动

通过 USB HID 接口直接读取 IMU 数据，无需 ZED SDK 或 CUDA。
基于 stereolabs/zed-open-capture 协议。

数据包含:
  - 加速度计 (m/s^2)
  - 陀螺仪 (deg/s, rad/s)
  - 磁力计 (uT)
  - 温度、气压、湿度

用法:
    from camera_driver.zed_imu import ZEDIMU

    imu = ZEDIMU()
    imu.start()
    data = imu.read()   # dict or None
    imu.stop()

    # 上下文管理器
    with ZEDIMU() as imu:
        data = imu.read()
"""

import fcntl
import os
import select
import struct
import threading
import time

# HID ioctl 常量 (_IOWR('H', nr, len))
def _HIDIOCSFEATURE(n):
    return (3 << 30) | (n << 16) | (0x48 << 8) | 6

def _HIDIOCGFEATURE(n):
    return (3 << 30) | (n << 16) | (0x48 << 8) | 7


# ---- ZED 2i HID 协议常量 ----
VID_ZED = 0x2b03
PID_ZED2i_MCU = 0xf881

REP_ID_SENSOR_DATA = 0x01
REP_ID_REQUEST_SET = 0x21
REP_ID_SENSOR_STREAM_STATUS = 0x32
RQ_CMD_PING = 0xF2

# 缩放因子 (from zed-open-capture)
DEFAULT_GRAVITY = 9.8189
ACC_SCALE = DEFAULT_GRAVITY * (8.0 / 32768.0)   # m/s^2 per LSB
GYRO_SCALE_DPS = 1000.0 / 32768.0                # deg/s per LSB
GYRO_SCALE_RAD = GYRO_SCALE_DPS * (3.14159265 / 180.0)  # rad/s per LSB
MAG_SCALE = 1.0 / 16.0                           # uT per LSB
TEMP_SCALE = 0.01                                 # deg C per LSB
TS_SCALE = 39062.5                                # raw * this = nanoseconds


class ZEDIMU:
    """ZED 2i IMU 传感器管理类

    通过 hidraw 直接读取 USB HID 接口获取 IMU 数据。
    """

    def __init__(self, hidraw_path=None, ping_interval=400):
        """
        Args:
            hidraw_path: hidraw 设备路径，默认自动搜索
            ping_interval: 发送 ping 保活的读取次数间隔 (~400次 ≈ 1秒)
        """
        self._hidraw_path = hidraw_path
        self._ping_interval = ping_interval

        self._fd = None
        self._running = False
        self._read_count = 0

        # 最新数据 (线程安全)
        self._lock = threading.Lock()
        self._latest = None

        # 后台读取线程
        self._thread = None

    # ---- 自动查找 hidraw 设备 ----

    @staticmethod
    def find_hidraw():
        """查找 ZED 2i MCU 对应的 hidraw 设备路径

        Returns:
            str: 如 '/dev/hidraw3'，未找到返回 None
        """
        import glob
        for path in sorted(glob.glob('/dev/hidraw*')):
            try:
                with open(f'/sys/class/hidraw/{os.path.basename(path)}/device/uevent') as f:
                    content = f.read()
                if f'HID_ID=0003:{VID_ZED:08X}:{PID_ZED2i_MCU:08X}' in content:
                    return path
            except Exception:
                continue
        return None

    # ---- 启动/停止 ----

    def start(self):
        """启动 IMU 数据流"""
        if self._running:
            return

        # 查找设备
        if self._hidraw_path is None:
            self._hidraw_path = self.find_hidraw()
        if self._hidraw_path is None:
            raise RuntimeError('未找到 ZED 2i IMU HID 设备 (VID=2b03, PID=f881)')

        # 打开设备
        try:
            self._fd = os.open(self._hidraw_path, os.O_RDWR)
        except OSError as e:
            raise RuntimeError(f'无法打开 {self._hidraw_path}: {e}，请检查 udev 规则权限')

        # 启用数据流 (Feature Report 0x32 = enable)
        buf = bytearray(2)
        buf[0] = REP_ID_SENSOR_STREAM_STATUS
        buf[1] = 0x01
        fcntl.ioctl(self._fd, _HIDIOCSFEATURE(2), bytes(buf))

        self._running = True
        self._read_count = 0

        # 启动后台读取线程
        self._thread = threading.Thread(target=self._read_loop, daemon=True)
        self._thread.start()

        print(f'[ZED-IMU] 已启动, 设备: {self._hidraw_path}')

    def stop(self):
        """停止 IMU 数据流"""
        if not self._running:
            return
        self._running = False

        if self._thread is not None:
            self._thread.join(timeout=3)
            self._thread = None

        if self._fd is not None:
            try:
                # 关闭数据流
                buf = bytearray(2)
                buf[0] = REP_ID_SENSOR_STREAM_STATUS
                buf[1] = 0x00
                fcntl.ioctl(self._fd, _HIDIOCSFEATURE(2), bytes(buf))
            except Exception:
                pass
            os.close(self._fd)
            self._fd = None

        print('[ZED-IMU] 已停止')

    def _read_loop(self):
        """后台读取线程"""
        while self._running:
            try:
                ready, _, _ = select.select([self._fd], [], [], 2.0)
                if not ready:
                    continue

                data = os.read(self._fd, 64)
                if len(data) < 46 or data[0] != REP_ID_SENSOR_DATA:
                    continue

                parsed = self._parse(data)
                if parsed:
                    with self._lock:
                        self._latest = parsed
                    self._read_count += 1

                    # 周期性 ping 保活
                    if self._read_count % self._ping_interval == 0:
                        self._send_ping()

            except OSError:
                if self._running:
                    print('[ZED-IMU] 读取错误，设备可能已断开')
                break
            except Exception as e:
                if self._running:
                    print(f'[ZED-IMU] 异常: {e}')

    def _send_ping(self):
        """发送 ping 保持数据流活跃 (约每秒一次)"""
        try:
            buf = bytearray(2)
            buf[0] = REP_ID_REQUEST_SET
            buf[1] = RQ_CMD_PING
            fcntl.ioctl(self._fd, _HIDIOCSFEATURE(2), bytes(buf))
        except Exception:
            pass

    # ---- 数据解析 ----

    @staticmethod
    def _parse(data):
        """解析 61 字节传感器报告

        参考 zed-open-capture sensorcapture_def.hpp 中的 RawData 结构
        """
        if len(data) < 46:
            return None

        imu_valid = data[1] == 0
        timestamp_raw = struct.unpack_from('<Q', data, 2)[0]
        gx, gy, gz = struct.unpack_from('<3h', data, 10)
        ax, ay, az = struct.unpack_from('<3h', data, 16)
        frame_sync = data[22]
        sync_cap = data[23]
        sync_count = struct.unpack_from('<I', data, 24)[0] if len(data) >= 28 else 0
        imu_temp = struct.unpack_from('<h', data, 28)[0] if len(data) >= 30 else 0

        # 磁力计 (可选)
        mag_valid = data[30] if len(data) > 30 else 0
        mx, my, mz = (0, 0, 0)
        if len(data) >= 37:
            mx, my, mz = struct.unpack_from('<3h', data, 31)

        # 运动检测
        camera_moving = data[37] if len(data) > 37 else 0
        moving_count = struct.unpack_from('<I', data, 38)[0] if len(data) >= 42 else 0
        camera_falling = data[42] if len(data) > 42 else 0
        falling_count = struct.unpack_from('<I', data, 43)[0] if len(data) >= 47 else 0

        # 环境传感器 (可选)
        env_valid = data[46] if len(data) > 46 else 0
        env_temp = struct.unpack_from('<h', data, 47)[0] if len(data) >= 49 else 0
        pressure = struct.unpack_from('<I', data, 49)[0] if len(data) >= 53 else 0
        humidity = struct.unpack_from('<I', data, 53)[0] if len(data) >= 57 else 0

        # 相机温度
        temp_cam_l = struct.unpack_from('<h', data, 57)[0] if len(data) >= 59 else 0
        temp_cam_r = struct.unpack_from('<h', data, 59)[0] if len(data) >= 61 else 0

        ts_ns = timestamp_raw * TS_SCALE

        return {
            'valid': imu_valid,
            'timestamp_ns': ts_ns,
            'timestamp_s': ts_ns / 1e9,

            # 加速度计 (m/s^2)
            'accel': (ax * ACC_SCALE, ay * ACC_SCALE, az * ACC_SCALE),

            # 陀螺仪 (rad/s 和 deg/s)
            'gyro_rad': (gx * GYRO_SCALE_RAD, gy * GYRO_SCALE_RAD, gz * GYRO_SCALE_RAD),
            'gyro_dps': (gx * GYRO_SCALE_DPS, gy * GYRO_SCALE_DPS, gz * GYRO_SCALE_DPS),

            # 磁力计 (uT)
            'mag': (mx * MAG_SCALE, my * MAG_SCALE, mz * MAG_SCALE),
            'mag_valid': mag_valid,

            # 温度 (°C)
            'imu_temp': imu_temp * TEMP_SCALE,

            # 环境传感器
            'env_valid': env_valid,
            'env_temp': env_temp * TEMP_SCALE,
            'pressure': pressure * 0.0001,  # FW >= v3.9
            'humidity': humidity * 0.01,

            # 同步信息
            'frame_sync': frame_sync,
        }

    # ---- 读取接口 ----

    def read(self):
        """获取最新一次 IMU 采样数据

        Returns:
            dict: IMU 数据，包含 accel/gyro_rad/gyro_dps/mag/温度等
            None: 未启动或无数据
        """
        with self._lock:
            return self._latest

    # ---- 上下文管理器 ----

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *args):
        self.stop()
        return False


# 快速测试
if __name__ == '__main__':
    print('ZED 2i IMU 测试 (Ctrl+C 退出)')
    print('-' * 60)

    with ZEDIMU() as imu:
        time.sleep(0.5)  # 等待第一批数据
        for i in range(30):
            data = imu.read()
            if data:
                a = data['accel']
                g = data['gyro_dps']
                print(f'[{i:3d}] Accel(m/s²): X={a[0]:+8.4f} Y={a[1]:+8.4f} Z={a[2]:+8.4f}  '
                      f'Gyro(dps): X={g[0]:+7.3f} Y={g[1]:+7.3f} Z={g[2]:+7.3f}  '
                      f'T={data["imu_temp"]:.1f}°C')
            else:
                print(f'[{i:3d}] 无数据')
            time.sleep(0.05)
