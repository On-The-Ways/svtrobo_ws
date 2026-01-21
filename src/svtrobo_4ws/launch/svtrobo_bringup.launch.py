#!/usr/bin/env python3
"""
4WS-4WD底盘系统启动文件 - ROS2 Humble
"""

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    # 获取功能包路径
    motor_control_share = get_package_share_directory('motor_control')
    
    # 1. 启动独立轮毂电机驱动节点
    zlac_canopen_node = Node(
         package='svtrobo_4ws',
         executable='zlac8015d_canopen_node',
         name='zlac8015d_canopen_node',
         output='screen'
    )
    
    # 2. 转速反馈节点
    zlac_rpm_node = Node(
        package='svtrobo_4ws',
        executable='zlac8015d_rpm_node',
        name='zlac8015d_rpm_node',
        output='screen'
    )
    
    # 3. 启动升降机构RS485控制节点
    # lift_rs485_node = Node(
    #     package='svtrobo_4ws',
    #     executable='lift_RS485_control',
    #     name='lift_RS485_control',
    #     output='screen'
    # )
    
    # 4. 启动独立转向（包含motor_test.launch.py）
    motor_test_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(motor_control_share, 'launch', 'motor_test.launch.py')
        )
    )
    
    return LaunchDescription([
        zlac_canopen_node,
        #zlac_rpm_node,
        #lift_rs485_node,
        #motor_test_launch,
    ])
