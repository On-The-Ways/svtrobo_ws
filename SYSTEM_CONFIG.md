# svt 服务器系统配置文档

> 最后更新: 2026-04-18
> 硬件: Jetson Orin (aarch64), L4T 5.15.185-tegra
> IP: 10.0.0.56 (eno1), 用户: svt

---

## 1. 系统信息

| 项目 | 值 |
|------|-----|
| Kernel | 5.15.185-tegra aarch64 PREEMPT |
| PCAN Driver | pcan.ko Release_20260126_n |
| ROS2 | Humble |
| Python | 3.10 |
| Node.js | v22.22.2 |
| pnpm | /usr/bin/pnpm |

## 2. systemd 自启服务 (共7个, 全部 enabled)

### 启动顺序与依赖链

```
sysinit.target
  └─ f710-fix.service (最早，Before=basic.target)
       └─ svtrobo-can.service (oneshot, 等 PCAN USB 就绪 ~30s)
            ├─ svtrobo-rosbridge.service (rosbridge WebSocket :9090)
            │    └─ svtrobo-chassis.service (chassis_control + lift_control)
            │         ├─ svtrobo-f710.service (F710 手柄, 等 js0 最多 30s)
            │         ├─ svtrobo-web.service (web_control :8080)
            │         └─ svtrobo-nodeapi.service (Node.js API :28181)
            └─ (以上均 Wants=svtrobo-can.service)
```

所有服务均 `Restart=on-failure, RestartSec=5`（f710-fix 和 svtrobo-can 除外，它们是 oneshot）。

### 2.1 f710-fix.service

**用途**: 解决 Jetson 内核内置 hid-logitech 驱动在 F710 上 probe 失败的问题。
**原理**: 在任何 USB 设备绑定前，设置 `ignore_special_drivers=1`，让 hid-core 跳过 hid-logitech，使用 hid-generic。

```ini
[Unit]
Description=F710 Gamepad Fix (Jetson hid-logitech workaround)
DefaultDependencies=no
Before=basic.target

[Service]
Type=oneshot
ExecStart=/bin/bash -c "echo 1 > /sys/module/hid/parameters/ignore_special_drivers"
RemainAfterExit=yes

[Install]
WantedBy=sysinit.target
```

**注意**: DO NOT 用 rmmod hid_logitech（builtin 不可能卸载）或 /etc/modprobe.d quirks（对 builtin 模块无效）。

### 2.2 svtrobo-can.service

**用途**: 等待 PCAN USB 设备注册、稳定后配置 CAN 接口 + 清理 FastRTPS 残留。

```ini
[Unit]
Description=Configure CAN interfaces for svtrobo
After=sysinit.target f710-fix.service
Wants=f710-fix.service

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/bin/bash -c '\
WAIT_DEV=0; \
while [ $WAIT_DEV -lt 90 ] && ! ip link show can0 >/dev/null 2>&1; do \
  sleep 1; WAIT_DEV=$((WAIT_DEV+1)); \
done; \
logger "svtrobo-can: can0 appeared after ${WAIT_DEV}s"; \
if [ $WAIT_DEV -ge 90 ]; then logger "svtrobo-can: FATAL can0 not found after 90s"; exit 1; fi; \
sleep 5; \
ERR_BEFORE=$(dmesg | grep -c "pcan.*err"); \
WAIT=0; \
while [ $WAIT -lt 30 ]; do \
  sleep 1; \
  ERR_NOW=$(dmesg | grep -c "pcan.*err"); \
  if [ "$ERR_NOW" -eq "$ERR_BEFORE" ]; then break; fi; \
  ERR_BEFORE=$ERR_NOW; WAIT=$((WAIT+1)); \
done; \
logger "svtrobo-can: PCAN stabilized after ${WAIT}s (errors: $ERR_BEFORE)"; \
for can_if in can0 can1; do \
  ip link set $can_if down 2>/dev/null; \
  ip link set $can_if type can bitrate 1000000 dbitrate 5000000 fd on 2>/dev/null; \
  ip link set $can_if up 2>/dev/null; \
done; \
for can_if in can2 can3; do \
  ip link set $can_if down 2>/dev/null; \
  ip link set $can_if type can bitrate 1000000 2>/dev/null; \
  ip link set $can_if up 2>/dev/null; \
done; \
for can_if in can4 can5; do \
  ip link set $can_if down 2>/dev/null; \
  ip link set $can_if type can bitrate 500000 2>/dev/null; \
  ip link set $can_if up 2>/dev/null; \
done; \
chmod 777 /dev/ttyACM0 2>/dev/null; \
rm -rf /dev/shm/fastrtps_* 2>/dev/null; \
logger "svtrobo-can: CAN configured, FastRTPS shm cleaned"'

[Install]
WantedBy=multi-user.target
```

