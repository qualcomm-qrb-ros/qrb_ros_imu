/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#ifndef QRB_ROS_IMU__IMU_COMPONENT_HPP_
#define QRB_ROS_IMU__IMU_COMPONENT_HPP_

#include <thread>

#include "qrb_ros_transport_imu_type/imu.hpp"
#include "qrb_sensor_client/sensor_client.hpp"
#include "rclcpp/rclcpp.hpp"

#define TOPIC_NAME "imu"
#define TOPIC_TYPE qrb_ros::transport::type::Imu
#define RETRY_MAX 30
#define RETRY_INTERVAL 2  // 2s

namespace qrb_ros
{
namespace imu
{
class ImuComponent : public rclcpp::Node
{
public:
  bool is_working;
  explicit ImuComponent(const rclcpp::NodeOptions & options);
  ~ImuComponent();

private:
  bool init();
  void connect_success();
  void retry_connection();
  void publish_msg();
  qrb::sensor_client::SensorClient sensor_client_;
  rclcpp::Publisher<TOPIC_TYPE>::SharedPtr publisher_;
  bool debug_ = false;
  bool running_;
  int retry_;
  std::shared_ptr<std::thread> thread_publish_msg_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace imu
}  // namespace qrb_ros

#endif