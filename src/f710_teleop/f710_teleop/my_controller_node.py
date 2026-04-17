import os
import re
import struct
import threading
import time
import urllib.request
import urllib.error
from typing import List

import rclpy
from geometry_msgs.msg import Twist
from rclpy.node import Node
from sensor_msgs.msg import Joy
from std_msgs.msg import Bool, Int32MultiArray, String


JS_EVENT_BUTTON = 0x01
JS_EVENT_AXIS = 0x02
JS_EVENT_INIT = 0x80


class LinuxJoystickReader(threading.Thread):
    """简单的 Linux /dev/input/jsX 手柄读取线程."""

    def __init__(self, device_path: str, axes_count: int = 8, buttons_count: int = 12):
        super().__init__(daemon=True)
        self.device_path = device_path
        self.axes: List[float] = [0.0] * axes_count
        self.buttons: List[int] = [0] * buttons_count
        self._stop_event = threading.Event()
        self._device_fd = None

    def run(self) -> None:
        while not self._stop_event.is_set():
            if self._device_fd is None:
                try:
                    self._device_fd = open(self.device_path, "rb", buffering=0)
                except OSError:
                    # 设备暂时不可用，稍后重试
                    time.sleep(1.0)
                    continue

            try:
                data = self._device_fd.read(8)
                if not data or len(data) < 8:
                    # 读到 EOF 或者短包，重连
                    self._device_fd.close()
                    self._device_fd = None
                    time.sleep(0.5)
                    continue

                _, value, etype, number = struct.unpack("IhBB", data)

                if etype & JS_EVENT_INIT:
                    etype &= ~JS_EVENT_INIT

                if etype == JS_EVENT_AXIS and 0 <= number < len(self.axes):
                    # 将 int16 [-32768, 32767] 映射到 [-1.0, 1.0]
                    self.axes[number] = float(value) / 32767.0
                elif etype == JS_EVENT_BUTTON and 0 <= number < len(self.buttons):
                    self.buttons[number] = 1 if value else 0
            except OSError:
                if self._device_fd is not None:
                    try:
                        self._device_fd.close()
                    except OSError:
                        pass
                    self._device_fd = None
                time.sleep(0.5)

    def stop(self) -> None:
        self._stop_event.set()
        if self._device_fd is not None:
            try:
                self._device_fd.close()
            except OSError:
                pass
            self._device_fd = None