**CAN 配置**:

| 接口 | 模式 | Bitrate | 用途 |
|------|------|---------|------|
| can0 | CAN FD | 1M/5M | 底盘控制 |
| can1 | CAN FD | 1M/5M | 底盘控制 |
| can2 | CAN 2.0 | 1M | 底盘控制 |
| can3 | CAN 2.0 | 1M | 底盘控制 |
| can4 | CAN 2.0 | 500K | 扩展 |
| can5 | CAN 2.0 | 500K | 扩展 |

### 2.3 svtrobo-rosbridge.service

```ini
[Unit]
Description=ROS2 Rosbridge WebSocket Server
After=network.target svtrobo-can.service
Wants=svtrobo-can.service

[Service]
Type=simple
User=svt
Environment=ROS_DOMAIN_ID=0
ExecStart=/bin/bash -c "source /opt/ros/humble/setup.bash && source /home/svt/svtrobo_ws/install/setup.bash && exec ros2 launch rosbridge_server rosbridge_websocket_launch.xml"
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**端口**: 9090 (WebSocket)

### 2.4 svtrobo-chassis.service

```ini
[Unit]
Description=svtrobo Chassis Control
After=svtrobo-rosbridge.service svtrobo-can.service
Wants=svtrobo-rosbridge.service svtrobo-can.service

[Service]
Type=simple
User=svt
Environment=ROS_DOMAIN_ID=0
ExecStart=/bin/bash -c "source /opt/ros/humble/setup.bash && source /home/svt/svtrobo_ws/install/setup.bash && exec ros2 launch chassis_control svtrobo_bringup.launch.py"
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**启动节点**: chassis_control_node, lift_control

**已知问题**: chassis_control_node crash 时 ros2 launch 父进程不退出，systemd 不会触发 Restart。需手动 `systemctl restart svtrobo-chassis`。检测方法: `ros2 node list` 不含 `/chassis_control`。

### 2.5 svtrobo-f710.service

```ini
[Unit]
Description=svtrobo F710 Gamepad Teleop
After=svtrobo-chassis.service f710-fix.service
Wants=svtrobo-chassis.service f710-fix.service

[Service]
Type=simple
User=svt
Environment=ROS_DOMAIN_ID=0
ExecStartPre=/bin/bash -c "for i in $(seq 1 30); do [ -e /dev/input/js0 ] && exit 0; sleep 1; done; exit 1"
ExecStart=/bin/bash -c "source /opt/ros/humble/setup.bash && source /home/svt/svtrobo_ws/install/setup.bash && exec ros2 launch f710_teleop f710_teleop.launch.py"
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**启动节点**: my_controller_node

**ExecStartPre**: 等待 /dev/input/js0 出现（最多30秒）

### 2.6 svtrobo-web.service

```ini
[Unit]
Description=svtrobo Web Control Panel
After=svtrobo-chassis.service

