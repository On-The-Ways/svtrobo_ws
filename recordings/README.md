# SVTROBO 数据采集目录说明

本文档描述 `recordings/` 目录下采集数据的组织结构、文件格式及内容含义。

> 最后更新: 2026-04-18

---

## 采集触发方式

数据采集可通过以下方式触发：

1. **Web 控制台**：点击右下角录制按钮
2. **F710 手柄**：X 按钮（index 0）开始采集，Y 按钮（index 3）停止采集
   - X 按钮仅首次按有效（防止误触重复触发）
   - 通过 HTTP 调用 web_control 的 /recording/start 和 /recording/stop 接口

### 自动相机管理

- 开始采集时，自动启动所有未运行的相机（d405_1、d405_2、zed）
- 停止采集时，仅关闭由采集启动的相机（手动启动的相机不受影响）
- 相机启动失败（如未连接）会跳过并记录警告，不影响其他数据采集

---

## 目录结构

每次录制会在 `recordings/` 下生成一个以时间戳命名的会话目录：

```
recordings/
└── YYYYMMDD_HHMMSS/                  # 录制会话（如 20260417_212712）
    ├── images/                        # 彩色图
    │   ├── zed/                       # ZED 2i (HD720, 2Hz, JPEG q95, ~161KB/帧)
    │   │   ├── 1744900000123456.jpg   # 文件名 = Unix 微秒时间戳
    │   │   └── ...
    │   ├── d405_1/                    # D405 #1 (1280x720, 2Hz, JPEG q95)
    │   ├── d405_2/                    # D405 #2 (1280x720, 2Hz, JPEG q95)
    ├── depth/                         # 深度图 (JET colormap JPEG)
    │   ├── zed/                       # ZED 深度 (2Hz, JPEG q95, ~52KB/帧, 0-20m归一化)
    │   ├── d405_1/                    # D405 #1 深度 (1280x720, 2Hz, JPEG q95, 0-1m归一化)
    │   └── d405_2/                    # D405 #2 深度 (1280x720, 2Hz, JPEG q95, 0-1m归一化)
    ├── pointcloud/                    # 3D点云 (float16, 全分辨率)
    │   └── zed/                       # ZED XYZRGBA float16 (720x1280, ~7MB/帧, 2Hz)
    │       ├── 1744900000123456.npz   # npz非压缩格式
    │       └── ...
    ├── rosbag/                        # ROS2 原始录制数据
    │   ├── rosbag_0.db3               # SQLite3 数据库
    │   └── metadata.yaml              # 录制元信息
    ├── imu.jsonl                      # IMU数据 (~70Hz)
    ├── summary.json                   # 录制摘要
    ├── chassis_joint_states.jsonl     # 关节/电机状态 (~10Hz)
    ├── chassis_diagnostics.jsonl      # 底盘诊断 (~10Hz)
    ├── svtrobot_cmd.jsonl             # 底盘运动指令
    ├── f710_joy.jsonl                 # 手柄输入
    └── lift_control_cmd.jsonl         # 升降控制指令
```

---

## 1. 图像数据 (`images/`)

### 1.1 概述

| 项目 | 说明 |
|------|------|
| 来源 | 3 个相机：D405 #1、D405 #2、ZED 2i |
| 格式 | JPEG (quality 95) |
| ZED 采集频率 | 15 Hz (相机内部), 2 Hz (录制保存, deadline-based) |
| D405 采集频率 | 6 Hz (相机内部), 2 Hz (录制保存, deadline-based) |
| 命名规则 | `{timestamp_us}.jpg`（Unix 微秒时间戳） |
| 对齐方式 | 文件名即精确采集时间戳（微秒级），可直接与 JSONL 中的 `_timestamp_ns`（纳秒级）对齐 |

> **采集触发方式**：可通过 Web 控制台的录制按钮或 F710 手柄按钮触发采集——手柄 **X 按钮** 开始采集，**Y 按钮** 停止采集。
>
> **自动相机管理**：开始采集时自动启动所有未运行的相机（d405_1/d405_2/zed），停止采集时自动关闭由采集启动的相机。

### 1.2 各相机参数

| 相机 | 类型 | 分辨率 | 深度模式 | 用途 |
|------|------|--------|----------|------|
| `d405_1` | RealSense D405 | 1280x720 | z16 | 前方近距离深度/彩色 |
| `d405_2` | RealSense D405 | 1280x720 | z16 | 侧方近距离深度/彩色 |
| `zed` | ZED 2i (SDK) | 1280x720 (HD720) | NEURAL | 全局视角立体视觉 |

