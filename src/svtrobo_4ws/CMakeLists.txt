cmake_minimum_required(VERSION 3.8)
project(svtrobo_4ws)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

## Find ament packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

###########
## Build ##
###########

include_directories(
  include
)

## C++ executables
add_executable(zlac8015d_rpm_node src/zlac8015d_rpm_node.cpp)
add_executable(zlac8015d_canopen_node src/zlac8015d_canopen.cpp)
add_executable(lift_RS485_control src/lift_RS485_control.cpp)

ament_target_dependencies(zlac8015d_rpm_node
  rclcpp
  std_msgs
)

ament_target_dependencies(zlac8015d_canopen_node
  rclcpp
  std_msgs
)

ament_target_dependencies(lift_RS485_control
  rclcpp
  std_msgs
)

#############
## Install ##
#############

## Install C++ executables
install(TARGETS
  zlac8015d_rpm_node
  zlac8015d_canopen_node
  lift_RS485_control
  DESTINATION lib/${PROJECT_NAME}
)

## Install Python scripts
install(PROGRAMS
  scripts/output_4s.py
  scripts/canfd_init.bash
  DESTINATION lib/${PROJECT_NAME}
)

## Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

## Install scripts directory
install(DIRECTORY scripts/
  DESTINATION share/${PROJECT_NAME}/scripts
  USE_SOURCE_PERMISSIONS
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