[Service]
Type=simple
User=svt
WorkingDirectory=/home/svt/svtrobo_ws/src/web_control
Environment=ROS_DOMAIN_ID=0
ExecStart=/bin/bash -c "source /opt/ros/humble/setup.bash && source /home/svt/svtrobo_ws/install/setup.bash && exec python3 server.py --host 0.0.0.0 --port 8080"
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**端口**: 8080 (HTTP)

### 2.7 svtrobo-nodeapi.service

```ini
[Unit]
Description=svtrobo ROS Process API (Node.js)
After=svtrobo-chassis.service

[Service]
Type=simple
User=svt
WorkingDirectory=/home/svt/ros_process_api
Environment=ROS2_WORKSPACE_DIR=/home/svt/svtrobo_ws
Environment=SUDO_PASSWORD=<REDACTED>
Environment=PATH=/usr/bin:/bin:/usr/local/bin
ExecStart=/usr/bin/node /home/svt/ros_process_api/app.bundle.js
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**端口**: 28181 (HTTP)

**环境变量覆盖**: app.bundle.js 硬编码了 /home/openarm 路径和 openarm 密码，通过 `ROS2_WORKSPACE_DIR` 和 `SUDO_PASSWORD` 环境变量覆盖。

## 3. udev 规则

### 3.1 PCAN udev 规则 (PCAN 驱动自带)

文件: `/etc/udev/rules.d/` 中 pcan 相关规则（驱动安装时自动生成），创建 `/dev/pcan-usb_pro_fd/` 等符号链接，MODE=0666。

### 3.2 CAN 接口重命名规则

文件: `/etc/udev/rules.d/` (use_pcan_as_can0_can1.sh 生成)

```
# Rename onboard mttcan to avoid can0/can1 conflicts.
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="mttcan", KERNEL=="can0", NAME="mttcan0"
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="mttcan", KERNEL=="can1", NAME="mttcan1"

# Map PEAK PCAN USB to can0/can1
SUBSYSTEM=="net", ACTION=="add", KERNEL=="can2", NAME="can0"
SUBSYSTEM=="net", ACTION=="add", KERNEL=="can3", NAME="can1"
```

**原理**: Jetson 内置 mttcan 会占 can0/can1 名称，通过 udev 将其重命名为 mttcan0/mttcan1，让 PCAN USB 设备使用 can0/can1。

## 4. USB 硬件拓扑

```
Bus 02 (USB 3.0, 10000M)
  └─ Hub (Realtek 4-port)
       └─ ZED 2i (f880, Video, 5000M)

Bus 01 (USB 2.0, 480M)
  ├─ AX210 Bluetooth
  └─ Hub (Realtek 4-port USB 2.0)
       ├─ Hub (Microchip 2514)
       │    ├─ F710 Gamepad (046d:c219, via hid-generic)
       │    ├─ PCAN-USB Pro FD (0c72:0011)
       │    ├─ PCAN-USB Pro FD (0c72:0011)
       │    ├─ PCAN-USB Pro FD (0c72:0011)
       │    └─ Hub (Microchip 2512)
       │         ├─ CH340 Serial (ttyACM0, 升降控制)
       │         └─ ZED 2i HID (f881, IMU)
       └─ Hub (Genesys Logic)
```

**3个 PCAN-USB Pro FD** 经两层 USB 2.0 Hub 连接，这是 err -71 的根因。

## 5. ROS2 节点配置

### 5.1 F710 手柄配置
文件: `src/f710_teleop/config/f710_teleop.yaml`

```yaml
# 关键参数
device_path: "/dev/input/js0"
deadzone: 0.4
publish_rate: 25.0
safety.gate_until_first_a: true
safety.estop_latch: true

# 速度
velocity.max_linear: 0.4
velocity.max_angular: 0.4
velocity.angular_scale: 5.0
speed_scale.initial: 0.25
speed_scale.max: 1.0
speed_scale.hard_max: 1.5

# 轴映射
axis.left_x: 0    # 左摇杆 左右
axis.left_y: 1    # 左摇杆 前后
axis.right_x: 2   # 右摇杆 左右 → angular.z

