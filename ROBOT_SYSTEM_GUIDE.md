# SVTROBO 底盘控制系统技术文档

> 适用于深度学习训练与推理部署的完整参考文档

---

## 目录

1. [系统概述](#1-系统概述)
2. [硬件架构](#2-硬件架构)
3. [ROS2 节点与通信拓扑](#3-ros2-节点与通信拓扑)
4. [底盘控制方法](#4-底盘控制方法)
5. [升降机构控制](#5-升降机构控制)
6. [运动学模型](#6-运动学模型)
7. [状态数据与参数](#7-状态数据与参数)
8. [安全机制](#8-安全机制)
9. [启动与运行](#9-启动与运行)
10. [深度学习数据采集指南](#10-深度学习数据采集指南)
11. [推理部署指南](#11-推理部署指南)
12. [文件结构](#12-文件结构)

---

## 1. 系统概述

本机器人是一台 **四轮独立转向 + 四轮独立驱动（4WIS4WID）** 的全向移动底盘，具备：

- 全向平移（前/后/左/右任意方向）
- 原地旋转
- 组合运动（平移 + 旋转同时进行）
- 升降机构控制

系统基于 **ROS2** 构建，运行在 Linux 平台上，通过 **CAN 总线** 和 **RS485 串口** 与底层硬件通信。

---

## 2. 硬件架构

### 2.1 硬件连接图

```
                        主控板 (Linux)
                    ┌─────────────────────┐
                    │                     │
         ┌──────────┤    CAN2 (can2)      ├──────────┐
         │          │                     │          │
    ┌────┴────┐ ┌───┴───┐ ┌────┴───┐ ┌───┴───┐     │
    │ RobStride│ │RobStride│ │RobStride│ │RobStride│  │
    │ Motor 1 │ │Motor 2 │ │Motor 3 │ │Motor 4 │   │
    │ (0x65)  │ │(0x66)  │ │(0x67)  │ │(0x68)  │    │
    │ 前左舵向 │ │前右舵向 │ │后左舵向 │ │后右舵向 │    │
    └─────────┘ └────────┘ └────────┘ └────────┘     │
                                                      │
         ┌──────────┤    CAN3 (can3)      ├──────────┐
         │          │                     │          │
    ┌────┴────┐ ┌───┴───┐                          │
    │ZLAC8015D│ │ZLAC8015D│                         │
    │ (ID:1)  │ │ (ID:2) │                         │
    │前轮驱动 │ │后轮驱动 │                         │
    │FL + FR  │ │RL + RR │                          │
    └─────────┘ └────────┘                          │
                                                      │
                    │   RS485 (/dev/ttyACM0)  │
                    └─────────────────────────┘
                               │
                        ┌──────┴──────┐
                        │  升降电机    │
                        │ (Modbus ID:1)│
                        └─────────────┘
```

### 2.2 舵向电机（RobStride）

| 属性 | 值 |
|------|-----|
| 数量 | 4 个 |
| 通信总线 | CAN2 (`can2`) |
| 主控 ID | 0xFF |
| 电机 CAN ID | 0x65 / 0x66 / 0x67 / 0x68 |
| 控制模式 | CSP（Cyclic Synchronous Position）|
| 位置范围 | ±4π rad |
| 最大速度 | 50 rad/s |
| 最大力矩 | 17 Nm |
| 反馈数据 | 位置、速度、力矩、温度 |

**电机映射：**

| 电机对象 | CAN ID | 安装位置 | ROS 参数初始角 |
|----------|--------|----------|---------------|
| motor1 | 0x65 | 前左（FL）| `robot.fl_motor_start_angle` |
| motor2 | 0x66 | 前右（FR）| `robot.fr_motor_start_angle` |
| motor3 | 0x67 | 后左（RL）| `robot.rl_motor_start_angle` |
| motor4 | 0x68 | 后右（RR）| `robot.rr_motor_start_angle` |

### 2.3 轮驱动器（ZLAC8015D）

| 属性 | 值 |
|------|-----|
| 数量 | 2 个（各含双通道） |
| 通信总线 | CAN3 (`can3`) |
| 协议 | CANopen (SDO) |
| 控制模式 | 速度模式（Profile Velocity） |
| 速度指令单位 | RPM（int16） |
| 速度读取单位 | 0.1 RPM |
| 可读取反馈 | 实际转速、编码器值、状态字、故障码 |

**驱动器映射：**

| 驱动器对象 | Node ID | 控制轮子 |
|------------|---------|----------|
| front_ | 1 | FL + FR（左前 + 右前）|
| rear_ | 2 | RL + RR（左后 + 右后）|

### 2.4 升降电机

| 属性 | 值 |
|------|-----|
| 通信接口 | RS485 串口 (`/dev/ttyACM0`) |
| 波特率 | 57600 |
| 协议 | Modbus RTU |
| 从机地址 | 0x01 |
| 速度范围 | 1 ~ 6000 RPM |
| 可读取 | 故障码、32位位置 |

### 2.5 轮子方向常量

```cpp
#define WHEEL_FL_DIRETION  1   // 左前
#define WHEEL_FR_DIRETION -1   // 右前
#define WHEEL_RL_DIRETION -1   // 左后
#define WHEEL_RR_DIRETION  1   // 右后
```

---

## 3. ROS2 节点与通信拓扑

### 3.1 节点列表

| 节点名 | 可执行文件 | 功能 |
|--------|-----------|------|
| `chassis_control` | `chassis_control` | 底盘运动控制（舵向 + 轮速）|
| `lift_control_node` | `lift_control` | 升降机构控制 |

### 3.2 Topic 列表

| Topic | 消息类型 | 方向 | 所属节点 | 频率 | 说明 |
|-------|---------|------|---------|------|------|
| `/svtrobot_cmd` | `geometry_msgs/msg/Twist` | Sub（输入）| `chassis_control` | 按需 | 底盘速度指令 |
| `/lift_control_cmd` | `std_msgs/msg/Int32MultiArray` | Sub（输入）| `lift_control_node` | 按需 | 升降控制指令 |
| `/chassis/joint_states` | `sensor_msgs/msg/JointState` | Pub（输出）| `chassis_control` | 100Hz | 底盘关节状态（舵向角度/速度/力矩、轮速） |
| `/chassis/cmd_feedback` | `geometry_msgs/msg/Twist` | Pub（输出）| `chassis_control` | 100Hz | 当前指令回显（vx, vy, wz） |
| `/chassis/diagnostics` | `chassis_control/msg/ChassisDiagnostics` | Pub（输出）| `chassis_control` | 100Hz | 诊断数据（电压、温度、错误码、实际轮速） |

### 3.3 已发布的状态/反馈 Topic

系统通过以下三个 Topic 发布底盘状态数据：

**`/chassis/joint_states`**（100Hz）— 标准关节状态：

| 字段 | name[0..3] | name[4..7] |
|------|-----------|-----------|
| `name` | `fl_steer`, `fr_steer`, `rl_steer`, `rr_steer` | `fl_wheel`, `fr_wheel`, `rl_wheel`, `rr_wheel` |
| `position` | 4 个舵向电机实际角度 (rad) | 0.0（轮子无位置反馈） |
| `velocity` | 4 个舵向电机速度 (rad/s) | 4 个轮子目标转速 (RPM) |
| `effort` | 4 个舵向电机力矩 (Nm) | 0.0（轮子无力矩反馈） |

**`/chassis/cmd_feedback`**（100Hz）— 当前指令回显：

| 字段 | 说明 |
|------|------|
| `linear.x` | 当前设定的前进速度 (m/s) |
| `linear.y` | 当前设定的横移速度 (m/s) |
| `angular.z` | 当前设定的旋转角速度 (rad/s) |

**`/chassis/diagnostics`**（100Hz）— 自定义诊断消息：

| 字段 | 类型 | 说明 |
|------|------|------|
| `header` | `std_msgs/Header` | 时间戳 |
| `vbus` | `float32` | 电机驱动器总线电压 (V) |
| `motor_temperatures` | `float32[4]` | 4 个舵向电机温度 (°C)：FL, FR, RL, RR |
| `motor_error_codes` | `uint8[4]` | 4 个舵向电机错误码：FL, FR, RL, RR |
| `wheel_speeds_actual` | `float32[4]` | ZLAC8015D 实际轮速 (RPM)：FL, FR, RL, RR |

> `vbus` 约每 20 秒从 RobStride 电机读取一次（参数 0x701C）。`wheel_speeds_actual` 约每 10 秒从 ZLAC8015D 读取一次。

### 3.4 ROS2 参数

通过 `config/params.yaml` 加载，运行时可通过命令读取/修改：

```bash
# 读取参数
ros2 param get /chassis_control robot.chassis_radius

# 列出所有参数
ros2 param list /chassis_control
```

| 参数名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `robot.chassis_radius` | double | 0.0875 | 底盘中心到轮子的距离（米） |
| `robot.wheel_perimeter` | double | 0.647 | 轮子周长（米） |
| `robot.fl_motor_start_angle` | double | 0.2 | 前左舵向零位角度（rad） |
| `robot.fr_motor_start_angle` | double | 4.9 | 前右舵向零位角度（rad） |
| `robot.rl_motor_start_angle` | double | 2.7 | 后左舵向零位角度（rad） |
| `robot.rr_motor_start_angle` | double | 3.5 | 后右舵向零位角度（rad） |

---

## 4. 底盘控制方法

### 4.1 控制输入

向 `/svtrobot_cmd` 发布 `geometry_msgs/msg/Twist` 消息：

```bash
ros2 topic pub /svtrobot_cmd geometry_msgs/msg/Twist \
  "{linear: {x: 0.5, y: 0.0, z: 0.0}, angular: {z: 0.0}}"
```

| 字段 | 变量名 | 含义 | 单位 | 有效范围建议 |
|------|--------|------|------|-------------|
| `linear.x` | `vx_set` | 前进/后退速度 | m/s | 取决于任务需求 |
| `linear.y` | `vy_set` | 左移/右移速度 | m/s | 取决于任务需求 |
| `angular.z` | `wz_set` | 原地旋转角速度 | rad/s | 取决于任务需求 |

> **注意**：`linear.z` 字段未使用。系统内部会对超速进行等比缩放，最终轮速不超过 100 RPM。

### 4.2 控制流程

```
用户发布 Twist 消息 (vx, vy, wz)
         │
         ▼
  svtrobot_cmd_callback()  ── 设置 vx_set, vy_set, wz_set
         │
         ▼
  ┌──────────────────────────────────────────┐
  │         excute_loop() (1ms 周期)          │
  │                                          │
  │  1. chassis_control_loop()               │
  │     - 逆运动学解算                       │
  │     - 4个轮子的目标角度 (atan2)           │
  │     - 4个轮子的目标转速 (rpm)            │
  │     - 最大速度等比缩放                   │
  │                                          │
  │  2. arc_judge()                          │
  │     - 最短路径转角判断                   │
  │     - 角度差 > π/2 时反转速度方向        │
  │                                          │
  │  3. 变化率限制 (RateLimiter)             │
  │     - 最大角度变化率: 5 rad/s            │
  │                                          │
  │  4. 低通滤波 (LowPassFilter, α=0.75)     │
  │                                          │
  │  5. 发送舵向命令 → 4x RobStride (CAN2)   │
  │     - CSP 模式, 速度上限 20 rad/s        │
  │                                          │
  │  6. 判断舵向是否到位 (误差 ≤ 0.1 rad)    │
  │     - 到位 → 发送轮速到 ZLAC8015D       │
  │     - 未到位 → 轮速设为 0               │
  └──────────────────────────────────────────┘
```

### 4.3 Python 控制示例

```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

class ChassisController(Node):
    def __init__(self):
        super().__init__('chassis_controller')
        self.pub = self.create_publisher(Twist, '/svtrobot_cmd', 10)

    def send_velocity(self, vx, vy, wz):
        """发送速度指令

        Args:
            vx: 前进速度 (m/s)
            vy: 横移速度 (m/s)
            wz: 旋转角速度 (rad/s)
        """
        msg = Twist()
        msg.linear.x = float(vx)
        msg.linear.y = float(vy)
        msg.angular.z = float(wz)
        self.pub.publish(msg)

    def stop(self):
        self.send_velocity(0.0, 0.0, 0.0)

# 使用示例
rclpy.init()
node = ChassisController()

# 前进 0.5 m/s
node.send_velocity(0.5, 0.0, 0.0)

# 左移 0.3 m/s
node.send_velocity(0.0, 0.3, 0.0)

# 原地旋转 0.5 rad/s
node.send_velocity(0.0, 0.0, 0.5)

# 边前进边旋转
node.send_velocity(0.3, 0.0, 0.3)

# 停止
node.stop()
```

### 4.4 命令行快速测试

```bash
# 启动底盘
ros2 launch chassis_control svtrobo_bringup.launch.py

# 前进
ros2 topic pub --once /svtrobot_cmd geometry_msgs/msg/Twist \
  "{linear: {x: 0.3, y: 0.0, z: 0.0}, angular: {z: 0.0}}"

# 持续发布（以 10Hz 持续前进）
ros2 topic pub -r 10 /svtrobot_cmd geometry_msgs/msg/Twist \
  "{linear: {x: 0.3, y: 0.0, z: 0.0}, angular: {z: 0.0}}"

# 停止
ros2 topic pub --once /svtrobot_cmd geometry_msgs/msg/Twist \
  "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {z: 0.0}}"
```

> **重要**：底盘节点在 Twist 消息停止发布后**不会自动停止**。控制循环会保持最后的速度设定值。如需停止，必须显式发送零速指令。

---

## 5. 升降机构控制

### 5.1 控制输入

向 `/lift_control_cmd` 发布 `std_msgs/msg/Int32MultiArray` 消息：

```bash
# 正转 300 RPM
ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [1, 300]}"

# 反转 200 RPM
ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [-1, 200]}"

# 停止
ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [0, 0]}"
```

| data[0] (方向) | data[1] (速度) | 说明 |
|----------------|----------------|------|
| `1` | 1 ~ 6000 | 正转，指定 RPM |
| `-1` | 1 ~ 6000 | 反转，指定 RPM |
| `0` | 任意 | 停止（断开使能） |

### 5.2 Python 控制示例

```python
from std_msgs.msg import Int32MultiArray

def control_lift(pub, direction, speed):
    """控制升降机构

    Args:
        direction: 1=正转, -1=反转, 0=停止
        speed: RPM (1~6000)
    """
    msg = Int32MultiArray()
    msg.data = [direction, speed]
    pub.publish(msg)
```

---

## 6. 运动学模型

### 6.1 坐标系定义

```
        ↑ y (vy > 0 左移)
        │
        │
        ├──────→ x (vx > 0 前进)
       ╱
      ╱
     ↓ wz > 0 逆时针旋转
```

四个轮子沿底盘对角线分布，对角线与坐标轴夹角为 45°。底盘半径 `R` = 0.0875m（中心到轮轴的距离）。

### 6.2 轮子布局

```
              前方 (x+)
         ┌─────────────────┐
        ╱  FL           FR  ╲
       ╱  motor1      motor2 ╲
      ╱   (0x65)      (0x66)  ╲
     │                           │
     │            ○              │  ← 底盘中心
     │                           │
      ╲   motor3      motor4  ╱
       ╲  (0x67)      (0x68) ╱
        ╲  RL           RR  ╱
         └─────────────────┘
              后方 (x-)
```

### 6.3 逆运动学公式

设底盘中心速度为 `(vx, vy)`，角速度为 `wz`，底盘半径为 `R`。

令 `k = R × 0.707107`（R × cos45°），每个轮子的速度分量为：

**前左（FL）：**
```
vx_fl = vx - wz × k
vy_fl = vy + wz × k
angle_fl = atan2(vy_fl, vx_fl) + fl_motor_start_angle
speed_fl = sqrt(vx_fl² + vy_fl²) × (60 / wheel_perimeter)
```

**前右（FR）：**
```
vx_fr = vx + wz × k
vy_fr = vy + wz × k
angle_fr = atan2(vy_fr, vx_fr) + fr_motor_start_angle
speed_fr = sqrt(vx_fr² + vy_fr²) × (60 / wheel_perimeter)
```

**后左（RL）：**
```
vx_rl = vx - wz × k
vy_rl = vy - wz × k
angle_rl = atan2(vy_rl, vx_rl) + rl_motor_start_angle
speed_rl = sqrt(vx_rl² + vy_rl²) × (60 / wheel_perimeter)
```

**后右（RR）：**
```
vx_rr = vx + wz × k
vy_rr = vy + wz × k
angle_rr = atan2(vy_rr, vx_rr) + rr_motor_start_angle
speed_rr = sqrt(vx_rr² + vy_rr²) × (60 / wheel_perimeter)
```

### 6.4 速度转换

线速度到轮速 RPM 的转换：

```
wheel_rpm_ratio = 60.0 / wheel_perimeter   // = 60 / 0.647 ≈ 92.74
speed_rpm = sqrt(vx² + vy²) × wheel_rpm_ratio
```

### 6.5 数值常量

| 常量 | 值 | 说明 |
|------|-----|------|
| `FILTER_ALPHA` | 0.75 | 低通滤波系数（越小越平滑） |
| `MAX_ANGLE_RATE` | 5.0 rad/s | 舵向角度最大变化率 |
| `MAX_WHEEL_SPEED` | 100.0 RPM | 轮速上限 |
| 控制周期 | 1 ms | `excute_loop` 循环间隔 |
| 舵向到位阈值 | 0.1 rad | 舵向到位后才驱动轮子 |
| CSP 速度上限 | 20 rad/s | 舵向电机 CSP 模式速度 |

---

## 7. 状态数据与参数

### 7.1 当前可获取的状态

#### ROS2 Topic（实时数据）

底盘 C++ 节点通过以下 Topic 发布实时状态数据（详见第 3.3 节）：

```bash
# 监听关节状态
ros2 topic echo /chassis/joint_states

# 监听指令回显
ros2 topic echo /chassis/cmd_feedback

# 监听诊断数据
ros2 topic echo /chassis/diagnostics
```

#### ROS2 参数（静态/半静态）

```bash
ros2 param get /chassis_control robot.chassis_radius     # → 0.0875
ros2 param get /chassis_control robot.wheel_perimeter    # → 0.647
ros2 param get /chassis_control robot.fl_motor_start_angle  # → 0.2
ros2 param get /chassis_control robot.fr_motor_start_angle  # → 4.9
ros2 param get /chassis_control robot.rl_motor_start_angle  # → 2.7
ros2 param get /chassis_control robot.rr_motor_start_angle  # → 3.5
```

这些参数在 `chassis_control_loop()` 中每周期读取，因此**运行时修改会立即生效**。

### 7.2 仅存在于节点内部的动态数据（部分已发布）

以下数据在节点内部通过 CAN 总线获取，其中大部分已通过 ROS Topic 发布（标注 ✓）。少数仍仅内部可用的数据标注 ✗。

#### 底盘控制参数（`chassis_control_para_t`）

#### 底盘控制参数（`chassis_control_para_t`）

| 变量 | 类型 | 单位 | 说明 | 发布状态 |
|------|------|------|------|---------|
| `vx_set` | double | m/s | 目标前后速度 | ✓ `/chassis/cmd_feedback` |
| `vy_set` | double | m/s | 目标左右速度 | ✓ `/chassis/cmd_feedback` |
| `wz_set` | double | rad/s | 目标旋转角速度 | ✓ `/chassis/cmd_feedback` |
| `front_left_angle` | double | rad | 前左舵向实际目标角度（经滤波后） | ✗ |
| `front_right_angle` | double | rad | 前右舵向实际目标角度 | ✗ |
| `rear_left_angle` | double | rad | 后左舵向实际目标角度 | ✗ |
| `rear_right_angle` | double | rad | 后右舵向实际目标角度 | ✗ |
| `front_left_speed` | double | RPM | 前左轮目标转速 | ✓ `/chassis/joint_states` velocity[4] |
| `front_right_speed` | double | RPM | 前右轮目标转速 | ✓ `/chassis/joint_states` velocity[5] |
| `rear_left_speed` | double | RPM | 后左轮目标转速 | ✓ `/chassis/joint_states` velocity[6] |
| `rear_right_speed` | double | RPM | 后右轮目标转速 | ✓ `/chassis/joint_states` velocity[7] |

#### RobStride 电机反馈（每个电机各一组）

| 变量 | 类型 | 单位 | 说明 | 发布状态 |
|------|------|------|------|---------|
| `position_` | float | rad | 舵向电机当前位置（实际反馈） | ✓ `/chassis/joint_states` position[0..3] |
| `velocity_` | float | rad/s | 舵向电机当前速度 | ✓ `/chassis/joint_states` velocity[0..3] |
| `torque_` | float | Nm | 舵向电机当前力矩 | ✓ `/chassis/joint_states` effort[0..3] |
| `temperature_` | float | °C | 电机温度（×0.1） | ✓ `/chassis/diagnostics` motor_temperatures |
| `error_code` | uint8 | - | 错误码 | ✓ `/chassis/diagnostics` motor_error_codes |
| `pattern` | uint8 | - | 模式状态 | ✗ |

#### ZLAC8015D 驱动器反馈

| 方法 | 返回值 | 说明 | 发布状态 |
|------|--------|------|---------|
| `read_actual_speed_lr_0p1rpm()` | `pair<int16, int16>` | 左右轮实际转速（0.1 RPM） | ✓ `/chassis/diagnostics` wheel_speeds_actual（约每 10s 更新） |
| `read_encoder_lr()` | `pair<int32, int32>` | 左右轮编码器值 | ✗ |
| `read_statusword_lr()` | `pair<uint16, uint16>` | 左右轮状态字 | ✗ |
| `read_fault_code_u32()` | `uint32` | 故障码 | ✗ |

### 7.3 数据流总结

```
  用户输入                  节点内部计算               CAN 反馈
 ──────────              ────────────────           ──────────
 Twist (vx,vy,wz)  ──→  chassis_control_para_t  ──→  发送到电机
     │                    │       │                      │
     │                    │       │                 motor1~4.position_
     │                    │       │                 motor1~4.velocity_
     │                    │       │                 motor1~4.torque_
     │                    │       │                 motor1~4.temperature_
     │                    ▼       ▼                      │
     │              ✓ ROS Topic 已发布                    │
     │                                    ZLAC8015D 实际轮速
     │                                    VBUS 电压
     ▼                                                    ▼
  /chassis/cmd_feedback (100Hz)           /chassis/joint_states (100Hz)
  /chassis/diagnostics  (100Hz)
```

---

## 8. 安全机制

### 8.1 速度限制

- **轮速上限**：任一轮速超过 100 RPM 时，所有轮速等比缩放，保持运动方向不变
- **舵向速度上限**：CSP 模式下限制为 20 rad/s

### 8.2 平滑过渡

- **变化率限制器**：舵向角度变化率不超过 5 rad/s，防止机械冲击
- **低通滤波器**：α = 0.75，平滑舵向角度指令

### 8.3 舵向到位保护

- 4 个舵向电机**全部到位**（误差 ≤ 0.1 rad）后才输出轮速
- 未到位时轮速强制为 0，防止轮子侧滑

### 8.4 最短路径转角

- 当目标角度与当前角度差 > π/2 + 0.02 rad 时，选择反向转动 + 反转轮速
- 避免舵向大角度回转，减少运动延迟

### 8.5 数据有效性检查

- 所有舵向角度值检查 `std::isfinite`（排除 NaN 和 Inf）
- 无效值回退为上次有效滤波输出

### 8.6 异常捕获

- 单个电机 CAN 命令失败不中断控制循环
- 控制循环外层 catch 全局异常，出错时 sleep 10ms 后继续

### 8.7 升降机构安全

- 速度自动钳位到 [1, 6000] RPM
- 支持故障码读取和自动复位
- 析构时自动断开使能

---

## 9. 启动与运行

### 9.1 编译

```bash
cd /home/openarm/svtrobo_ws
colcon build --packages-select chassis_control
source install/setup.bash
```

### 9.2 启动底盘

```bash
ros2 launch chassis_control svtrobo_bringup.launch.py
```

该 launch 文件同时启动两个节点：
1. `chassis_control` — 底盘运动控制
2. `lift_control` — 升降机构控制

### 9.3 单独启动

```bash
# 仅启动底盘控制
ros2 run chassis_control chassis_control

# 仅启动升降控制
ros2 run chassis_control lift_control
```

### 9.4 调试命令

```bash
# 查看活跃节点
ros2 node list

# 查看话题列表
ros2 topic list

# 查看话题信息
ros2 topic info /svtrobot_cmd

# 实时监听话题
ros2 topic echo /svtrobot_cmd

# 查看节点参数
ros2 param list /chassis_control
ros2 param dump /chassis_control
```

---

## 10. 深度学习数据采集指南

### 10.1 需要采集的数据

对于深度学习训练，以下数据是有价值的：

#### 输入特征（观测 / Observation）

| 数据 | 来源 | 类型 | 说明 |
|------|------|------|------|
| 目标速度 (vx, vy, wz) | `/svtrobot_cmd` 订阅 | 3 × float | 用户/策略网络的指令 |
| 4个舵向实际角度 | motor1~4.position_ | 4 × float | CAN 反馈，rad |
| 4个舵向实际速度 | motor1~4.velocity_ | 4 × float | CAN 反馈，rad/s |
| 4个轮子目标转速 | chassis_control_para | 4 × float | RPM |
| 4个舵向力矩 | motor1~4.torque_ | 4 × float | CAN 反馈，Nm |
| 电机温度 | motor1~4.temperature_ | 4 × float | °C |
| 外部传感器数据 | 依传感器而定 | - | IMU、激光雷达、相机等 |

#### 输出标签（动作 / Action）

| 数据 | 类型 | 说明 |
|------|------|------|
| 目标速度 (vx, vy, wz) | 3 × float | 即为动作空间，与输入特征中的指令相同 |

#### 状态/奖励信号

| 数据 | 类型 | 说明 |
|------|------|------|
| 舵向跟踪误差 | 4 × float | 目标角 - 实际角，用于评估跟踪性能 |
| 力矩 | 4 × float | 可用作能量消耗指标 |
| 到位标志 | bool | 4个舵向是否全部到位 |

### 10.2 已实现的状态发布

当前系统已通过以下 Topic 发布状态数据（详见第 3.3 节）：

- `/chassis/joint_states`（100Hz）— 舵向角度/速度/力矩 + 轮子目标转速
- `/chassis/cmd_feedback`（100Hz）— 当前指令回显
- `/chassis/diagnostics`（100Hz）— VBUS 电压、电机温度、错误码、ZLAC8015D 实际轮速

数据通过 `publish_decimation` 机制降频：控制循环 1ms 周期，每 10 个循环发布一次（即 100Hz）。

> VBUS 电压约每 20 秒从 RobStride 电机读取一次（参数 0x701C），ZLAC8015D 实际轮速约每 10 秒读取一次。

### 10.3 Python 数据记录脚本示例

### 10.3 Python 数据记录脚本示例

```python
#!/usr/bin/env python3
"""
数据记录脚本 — 记录底盘指令和状态用于深度学习训练
"""
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import JointState
from chassis_control.msg import ChassisDiagnostics
import csv
import time

class DataRecorder(Node):
    def __init__(self, output_file='chassis_data.csv'):
        super().__init__('data_recorder')
        self.output_file = output_file
        self.data = []

        self.cmd_sub = self.create_subscription(
            Twist, '/svtrobot_cmd', self.cmd_callback, 10)
        self.state_sub = self.create_subscription(
            JointState, '/chassis/joint_states', self.state_callback, 10)
        self.diag_sub = self.create_subscription(
            ChassisDiagnostics, '/chassis/diagnostics', self.diag_callback, 10)

        self.last_cmd = {'vx': 0.0, 'vy': 0.0, 'wz': 0.0}
        self.last_state = None
        self.last_diag = None

        self.get_logger().info(f'数据记录已启动，输出文件: {output_file}')

    def cmd_callback(self, msg):
        self.last_cmd = {
            'vx': msg.linear.x,
            'vy': msg.linear.y,
            'wz': msg.angular.z,
        }

    def state_callback(self, msg):
        row = {
            'timestamp': time.time(),
            'vx_cmd': self.last_cmd['vx'],
            'vy_cmd': self.last_cmd['vy'],
            'wz_cmd': self.last_cmd['wz'],
        }
        # 添加关节状态数据
        for i, name in enumerate(msg.name):
            if i < len(msg.position):
                row[f'{name}_pos'] = msg.position[i]
            if i < len(msg.velocity):
                row[f'{name}_vel'] = msg.velocity[i]
            if i < len(msg.effort):
                row[f'{name}_eff'] = msg.effort[i]

        self.data.append(row)

    def save(self):
        if not self.data:
            return
        keys = self.data[0].keys()
        with open(self.output_file, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=keys)
            writer.writeheader()
            writer.writerows(self.data)
        self.get_logger().info(f'已保存 {len(self.data)} 条记录到 {self.output_file}')


if __name__ == '__main__':
    rclpy.init()
    recorder = DataRecorder('chassis_data.csv')
    try:
        rclpy.spin(recorder)
    except KeyboardInterrupt:
        recorder.save()
    rclpy.shutdown()
```

### 10.4 推荐的数据采集方案

对于强化学习（RL）训练，建议的采集方案：

1. **阶段一：开环数据采集**
   - 遥控底盘执行各种运动（前进、后退、横移、旋转、组合）
   - 记录 (指令, 状态) 对
   - 用于行为克隆（BC）预训练

2. **阶段二：闭环在线训练**
   - 在仿真环境中训练策略（推荐使用 Isaac Gym / MuJoCo）
   - 将训练好的策略部署到真实机器人
   - 通过 ROS2 Topic 接收策略输出并发送控制指令

3. **数据频率**
   - 控制循环：1ms（1000Hz）
   - 建议发布频率：100Hz（每 10ms 发布一次，平衡数据量和性能）
   - 深度学习推理频率：10~50Hz（取决于模型复杂度）

---

## 11. 推理部署指南

### 11.1 推理节点架构

```
┌─────────────────────────────────────────────────┐
│              ROS2 推理节点                        │
│                                                   │
│  Sub: /chassis/joint_states (传感器反馈)          │
│  Sub: /chassis/diagnostics (诊断数据)             │
│  Sub: /task_goal (任务目标，可选)                  │
│                                                   │
│         ┌─────────────────┐                      │
│         │   DL 推理引擎    │                      │
│         │ (ONNX/PyTorch)  │                      │
│         └────────┬────────┘                      │
│                  │                                │
│  Pub: /svtrobot_cmd (Twist)                      │
│                                                   │
└─────────────────────────────────────────────────┘
```

### 11.2 Python 推理节点模板

```python
#!/usr/bin/env python3
"""
深度学习推理节点 — 将模型输出转化为底盘控制指令
"""
import numpy as np
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import JointState

class InferenceNode(Node):
    def __init__(self, model_path):
        super().__init__('dl_inference')
        self.model = self.load_model(model_path)

        # 订阅状态反馈
        self.state_sub = self.create_subscription(
            JointState, '/chassis/joint_states',
            self.state_callback, 10)

        # 发布控制指令
        self.cmd_pub = self.create_publisher(
            Twist, '/svtrobot_cmd', 10)

        # 推理频率控制
        self.timer = self.create_timer(0.05, self.inference_step)  # 20Hz
        self.latest_state = None

        self.get_logger().info('推理节点已启动')

    def load_model(self, path):
        """加载模型 — 根据你的框架修改"""
        # 示例：ONNX Runtime
        # import onnxruntime as ort
        # return ort.InferenceSession(path)
        # 示例：PyTorch
        # import torch
        # return torch.jit.load(path)
        return None

    def state_callback(self, msg):
        """接收状态反馈"""
        self.latest_state = msg

    def inference_step(self):
        """执行一次推理"""
        if self.latest_state is None:
            return

        # 构造观测向量（根据模型输入格式调整）
        obs = self.build_observation(self.latest_state)

        # 模型推理
        action = self.predict(obs)

        # 发布控制指令
        self.publish_action(action)

    def build_observation(self, state_msg):
        """从 ROS 消息构造模型输入"""
        # 示例：提取舵向角度和速度
        steer_pos = state_msg.position[:4]   # 4个舵向角度
        steer_vel = state_msg.velocity[:4]   # 4个舵向速度
        wheel_vel = state_msg.velocity[4:8]  # 4个轮速

        obs = np.array(list(steer_pos) + list(steer_vel) +
                       list(wheel_vel), dtype=np.float32)
        return obs.reshape(1, -1)

    def predict(self, obs):
        """运行模型推理"""
        # 根据你的模型框架实现
        # return self.model.run(None, {'obs': obs})[0]
        pass

    def publish_action(self, action):
        """将模型输出发布为控制指令"""
        msg = Twist()
        msg.linear.x = float(np.clip(action[0], -1.0, 1.0))  # vx
        msg.linear.y = float(np.clip(action[1], -1.0, 1.0))  # vy
        msg.angular.z = float(np.clip(action[2], -1.0, 1.0))  # wz
        self.cmd_pub.publish(msg)

        # 安全超时：如果状态反馈中断，自动停止
        # 可通过检查 latest_state 的时间戳实现


def main():
    rclpy.init()
    node = InferenceNode('model.onnx')
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

### 11.3 安全注意事项

在推理部署时，务必实现以下安全措施：

1. **指令超时**：如果状态反馈中断超过 N ms，自动发送零速指令
2. **输出钳位**：限制模型输出在安全范围内（速度、角速度上限）
3. **人工接管**：保留手动发布 `/svtrobot_cmd` 的能力，可随时覆盖模型输出
4. **紧急停止**：准备好 `Ctrl+C` 或外部信号触发的停止机制

---

## 12. 文件结构

```
svtrobo_ws/
├── src/
│   └── chassis_control/
│       ├── CMakeLists.txt                        # 构建配置
│       ├── package.xml                           # 包描述
│       ├── config/
│       │   └── params.yaml                       # ROS2 参数配置
│       ├── launch/
│       │   └── svtrobo_bringup.launch.py         # 启动文件
│       ├── msg/
│       │   └── ChassisDiagnostics.msg            # 诊断消息定义
│       ├── include/chassis_control/
│       │   ├── chassis_control.h                  # 主控制节点头文件
│       │   ├── steering_motor.h                   # RobStride 舵向电机驱动
│       │   ├── wheel_motor.h                      # ZLAC8015D 轮驱动器
│       │   └── filters.h                          # 低通滤波 & 变化率限制
│       ├── scripts/
│       │   ├── svtrobo_controller.py              # Python 控制器封装
│       │   └── test_chassis.py                    # 交互式测试脚本
│       └── src/
│           ├── main.cpp                           # 底盘控制入口
│           ├── chassis_control.cpp                # 主控制逻辑
│           ├── steering_motor.cpp                 # 舵向电机实现
│           ├── wheel_motor.cpp                    # 轮驱动器实现
│           ├── filters.cpp                        # 滤波器实现
│           └── lift_RS485_control.cpp             # 升降机构控制
├── camera_driver/
│   └── camera_driver/
│       ├── __init__.py                            # 模块入口
│       ├── realsense_camera.py                    # RealSense D405 驱动
│       └── zed_camera.py                          # ZED 2i 驱动
├── web_control/
│   ├── server.py                                  # aiohttp Web 服务器
│   ├── start_web.sh                               # 一键启动脚本
│   └── static/
│       ├── index.html                             # 控制台主页面
│       ├── css/style.css                          # 样式表
│       └── js/
│           ├── app.js                             # 主应用（rosbridge 连接管理）
│           ├── chassis.js                         # 底盘键盘控制
│           ├── chassis-status.js                  # 底盘电机状态面板
│           ├── camera.js                          # 相机画面控制
│           ├── diagnostics.js                     # 电池/电机诊断面板
│           ├── lift.js                            # 升降机构控制
│           ├── status.js                          # ROS 话题状态监控
│           └── roslib.min.js                      # roslibjs 库
├── ROBOT_SYSTEM_GUIDE.md                          # 本文档
├── PYTHON_API_GUIDE.md                            # Python API 使用指南
├── CAMERA_DRIVER_GUIDE.md                         # 相机驱动使用指南
└── WEB_CONTROL_GUIDE.md                           # Web 控制台使用指南
```

---

## 附录 A：RobStride 电机支持的通信类型

| 通信类型 | ID | 说明 |
|---------|-----|------|
| MotorRequest | 0x02 | 电机状态反馈 |
| MotorEnable | 0x03 | 电机使能 |
| MotorStop | 0x04 | 电机停止 |
| SetPosZero | 0x06 | 设置零位 |
| Can_ID | 0x07 | 更改 CAN ID |
| Control_Mode | 0x12 | 设置控制模式 |
| GetSingleParameter | 0x11 | 读取参数 |
| SetSingleParameter | 0x12 | 写入参数 |
| ErrorFeedback | 0x15 | 故障反馈 |

## 附录 B：RobStride 电机控制模式

| 模式 | ID | 说明 |
|------|-----|------|
| move_control_mode | 0 | 运控模式（力矩+位置+速度+KP+KD） |
| PosPP_control_mode | 1 | 位置模式 PP |
| Speed_control_mode | 2 | 速度模式 |
| Elect_control_mode | 3 | 电流模式 |
| Set_Zero_mode | 4 | 零点模式 |
| PosCSP_control_mode | 5 | 位置模式 CSP（当前使用） |

## 附录 C：RobStride 电机可读写参数

| 地址 | 名称 | 读/写 | 类型 | 范围 | 单位 |
|------|------|-------|------|------|------|
| 0x7005 | run_mode | R/W | uint8 | 0~5 | - |
| 0x7006 | iq_ref | R/W | float | -23~23 | A |
| 0x700A | spd_ref | R/W | float | -30~30 | rad/s |
| 0x700B | limit_torque | R/W | float | 0~12 | Nm |
| 0x7010 | cur_kp | R/W | float | - | - |
| 0x7011 | cur_ki | R/W | float | - | - |
| 0x7014 | cur_filt_gain | R/W | float | 0~1.0 | - |
| 0x7016 | loc_ref | R/W | float | - | rad |
| 0x7017 | limit_spd | R/W | float | 0~30 | rad/s |
| 0x7018 | limit_cur | R/W | float | 0~23 | A |
| 0x7019 | mechPos | R | float | - | rad |
| 0x701A | iqf | R | float | -23~23 | A |
| 0x701B | mechVel | R | float | -30~30 | rad/s |
| 0x701C | VBUS | R | float | - | V |
| 0x701D | rotation | R | int16 | - | 圈数 |
