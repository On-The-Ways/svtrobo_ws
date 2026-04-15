# SVTROBO 数据采集目录说明

本文档描述 `recordings/` 目录下采集数据的组织结构、文件格式及内容含义。

---

## 目录结构

每次录制会在 `recordings/` 下生成一个以时间戳命名的会话目录：

```
recordings/
└── YYYYMMDD_HHMMSS/                  # 录制会话（如 20260414_165813）
    ├── images/                        # 相机图像
    │   ├── d405_1/                    # D405 #1 相机（前方）
    │   │   ├── 1713163200123456.jpg   # 文件名 = Unix 微秒时间戳
    │   │   ├── 1713163201123456.jpg
    │   │   └── ...
    │   ├── d405_2/                    # D405 #2 相机（侧方）
    │   │   ├── 1713163200234567.jpg
    │   │   └── ...
    │   └── zed/                       # ZED 2i 相机（全局视角）
    │       ├── 1713163200345678.jpg
    │       └── ...
    ├── rosbag/                        # ROS2 原始录制数据
    │   ├── rosbag_0.db3               # SQLite3 数据库（二进制，ROS2 bag 格式）
    │   └── metadata.yaml              # 录制元信息
    ├── chassis_diagnostics.jsonl      # 底盘诊断数据（JSONL）
    ├── chassis_joint_states.jsonl     # 关节/电机状态数据（JSONL）
    ├── f710_joy.jsonl                 # 手柄输入数据（JSONL）
    ├── lift_control_cmd.jsonl         # 升降控制指令（JSONL）
    └── svtrobot_cmd.jsonl             # 底盘运动指令（JSONL）
```

---

## 1. 图像数据 (`images/`)

### 1.1 概述

| 项目 | 说明 |
|------|------|
| 来源 | 3 个相机：D405 #1、D405 #2、ZED 2i |
| 格式 | JPEG（质量参数 70） |
| 采集频率 | 10 Hz（每秒 10 帧） |
| 命名规则 | `{timestamp_us}.jpg`（Unix 微秒时间戳，如 `1713163200123456.jpg`） |
| 对齐方式 | 文件名即精确采集时间戳（微秒级），可直接与 JSONL 中的 `_timestamp_ns`（纳秒级）对齐 |

### 1.2 各相机参数

| 相机 | 类型 | 分辨率 | 用途 |
|------|------|--------|------|
| `d405_1` | RealSense D405 | 640x480 | 前方近距离深度/彩色 |
| `d405_2` | RealSense D405 | 640x480 | 侧方近距离深度/彩色 |
| `zed` | ZED 2i | 672x376 | 全局视角立体视觉 |

---

## 2. ROS2 Bag 原始数据 (`rosbag/`)

### 2.1 `rosbag_0.db3`

ROS2 bag 的原始录制文件，SQLite3 数据库格式，内部存储各 topic 的 CDR 序列化消息。

- **格式**：SQLite3 数据库 + CDR (Common Data Representation) 二进制序列化
- **是否可删除**：JSONL 文件已包含所有解析后的数据，db3 可安全删除以节省空间
- **查看方式**：`ros2 bag play <目录>` 回放，或 `sqlite3 rosbag_0.db3` 直接查询

### 2.2 `metadata.yaml`

ROS2 bag 的元数据文件，记录了：

```yaml
rosbag2_bagfile_information:
  version: 5
  storage_identifier: sqlite3
  duration:
    nanoseconds: 4084864962          # 录制总时长（纳秒）
  starting_time:
    nanoseconds_since_epoch: ...     # 起始时间戳
  message_count: 114                 # 消息总数
  topics_with_message_count:         # 各 topic 信息
    - topic_metadata:
        name: /chassis/diagnostics
        type: chassis_control/msg/ChassisDiagnostics
        serialization_format: cdr
      message_count: 57
    - ...
  relative_file_paths:
    - rosbag_0.db3
```

---

## 3. JSONL 传感器数据

