# SVTROBO Web 控制台使用文档

> 基于 roslibjs + aiohttp 的浏览器端机器人控制面板

---

## 目录

1. [系统概述](#1-系统概述)
2. [架构说明](#2-架构说明)
3. [前置条件](#3-前置条件)
4. [快速启动](#4-快速启动)
5. [功能模块](#5-功能模块)
6. [ROS Topic 通信映射](#6-ros-topic-通信映射)
7. [相机画面](#7-相机画面)
8. [底盘键盘控制](#8-底盘键盘控制)
9. [升降机构控制](#9-升降机构控制)
10. [电机状态面板](#10-电机状态面板)
11. [电池与诊断面板](#11-电池与诊断面板)
12. [文件结构](#12-文件结构)
13. [常见问题与排错](#13-常见问题与排错)

---

## 1. 系统概述

Web 控制台提供以下功能：

- **相机画面**：实时 MJPEG 视频流（D405 #1、D405 #2、ZED 2i）
- **底盘控制**：浏览器端 WASD/Q/E 键盘控制，支持速度调节
- **升降机构**：竖向滑条控制升降速度，上升/下降/停止按钮
- **电机状态**：实时显示 4 个舵向电机的角度/速度/力矩及轮速（目标 vs 实际）
- **电池与诊断**：总线电压（VBUS）、电量估算、电机温度、错误码

所有底盘控制通过 **rosbridge WebSocket** 转发 ROS2 Topic 消息，无需在机器人上安装桌面环境。

---

## 2. 架构说明

```
┌──────────────────────────────────────────────────────────────┐
│                       浏览器                                 │
│                                                              │
│  index.html + JS 模块（app / chassis / chassis-status /      │
│  camera / diagnostics / lift / status）                      │
│                                                              │
│         │ ws://host:9090              │ http://host:8080     │
│         │ (roslibjs)                  │ (MJPEG / REST)      │
└─────────┼──────────────────────────────┼─────────────────────┘
          │                              │
          ▼                              ▼
┌─────────────────┐          ┌──────────────────────────┐
│ rosbridge_server │          │  aiohttp Web Server       │
│  (port 9090)     │          │  (server.py, port 8080)   │
│                  │          │                            │
│  ROS2 ↔ WebSocket│          │  - 静态文件托管             │
│  协议转换         │          │  - 相机 MJPEG 流           │
└────────┬─────────┘          │  - 相机启停 REST API       │
         │                     └────────────┬──────────────┘
         │ ROS2 DDS                        │ camera_driver
         ▼                                  ▼
┌──────────────────────────────────────────────────┐
│              ROS2 节点 (底盘 C++ 节点)              │
│                                                    │
│  /chassis/joint_states  (100Hz)                    │
│  /chassis/cmd_feedback  (100Hz)                    │
│  /chassis/diagnostics   (100Hz)                    │
│  /svtrobot_cmd         (控制指令)                   │
│  /lift_control_cmd     (升降控制)                   │
└──────────────────────────────────────────────────┘
```

**两个服务：**

| 服务 | 端口 | 功能 |
|------|------|------|
| `rosbridge_server` | 9090 | ROS2 ↔ WebSocket 协议转换，传输 Topic 消息 |
| `aiohttp Web Server` | 8080 | 静态文件托管、相机 MJPEG 流、相机启停 REST API |

---

## 3. 前置条件

### 依赖

```bash
# ROS2 环境
source /opt/ros/humble/setup.bash

# rosbridge_server
sudo apt install ros-humble-rosbridge-server

# Python 依赖
pip3 install aiohttp opencv-python numpy

# 相机驱动（如需使用相机功能）
pip3 install pyrealsense2
```

### 底盘节点

Web 控制台依赖底盘 C++ 节点发布的状态数据：

```bash
cd /home/openarm/svtrobo_ws
colcon build --packages-select chassis_control
source install/setup.bash
ros2 launch chassis_control svtrobo_bringup.launch.py
```

---

## 4. 快速启动

### 方式一：一键启动（推荐）

```bash
cd /home/openarm/svtrobo_ws/web_control
bash start_web.sh
```

该脚本自动启动 rosbridge_server 和 aiohttp Web 服务器。

### 方式二：手动启动

```bash
# 终端 1：启动 rosbridge_server
ros2 launch rosbridge_server rosbridge_websocket_launch.xml port:=9090

# 终端 2：启动 Web 服务器
cd /home/openarm/svtrobo_ws/web_control
python3 server.py --host 0.0.0.0 --port 8080
```

### 访问控制台

在浏览器中打开：

```
http://<机器人IP>:8080
```

页面顶部输入 rosbridge 地址（如 `ws://localhost:9090`），点击 **连接** 按钮。

> 如果浏览器和机器人不在同一台机器上，将 `localhost` 替换为机器人的实际 IP 地址。

---

## 5. 功能模块

### 模块概览

| JS 模块 | 文件 | 功能 |
|---------|------|------|
| App | `app.js` | rosbridge 连接管理，模块初始化调度 |
| Chassis | `chassis.js` | WASD/Q/E 键盘控制，速度滑条，指令发布 |
| ChassisStatus | `chassis-status.js` | 舵向电机角度/速度/力矩 + 轮速实时显示 |
| Camera | `camera.js` | 相机启停控制，MJPEG 流显示 |
| Diagnostics | `diagnostics.js` | VBUS 电压、电量、电机温度、错误码 |
| Lift | `lift.js` | 升降机构速度滑条 + 方向按钮 |
| StatusMonitor | `status.js` | ROS Topic 列表显示 |

---

## 6. ROS Topic 通信映射

Web 控制台通过 rosbridge WebSocket 订阅和发布以下 ROS2 Topic：

| Web 模块 | ROS Topic | 消息类型 | 方向 | 用途 |
|----------|-----------|---------|------|------|
| Chassis | `/svtrobot_cmd` | `geometry_msgs/Twist` | 发布 | 底盘速度指令 |
| Chassis | `/chassis/cmd_feedback` | `geometry_msgs/Twist` | 订阅 | 速度反馈显示 |
| ChassisStatus | `/chassis/joint_states` | `sensor_msgs/JointState` | 订阅 | 舵向角度/速度/力矩 + 轮速目标值 |
| ChassisStatus | `/chassis/diagnostics` | `chassis_control/msg/ChassisDiagnostics` | 订阅 | ZLAC8015D 实际轮速 |
| Diagnostics | `/chassis/diagnostics` | `chassis_control/msg/ChassisDiagnostics` | 订阅 | VBUS、温度、错误码 |
| Lift | `/lift_control_cmd` | `std_msgs/Int32MultiArray` | 发布 | 升降控制指令 |

---

## 7. 相机画面

### 支持的相机

| 名称 | 类型 | 分辨率 | FPS |
|------|------|--------|-----|
| D405 #1 | RealSense | 640x480 | 15 |
| D405 #2 | RealSense | 640x480 | 15 |
| ZED 2i | OpenCV V4L2 | 672x376 | 15 |

### 操作方式

1. 在"相机画面"区域，点击对应相机的 **启动** 按钮
2. Web 后端通过 `camera_driver` 模块启动相机采集线程
3. 画面通过 MJPEG 流 (`/camera/{name}`) 实时推送到浏览器
4. 点击 **停止** 按钮关闭相机并释放资源

### REST API

| 端点 | 方法 | 说明 |
|------|------|------|
| `/camera/{name}` | GET | MJPEG 视频流（name: `d405_1` / `d405_2` / `zed`） |
| `/camera/start` | POST | 启动相机（body: `{"camera": "d405_1"}`） |
| `/camera/stop` | POST | 停止相机（body: `{"camera": "d405_1"}`） |
| `/camera/status` | GET | 获取所有相机状态 |

> 相机采集线程以后台守护线程运行，帧通过有界队列传递，MJPEG 编码质量为 70。

---

## 8. 底盘键盘控制

### 操作方式

1. 鼠标点击"底盘与升降控制"面板区域获取焦点
2. 页面显示"点击此处启用键盘控制"提示消失后即可操控
3. 按下方向键立即运动，松开或按空格停止

### 键盘映射

```
  W ── 前进 (vx > 0)
  S ── 后退 (vx < 0)
  A ── 左移 (vy > 0)
  D ── 右移 (vy < 0)
  Q ── 逆时针旋转 (wz > 0)
  E ── 顺时针旋转 (wz < 0)
  空格 ── 停止
```

支持组合键（如 W+A 斜向前进左移、W+Q 弧线运动），合成速度会自动归一化。

### 速度控制

速度滑条范围：**0.05 ~ 0.50 m/s**，默认 0.30 m/s。旋转速度固定为 0.50 rad/s。

### 发布频率

键盘按下期间以 **20Hz**（50ms 间隔）持续发布速度指令，确保底盘持续运动。松开所有键后发送零速指令。

### 安全机制

- 面板失焦（点击其他区域）时自动停止底盘
- 键盘松开时发送零速指令
- 底盘内部有舵向到位保护和轮速上限机制

---

## 9. 升降机构控制

### 操作方式

控制面板位于底盘控制右侧，包含：

- **速度滑条**：0 ~ 500 RPM，默认 300 RPM
- **上升按钮**：正转，以滑条设定速度运行
- **下降按钮**：反转，以滑条设定速度运行
- **停止按钮**：断开使能

### ROS Topic

通过 `/lift_control_cmd`（`std_msgs/msg/Int32MultiArray`）发送控制指令：

```javascript
// 上升
msg.data = [1, speed]    // direction=1, speed=RPM

// 下降
msg.data = [-1, speed]   // direction=-1

// 停止
msg.data = [0, 0]        // direction=0
```

---

## 10. 电机状态面板

### 显示内容

**舵向电机表：**

| 列 | 数据来源 | 单位 |
|----|---------|------|
| 角度 | `/chassis/joint_states` position[0..3] | 度（从 rad 转换） |
| 速度 | `/chassis/joint_states` velocity[0..3] | rad/s |
| 力矩 | `/chassis/joint_states` effort[0..3] | Nm |

**轮速表：**

| 列 | 数据来源 | 单位 |
|----|---------|------|
| 目标 | `/chassis/joint_states` velocity[4..7] | RPM |
| 实际 | `/chassis/diagnostics` wheel_speeds_actual | RPM |

> 实际轮速从 ZLAC8015D 驱动器读取，约每 10 秒更新一次。当轮速 > 1 RPM 时，实际值显示为蓝色高亮。

---

## 11. 电池与诊断面板

### 电池状态

| 显示项 | 数据来源 | 说明 |
|--------|---------|------|
| 电压 | `diagnostics.vbus` | RobStride 电机 VBUS 读取（约每 20s 更新） |
| 电量 | `diagnostics.vbus` | 基于 6S LiPo (19.8V~25.2V) 线性估算 |
| 温度 | `diagnostics.motor_temperatures` | 4 个电机中的最高温度 |

电量颜色指示：
- > 50%：绿色
- 20%~50%：黄色
- < 20%：红色

### 电机诊断

每个舵向电机显示温度和错误码：

| 显示项 | 颜色规则 |
|--------|---------|
| 温度 > 70°C | 红色 |
| 温度 > 50°C | 黄色 |
| 温度 ≤ 50°C | 默认色 |
| 错误码 = 0 | 显示 "OK" |
| 错误码 ≠ 0 | 显示 "ERR:0xXX" |

> **注意**：rosbridge 将 `uint8[]` 类型编码为 base64 字符串，Web 端已做自动解码处理。

---

## 12. 文件结构

```
web_control/
├── server.py                  # aiohttp Web 服务器（静态文件 + 相机流 + REST API）
├── start_web.sh               # 一键启动脚本（rosbridge + aiohttp）
└── static/
    ├── index.html             # 控制台主页面
    ├── css/
    │   └── style.css          # 样式表
    └── js/
        ├── roslib.min.js      # roslibjs 库（WebSocket ROS 通信）
        ├── app.js             # 主应用（rosbridge 连接管理、模块初始化）
        ├── chassis.js         # 底盘键盘控制 + 速度滑条
        ├── chassis-status.js  # 舵向电机角度/速度/力矩 + 轮速显示
        ├── camera.js          # 相机启停控制 + MJPEG 显示
        ├── diagnostics.js     # VBUS/温度/错误码诊断面板
        ├── lift.js            # 升降机构控制
        └── status.js          # ROS Topic 列表显示
```

---

## 13. 常见问题与排错

### Q1: 连接 rosbridge 失败

1. 确认 rosbridge_server 已启动：`ros2 node list` 应看到 `/rosbridge_websocket_server`
2. 检查端口是否正确（默认 9090）：`netstat -tlnp | grep 9090`
3. 如果浏览器不在机器人本机，将 `localhost` 替换为机器人 IP
4. 检查防火墙是否放行了 9090 端口

### Q2: 底盘控制无响应

1. 确认底盘 C++ 节点已启动并完成初始化
2. 点击控制面板获取焦点（提示文字消失）
3. 检查 rosbridge 连接状态是否为"已连接"
4. 终端运行 `ros2 topic echo /svtrobot_cmd` 确认指令是否发出

### Q3: 相机画面无法显示

1. 确认相机已连接：`rs-enumerate-devices --short`（RealSense）或 `ls /dev/video*`（ZED）
2. 确认 `pyrealsense2` 已安装：`pip3 show pyrealsense2`
3. 点击"启动"按钮后等待 1~2 秒（相机预热需要时间）
4. 检查 Web 服务器日志中的错误信息

### Q4: 电机状态面板显示 "--"

1. 确认底盘 C++ 节点正在运行
2. 确认 rosbridge 已连接
3. 检查 Topic 是否发布：`ros2 topic hz /chassis/joint_states`

### Q5: 电压/温度显示 "--"

- VBUS 约每 20 秒更新一次，首次读取需要等待
- 电机温度在 `motor_temperatures` 中提供，确保 `ChassisDiagnostics` 消息正常发布
- 检查：`ros2 topic echo /chassis/diagnostics --once`

### Q6: 错误码显示异常（非数字字符）

rosbridge 对 `uint8[]` 使用 base64 编码，Web 端已做解码处理。如仍显示异常，检查 `diagnostics.js` 中的 base64 解码逻辑。

### Q7: 如何从外部网络访问？

```bash
# 确保防火墙放行端口
sudo ufw allow 8080
sudo ufw allow 9090

# 启动时绑定所有接口
python3 server.py --host 0.0.0.0 --port 8080
```

然后在浏览器访问 `http://<机器人IP>:8080`，rosbridge 地址填写 `ws://<机器人IP>:9090`。

### Q8: 如何自定义相机配置？

编辑 `server.py` 中的 `CAMERA_CONFIG` 字典：

```python
CAMERA_CONFIG = {
    'd405_1': {'type': 'realsense', 'serial': '409122272399', 'size': (640, 480), 'fps': 15},
    'd405_2': {'type': 'realsense', 'serial': '409122273344', 'size': (640, 480), 'fps': 15},
    'zed':    {'type': 'zed',       'serial': None,           'size': None,       'fps': 15},
}
```

修改后重启 Web 服务器生效。
