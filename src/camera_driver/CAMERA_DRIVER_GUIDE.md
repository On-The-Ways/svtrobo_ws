# SVTROBO 相机采集模块使用文档

> 文件位置：`camera_driver/camera_driver/`

---

## 目录

1. [系统概述](#1-系统概述)
2. [硬件信息](#2-硬件信息)
3. [前置条件](#3-前置条件)
4. [快速开始](#4-快速开始)
5. [RealSense D405 API](#5-realsense-d405-api)
6. [ZED 2i API](#6-zed-2i-api)
7. [ROS2 相机节点](#7-ros2-相机节点)
8. [文件结构](#8-文件结构)
9. [注意事项与常见问题](#9-注意事项与常见问题)

---

## 1. 系统概述

本模块提供 RealSense D405 和 ZED 2i 相机的 Python 采集接口，支持：

- 彩色图像 + 深度图同时采集
- 自动深度对齐到彩色图（RealSense）
- 上下文管理器自动管理生命周期
- 采集并保存到磁盘（按相机名称区分）
- 相机内参获取
- 彩色点云生成（RealSense）

---

## 2. 硬件信息

### 2.1 已连接相机

| 相机 | 型号 | 序列号 | USB 设备 |
|------|------|--------|----------|
| D405 #1 | Intel RealSense D405 | `409122272399` | Bus 001 |
| D405 #2 | Intel RealSense D405 | `409122273344` | Bus 001 |
| ZED 2i | STEREOLABS ZED 2i | - | `/dev/video0` |

### 2.2 分辨率支持

**D405 支持分辨率（彩色 / 深度）：**

| 分辨率 | 彩色 fps | 深度 fps | 当前默认 |
|--------|----------|----------|----------|
| 1280x720 | 5/10/15 | 5 | **默认** |
| 848x480 | 5/10 | 5/10 | |
| 640x480 | 5/15/30 | 5/15/30 | |
| 640x360 | - | 30 | |
| 480x270 | 5/15/30/60 | 5/15/30/60 | |
| 424x240 | 5/15/30/60 | - | |

> 注意：彩色和深度流需要相同 fps 才能同时运行。1280x720 最高 5fps。

**ZED 2i（OpenCV V4L2 模式）：**

| 分辨率 | 说明 |
|--------|------|
| 1344x376 (VGA) | Side-by-Side，左右各 672x376，当前唯一可用 |

> ZED SDK 需要 NVIDIA GPU + CUDA，本机无 GPU，仅支持 OpenCV 降级模式。

---

## 3. 前置条件

### 依赖安装

```bash
# RealSense SDK
sudo apt install librealsense2-dev librealsense2-utils
pip3 install pyrealsense2

# 通用依赖（通常已安装）
pip3 install numpy opencv-python
```

### 验证设备

```bash
# 列出 RealSense 设备
rs-enumerate-devices --short

# 列出视频设备
ls /dev/video*

# 查看 ZED 设备
cat /sys/class/video4linux/video0/name
# 应输出: ZED 2i
```

### 导入模块

```python
import sys
sys.path.insert(0, '/home/openarm/svtrobo_ws')

from camera_driver.camera_driver import RealSenseCamera, ZEDCamera
```

---

## 4. 快速开始

### 4.1 RealSense D405

```python
import sys
sys.path.insert(0, '/home/openarm/svtrobo_ws')
from camera_driver.camera_driver import RealSenseCamera

# 列出所有设备
devices = RealSenseCamera.list_devices()
for d in devices:
    print(f"  {d['name']} SN:{d['serial']}")

# 采集并保存
with RealSenseCamera(serial='409122272399') as cam:
    color_path, depth_path = cam.capture_and_save(name='d405_1')
    print(f'已保存: {color_path}')

# 仅获取 numpy 数组
with RealSenseCamera(serial='409122272399') as cam:
    color, depth = cam.capture()
    print(f'彩色: {color.shape}, 深度: {depth.shape}')
```

### 4.2 ZED 2i

```python
import sys
sys.path.insert(0, '/home/openarm/svtrobo_ws')
from camera_driver.camera_driver import ZEDCamera

with ZEDCamera() as zed:
    left, depth = zed.capture()
    print(f'左眼: {left.shape}, 深度: {depth.shape}')

# 采集并保存
with ZEDCamera() as zed:
    left_path, depth_path = zed.capture_and_save(name='zed_2i')
```

---

## 5. RealSense D405 API

> 源文件：`camera_driver/camera_driver/realsense_camera.py`

### 5.1 构造函数

```python
RealSenseCamera(
    serial='',              # 设备序列号，空字符串自动选第一个
    color_size=(1280, 720), # (width, height) 彩色图分辨率
    depth_size=(1280, 720), # (width, height) 深度图分辨率
    fps=5,                  # 帧率（1280x720 最高 5fps）
)
```

**参数说明：**

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `serial` | str | `''` | 设备序列号，空则选第一个设备 |
| `color_size` | tuple | `(1280, 720)` | 彩色图 (宽, 高) |
| `depth_size` | tuple | `(1280, 720)` | 深度图 (宽, 高) |
| `fps` | int | `5` | 帧率，高分辨率下受硬件限制 |

### 5.2 `start()` — 启动相机

```python
cam = RealSenseCamera(serial='409122272399')
cam.start()
# 使用完毕后
cam.stop()
```

预热丢弃前 30 帧，确保曝光和白平衡稳定。

### 5.3 `stop()` — 停止相机

释放 pipeline 资源。

### 5.4 `capture()` — 采集一帧

```python
color, depth = cam.capture()
# color: numpy (H, W, 3) BGR uint8
# depth: numpy (H, W) uint16, 单位: depth_scale (默认 0.001 米)
# 失败返回 (None, None)
```

深度图已对齐到彩色图。深度值 × `depth_scale` = 实际距离（米）。

### 5.5 `capture_and_save()` — 采集并保存

```python
color_path, depth_path = cam.capture_and_save(
    save_dir=None,  # 保存目录，默认 camera_driver/captures/
    name='d405_1',  # 文件名前缀
)
# 返回: ('.../d405_1_20260410_123456_color.png', '.../d405_1_20260410_123456_depth.png')
# 失败返回: (None, None)
```

**参数说明：**

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `save_dir` | str | `None` | 保存目录，默认 `camera_driver/captures/` |
| `name` | str | 设备 SN | 文件名前缀，用于区分不同相机 |

**输出文件命名规则：**

```
{name}_{YYYYMMDD}_{HHMMSS}_color.png
{name}_{YYYYMMDD}_{HHMMSS}_depth.png
```

### 5.6 `get_intrinsics()` — 获取相机内参

```python
intrinsics = cam.get_intrinsics()
# {
#   'fx': 393.5, 'fy': 392.6,
#   'ppx': 318.2, 'ppy': 239.2,
#   'width': 640, 'height': 480,
#   'coeffs': [0.0, 0.0, 0.0, 0.0, 0.0]
# }
```

### 5.7 `get_depth_scale()` — 获取深度比例因子

```python
scale = cam.get_depth_scale()  # 例: 0.001
# depth_米 = depth_像素值 × scale
```

### 5.8 `capture_pointcloud()` — 采集彩色点云

```python
points = cam.capture_pointcloud(max_depth=1.0)
# numpy (N, 6): [x, y, z, r, g, b]
# x/y/z 单位: 米
# 超过 max_depth 的点被过滤
```

### 5.9 `list_devices()` — 列出设备（静态方法）

```python
devices = RealSenseCamera.list_devices()
# [{'name': 'Intel RealSense D405', 'serial': '409122272399', 'firmware': '5.15.1.55'}, ...]
```

### 5.10 上下文管理器

```python
# 推荐：自动管理生命周期
with RealSenseCamera(serial='409122272399') as cam:
    color, depth = cam.capture()
# 退出 with 时自动 stop()
```

### 5.11 同时使用两台 D405

```python
cam1 = RealSenseCamera(serial='409122272399')
cam2 = RealSenseCamera(serial='409122273344')

cam1.start()
cam2.start()

color1, depth1 = cam1.capture()
color2, depth2 = cam2.capture()

cam1.stop()
cam2.stop()
```

> 注意：不要同时启动超过 USB 带宽限制的组合。两台 D405 在 640x480@30fps 下可同时工作，1280x720@5fps 也可。

---

## 6. ZED 2i API

> 源文件：`camera_driver/camera_driver/zed_camera.py`

### 6.1 当前限制

本机无 NVIDIA GPU，无法安装 ZED SDK，运行在 **OpenCV V4L2 降级模式**：

| 功能 | 状态 |
|------|------|
| 左眼彩色图 | 672x376 |
| 右眼彩色图 | 672x376（通过 `capture_stereo()`） |
| 深度图 | SGBM 估算，精度较低 |
| 高分辨率 | 不支持（需要 SDK） |
| IMU | 不支持（需要 SDK） |

### 6.2 构造函数

```python
ZEDCamera(
    resolution='HD720',    # 仅 VGA 实际生效
    fps=30,                # 帧率
    depth_mode='NEURAL',   # 仅 SDK 模式生效
    min_depth=100.0,       # 最小深度 mm，仅 SDK 模式生效
    force_opencv=False,    # 强制 OpenCV 模式
)
```

### 6.3 `capture()` — 采集一帧

```python
left, depth = zed.capture()
# left: numpy (376, 672, 3) BGR uint8
# depth: numpy (376, 672) uint16, 单位 mm（SGBM 估算）
# 失败返回 (None, None)
```

### 6.4 `capture_stereo()` — 采集左右眼 + 深度

```python
left, right, depth = zed.capture_stereo()
# left:  (376, 672, 3) BGR uint8
# right: (376, 672, 3) BGR uint8
# depth: (376, 672) uint16 mm
```

### 6.5 `capture_and_save()` — 采集并保存

```python
left_path, depth_path = zed.capture_and_save(
    save_dir=None,
    name='zed_2i',
)
# 输出: zed_2i_{timestamp}_left.png, zed_2i_{timestamp}_depth.png
```

### 6.6 `get_intrinsics()` — 获取内参

```python
intrinsics = zed.get_intrinsics()
# 仅 SDK 模式可用，OpenCV 模式返回 None
```

### 6.7 上下文管理器

```python
with ZEDCamera() as zed:
    left, depth = zed.capture()
```

---

## 7. ROS2 相机节点

除了 Python 采集 API，camera_driver 还提供了 ROS2 节点，可直接发布 sensor_msgs/Image 和 CameraInfo 话题。

### 7.1 RealSense D405 ROS2 节点

> 源文件：`camera_driver/camera_driver/realsense_node.py`

**发布话题：**

| 话题 | 消息类型 | 说明 |
|------|---------|------|
| `{namespace}/color/image_raw` | `sensor_msgs/Image` | 彩色图 (BGR8) |
| `{namespace}/depth/image_raw` | `sensor_msgs/Image` | 深度图 (16UC1, 对齐到彩色) |
| `{namespace}/color/camera_info` | `sensor_msgs/CameraInfo` | 彩色相机内参 |
| `{namespace}/depth/camera_info` | `sensor_msgs/CameraInfo` | 深度相机内参 |

**参数：**

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `serial_number` | str | `''` | 设备序列号，空则选第一个 |
| `namespace` | str | `'d405_1'` | 话题命名空间 |
| `color_width` | int | `640` | 彩色图宽度 |
| `color_height` | int | `480` | 彩色图高度 |
| `depth_width` | int | `640` | 深度图宽度 |
| `depth_height` | int | `480` | 深度图高度 |
| `fps` | int | `30` | 帧率 |
| `frame_id` | str | `'camera_link'` | TF frame ID |

**启动：**

```bash
ros2 run camera_driver realsense_node --ros-args \
  -p serial_number:=409122272399 \
  -p namespace:=d405_1
```

### 7.2 ZED 2i ROS2 节点

> 源文件：`camera_driver/camera_driver/zed_node.py`

双模式：SDK 优先（需 NVIDIA GPU），OpenCV 降级（SGBM 估算深度）。

**发布话题：**

| 话题 | 消息类型 | 说明 |
|------|---------|------|
| `zed/left/image_raw` | `sensor_msgs/Image` | 左眼彩色图 (BGR8) |
| `zed/right/image_raw` | `sensor_msgs/Image` | 右眼彩色图 (BGR8，仅 OpenCV 模式) |
| `zed/depth/image_raw` | `sensor_msgs/Image` | 深度图 (16UC1) |
| `zed/left/camera_info` | `sensor_msgs/CameraInfo` | 左眼相机内参（仅 SDK 模式） |

**参数：**

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `resolution` | str | `'HD720'` | 分辨率：HD2K/HD1080/HD720/VGA |
| `fps` | int | `30` | 帧率 |
| `depth_mode` | str | `'NEURAL'` | SDK 深度模式：NEURAL/ULTRA/QUALITY/PERFORMANCE |
| `min_depth` | float | `100.0` | 最小深度 mm（仅 SDK） |
| `frame_id` | str | `'zed_link'` | TF frame ID |
| `force_opencv` | bool | `False` | 强制使用 OpenCV 模式 |

**启动：**

```bash
ros2 run camera_driver zed_node --ros-args \
  -p resolution:=HD720 -p depth_mode:=NEURAL
```

---

## 8. 文件结构

```
svtrobo_ws/
└── camera_driver/
    ├── camera_driver/
    │   ├── __init__.py              # 模块入口，导出 RealSenseCamera, ZEDCamera
    │   ├── realsense_camera.py      # D405 采集模块
    │   ├── realsense_node.py        # D405 ROS2 发布节点
    │   ├── zed_camera.py            # ZED 2i 采集模块
    │   └── zed_node.py              # ZED 2i ROS2 发布节点
    └── captures/                    # 默认保存目录
        ├── d405_1_*_color.png       # D405 #1 彩色图
        ├── d405_1_*_depth.png       # D405 #1 深度图
        ├── d405_2_*_color.png       # D405 #2 彩色图
        ├── d405_2_*_depth.png       # D405 #2 深度图
        ├── zed_2i_*_left.png        # ZED 左眼图像
        └── zed_2i_*_depth.png       # ZED 深度图
```

---

## 9. 注意事项与常见问题

### Q1: D405 启动报错 `Couldn't resolve requests`

分辨率/fps 组合不兼容。参考第 2.2 节支持的分辨率表，确保彩色和深度流使用相同 fps。

### Q2: 两台 D405 同时使用时帧率下降

USB 带宽限制。降低分辨率或帧率：

```python
cam = RealSenseCamera(serial='...', color_size=(640, 480), depth_size=(640, 480), fps=15)
```

### Q3: ZED 启动报错 `未找到 ZED 相机设备`

检查 ZED 是否被其他程序占用：

```bash
# 检查设备
cat /sys/class/video4linux/video0/name
# 如果不是 ZED，检查 USB 连接
lsusb | grep STEREOLABS
```

### Q4: 深度图看起来全黑

D405 深度图是 uint16 格式，普通图片查看器可能显示为全黑。用代码查看实际数值：

```python
import cv2
depth = cv2.imread('depth.png', cv2.IMREAD_UNCHANGED)
print(f'范围: [{depth.min()}, {depth.max()}]')
```

或转为伪彩色可视化：

```python
depth_color = cv2.applyColorMap(
    cv2.convertScaleAbs(depth, alpha=0.03), cv2.COLORMAP_JET)
cv2.imwrite('depth_visual.png', depth_color)
```

### Q5: 如何在有 GPU 的机器上使用 ZED SDK？

安装 ZED SDK 后，代码自动检测并使用 SDK 模式，无需修改代码：

```bash
# 下载并安装 ZED SDK（需要 NVIDIA GPU + CUDA）
# https://www.stereolabs.com/developers/release/
pip3 install pyzed
```

SDK 模式自动获得：高分辨率深度、NEURAL 深度模式、IMU 数据、点云等。

### Q6: 如何修改默认保存路径？

```python
cam.capture_and_save(save_dir='/your/custom/path', name='d405_1')
```

### Q7: 相机预热时间长吗？

预热丢弃 30 帧，约 1-6 秒（取决于帧率）。如果不需要稳定曝光，可以修改 `realsense_camera.py` 中的预热帧数。
