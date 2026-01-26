#!/usr/bin/env python3
"""
svtrobo 底盘系统启动文件 - ROS2 Humble
"""

from launch import LaunchDescription
from launch_ros.actions import Node
import os


def generate_launch_description():
    # 1. 启动底盘驱动节点
    classis_control_node = Node(
         package='classis_control',
         executable='classis_control',
         name='classis_control',
         output='screen'
    )
    
    # 2. 启动升降机构RS485控制节点
    lift_rs485_node = Node(
         package='classis_control',
         executable='lift_control',
         name='lift_control',
         output='screen'
    )
    
    return LaunchDescription([
        classis_control_node,
        lift_rs485_node,
    ])
