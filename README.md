# SVTROBO - 四轮独立转向/独立驱动全向移动机器人

基于 ROS2 Humble 的 4WIS4WID 全向移动机器人控制系统，支持 Web 浏览器、F710 手柄、Python API 多种控制方式。

## 项目结构

```
svtrobo_ws/
├── src/
│   ├── start_all.sh                                  # 一键启动全部服务
│   ├── stop_all.sh                                   # 一键停止全部服务
│   ├── chassis_control/                              # 底盘控制 (C++, 1kHz)
│   │   ├── src/                                      #   底盘/舵向/轮驱/升降/滤波 实现文件
│   │   ├── include/chassis_control/                  #   头文件
│   │   ├── config/params.yaml                        #   ROS2 参数（底盘半径、零位角度等）
│   │   ├── launch/svtrobo_bringup.launch.py          #   底盘+升降 启动文件
│   │   ├── msg/ChassisDiagnostics.msg                #   自定义诊断消息
│   │   └── scripts/                                  #   Python 控制器封装 & 测试脚本
│   ├── f710_teleop/                                  # F710 手柄遥操作
│   │   ├── f710_teleop/my_controller_node.py         #   手柄节点（X/D 模式检测）
│   │   ├── config/f710_teleop.yaml                   #   手柄参数（死区、速度上限等）
│   │   └── launch/f710_teleop.launch.py              #   启动文件
│   ├── web_control/                                  # Web 控制台 (aiohttp)
│   │   ├── server.py                                 #   Web 服务器 (8080)
│   │   ├── bag_converter.py                          #   bag (.db3) → JSONL 自动转换
│   │   ├── static/                                   #   前端（HTML/CSS/JS 模块）
│   │   └── WEB_CONTROL_GUIDE.md                      #   Web 控制台使用说明
│   └── camera_driver/                                # 摄像头驱动 (Python)
│       └── camera_driver/
│           ├── realsense_camera.py                    #   RealSense D405 驱动
│           ├── realsense_node.py                      #   RealSense ROS2 节点
│           ├── zed_camera.py                          #   ZED 2i 驱动 (SDK + sl.Mat复用)
│           ├── zed_node.py                            #   ZED ROS2 节点
│           └── CAMERA_DRIVER_GUIDE.md                 #   摄像头驱动 API 文档
├── scripts/                                          # 运维脚本
│   ├── chassis_watchdog.sh                           #   底盘节点存活监控
│   └── pcan_monitor.sh                               #   PCAN/CAN 状态监控
├── systemd/                                          # systemd service 文件备份
├── recordings/                                       # 录制数据存储（含 README 格式说明）
├── SYSTEM_CONFIG.md                                  # 系统配置文档 (9个systemd服务)
├── ROBOT_SYSTEM_GUIDE.md                             # 系统完整技术文档
└── README.md                                         # 本文件
```

## 系统架构

```
┌─────────────────────────────────────────────────────┐
│                   控制接口                            │
│  Web 浏览器 (roslibjs)  │  F710 手柄 (/dev/input/js0)  │  Python API  │
└────────────┬────────────┴──────────┬──────────────┘
             │                       │
       rosbridge (9090)         /f710/enable
             │                       │
┌────────────▼───────────────────────▼──────────────┐
│  /svtrobot_cmd (Twist)    /lift_control_cmd       │
└────────────┬──────────────────────┬──────────────┘
             │                      │
   ┌─────────▼──────────┐  ┌───────▼───────┐
   │  chassis_control    │  │  lift_control  │
   │  (C++, 1000Hz)      │  │  (RS485/Modbus)│
   └──┬───┬───┬───┬───┘  └───────────────┘
      │   │   │   │
   CAN2 转向电机  CAN3 轮电机
   0x65 0x66 0x67 0x68    ZLAC8015D ×2
   RobStride ×4
```

## 功能包说明

### chassis_control — 底盘与升降控制

