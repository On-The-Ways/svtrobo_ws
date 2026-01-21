from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # 获取包的share目录路径
    package_dir = get_package_share_directory('classis_control')
    
    # 配置文件路径
    config_file = os.path.join(package_dir, 'config', 'start_angles.yaml')
    
    return LaunchDescription([
        Node(
            package='classis_control',
            executable='classis_control',
            name='motor_control_set_node',
            parameters=[config_file],  # 加载配置文件
            output='screen',
        ),
    ])

