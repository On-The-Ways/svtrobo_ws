
#!/usr/bin/env python3
import csv
import os
import threading
import time
from typing import Dict, List, Optional, Tuple

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from std_msgs.msg import Float64MultiArray, String

DPAD_H_AXIS = 0
DPAD_V_AXIS = 1
BUTTON_A = 1

LEFT_ORDER = [
    'openarm_left_joint1',
    'openarm_left_joint2',
    'openarm_left_joint3',
    'openarm_left_joint4',
    'openarm_left_joint5',
    'openarm_left_joint6',
    'openarm_left_joint7',
]
RIGHT_ORDER = [
    'openarm_right_joint1',
    'openarm_right_joint2',
    'openarm_right_joint3',
    'openarm_right_joint4',
    'openarm_right_joint5',
    'openarm_right_joint6',
    'openarm_right_joint7',
]


class ArmMotionPlayer(Node):
    def __init__(self):
        super().__init__('arm_motion_player')

        self.declare_parameter('joy_topic', '/f710/joy')
        self.declare_parameter('command_topic', '/arm/motion_cmd')
        self.declare_parameter('status_topic', '/arm/motion_status')
        self.declare_parameter('left_arm_topic', '/left_forward_position_controller/commands')
        self.declare_parameter('right_arm_topic', '/right_forward_position_controller/commands')
        self.declare_parameter('data_dir', '/home/svt/svtrobo_ws/src/arm_motion_data')
        self.declare_parameter('dpad_deadzone', 0.5)
        self.declare_parameter('playback_speed', 0.3)
        self.declare_parameter('require_a_to_arm', True)
        self.declare_parameter('button_a', BUTTON_A)
        self.declare_parameter('dpad_map.up', 'walk')
        self.declare_parameter('dpad_map.down', 'grasp')
        self.declare_parameter('dpad_map.left', 'turn')
        self.declare_parameter('dpad_map.right', 'stop')
        self.declare_parameter('motion_files.walk', 'walk.csv')
        self.declare_parameter('motion_files.grasp', 'grasp.csv')
        self.declare_parameter('motion_files.turn', 'turn.csv')

        joy_topic = self.get_parameter('joy_topic').value
        self.command_topic = self.get_parameter('command_topic').value
        status_topic = self.get_parameter('status_topic').value
        self.left_arm_topic = self.get_parameter('left_arm_topic').value
        self.right_arm_topic = self.get_parameter('right_arm_topic').value
        self.data_dir = self.get_parameter('data_dir').value
        self.dpad_deadzone = float(self.get_parameter('dpad_deadzone').value)
        self.playback_speed = max(0.05, float(self.get_parameter('playback_speed').value))
        self.require_a_to_arm = bool(self.get_parameter('require_a_to_arm').value)
        self.button_a = int(self.get_parameter('button_a').value)

        self.dpad_map = {
            'up': self.get_parameter('dpad_map.up').value,
            'down': self.get_parameter('dpad_map.down').value,
            'left': self.get_parameter('dpad_map.left').value,
            'right': self.get_parameter('dpad_map.right').value,
        }
        motion_files = {
            'walk': self.get_parameter('motion_files.walk').value,
            'grasp': self.get_parameter('motion_files.grasp').value,
            'turn': self.get_parameter('motion_files.turn').value,
        }

        self.left_pub = self.create_publisher(Float64MultiArray, self.left_arm_topic, 10)
        self.right_pub = self.create_publisher(Float64MultiArray, self.right_arm_topic, 10)
        self.status_pub = self.create_publisher(String, status_topic, 10)
        self.cmd_sub = self.create_subscription(String, self.command_topic, self.cmd_callback, 10)
        self.joy_sub = self.create_subscription(Joy, joy_topic, self.joy_callback, 10)

        self.prev_dpad_h = 0.0
        self.prev_dpad_v = 0.0
        self.prev_a = 0
        self.motion_armed = not self.require_a_to_arm
        self._play_lock = threading.Lock()
        self._stop_event = threading.Event()
        self._thread: Optional[threading.Thread] = None
        self.current_motion: Optional[str] = None
        self.last_left: Optional[List[float]] = None
        self.last_right: Optional[List[float]] = None
        self.frames: Dict[str, List[Tuple[int, List[float], List[float]]]] = {}

        for name, rel in motion_files.items():
            path = os.path.join(self.data_dir, rel)
            self.frames[name] = self._load_motion_csv(path)
            self.get_logger().info(f"加载动作 {name}: {len(self.frames[name])} 帧 <- {path}")

        self.publish_status('armed' if self.motion_armed else 'locked')
        self.get_logger().info(f"D-pad 动作映射: {self.dpad_map}")
        self.get_logger().info(f"动作指令话题: {self.command_topic}")
        self.get_logger().info(f"播放速度倍率: {self.playback_speed:.2f}x")
        if self.require_a_to_arm:
            self.get_logger().info('动作播放默认锁定，需先按 A 解锁')

    def _load_motion_csv(self, path: str) -> List[Tuple[int, List[float], List[float]]]:
        if not os.path.exists(path):
            raise FileNotFoundError(path)
        frames: List[Tuple[int, List[float], List[float]]] = []
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            reader = csv.DictReader(f)
            missing = [c for c in LEFT_ORDER + RIGHT_ORDER if c not in reader.fieldnames]
            if missing:
                raise ValueError(f"{path} 缺少列: {missing}")
            for row in reader:
                ts = int(row.get('ros_timestamp_ns') or row.get('system_timestamp_ns') or 0)
                left = [float(row[name]) for name in LEFT_ORDER]
                right = [float(row[name]) for name in RIGHT_ORDER]
                frames.append((ts, left, right))
        if not frames:
            raise ValueError(f"{path} 无有效帧")
        return frames

    def publish_status(self, text: str):
        msg = String()
        msg.data = text
        self.status_pub.publish(msg)
        self.get_logger().info(f"发布状态: {text}")

    def joy_callback(self, msg: Joy):
        buttons = list(msg.buttons)
        a_now = buttons[self.button_a] if 0 <= self.button_a < len(buttons) else 0
        if a_now and not self.prev_a and not self.motion_armed:
            self.motion_armed = True
            self.get_logger().info('A 按下：动作播放已解锁')
            self.publish_status('armed')
        self.prev_a = a_now

        axes = list(msg.axes)
        if len(axes) <= max(DPAD_H_AXIS, DPAD_V_AXIS):
            return
        h = axes[DPAD_H_AXIS]
        v = axes[DPAD_V_AXIS]
        if abs(h) > self.dpad_deadzone and abs(self.prev_dpad_h) <= self.dpad_deadzone:
            self._handle_direction('right' if h > 0 else 'left')
        if abs(v) > self.dpad_deadzone and abs(self.prev_dpad_v) <= self.dpad_deadzone:
            self._handle_direction('down' if v > 0 else 'up')
        self.prev_dpad_h = h
        self.prev_dpad_v = v

    def _handle_direction(self, direction: str):
        cmd = self.dpad_map.get(direction)
        if not cmd:
            return
        if cmd != 'stop' and not self.motion_armed:
            self.get_logger().warn('动作未解锁，按 A 后再用 D-pad 触发')
            self.publish_status('locked')
            return
        self.handle_motion_command(cmd)

    def cmd_callback(self, msg: String):
        cmd = msg.data.strip()
        if cmd == 'arm':
            self.motion_armed = True
            self.get_logger().info('收到 arm 命令：动作播放已解锁')
            self.publish_status('armed')
            return
        self.handle_motion_command(cmd)

    def handle_motion_command(self, cmd: str):
        if not cmd:
            return
        if cmd == 'stop':
            self.stop_motion('manual stop at current pose')
            return
        if cmd not in self.frames:
            self.get_logger().warn(f"未知动作: {cmd}")
            self.publish_status(f"error:unknown:{cmd}")
            return
        if not self.motion_armed:
            self.get_logger().warn('动作未解锁，忽略播放请求')
            self.publish_status('locked')
            return
        self.start_motion(cmd)

    def start_motion(self, motion_name: str):
        self.stop_motion(f"switch to {motion_name}", log_if_idle=False)
        with self._play_lock:
            self._stop_event = threading.Event()
            self.current_motion = motion_name
            self._thread = threading.Thread(
                target=self._play_motion,
                args=(motion_name, self._stop_event),
                daemon=True,
                name=f"arm-motion-{motion_name}",
            )
            self._thread.start()
        self.get_logger().info(f"开始播放动作: {motion_name}")
        self.publish_status(f"playing:{motion_name}")

    def stop_motion(self, reason: str, log_if_idle: bool = True):
        thread = None
        stop_event = None
        with self._play_lock:
            thread = self._thread
            stop_event = self._stop_event if self._thread else None
            self._thread = None
            self.current_motion = None
        if stop_event is not None:
            stop_event.set()
        if thread is not None and thread.is_alive():
            thread.join(timeout=1.0)
            if self.last_left is not None and self.last_right is not None:
                self._publish_frame(self.last_left, self.last_right)
            self.get_logger().info(f"动作已停止并保持当前姿态: {reason}")
        elif log_if_idle:
            self.get_logger().info('当前无动作在播放')
        self.publish_status('armed' if self.motion_armed else 'locked')

    def _publish_frame(self, left: List[float], right: List[float]):
        self.last_left = list(left)
        self.last_right = list(right)
        lmsg = Float64MultiArray()
        lmsg.data = list(left)
        rmsg = Float64MultiArray()
        rmsg.data = list(right)
        self.left_pub.publish(lmsg)
        self.right_pub.publish(rmsg)

    def _play_motion(self, motion_name: str, stop_event: threading.Event):
        frames = self.frames[motion_name]
        prev_ts = frames[0][0]
        try:
            for idx, (ts, left, right) in enumerate(frames):
                if stop_event.is_set():
                    return
                if idx > 0:
                    dt_ns = max(0, ts - prev_ts)
                    time.sleep((dt_ns / 1e9) / self.playback_speed)
                self._publish_frame(left, right)
                prev_ts = ts
            self.get_logger().info(f"动作播放完成: {motion_name}")
        finally:
            with self._play_lock:
                if self._thread is threading.current_thread():
                    self._thread = None
                    self.current_motion = None
            self.publish_status('armed' if self.motion_armed else 'locked')


def main():
    rclpy.init()
    node = ArmMotionPlayer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.stop_motion('shutdown', log_if_idle=False)
        node.destroy_node()
        rclpy.shutdown()
