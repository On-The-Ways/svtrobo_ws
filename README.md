# SVTROBO - 四轮独立转向/独立驱动全向移动机器人

基于 ROS2 Humble 的 4WIS4WID 全向移动机器人控制系统，支持 Web 浏览器、F710 手柄、Python API 多种控制方式。

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
- 左摇杆控制前后/平移，右摇杆控制转向
- A 解锁使能，B 急停，LB/RB 升降，LT/RT 加减速
- 40% 死区 + 低通滤波，运动平滑
- 发布原始手柄数据到 `/f710/joy`，支持录制
- Web 前端可通过 `/f710/enable` 远程启停

```
ros2 launch f710_teleop f710_teleop.launch.py
```

### web_control — Web 控制台

基于 aiohttp + roslibjs 的浏览器控制界面。

- 实时 MJPEG 摄像头画面（D405 ×2, ZED 2i）
- WASD/Q/E 键盘控制底盘
- 升降控制、电机状态、电池/温度诊断
- 手柄/Web 模式一键切换（默认手柄模式）
- 数据采集：ros2 bag + 摄像头帧同步录制

```
python3 src/web_control/server.py          # Web 服务 (8080)
ros2 launch rosbridge_server rosbridge_websocket_launch.py  # rosbridge (9090)
```

### camera_driver — 摄像头驱动

支持 RealSense D405 和 ZED 2i 的 Python 采集模块。

- 彩色图 + 深度图采集，深度对齐到彩色
- 相机内参获取、点云生成（RealSense）
- 上下文管理器自动释放资源

## 快速启动

### 一键启动全部服务

```bash
cd ~/svtrobo_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
bash src/start_all.sh
```

启动内容：底盘控制 → rosbridge → Web 服务。手柄设备检测到则自动启动手柄节点。

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
| `/chassis/joint_states` | sensor_msgs/JointState | 底盘 → 外部 | 电机位置/速度/力矩 (100Hz) |
| `/chassis/diagnostics` | ChassisDiagnostics | 底盘 → 外部 | 电压/温度/错误码 (100Hz) |

## 数据录制

Web 控制台右下角录制按钮，采集内容：

- ROS2 bag：`/svtrobot_cmd` `/lift_control_cmd` `/f710/joy` `/chassis/joint_states` `/chassis/diagnostics`
- 摄像头帧：D405 ×2 + ZED，1fps 保存为 JPEG

保存路径：`~/svtrobo_ws/recordings/<时间戳>/`

## 文档索引

| 文档 | 内容 |
|------|------|
| [ROBOT_SYSTEM_GUIDE.md](ROBOT_SYSTEM_GUIDE.md) | 系统完整技术文档 |
| [src/chassis_control/PYTHON_API_GUIDE.md](src/chassis_control/PYTHON_API_GUIDE.md) | Python 控制接口文档 |
| [src/web_control/WEB_CONTROL_GUIDE.md](src/web_control/WEB_CONTROL_GUIDE.md) | Web 控制台使用说明 |
| [src/camera_driver/CAMERA_DRIVER_GUIDE.md](src/camera_driver/CAMERA_DRIVER_GUIDE.md) | 摄像头驱动 API 文档 |
| [src/f710_teleop/手柄操作指导说明.md](src/f710_teleop/手柄操作指导说明.md) | 手柄按键说明 |