JSONL (JSON Lines) 格式：每行一条独立的 JSON 对象，便于逐行读取和流式处理。

所有记录都包含一个 `_timestamp_ns` 字段，表示 ROS2 消息的纳秒级时间戳，可用于跨 topic 时间对齐。

---

### 3.1 `chassis_joint_states.jsonl` — 关节/电机状态

- **ROS2 Topic**：`/chassis/joint_states`
- **消息类型**：`sensor_msgs/msg/JointState`
- **发布频率**：~10 Hz
- **内容示例**：

```json
{
  "header": {
    "stamp": { "sec": 1776157093, "nanosec": 482907979 },
    "frame_id": ""
  },
  "name": ["fl_steer", "fr_steer", "rl_steer", "rr_steer",
           "fl_wheel", "fr_wheel", "rl_wheel", "rr_wheel"],
  "position": [3.34, 4.90, 2.70, 3.50, 0.0, 0.0, 0.0, 0.0],
  "velocity": [0.005, -0.049, 0.096, 0.041, 0.0, 0.0, 0.0, 0.0],
  "effort":   [0.063, -0.089, -0.210, 0.209, 0.0, 0.0, 0.0, 0.0],
  "_timestamp_ns": 1776157093483028005
}
```

**字段说明**：

| 字段 | 含义 |
|------|------|
| `name` | 8 个关节名称，前 4 个为转向电机（steer），后 4 个为驱动轮（wheel） |
| `position` | 位置/角度（弧度）。steer 为转向角，wheel 通常为 0（无编码器位置反馈） |
| `velocity` | 速度（rad/s）。steer 为转向角速度，wheel 为轮子角速度 |
| `effort` | 力矩/电流（N·m 或归一化值） |

**关节顺序**：`FL`（前左）→ `FR`（前右）→ `RL`（后左）→ `RR`（后右），先 steer 后 wheel。

---

### 3.2 `chassis_diagnostics.jsonl` — 底盘诊断

- **ROS2 Topic**：`/chassis/diagnostics`
- **消息类型**：`chassis_control/msg/ChassisDiagnostics`（自定义消息）
- **发布频率**：~10 Hz
- **内容示例**：

```json
{
  "header": {
    "stamp": { "sec": 1776157093, "nanosec": 482982352 },
    "frame_id": ""
  },
  "vbus": 24.60,
  "motor_temperatures": [26.0, 26.0, 26.0, 26.0],
  "motor_error_codes": [0, 0, 0, 0],
  "wheel_speeds_actual": [0.0, 0.0, 0.0, 0.0],
  "_timestamp_ns": 1776157093483042294
}
```

**字段说明**：

| 字段 | 含义 | 单位 |
|------|------|------|
| `vbus` | 电机驱动器总线电压 | V |
| `motor_temperatures` | 4 个转向电机温度 [FL, FR, RL, RR] | °C |
| `motor_error_codes` | 4 个转向电机错误码 [FL, FR, RL, RR]，0 表示正常 | - |
| `wheel_speeds_actual` | 4 个轮子实际转速（来自 ZLAC8015D 驱动器）[FL, FR, RL, RR] | RPM |

---

### 3.3 `svtrobot_cmd.jsonl` — 底盘运动指令

- **ROS2 Topic**：`/svtrobot_cmd`
- **消息类型**：`geometry_msgs/msg/Twist`
- **发布频率**：~50 Hz
- **内容示例**：

```json
{
  "linear":  { "x": 0.5, "y": 0.0, "z": 0.0 },
  "angular": { "x": 0.0, "y": 0.0, "z": 0.3 },
  "_timestamp_ns": 1776157093448645403
}
```

**字段说明**：

