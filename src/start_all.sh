#!/bin/bash
# SVTROBO 一键启动所有服务
# 启动: F710修复、底盘控制、升降控制、IMU、rosbridge、web控制服务

set +e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WS_DIR="/home/svt/svtrobo_ws"

source /opt/ros/humble/setup.bash 2>/dev/null
source $WS_DIR/install/setup.bash 2>/dev/null

echo "======================================"
echo "  SVTROBO 一键启动"
echo "======================================"

# [0/6] F710 手柄驱动修复
echo "[0/6] 修复 F710 手柄驱动..."
if [ -e /dev/input/js0 ]; then
    echo "  [跳过] /dev/input/js0 已存在"
else
    /usr/local/bin/f710-fix.sh 2>/dev/null
    if [ -e /dev/input/js0 ]; then
        echo "  [已修复] F710 驱动"
    else
        echo "  [警告] F710 修复失败或手柄未连接"
    fi
fi

# [0.5/6] 初始化 CAN 总线
echo "[0/6] 初始化 CAN 总线..."
for iface in can2 can3; do
    state=$(cat /sys/class/net/$iface/operstate 2>/dev/null)
    if [ "$state" != "up" ]; then
        echo '123456' | sudo -S ip link set $iface up type can bitrate 500000 2>/dev/null
        echo "  [已启动] $iface"
    else
        echo "  [跳过] $iface 已经 UP"
    fi
done

# [1/6] 底盘控制 + 升降控制
echo "[1/6] 启动底盘与升降控制..."
ros2 launch chassis_control svtrobo_bringup.launch.py &
CHASSIS_PID=$!
sleep 3
if ! kill -0 $CHASSIS_PID 2>/dev/null; then
    echo "  [警告] 底盘控制启动失败 (CAN 总线未连接?)"
    CHASSIS_PID=""
else
    echo "  [已启动] 底盘控制 + 升降控制 (PID: $CHASSIS_PID)"
fi

# [2/6] ZED IMU 节点 (直接运行 Python, 不需要 ros2 run)
echo "[2/6] 启动 ZED IMU 节点..."
if python3 -c "import pyzed.sl" 2>/dev/null; then
    cd $WS_DIR/src/camera_driver && python3 -m camera_driver.zed_imu_node --ros-args -p frame_id:=zed_imu_link &
    IMU_PID=$!
    sleep 2
    if kill -0 $IMU_PID 2>/dev/null; then
        echo "  [已启动] ZED IMU 节点 (PID: $IMU_PID)"
    else
        echo "  [警告] ZED IMU 节点启动失败"
        IMU_PID=""
    fi
else
    echo "  [跳过] ZED SDK 未安装"
    IMU_PID=""
fi

# [3/6] rosbridge
echo "[3/6] 启动 rosbridge (端口 9090)..."
ros2 launch rosbridge_server rosbridge_websocket_launch.xml port:=9090 &
ROSBRIDGE_PID=$!
sleep 2
if ! kill -0 $ROSBRIDGE_PID 2>/dev/null; then
    echo "  [错误] rosbridge 启动失败"
    kill $CHASSIS_PID 2>/dev/null
    kill $IMU_PID 2>/dev/null
    exit 1
fi
echo "  [已启动] rosbridge (PID: $ROSBRIDGE_PID)"

# [4/6] web 控制服务 (用 conda 环境, 有 pyrealsense2)
echo "[4/6] 启动 web 控制服务 (端口 8080)..."
cd "$SCRIPT_DIR/web_control"
source /home/svt/miniconda3/etc/profile.d/conda.sh
conda activate svtrobo
python3 server.py --host 0.0.0.0 --port 8080 &
SERVER_PID=$!
sleep 2
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "  [错误] web 控制服务启动失败"
    kill $CHASSIS_PID 2>/dev/null
    kill $IMU_PID 2>/dev/null
    kill $ROSBRIDGE_PID 2>/dev/null
    exit 1
fi
echo "  [已启动] web 控制服务 (PID: $SERVER_PID)"

# [5/6] f710 手柄控制
echo "[5/6] 启动 f710 手柄控制..."
if [ -e /dev/input/js0 ]; then
    ros2 launch f710_teleop f710_teleop.launch.py &
    F710_PID=$!
    sleep 2
    if kill -0 $F710_PID 2>/dev/null; then
        echo "  [已启动] 手柄控制节点 (PID: $F710_PID)"
    else
        echo "  [警告] 手柄控制节点启动失败"
        F710_PID=""
    fi
else
    echo "  [跳过] 未检测到手柄设备 /dev/input/js0"
    F710_PID=""
fi

echo ""
echo "======================================"
echo "  所有服务已启动!"
echo "  浏览器访问: http://$(hostname -I | awk '{print $1}'):8080"
echo "======================================"
echo ""
echo "按 Ctrl+C 停止所有服务..."

cleanup() {
    echo ""
    echo "正在停止所有服务..."
    ros2 topic pub --once /svtrobot_cmd geometry_msgs/msg/Twist "{linear: {x: 0, y: 0, z: 0}, angular: {x: 0, y: 0, z: 0}}" 2>/dev/null
    ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [0, 0]}" 2>/dev/null
    sleep 0.5
    kill $F710_PID 2>/dev/null
    kill $IMU_PID 2>/dev/null
    kill $SERVER_PID 2>/dev/null
    kill $CHASSIS_PID 2>/dev/null
    kill $ROSBRIDGE_PID 2>/dev/null
    pkill -f "my_controller_node" 2>/dev/null
    pkill -f "zed_imu_node" 2>/dev/null
    wait 2>/dev/null
    echo "所有服务已停止。"
}
trap cleanup SIGINT SIGTERM

wait
