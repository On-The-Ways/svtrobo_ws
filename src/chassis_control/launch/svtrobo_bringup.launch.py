from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_dir = get_package_share_directory("chassis_control")

    params_yaml_path = os.path.join(pkg_dir, "config", "params.yaml")


    chassis_control_node = Node(
        package="chassis_control",        # 功能包名
        executable="chassis_control",  # 可执行文件名
        name="chassis_control",        # 节点名
        output="screen",              # 日志打印到终端
        # 将YAML文件加载到ROS2参数服务器，该节点自动关联这些参数
        parameters=[params_yaml_path]
    )
    
    # 启动升降机构RS485控制节点
    lift_rs485_node = Node(
         package='chassis_control',
         executable='lift_control',
         name='lift_control',
         output='screen'
    )

    # 4. 返回启动节点
    return LaunchDescription([
        chassis_control_node,
        lift_rs485_node
    ])