class F710TeleopNode(Node):
    """根据 docs/方案.md 与 手柄数据说明 实现的控制节点."""

    @staticmethod
    def _detect_gamepad_mode(device_path: str) -> str:
        """通过 sysfs 设备名判断手柄模式.

        D 模式 (DirectInput): 设备名含 "Logitech" 或 "F710"
        X 模式 (XInput):      设备名含 "X-Box" / "Xbox" / "Microsoft"
        """
        try:
            m = re.search(r'js(\d+)', device_path)
            if not m:
                return "unknown"
            name_path = f"/sys/class/input/js{m.group(1)}/device/name"
            with open(name_path, "r") as f:
                dev_name = f.read().strip()
            if any(kw in dev_name for kw in ("X-Box", "Xbox", "Microsoft")):
                return "X"
            if any(kw in dev_name for kw in ("Logitech", "F710")):
                return "D"
            return "unknown"
        except (OSError, IOError):
            return "unknown"

    def __init__(self) -> None:
        super().__init__("my_controller_node")

        # 参数配置（默认为 /dev/input/js1，可被 YAML / 命令行覆盖）
        self.declare_parameter("device_path", "/dev/input/js1")
        self.declare_parameter("deadzone", 0.1)
        # true：死区外线性拉伸到 [-1,1]，避免刚过死区就接近满速
        self.declare_parameter("deadzone.remap", True)
        self.declare_parameter("publish_rate", 25.0)

        # 速度配置（底盘线速度 / 角速度基准）
        self.declare_parameter("velocity.max_linear", 1.0)
        self.declare_parameter("velocity.max_angular", 1.0)

        # 通过 LT / RT 调整的缩放系数：
        # speed_scale.max 作为“默认上限”，speed_scale.hard_max 作为“绝对上限”
        # 最终范围: speed_scale ∈ [min, hard_max]
        self.declare_parameter("speed_scale.min", 0.01)
        self.declare_parameter("speed_scale.max", 1.0)
        self.declare_parameter("speed_scale.hard_max", 1.5)
        self.declare_parameter("speed_scale.initial", 1.0)
        self.declare_parameter("speed_scale.step", 0.05)

        # 升降配置
        self.declare_parameter("lift.speed", 500)

        # 轴与按钮索引（可根据实际手柄映射调整）
        self.declare_parameter("axis.left_x", 0)   # 左摇杆 左右 (X)
        self.declare_parameter("axis.left_y", 1)   # 左摇杆 前后 (Y)
        # 对于当前手柄：Z / Rz 多数情况下对应右摇杆两个方向
        # 这里默认使用 2 作为“右摇杆左右”轴，如果不符可通过参数 axis.right_x 覆盖
        self.declare_parameter("axis.right_x", 2)  # 右摇杆 左右 (Z)
        # 左手柄左右：发送的 linear.y 是否取反（true=发送时取反）
        self.declare_parameter("invert.left_x", False)
        # 右手柄左右：发送的 angular.z 是否取反（true=发送时取反）
        self.declare_parameter("invert.right_x", False)
        # 右手柄有输入时是否禁用左手柄左右（true=用右摇杆时 linear.y 不发）
        self.declare_parameter("disable_left_x_when_right_active", False)
        # 右手柄角速度缩放（>1 加大转向）
        self.declare_parameter("velocity.angular_scale", 1.0)
        # true=定时器一直发 cmd_vel（即使摇杆回中）；false=仅在有速度或松杆瞬间发一次 0（不操作时不刷屏）
        self.declare_parameter("cmd_vel.publish_always", True)
        # 开机自启时：前若干秒只发全 0，避免手柄未就绪/扳机轴误映射导致乱动
        self.declare_parameter("startup.zero_cmd_sec", 0.0)
        # 0=关闭；0~1 越大越跟手、越小越稳（抑制摇杆噪声导致的偶发抖动）
        self.declare_parameter("axis.smoothing_alpha", 0.35)
        self.declare_parameter("axis.lt", 2)       # 左扳机
        self.declare_parameter("axis.rt", 5)       # 右扳机

        # true：首次按 A（上升沿）之前不发任何速度/升降（默认推荐，防开机乱动）
        self.declare_parameter("safety.gate_until_first_a", True)
        # true：必须按住 A 才允许速度/升降（松手即停）。与 gate 可同时开；仅 gate 时建议 false
        self.declare_parameter("safety.require_deadman", False)
        # true：B 上升沿急停并锁存，短按即可；再按 A（上升沿）解除。false：仅当帧发 0，不锁存
        self.declare_parameter("safety.estop_latch", True)

        # 1 是 A，2 是 B
        self.declare_parameter("button.a", 1)
        self.declare_parameter("button.b", 2)
        self.declare_parameter("button.lb", 4)
        self.declare_parameter("button.rb", 5)
        # LT / RT 用作加减速度缩放因子
        self.declare_parameter("button.lt", 8)
        self.declare_parameter("button.rt", 9)
        # X / Y 按钮用于采集控制（X=开始, Y=结束）
        self.declare_parameter("button.x", 0)
        self.declare_parameter("button.y", 3)

        # 采集（录制）功能：通过 HTTP 调用 web_control 的 /recording API
        self.declare_parameter("recording.enabled", True)
        self.declare_parameter("recording.server_url", "http://127.0.0.1:8080")

        device_path = self.get_parameter("device_path").get_parameter_value().string_value
        self.deadzone = float(self.get_parameter("deadzone").value)
        self.deadzone_remap = bool(self.get_parameter("deadzone.remap").value)
        publish_rate = float(self.get_parameter("publish_rate").value)
        self.startup_zero_cmd_sec = float(self.get_parameter("startup.zero_cmd_sec").value)
        self.axis_smoothing_alpha = float(self.get_parameter("axis.smoothing_alpha").value)
        self.gate_until_first_a = bool(self.get_parameter("safety.gate_until_first_a").value)
        self.require_deadman = bool(self.get_parameter("safety.require_deadman").value)
        self.estop_latch = bool(self.get_parameter("safety.estop_latch").value)

        self.max_linear = float(self.get_parameter("velocity.max_linear").value)
        self.max_angular = float(self.get_parameter("velocity.max_angular").value)

        # LT / RT 控制的缩放系数
        self.speed_min = float(self.get_parameter("speed_scale.min").value)
        self.speed_max = float(self.get_parameter("speed_scale.max").value)  # 默认上限
        self.speed_hard_max = float(self.get_parameter("speed_scale.hard_max").value)  # 绝对上限
        self.speed_scale = float(self.get_parameter("speed_scale.initial").value)
        self.speed_step = float(self.get_parameter("speed_scale.step").value)
        if self.speed_hard_max < self.speed_max:
            self.get_logger().warn(
                "speed_scale.hard_max < speed_scale.max, 已自动使用 speed_scale.max 作为 hard_max"
            )
            self.speed_hard_max = self.speed_max
        # 实际上限：取 max 与 hard_max 中较小者（此前未使用 speed_max，导致 yaml 里 max 不生效）
        self._speed_scale_upper = min(self.speed_max, self.speed_hard_max)
        initial_raw = self.speed_scale
        self.speed_scale = max(
            self.speed_min, min(self._speed_scale_upper, self.speed_scale)
        )
        if abs(initial_raw - self.speed_scale) > 1e-6:
            self.get_logger().info(
                f"speed_scale.initial ({initial_raw:.3f}) 已夹紧到 [min, max∩hard_max] -> {self.speed_scale:.3f}"
            )

        self.lift_speed = int(self.get_parameter("lift.speed").value)

        self.axis_left_x = int(self.get_parameter("axis.left_x").value)
        self.axis_left_y = int(self.get_parameter("axis.left_y").value)
        self.axis_right_x = int(self.get_parameter("axis.right_x").value)
        self.invert_left_x = bool(self.get_parameter("invert.left_x").value)
        self.invert_right_x = bool(self.get_parameter("invert.right_x").value)
        self.disable_left_x_when_right_active = bool(
            self.get_parameter("disable_left_x_when_right_active").value
        )
        self.angular_scale = float(self.get_parameter("velocity.angular_scale").value)
        self.cmd_vel_publish_always = bool(
            self.get_parameter("cmd_vel.publish_always").value
        )
        self.axis_lt = int(self.get_parameter("axis.lt").value)
        self.axis_rt = int(self.get_parameter("axis.rt").value)

        self.btn_a = int(self.get_parameter("button.a").value)
        self.btn_b = int(self.get_parameter("button.b").value)
        self.btn_lb = int(self.get_parameter("button.lb").value)
        self.btn_rb = int(self.get_parameter("button.rb").value)
        self.btn_lt = int(self.get_parameter("button.lt").value)
        self.btn_rt = int(self.get_parameter("button.rt").value)
        self.btn_x = int(self.get_parameter("button.x").value)
        self.btn_y = int(self.get_parameter("button.y").value)

        # 采集（录制）状态
        self.recording_enabled = bool(self.get_parameter("recording.enabled").value)
        self.recording_server_url = self.get_parameter("recording.server_url").get_parameter_value().string_value
        self._recording_active = False  # 是否正在录制（首次X才生效）
        self._prev_x = 0
        self._prev_y = 0

        # 手柄读取线程
        self.joy = LinuxJoystickReader(device_path=device_path)
        self.joy.start()

        # 外部使能控制（Web 端可通过 /f710/enable 远程启停手柄控制）
        self._f710_enabled = True
        self.enable_sub = self.create_subscription(Bool, "/f710/enable", self._on_enable_cmd, 10)
        self.status_pub = self.create_publisher(Bool, "/f710/status", 10)

        # 手柄模式检测与发布 (X / D / unknown)
        self._gamepad_mode = self._detect_gamepad_mode(device_path)
        self.mode_pub = self.create_publisher(String, "/f710/mode", 10)
        self._mode_publish_counter = 0

        # 发布者
        self.cmd_vel_pub = self.create_publisher(Twist, "/svtrobot_cmd", 10)
        self.lift_pub = self.create_publisher(Int32MultiArray, "/lift_control_cmd", 10)
        self.joy_pub = self.create_publisher(Joy, "/f710/joy", 10)

        # 发布频率
        self.timer = self.create_timer(1.0 / publish_rate, self._on_timer)
        self._t_mono_start = time.monotonic()

        # 升降状态，仅在变化时发布
        self._last_lift_direction = 0
        self._last_lift_speed = 0

        # LT/RT 触发沿检测
        self._prev_lt_pressed = 0
        self._prev_rt_pressed = 0
        # 上一周期是否发过非零 cmd_vel（用于松杆时补发一次停车）
        self._prev_cmd_vel_active = False
        self._filt_axis = {"lx": 0.0, "ly": 0.0, "rx": 0.0}
        self._prev_a = 0
        self._prev_b = 0
        self._estop_latched = False
        self._armed = False  # 已按过至少一次 A，允许发速度/升降

        self.get_logger().info(
            f"F710TeleopNode 已启动，设备: {device_path}, 频率: {publish_rate} Hz, "
            f"gate_until_first_a={self.gate_until_first_a}, require_deadman={self.require_deadman}, "
            f"estop_latch={self.estop_latch}, recording={self.recording_enabled}"
        )

    def _recording_request(self, endpoint: str) -> bool:
        """向 web_control 发送录制请求（/recording/start 或 /recording/stop）。
        在后台线程中执行，不阻塞定时器回调。返回 True 表示请求已发出（不保证成功）。"""
        if not self.recording_enabled:
            return False

        def _do():
            try:
                url = self.recording_server_url + endpoint
                req = urllib.request.Request(
                    url, method='POST',
                    headers={'Content-Type': 'application/json'},
                    data=b'{}',
                )
                with urllib.request.urlopen(req, timeout=5) as resp:
                    result = resp.read().decode()
                    self.get_logger().info(f"录制 {endpoint}: {result.strip()}")
            except urllib.error.URLError as e:
                self.get_logger().warn(f"录制 {endpoint} 请求失败: {e.reason}")
            except Exception as e:
                self.get_logger().warn(f"录制 {endpoint} 异常: {e}")

        threading.Thread(target=_do, daemon=True, name='recording-req').start()
        return True

    def _handle_recording_buttons(self, x_now: int, y_now: int) -> None:
        """处理 X/Y 按钮的采集控制。
        X（上升沿）：开始录制，仅首次有效（重复按忽略，防止误触）。
        Y（上升沿）：停止录制。
        """
        if not self.recording_enabled:
            return

        # X 上升沿 → 开始录制（仅当未在录制时）
        if x_now and not self._prev_x and not self._recording_active:
            self._recording_active = True  # 立即标记，防止重复触发
            self._recording_request('/recording/start')
            self.get_logger().info("手柄 X 按钮：开始采集")

        # Y 上升沿 → 停止录制
        if y_now and not self._prev_y and self._recording_active:
            self._recording_active = False
            self._recording_request('/recording/stop')
            self.get_logger().info("手柄 Y 按钮：停止采集")

        self._prev_x = x_now
        self._prev_y = y_now

    def _publish_status(self) -> None:
        msg = Bool()
        msg.data = self._f710_enabled
        self.status_pub.publish(msg)

    def _on_enable_cmd(self, msg: Bool) -> None:
        new_state = bool(msg.data)
        if new_state == self._f710_enabled:
            return
        self._f710_enabled = new_state
        if not new_state:
            self._publish_all_stop()
            self.get_logger().warn("F710 手柄控制已由外部禁用（/f710/enable → false）")
        else:
            self.get_logger().info("F710 手柄控制已恢复（/f710/enable → true）")
        self._publish_status()

    def _apply_deadzone(self, value: float) -> float:
        a = abs(value)
        if a < self.deadzone:
            return 0.0
        if not self.deadzone_remap or self.deadzone >= 1.0 - 1e-9:
            return value
        # 将 (deadzone, 1.0] 线性映射到 (0, 1.0]，避免刚过死区就接近满指令
        sign = 1.0 if value > 0 else -1.0
        return sign * (a - self.deadzone) / (1.0 - self.deadzone)

    def _twist_is_idle(self, twist: Twist) -> bool:
        eps = 1e-9
        return (
            abs(twist.linear.x) < eps
            and abs(twist.linear.y) < eps
            and abs(twist.linear.z) < eps
            and abs(twist.angular.x) < eps
            and abs(twist.angular.y) < eps
            and abs(twist.angular.z) < eps
        )

    def _sanitize_twist(self, twist: Twist) -> None:
        """静止时打成真正 0.0，避免取反后出现 -0.0。"""
        eps = 1e-9
        for attr in ("x", "y", "z"):
            v = getattr(twist.linear, attr)
            setattr(twist.linear, attr, 0.0 if abs(v) < eps else float(v))
            v = getattr(twist.angular, attr)
            setattr(twist.angular, attr, 0.0 if abs(v) < eps else float(v))

    def _get_axis(self, index: int) -> float:
        if 0 <= index < len(self.joy.axes):
            return self.joy.axes[index]
        return 0.0

    def _smooth_axis(self, key: str, raw: float) -> float:
        a = min(max(self.axis_smoothing_alpha, 0.0), 1.0)
        if a <= 1e-9:
            return raw
        prev = self._filt_axis[key]
        out = prev + a * (raw - prev)
        self._filt_axis[key] = out
        return out

    def _get_button(self, index: int) -> int:
        if 0 <= index < len(self.joy.buttons):
            return self.joy.buttons[index]
        return 0

    def _handle_speed_scale_change(self) -> None:
        """
        使用 LT / RT 按钮加减底盘速度缩放因子:
        - RT：增加 speed_scale
        - LT：减小 speed_scale
        范围: [speed_min, min(speed_max, speed_hard_max)]
        """
        lt_pressed = self._get_button(self.btn_lt)
        rt_pressed = self._get_button(self.btn_rt)

        # RT 上升沿：加速
        if rt_pressed and not self._prev_rt_pressed:
            old = self.speed_scale
            self.speed_scale = min(
                self._speed_scale_upper, self.speed_scale + self.speed_step
            )
            self.get_logger().info(
                f"RT 加速: {old:.2f} -> {self.speed_scale:.2f}"
            )

        # LT 上升沿：减速
        if lt_pressed and not self._prev_lt_pressed:
            old = self.speed_scale
            self.speed_scale = max(self.speed_min, self.speed_scale - self.speed_step)
            self.get_logger().info(
                f"LT 减速: {old:.2f} -> {self.speed_scale:.2f}"
            )

        self._prev_lt_pressed = lt_pressed
        self._prev_rt_pressed = rt_pressed

    def _compute_cmd_vel(self) -> Twist:
        left_y = self._apply_deadzone(
            self._smooth_axis("ly", self._get_axis(self.axis_left_y))
        )
        left_x = self._apply_deadzone(
            self._smooth_axis("lx", self._get_axis(self.axis_left_x))
        )
        right_x = self._apply_deadzone(
            self._smooth_axis("rx", self._get_axis(self.axis_right_x))
        )

        # 底盘速度根据当前 speed_scale 进行缩放
        # 这里对前后方向取反，使“推前为正”（根据当前手柄坐标系调整）
        vx = -left_y * self.max_linear * self.speed_scale
        vy = left_x * self.max_linear * self.speed_scale
        wz = right_x * self.max_angular * self.speed_scale * self.angular_scale

        # 右手柄有输入时禁用左手柄左右（不发 linear.y）
        if self.disable_left_x_when_right_active and abs(right_x) > 0:
            vy = 0.0

        # 右手柄取反：左手柄往后（left_y > 0）时不取反，其余情况按 invert.right_x
        apply_right_invert = self.invert_right_x and (left_y <= 0)

        msg = Twist()
        msg.linear.x = vx
        msg.linear.y = -vy if self.invert_left_x else vy  # 发送时对 vy 取反
        msg.angular.z = -wz if apply_right_invert else wz  # 发送时对 angular.z 取反（往后时不取反）
        return msg

    def _compute_lift_command(self) -> (int, int):
        lb = self._get_button(self.btn_lb)
        rb = self._get_button(self.btn_rb)

        if rb and not lb:
            direction = 1
        elif lb and not rb:
            direction = -1
        else:
            direction = 0

        speed = self.lift_speed if direction != 0 else 0
        return direction, speed

    def _publish_all_stop(self) -> None:
        twist = Twist()
        self.cmd_vel_pub.publish(twist)
        lift_msg = Int32MultiArray()
        lift_msg.data = [0, 0]
        self.lift_pub.publish(lift_msg)
        self._last_lift_direction = 0
        self._last_lift_speed = 0
        self._prev_cmd_vel_active = False

    def _publish_mode(self) -> None:
        """发布手柄模式 (X/D/unknown)，约每秒发布一次."""
        self._mode_publish_counter += 1
        # publish_rate 默认 25Hz，每 25 次即约 1 秒发一次
        if self._mode_publish_counter >= 25:
            self._mode_publish_counter = 0
            # 重新检测（手柄可能被拔插或切换）
            new_mode = self._detect_gamepad_mode(self.joy.device_path)
            if new_mode != self._gamepad_mode:
                if new_mode == "X":
                    self.get_logger().warn(
                        f"手柄模式从 {self._gamepad_mode} 变为 X 模式，已阻断控制！请拨到 D 模式"
                    )
                elif new_mode == "D" and self._gamepad_mode == "X":
                    self.get_logger().info("手柄已切换到 D 模式，恢复正常控制")
                self._gamepad_mode = new_mode
            msg = String()
            msg.data = self._gamepad_mode
            self.mode_pub.publish(msg)

    def _on_timer(self) -> None:
        # 发布原始手柄状态（始终发布，供录制使用）
        joy_msg = Joy()
        joy_msg.header.stamp = self.get_clock().now().to_msg()
        joy_msg.axes = list(self.joy.axes)
        joy_msg.buttons = list(self.joy.buttons)
        self.joy_pub.publish(joy_msg)

        # 定期检测并发布手柄模式
        self._publish_mode()

        # 采集按钮处理（独立于底盘控制，任何状态下都响应）
        x_now = self._get_button(self.btn_x)
        y_now = self._get_button(self.btn_y)
        self._handle_recording_buttons(x_now, y_now)

        # X 模式下阻断所有控制指令
        if self._gamepad_mode == "X":
            self._publish_all_stop()
            self._publish_status()
            return

        # 外部禁用时只发全零
        if not self._f710_enabled:
            self._publish_all_stop()
            self._publish_status()
            return

        a_now = self._get_button(self.btn_a)
        b_now = self._get_button(self.btn_b)
        try:
            # B 上升沿：急停（短按即可；锁存后需按 A 上升沿解除）
            if b_now and not self._prev_b:
                if self.estop_latch:
                    self._estop_latched = True
                self._publish_all_stop()
                self.get_logger().warn(
                    "急停(B)：已清零"
                    + ("并锁存，请再按一次 A 解除" if self.estop_latch else "")
                )
                return

            if self._estop_latched and a_now and not self._prev_a:
                self._estop_latched = False
                self.get_logger().info("急停已解除（A）")

            if self._estop_latched:
                self._publish_all_stop()
                return

            in_startup = (
                self.startup_zero_cmd_sec > 0.0
                and (time.monotonic() - self._t_mono_start) < self.startup_zero_cmd_sec
            )
            if in_startup:
                twist = Twist()
                self.cmd_vel_pub.publish(twist)
                self._prev_cmd_vel_active = False
                return

            # 未按过 A 前：不发速度/升降（仅发全 0，与急停类似）
            if self.gate_until_first_a and not self._armed:
                if a_now and not self._prev_a:
                    self._armed = True
                    self.get_logger().info("已使能：检测到 A，开始允许发速度/升降指令")
                if not self._armed:
                    twist = Twist()
                    self.cmd_vel_pub.publish(twist)
                    self._prev_cmd_vel_active = False
                    if self._last_lift_direction != 0 or self._last_lift_speed != 0:
                        lift_msg = Int32MultiArray()
                        lift_msg.data = [0, 0]
                        self.lift_pub.publish(lift_msg)
                        self._last_lift_direction = 0
                        self._last_lift_speed = 0
                    return

            if self.require_deadman and not a_now:
                twist = Twist()
                self._sanitize_twist(twist)
                if self.cmd_vel_publish_always:
                    self.cmd_vel_pub.publish(twist)
                elif self._prev_cmd_vel_active:
                    self.cmd_vel_pub.publish(twist)
                    self._prev_cmd_vel_active = False
                if self._last_lift_direction != 0 or self._last_lift_speed != 0:
                    lift_msg = Int32MultiArray()
                    lift_msg.data = [0, 0]
                    self.lift_pub.publish(lift_msg)
                    self._last_lift_direction = 0
                    self._last_lift_speed = 0
                return

            # LT / RT 调整底盘速度缩放（仅使能时）
            self._handle_speed_scale_change()

            # 发布底盘速度（静止时数值归零，避免 linear.x: -0.0 等）
            twist = self._compute_cmd_vel()
            self._sanitize_twist(twist)
            idle = self._twist_is_idle(twist)
            if self.cmd_vel_publish_always:
                self.cmd_vel_pub.publish(twist)
                self._prev_cmd_vel_active = not idle
            else:
                if not idle:
                    self.cmd_vel_pub.publish(twist)
                    self._prev_cmd_vel_active = True
                elif self._prev_cmd_vel_active:
                    self.cmd_vel_pub.publish(twist)
                    self._prev_cmd_vel_active = False

            direction, speed = self._compute_lift_command()
            if direction != self._last_lift_direction or speed != self._last_lift_speed:
                lift_msg = Int32MultiArray()
                lift_msg.data = [direction, speed]
                self.lift_pub.publish(lift_msg)
                self._last_lift_direction = direction
                self._last_lift_speed = speed
        finally:
            self._prev_a = a_now
            self._prev_b = b_now

    def destroy_node(self) -> bool:
        self.joy.stop()
        self._publish_all_stop()
        self._publish_status()
        return super().destroy_node()


def main(args=None) -> None:
    rclpy.init(args=args)
    node = F710TeleopNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

