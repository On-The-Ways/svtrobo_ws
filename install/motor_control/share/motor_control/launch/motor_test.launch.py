#!/usr/bin/env python3
"""
Motor test launch file for ROS2 Humble
"""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    motor_controller_node = Node(
        package='motor_control',
        executable='motor_control_test',
        name='motor_ctrler',
        output='screen'
    )
    
    return LaunchDescription([
        motor_controller_node,
    ])

