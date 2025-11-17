/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_SENSOR_CLIENT__SENSOR_CLIENT_HPP_
#define QRB_SENSOR_CLIENT__SENSOR_CLIENT_HPP_

#include <stdint.h>
#include <sys/un.h>

#include <mutex>
#include <string>
#include <vector>

#include "qrb_sensor_client/imu_sensor.hpp"
#include "qrb_sensor_client/sns_direct_channel_buffer.hpp"

#define SOCKET_PATH "/dev/shm/server_socket"

#define GETCONFIG 0
#define START 1
#define STOP 2

#define IMU 0
#define SENSOR_SIZE 1

namespace qrb
{
namespace sensor_client
{
enum class SensorType
{
  IMU_TYPE,
};

class SensorClient
{
public:
  SensorClient()
  {
    for (int i = 0; i < SENSOR_SIZE; i++) {
      sensor_statue[i] = false;
    }
  }

  ~SensorClient()
  {
    if (_client_fd != 0) {
      disconnect_server();
    }
  }

  bool get_imu_data(sensors_event_t ** accel_ptr,
      sensors_event_t ** gyro_ptr,
      int32_t * sample_count);
  bool create_connection();
  void disconnect_server();

private:
  bool get_config_data(SensorType type);
  int get_sensors_fd(int fd_number, std::vector<int> & sensors_fd);

  std::mutex mtx_;
  int _client_fd{ 0 };

  bool sensor_statue[SENSOR_SIZE];
  IMUSensor sensor;
  std::string logger_ = "SensorClient";
};
}  // namespace sensor_client
}  // namespace qrb
#endif
