#!/bin/bash
# SVTROBO 一键停止所有服务
# 先发送停止指令，再停止所有节点

echo "正在停止所有 SVTROBO 服务..."

source /opt/ros/humble/setup.bash 2>/dev/null

# 先发送停止指令，确保机器人安全停下
echo "发送停止指令..."
timeout 2 ros2 topic pub --once /svtrobot_cmd geometry_msgs/msg/Twist "{linear: {x: 0, y: 0, z: 0}, angular: {x: 0, y: 0, z: 0}}" 2>/dev/null && echo "  [已发送] 底盘停止指令" || echo "  [跳过] 底盘节点未运行"
timeout 2 ros2 topic pub --once /lift_control_cmd std_msgs/msg/Int32MultiArray "{data: [0, 0]}" 2>/dev/null && echo "  [已发送] 升降停止指令" || echo "  [跳过] 升降节点未运行"

sleep 0.5

# 停止 web 控制服务
pkill -f "python3 server.py" 2>/dev/null && echo "  [已停止] web 控制服务" || echo "  [未运行] web 控制服务"

# 停止 f710 手柄节点
pkill -f "my_controller_node" 2>/dev/null && echo "  [已停止] f710 手柄节点" || echo "  [未运行] f710 手柄节点"

# 停止底盘和升降控制
pkill -f "chassis_control_node" 2>/dev/null && echo "  [已停止] 底盘控制节点" || echo "  [未运行] 底盘控制节点"
pkill -f "lift_control" 2>/dev/null && echo "  [已停止] 升降控制节点" || echo "  [未运行] 升降控制节点"

# 停止 ros2 launch 进程
pkill -f "ros2 launch" 2>/dev/null && echo "  [已停止] ros2 launch 进程" || echo "  [未运行] ros2 launch 进程"

# 停止 rosbridge
pkill -f "rosbridge_websocket" 2>/dev/null && echo "  [已停止] rosbridge" || echo "  [未运行] rosbridge"

sleep 1
echo "所有服务已停止。"
