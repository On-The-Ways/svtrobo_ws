#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
import time
from std_msgs.msg import Int32, Float64
from geometry_msgs.msg import Twist


class ChassisMonitor(Node):
    def __init__(self):
        super().__init__('monitor_chassis')
        
        # 电机转速
        self.front_left_cmd = 0
        self.front_right_cmd = 0
        self.rear_left_cmd = 0
        self.rear_right_cmd = 0
        
        # 转向角度
        self.front_left_steer = 0.0
        self.front_right_steer = 0.0
        self.rear_left_steer = 0.0
        self.rear_right_steer = 0.0
        
        # 创建订阅器
        self.create_subscription(Int32, '/front_left_cmd', self.fl_cmd_callback, 10)
        self.create_subscription(Int32, '/front_right_cmd', self.fr_cmd_callback, 10)
        self.create_subscription(Int32, '/rear_left_cmd', self.rl_cmd_callback, 10)
        self.create_subscription(Int32, '/rear_right_cmd', self.rr_cmd_callback, 10)
        
        self.create_subscription(Float64, '/front_left_steer', self.fl_steer_callback, 10)
        self.create_subscription(Float64, '/front_right_steer', self.fr_steer_callback, 10)
        self.create_subscription(Float64, '/rear_left_steer', self.rl_steer_callback, 10)
        self.create_subscription(Float64, '/rear_right_steer', self.rr_steer_callback, 10)
        
        # 订阅cmd_vel，每次发布时输出一次
        self.create_subscription(Twist, '/cmd_vel', self.cmd_vel_callback, 10)
    
    def fl_cmd_callback(self, msg):
        self.front_left_cmd = msg.data
    
    def fr_cmd_callback(self, msg):
        self.front_right_cmd = msg.data
    
    def rl_cmd_callback(self, msg):
        self.rear_left_cmd = msg.data
    
    def rr_cmd_callback(self, msg):
        self.rear_right_cmd = msg.data
    
    def fl_steer_callback(self, msg):
        self.front_left_steer = msg.data
    
    def fr_steer_callback(self, msg):
        self.front_right_steer = msg.data
    
    def rl_steer_callback(self, msg):
        self.rear_left_steer = msg.data
    
    def rr_steer_callback(self, msg):
        self.rear_right_steer = msg.data
    
    def cmd_vel_callback(self, msg):
        # 每次收到cmd_vel时输出一次
        time.sleep(0.05)  # 等待50ms让控制器计算完成
        print(f"RPM: FL={self.front_left_cmd:4d} FR={self.front_right_cmd:4d} "
              f"RL={self.rear_left_cmd:4d} RR={self.rear_right_cmd:4d} | "
              f"Steer: FL={self.front_left_steer:6.3f} FR={self.front_right_steer:6.3f} "
              f"RL={self.rear_left_steer:6.3f} RR={self.rear_right_steer:6.3f}")


def main(args=None):
    rclpy.init(args=args)
    monitor = ChassisMonitor()
    
    try:
        rclpy.spin(monitor)
    except KeyboardInterrupt:
        pass
    finally:
        monitor.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