核心 C++ 控制节点，1kHz 控制循环。

- **4 轮独立转向**：4 个 RobStride 电机（CAN2），闭环角度控制
- **4 轮独立驱动**：2 个 ZLAC8015D 驱动器（CAN3），左右轮各控一对
- **升降机构**：RS485/Modbus 控制
- **指令超时保护**：0.5s 无新指令自动归零
- **诊断发布**：电压、温度、错误码、轮速（100Hz）

```
ros2 launch chassis_control svtrobo_bringup.launch.py
```

### f710_teleop — F710 手柄遥操作

Python ROS2 节点，直接读取 `/dev/input/jsX` 设备。

- **手柄模式：D 模式（DirectInput）**，不是 X 模式
- **X/D 模式自动检测**：读取 sysfs 设备名称判断手柄模式，X 模式自动禁用控制并前端警告
- 左摇杆控制前后/平移，右摇杆控制转向
- A 解锁使能，B 急停，LB/RB 升降，LT/RT 加减速
- 40% 死区 + 低通滤波，运动平滑
- 发布原始手柄数据到 `/f710/joy`，支持录制
- Web 前端可通过 `/f710/enable` 远程启停
- 发布模式状态到 `/f710/mode`（"X"/"D"/"unknown"）

```
ros2 launch f710_teleop f710_teleop.launch.py
```

### web_control — Web 控制台

基于 aiohttp + roslibjs 的浏览器控制界面。

- 实时 MJPEG 摄像头画面（D405 ×2, ZED 2i）
- WASD/Q/E 键盘控制底盘
- 升降控制、电机状态、电池/温度诊断
- 手柄/Web 模式一键切换（默认手柄模式）
- X 模式手柄自动检测与前端警告横幅
- 数据采集：彩色图(13.5fps) + 深度图 + 点云(~1.3Hz) + IMU(~70Hz) + ROS2 bag
- **录制结束后自动转换**：bag (.db3) → JSONL 格式，方便深度学习训练
- IMU 实时 WebSocket 推送 (/ws/imu) + HTTP 回退 (/api/imu)
- ROS 连接断开时统一清理所有模块状态（摄像头、底盘、诊断面板归位）

```
python3 src/web_control/server.py          # Web 服务 (8080)
ros2 launch rosbridge_server rosbridge_websocket_launch.py  # rosbridge (9090)
```

### camera_driver — 摄像头驱动

支持 RealSense D405 和 ZED 2i 的 Python 采集模块。

- **ZED SDK 模式**：HD720 彩色图 + NEURAL 深度 + XYZRGBA 点云 + IMU (~70Hz)
- **RealSense D405**：彩色图 + 深度图，快速设备检测避免阻塞
- sl.Mat 对象复用，减少每帧 C++ 堆分配开销
- 相机内参获取、点云降采样 (2x)
- IMU 独立线程读取，前端 WebSocket 实时推送

## 快速启动

### 一键启动全部服务

```bash
cd ~/svtrobo_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
bash src/start_all.sh
```

启动内容：底盘控制 → rosbridge → Web 服务。手柄设备 `/dev/input/js0` 检测到则自动启动手柄节点。

### 一键停止

```bash
bash src/stop_all.sh
```

### 单独启动

```bash
# 仅底盘 + 升降
ros2 launch chassis_control svtrobo_bringup.launch.py

# 仅手柄
ros2 launch f710_teleop f710_teleop.launch.py

# Web 控制（需先启动 rosbridge）
ros2 launch rosbridge_server rosbridge_websocket_launch.py
python3 src/web_control/server.py
```

### 从源码构建

```bash
cd ~/svtrobo_ws
source /opt/ros/humble/setup.bash
colcon build --base-paths src
source install/setup.bash
```

## ROS2 话题

