cmake_minimum_required(VERSION 3.8)
project(qrb_sensor_client VERSION 1.3.0)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find dependencies
find_package(ament_cmake_auto REQUIRED)
include_directories(include)

# add targets
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/sensor_client.cpp
  src/imu_sensor.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(BUILD_TESTING)

  ament_auto_add_executable(qrb_sensor_client_test
    test/src/sensor_client_test.cpp
  )

  target_link_libraries(qrb_sensor_client_test
    ${PROJECT_NAME}
  )

  install(TARGETS qrb_sensor_client_test
    RUNTIME DESTINATION bin)

endif()

ament_auto_package()
