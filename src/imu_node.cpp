/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "qrb_ros_imu/imu_component.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  rclcpp::executors::SingleThreadedExecutor exec;
  auto node = std::make_shared<qrb_ros::imu::ImuComponent>(options);
  exec.add_node(node);
  while (rclcpp::ok() && node->is_working) {
    exec.spin_once();
  }
  rclcpp::shutdown();
  return 0;
}