---

## 2. 深度图数据 (`depth/`)

| 项目 | 说明 |
|------|------|
| 格式 | JPEG q95 (JET colormap 着色后编码) |
| ZED 深度范围 | 0-20m 归一化 |
| D405 深度范围 | 0-1m 归一化 |
| 采集频率 | 与彩色图同步 2Hz (deadline-based) |
| JPEG质量 | 95 |

> **注意**：深度图是归一化+colormap着色后的可视化JPEG，非原始深度数据。原始深度值为 float32 (mm)，归一化到 [0,1] 后用 `cv2.COLORMAP_JET` 着色。

---

## 3. 点云数据 (`pointcloud/`)

| 项目 | 说明 |
|------|------|
| 格式 | npz (numpy 非压缩)，key 为 `xyzrgba` |
| 数据类型 | float16, shape (H, W, 4) — X, Y, Z (mm) + RGBA 打包 (可配置PC_DTYPE) |
| 采样频率 | 2 Hz (deadline-based) |
| 单帧大小 | ~7MB (float16, 全分辨率 720x1280; float32 时 ~14MB) |
| 仅 ZED | 仅 ZED 2i 支持点云采集

> 采集时使用 float16 精度保存（全分辨率 720x1280），每帧从 14MB(float32) 降至 ~7MB，2Hz录制。可通过 server.py 中 `PC_DOWNSAMPLE`（降采样，默认1=全分辨率）和 `PC_DTYPE`（默认float16）参数随时调整。 |

> 使用 `np.load("xxx.npz")["xyzrgba"]` 读取。

---

## 4. IMU 数据 (`imu.jsonl`)

IMU 数据直接从 ZED SDK 读取，写入 JSONL 文件（不经过 ROS2 话题）。

- **频率**：~70 Hz
- **来源**：ZED 2i 内置 IMU（独立线程读取）
- **内容示例**：

```json
{
  "accel": {"x": 0.12, "y": -0.03, "z": -9.81},
  "gyro_dps": {"x": 0.01, "y": -0.02, "z": 0.00},
  "gyro_rad": {"x": 0.0001, "y": -0.0003, "z": 0.0},
  "mag": {"x": 0.21, "y": -0.05, "z": -0.43},
  "mag_valid": true,
  "imu_temp": 32.5,
  "pressure": 1013.25,
  "env_temp": 25.0,
  "timestamp_ns": 1744900000123456000,
  "timestamp_s": 1744900000.123
}
```

**字段说明**：

| 字段 | 含义 | 单位 |
|------|------|------|
| `accel` | 加速度 (x, y, z) | m/s² |
| `gyro_dps` | 角速度 (x, y, z) | deg/s |
| `gyro_rad` | 角速度 (x, y, z) | rad/s |
| `mag` | 磁力计 (x, y, z) | 无量纲 |
| `mag_valid` | 磁力计数据是否有效 | bool |
| `imu_temp` | IMU 温度 | °C |
| `pressure` | 气压 | hPa |
| `env_temp` | 环境温度 | °C |
| `timestamp_ns` | 纳秒级时间戳 | ns |
| `timestamp_s` | 秒级时间戳 | s |

---

## 5. ROS2 Bag 原始数据 (`rosbag/`)

### 5.1 录制的 ROS2 话题

| Topic | 消息类型 | 说明 |
|-------|---------|------|
| `/svtrobot_cmd` | `geometry_msgs/msg/Twist` | 底盘速度指令 |
| `/lift_control_cmd` | `std_msgs/msg/Int32MultiArray` | 升降控制指令 |
| `/chassis/joint_states` | `sensor_msgs/msg/JointState` | 底盘关节状态 |
| `/chassis/diagnostics` | `chassis_control/msg/ChassisDiagnostics` | 底盘诊断 |
| `/f710/joy` | `sensor_msgs/msg/Joy` | 手柄输入 |

### 5.2 `metadata.yaml`

ROS2 bag 的元数据文件，记录了录制时长、话题信息、消息计数等。

### 5.3 自动 JSONL 转换

录制结束后，`bag_converter.py` 自动将 .db3 转换为 JSONL 格式。每个话题生成一个 .jsonl 文件，所有记录包含 `_timestamp_ns` 字段用于多话题时间对齐。

手动转换：

