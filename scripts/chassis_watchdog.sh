#!/bin/bash
# chassis_watchdog.sh - Monitor chassis_control_node and restart service on crash
# P0-1: Auto-detect and restart when chassis_control_node dies

# Source ROS environment
source /opt/ros/humble/setup.bash
source /home/svt/svtrobo_ws/install/setup.bash

LOG_FILE="/tmp/chassis_watchdog.log"
NODE_NAME="chassis_control"
SERVICE_NAME="svtrobo-chassis"
CHECK_INTERVAL=5
FAIL_THRESHOLD=2

log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> "$LOG_FILE"
}

fail_count=0

log "=== chassis_watchdog started ==="

while true; do
    # Only monitor when svtrobo-chassis service is active
    if ! systemctl is-active --quiet "$SERVICE_NAME"; then
        # Service not running, reset counter and wait
        if [ $fail_count -ne 0 ]; then
            log "Service $SERVICE_NAME is not active, resetting fail counter"
            fail_count=0
        fi
        sleep $CHECK_INTERVAL
        continue
    fi

    # Check if chassis_control node is alive via ROS2 CLI
    if ! ros2 node list 2>/dev/null | grep -q "/${NODE_NAME}"; then
        fail_count=$((fail_count + 1))
        log "WARNING: /${NODE_NAME} not detected (fail ${fail_count}/${FAIL_THRESHOLD})"

        if [ $fail_count -ge $FAIL_THRESHOLD ]; then
            log "ACTION: Restarting $SERVICE_NAME (node /${NODE_NAME} missing for ${fail_count} consecutive checks)"
            echo '123456' | sudo -S systemctl restart "$SERVICE_NAME"
            log "ACTION: $SERVICE_NAME restart triggered"
            fail_count=0
            # Wait for service to come back up before resuming checks
            sleep 10
            continue
        fi
    else
        # Node is alive, reset counter
        if [ $fail_count -ne 0 ]; then
            log "OK: /${NODE_NAME} recovered, resetting fail counter"
        fi
        fail_count=0
    fi

    sleep $CHECK_INTERVAL
done