| 字段 | 含义 | 单位 |
|------|------|------|
| `linear.x` | 前进/后退速度 | m/s |
| `linear.y` | 左右平移速度（全向底盘有效） | m/s |
| `linear.z` | 垂直方向（通常为 0） | m/s |
| `angular.x` | 绕 X 轴旋转（通常为 0） | rad/s |
| `angular.y` | 绕 Y 轴旋转（通常为 0） | rad/s |
| `angular.z` | 偏航角速度（左转/右转） | rad/s |

---

### 3.4 `f710_joy.jsonl` — 手柄输入

- **ROS2 Topic**：`/f710/joy`
- **消息类型**：`sensor_msgs/msg/Joy`
- **发布频率**：~50 Hz
- **内容示例**：

```json
{
  "header": {
    "stamp": { "sec": 1776157093, "nanosec": 453216949 },
    "frame_id": ""
  },
  "axes": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
  "buttons": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  "_timestamp_ns": 1776157093453430727
}
```

**字段说明**：

| 字段 | 含义 |
|------|------|
| `axes` | 8 个摇杆/扳机轴值，范围 [-1.0, 1.0] |
| `buttons` | 12 个按键状态，0 = 未按下，1 = 按下 |

> 轴和按键的具体映射取决于 `f710_teleop` 节点的配置。

---

### 3.5 `lift_control_cmd.jsonl` — 升降控制指令

- **ROS2 Topic**：`/lift_control_cmd`
- **消息类型**：`std_msgs/msg/Int32MultiArray`
- **发布频率**：~25 Hz
- **内容示例**：

```json
{
  "layout": {
    "dim": [],
    "data_offset": 0
  },
  "data": [0, 0],
  "_timestamp_ns": 1776157093448658004
}
```

**字段说明**：

| 字段 | 含义 |
|------|------|
| `data[0]` | 左升降机构目标位置 |
| `data[1]` | 右升降机构目标位置 |

---

## 4. 数据频率汇总

| Topic | 文件 | 频率 | 驱动来源 |
|-------|------|------|----------|
| `/chassis/joint_states` | `chassis_joint_states.jsonl` | ~10 Hz | chassis_control 节点 |
| `/chassis/diagnostics` | `chassis_diagnostics.jsonl` | ~10 Hz | chassis_control 节点 |
| `/svtrobot_cmd` | `svtrobot_cmd.jsonl` | ~50 Hz | Web 控制或手柄节点 |
| `/f710/joy` | `f710_joy.jsonl` | ~50 Hz | f710_teleop 节点 |
| `/lift_control_cmd` | `lift_control_cmd.jsonl` | ~25 Hz | f710_teleop 节点 |
| 相机帧 | `images/*/{timestamp_us}.jpg` | 10 Hz | CameraManager |

频率由各 ROS2 节点的发布设置决定，`ros2 bag record` 忠实记录原始频率。

---

## 5. 时间对齐

所有数据源通过 `_timestamp_ns`（纳秒级 Unix 时间戳）进行时间对齐：

- JSONL 中的 `_timestamp_ns` 字段
- 图像帧的文件名即精确的采集时间戳（微秒级），直接转为纳秒即可与传感器数据对齐

对齐方式：图像文件名中的微秒时间戳 `T_us`，对应纳秒时间戳 `T_ns = T_us * 1000`，在 JSONL 中查找 `_timestamp_ns` 最接近的传感器记录。

---

## 6. 数据读取示例

### Python 读取 JSONL

```python
import json

with open('chassis_joint_states.jsonl') as f:
    for line in f:
        record = json.loads(line)
        print(record['_timestamp_ns'], record['position'])
```

### 将 db3 转换为 JSONL（手动）

如果某个录制会话缺少 JSONL 文件（旧数据），可手动运行转换：

```bash
python3 src/web_control/bag_converter.py recordings/YYYYMMDD_HHMMSS/rosbag recordings/YYYYMMDD_HHMMSS
```

如需转换后删除 db3 文件以节省空间：

```bash
python3 src/web_control/bag_converter.py recordings/YYYYMMDD_HHMMSS/rosbag recordings/YYYYMMDD_HHMMSS --delete-db
```
