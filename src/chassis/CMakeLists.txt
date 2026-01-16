cmake_minimum_required(VERSION 3.8)

project(chassis)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ament packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(urdf REQUIRED)
find_package(robot_state_publisher REQUIRED)
find_package(rviz_common REQUIRED)


# Install directories
install(DIRECTORY config launch meshes urdf
  DESTINATION share/${PROJECT_NAME}/
)

# Export package
ament_package()

