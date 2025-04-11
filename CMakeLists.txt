cmake_minimum_required(VERSION 3.8.2)

project(qrb_ros_imu
        LANGUAGES C CXX)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories (./include)


find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/imu_component.cpp
)

ament_auto_add_executable(imu_node
  src/imu_node.cpp
)

rclcpp_components_register_nodes(${PROJECT_NAME}
  PLUGIN "qrb_ros::imu::ImuComponent"
)

target_link_libraries(imu_node
  ${PROJECT_NAME}
)

if(BUILD_TESTING)
  ament_auto_add_library(${PROJECT_NAME}_test SHARED
    test/src/imu_test.cpp
  )

  ament_auto_add_executable(imu_test
    test/src/imu_test.cpp
  )

  rclcpp_components_register_nodes(${PROJECT_NAME}_test
    PLUGIN "qrb_ros::imu::ImuListener"
  )
endif()

ament_auto_package(INSTALL_TO_SHARE launch)