# 按钮映射 (D-mode, hid-generic)
button.a: 1
button.b: 2
button.lb: 4
button.rb: 5
button.lt: 6
button.rt: 7

# 采集按钮
button.x: 0       # 开始录制
button.y: 3       # 停止录制
recording.enabled: true
recording.server_url: "http://127.0.0.1:8080"

# 取反
invert.left_x: true
invert.right_x: true
disable_left_x_when_right_active: true

# 升降
lift.speed: 500
```

### 5.2 底盘参数
文件: `src/chassis_control/config/params.yaml`

```yaml
chassis_control:
  ros__parameters:
    robot:
      chassis_radius: 0.0875
      wheel_perimeter: 0.647
      fl_motor_start_angle: 0.2
      fr_motor_start_angle: 4.9
      rl_motor_start_angle: 2.7
      rr_motor_start_angle: 3.5
```

## 6. 网络配置

| 接口 | 状态 | 地址 |
|------|------|------|
| eno1 | UP | 10.0.0.56/24 |
| mttcan0 | DOWN | (Jetson 内置 CAN, udev 重命名) |
| mttcan1 | DOWN | (Jetson 内置 CAN, udev 重命名) |
| can0-5 | UP | (PCAN USB) |
| docker0 | DOWN | 172.17.0.1/16 |

## 7. 故障恢复快速参考

### CAN 全部 DOWN (PCAN err-71)
```bash
# 快速恢复 (modprobe 重载 + CAN 配置 + 服务重启)
ssh svt@10.0.0.56 "echo '123456' | sudo -S modprobe -r pcan && sleep 2 && echo '123456' | sudo -S modprobe pcan && sleep 5 && echo '123456' | sudo -S systemctl restart svtrobo-can && echo '123456' | sudo -S systemctl restart svtrobo-rosbridge && sleep 2 && echo '123456' | sudo -S systemctl restart svtrobo-chassis svtrobo-f710 svtrobo-web svtrobo-nodeapi && rm -rf /dev/shm/fastrtps_* && echo RECOVERED"
```

### chassis_control_node 崩溃 (SDO write timeout)
```bash
ssh svt@10.0.0.56 "echo '123456' | sudo -S systemctl restart svtrobo-chassis svtrobo-f710"
```

### DDS 发现失败 (节点互相看不到)
```bash
ssh svt@10.0.0.56 "echo '123456' | sudo -S rm -rf /dev/shm/fastrtps_* && echo '123456' | sudo -S systemctl restart svtrobo-rosbridge svtrobo-chassis svtrobo-f710"
```

### F710 手柄无摇杆数据
换电池。低电量时摇杆轴最先失效，按键和扳机仍有响应。

### 手柄设备丢失 (/dev/input/js0 不存在)
```bash
ssh svt@10.0.0.56 "echo '123456' | sudo -S sh -c 'echo 1 > /sys/module/hid/parameters/ignore_special_drivers && echo 1-4.2.1.4 > /sys/bus/usb/drivers/usb/unbind && sleep 2 && echo 1-4.2.1.4 > /sys/bus/usb/drivers/usb/bind && sleep 3 && ls -la /dev/input/js0'"
```
注意: USB 路径会随 Hub 布局变化，用 `lsusb -t` 查当前路径。

## 8. 已知限制 (待改进)

1. **chassis_control_node 崩溃不触发 systemd 重启** — ros2 launch 父进程不退出，systemd 不会自动恢复
2. **PCAN err-71 运行时不稳定** — USB 2.0 Hub + 3个PCAN 过载，可能随时掉线
3. **FastRTPS shm 残留** — 频繁重启后 DDS 发现完全失败
4. **ZED 2i IMU** — HID 绑定不稳定，需手动 unbind/bind
5. **F710 低电量** — 摇杆轴先失效，无低电量告警