```bash
python3 src/web_control/bag_converter.py recordings/YYYYMMDD_HHMMSS/rosbag recordings/YYYYMMDD_HHMMSS
python3 src/web_control/bag_converter.py recordings/YYYYMMDD_HHMMSS/rosbag recordings/YYYYMMDD_HHMMSS --delete-db
```

---

## 6. 录制摘要 (`summary.json`)

录制结束时自动生成，包含：

```json
{
  "duration_seconds": 27.0,
  "total_size_mb": 549.79,
  "cameras": {
    "zed": {"images": 358, "depth": 358}
  },
  "pointcloud": {"zed": 36}
}
```

---

## 7. JSONL 传感器数据

所有 JSONL 记录都包含 `_timestamp_ns` 字段（纳秒级），用于跨话题时间对齐。

### 7.1 `chassis_joint_states.jsonl` — 关节/电机状态

- **ROS2 Topic**：`/chassis/joint_states`，~10 Hz
- **字段**：8 个关节 (FL/FR/RL/RR steer + wheel) 的 position/velocity/effort

### 7.2 `chassis_diagnostics.jsonl` — 底盘诊断

- **ROS2 Topic**：`/chassis/diagnostics`，~10 Hz
- **字段**：vbus (V), motor_temperatures[4] (°C), motor_error_codes[4], wheel_speeds_actual[4] (RPM)

### 7.3 `svtrobot_cmd.jsonl` — 底盘运动指令

- **ROS2 Topic**：`/svtrobot_cmd`，~50 Hz
- **字段**：linear.x/y/z (m/s), angular.x/y/z (rad/s)

### 7.4 `f710_joy.jsonl` — 手柄输入

- **ROS2 Topic**：`/f710/joy`，~50 Hz
- **字段**：axes[8] (-1~1), buttons[12] (0/1)

### 7.5 `lift_control_cmd.jsonl` — 升降控制指令

- **ROS2 Topic**：`/lift_control_cmd`，~25 Hz
- **字段**：data[0] (方向), data[1] (速度 RPM)

---

## 8. 数据频率汇总

| 数据 | 文件 | 频率 | 来源 |
|------|------|------|------|
| ZED 左眼彩色 | `images/zed/*.jpg` | 2 Hz | ZED SDK capture, deadline-based |
| ZED 右眼彩色 | `images/zed_right/*.jpg` | 2 Hz | ZED SDK capture, 与左眼同步 |
| ZED 深度图 | `depth/zed/*.jpg` | 2 Hz | ZED SDK retrieve_measure |
| ZED 点云 | `pointcloud/zed/*.npz` | 2 Hz | ZED SDK, float16 (720x1280, ~7MB/帧) |
| IMU | `imu.jsonl` | ~70 Hz | ZED SDK get_imu_data |
| 底盘关节状态 | `chassis_joint_states.jsonl` | ~10 Hz | ROS2 /chassis/joint_states |
| 底盘诊断 | `chassis_diagnostics.jsonl` | ~10 Hz | ROS2 /chassis/diagnostics |
| 底盘指令 | `svtrobot_cmd.jsonl` | ~50 Hz | ROS2 /svtrobot_cmd |
| 手柄数据 | `f710_joy.jsonl` | ~50 Hz | ROS2 /f710/joy |
| 升降指令 | `lift_control_cmd.jsonl` | ~25 Hz | ROS2 /lift_control_cmd |

---

## 9. 时间对齐

所有数据源通过时间戳对齐：

- **图像/深度/点云**：文件名 = Unix 微秒时间戳
- **IMU**：`timestamp_ns` 字段（纳秒级）
- **JSONL 传感器**：`_timestamp_ns` 字段（纳秒级）

对齐方式：图像文件名 `T_us` → `T_ns = T_us * 1000`，在 JSONL 中找最近的记录。

---

## 10. 数据读取示例

### Python 读取 JSONL

```python
import json

with open('chassis_joint_states.jsonl') as f:
    for line in f:
        record = json.loads(line)
        print(record['_timestamp_ns'], record['position'])
```

### Python 读取 IMU

```python
import json

with open('imu.jsonl') as f:
    for line in f:
        imu = json.loads(line)
        print(imu['timestamp_s'], imu['accel']['z'])  # 重力Z轴
```

### Python 读取点云

```python
import numpy as np

data = np.load("pointcloud/zed/1744900000123456.npz")["xyzrgba"]
# data.shape = (720, 1280, 4), dtype=float16
# X, Y, Z 单位 mm, 第4列 RGBA 打包
```