| 话题 | 类型 | 方向 | 说明 |
|------|------|------|------|
| `/svtrobot_cmd` | geometry_msgs/Twist | 控制 → 底盘 | 线速度 x/y + 角速度 z |
| `/lift_control_cmd` | std_msgs/Int32MultiArray | 控制 → 升降 | [方向, 速度] |
| `/f710/joy` | sensor_msgs/Joy | 手柄 → 外部 | 原始摇杆/按钮状态 |
| `/f710/enable` | std_msgs/Bool | Web → 手柄 | 启停手柄控制 |
| `/f710/status` | std_msgs/Bool | 手柄 → Web | 手柄使能状态 |
| `/f710/mode` | std_msgs/String | 手柄 → Web | 手柄模式检测："X"/"D"/"unknown" |
| `/chassis/joint_states` | sensor_msgs/JointState | 底盘 → 外部 | 电机位置/速度/力矩 (100Hz) |
| `/chassis/diagnostics` | ChassisDiagnostics | 底盘 → 外部 | 电压/温度/错误码 (100Hz) |

## 数据录制

Web 控制台右下角录制按钮或 F710 手柄 X/Y 按钮，自动采集：

| 数据 | 格式 | 频率 | 说明 |
|------|------|------|------|
| ZED 彩色图 | JPEG | 13.5 fps | 1280x720, ~49KB/帧 |
| ZED 深度图 | JPEG (JET colormap) | 13.5 fps | 归一化着色后保存 |
| ZED 点云 | npz | ~1.3 fps | 2x降采样 (360x640), ~3.5MB/帧 |
| IMU | imu.jsonl | ~70 Hz | accel/gyro/mag/pressure/temp |
| ROS2 bag | db3 | 原始频率 | 5个话题，录制结束自动转JSONL |
| 录制摘要 | summary.json | - | 时长/帧数/大小 |

> **前端只显示彩色流**，深度/点云数据仅在后台录制保存。每小时约 20GB。

保存路径：`~/svtrobo_ws/recordings/<时间戳>/`

```
recordings/YYYYMMDD_HHMMSS/
├── images/zed/                  # 彩色图 JPEG (13.5fps)
├── depth/zed/                   # 深度图 JET colormap JPEG (13.5fps)
├── pointcloud/zed/              # 点云 npz (2x降采样, ~1.3Hz)
├── rosbag/                      # ROS2 bag (5个话题)
├── imu.jsonl                    # IMU数据 (~70Hz)
├── summary.json                 # 录制摘要
├── chassis_joint_states.jsonl   # 8 关节状态 (~10Hz)
├── chassis_diagnostics.jsonl    # 电压/温度/错误码/轮速 (~10Hz)
├── svtrobot_cmd.jsonl           # 底盘速度指令 (~50Hz)
├── f710_joy.jsonl               # 手柄原始数据 (~50Hz)
└── lift_control_cmd.jsonl       # 升降控制指令 (~25Hz)
```

> 所有 JSONL 记录均包含 `_timestamp_ns` 字段，可用于多话题时间对齐。详见 [recordings/README.md](recordings/README.md)。

## 文档索引

| 文档 | 内容 |
|------|------|
| [ROBOT_SYSTEM_GUIDE.md](ROBOT_SYSTEM_GUIDE.md) | 系统完整技术文档 |
| [recordings/README.md](recordings/README.md) | 录制数据格式与 JSONL 字段说明 |
| [src/chassis_control/PYTHON_API_GUIDE.md](src/chassis_control/PYTHON_API_GUIDE.md) | Python 控制接口文档 |
| [src/web_control/WEB_CONTROL_GUIDE.md](src/web_control/WEB_CONTROL_GUIDE.md) | Web 控制台使用说明 |
| [src/camera_driver/CAMERA_DRIVER_GUIDE.md](src/camera_driver/CAMERA_DRIVER_GUIDE.md) | 摄像头驱动 API 文档 |
| [src/f710_teleop/手柄操作指导说明.md](src/f710_teleop/手柄操作指导说明.md) | 手柄按键说明 |
