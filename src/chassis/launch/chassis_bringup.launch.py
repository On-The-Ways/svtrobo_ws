#!/usr/bin/env python3
"""
Launch file for bringing up the chassis
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # Get package directory
    chassis_share = get_package_share_directory('chassis')

    # Robot State Publisher Node
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[
            {'robot_description': os.path.join(chassis_share, 'urdf', 'chassis.urdf')}
        ]
    )

    # Joint State Publisher Node
    joint_state_publisher_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        output='screen',
        parameters=[
            os.path.join(chassis_share, 'config', 'joint_names_chassis.yaml')
        ]
    )

    return LaunchDescription([
        robot_state_publisher_node,
        joint_state_publisher_node
    ])