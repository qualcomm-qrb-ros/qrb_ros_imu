/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include "sensor_msgs/msg/imu.hpp"

#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <string>

#include "qrb_ros_transport_imu_type/imu.hpp"

namespace qrb_ros
{
namespace imu
{
class ImuListener : public rclcpp::Node
{
public:
  explicit ImuListener(const rclcpp::NodeOptions & options) : Node("imu_test", options)
  {
    auto callback = [this](const std::shared_ptr<const qrb_ros::transport::type::Imu> container) {
      long time_nanosec = (get_clock()->now()).nanoseconds();
      long receive_nanosec =
          container->header.stamp.sec * 1000000000LL + container->header.stamp.nanosec;
      RCLCPP_INFO(this->get_logger(), "receive imu data time: %ld", receive_nanosec);
      RCLCPP_INFO(this->get_logger(), "imu accel data x: %f, y: %f, z:%f",
          container->acceleration->acceleration.x, container->acceleration->acceleration.y,
          container->acceleration->acceleration.z);
      RCLCPP_INFO(this->get_logger(), "imu gyro data x: %f, y: %f, z:%f", container->gyro->gyro.x,
          container->gyro->gyro.y, container->gyro->gyro.z);
      RCLCPP_INFO(
          this->get_logger(), "receive imu data latency: %ld ns", time_nanosec - receive_nanosec);
    };
    RCLCPP_INFO(this->get_logger(), "Subscribing to topic: imu");
    rclcpp::SubscriptionOptions sub_options;
    sub_options.use_intra_process_comm = rclcpp::IntraProcessSetting::Enable;

    sub_ = create_subscription<qrb_ros::transport::type::Imu>("imu", 40, callback, sub_options);
  }

private:
  rclcpp::Subscription<qrb_ros::transport::type::Imu>::SharedPtr sub_;
};
}  // namespace imu
}  // namespace qrb_ros

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor exec;
  rclcpp::NodeOptions options;

  auto test_node = std::make_shared<qrb_ros::imu::ImuListener>(options);
  exec.add_node(test_node);
  exec.spin();

  rclcpp::shutdown();
  return 0;
}

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(qrb_ros::imu::ImuListener)