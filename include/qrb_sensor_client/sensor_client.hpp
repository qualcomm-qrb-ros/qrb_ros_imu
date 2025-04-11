/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_SENSOR_CLIENT__SENSOR_CLIENT_HPP_
#define QRB_SENSOR_CLIENT__SENSOR_CLIENT_HPP_

#include <stdint.h>
#include <sys/un.h>

#include <string>

#include "qrb_sensor_client/sns_direct_channel_buffer.hpp"

#define STOP 1
#define SOCKET_PATH "/dev/shm/server_socket"

namespace qrb
{
namespace sensor_client
{
class SensorClient
{
public:
  bool GetImuData(sensors_event_t ** accel_ptr,
      sensors_event_t ** gyro_ptr,
      int32_t * sample_count);
  bool CreateConnection();
  void DisconnectServer();
  int ReadMsg(char * buffer, int len);

private:
  int _client_fd{ 0 };
};
}  // namespace sensor_client
}  // namespace qrb
#endif
