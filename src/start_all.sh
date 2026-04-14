#!/bin/bash
# SVTROBO 一键启动所有服务
# 启动: 底盘控制、升降控制、rosbridge、web控制服务

set +e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

source /opt/ros/humble/setup.bash 2>/dev/null
source /home/openarm/svtrobo_ws/install/setup.bash 2>/dev/null

echo "======================================"
echo "  SVTROBO 一键启动"
echo "======================================"

# [1/4] 底盘控制 + 升降控制
echo "[1/4] 启动底盘与升降控制..."
ros2 launch chassis_control svtrobo_bringup.launch.py &
CHASSIS_PID=$!
sleep 3
if ! kill -0 $CHASSIS_PID 2>/dev/null; then
    echo "  [错误] 底盘控制启动失败"
    exit 1
fi
echo "  [已启动] 底盘控制 + 升降控制 (PID: $CHASSIS_PID)"

# [2/4] rosbridge
echo "[2/4] 启动 rosbridge (端口 9090)..."
ros2 launch rosbridge_server rosbridge_websocket_launch.xml port:=9090 &
ROSBRIDGE_PID=$!
sleep 2
if ! kill -0 $ROSBRIDGE_PID 2>/dev/null; then
    echo "  [错误] rosbridge 启动失败"
    kill $CHASSIS_PID 2>/dev/null
    exit 1
fi
echo "  [已启动] rosbridge (PID: $ROSBRIDGE_PID)"

# [3/4] web 控制服务
echo "[3/4] 启动 web 控制服务 (端口 8080)..."
cd "$SCRIPT_DIR/web_control"
python3 server.py --host 0.0.0.0 --port 8080 &
SERVER_PID=$!
sleep 2
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "  [错误] web 控制服务启动失败"
    kill $CHASSIS_PID 2>/dev/null
    kill $ROSBRIDGE_PID 2>/dev/null
    exit 1
fi
echo "  [已启动] web 控制服务 (PID: $SERVER_PID)"

# [4/4] f710 手柄控制
echo "[4/4] 启动 f710 手柄控制..."
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
echo "  浏览器访问: http://localhost:8080"
echo "  手动停止: bash src/stop_all.sh"
echo "======================================"
echo ""
echo "按 Ctrl+C 停止所有服务..."

cleanup() {
    echo ""
    echo "正在停止所有服务..."
    # 先发停止指令
    ros2 topic pub --once /svtrobot_cmd geometry_msgs/msg/Twist "{linear: {x: 0, y: 0, z: 0}, angular: {x: 0, y: 0, z: 0}}" 2>/dev/null
    ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [0, 0]}" 2>/dev/null
    sleep 0.5
    kill $F710_PID 2>/dev/null
    kill $SERVER_PID 2>/dev/null
    kill $CHASSIS_PID 2>/dev/null
    kill $ROSBRIDGE_PID 2>/dev/null
    pkill -f "my_controller_node" 2>/dev/null
    wait 2>/dev/null
    echo "所有服务已停止。"
}
trap cleanup SIGINT SIGTERM

wait
