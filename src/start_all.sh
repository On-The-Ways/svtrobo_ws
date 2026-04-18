#!/bin/bash
# SVTROBO 一键启动所有服务 (通过 systemd)
# 当前共9个systemd服务，按依赖顺序启动

set -e

SUDO="echo '123456' | sudo -S"

echo "======================================"
echo "  SVTROBO 一键启动 (systemd)"
echo "======================================"

# 服务列表（按依赖顺序）
SERVICES=(
    "f710-fix"                # [1] F710 HID修复 (inactive正常)
    "svtrobo-can"             # [2] CAN总线初始化
    "svtrobo-rosbridge"       # [3] rosbridge (9090)
    "svtrobo-chassis"         # [4] 底盘+升降控制 (1kHz)
    "svtrobo-f710"            # [5] F710手柄遥操作
    "svtrobo-web"             # [6] Web控制台 (8080)
    "svtrobo-nodeapi"         # [7] Node API (28181)
    "svtrobo-chassis-watchdog" # [8] 底盘节点存活监控
    "pcan-monitor"            # [9] PCAN/CAN状态监控
)

for svc in "${SERVICES[@]}"; do
    $SUDO systemctl start $svc 2>/dev/null
    status=$(systemctl is-active $svc 2>/dev/null)
    if [ "$status" = "active" ]; then
        echo "  [OK] $svc"
    elif [ "$svc" = "f710-fix" ]; then
        echo "  [--] $svc (inactive正常，一次性修复脚本)"
    else
        echo "  [!!] $svc: $status"
    fi
done

echo ""
echo "======================================"
echo "  所有服务已启动!"
echo "  Web控制台: http://$(hostname -I | awk '{print $1}'):8080"
echo "======================================"
echo ""
echo "服务状态概览:"
systemctl is-active "${SERVICES[@]}" | paste -d' ' <(printf '%s\n' "${SERVICES[@]}") - | column -t
