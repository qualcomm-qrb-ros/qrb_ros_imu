/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_SENSOR_CLIENT__IMU_SENSOR_HPP_
#define QRB_SENSOR_CLIENT__IMU_SENSOR_HPP_

#include <stdint.h>
#include <sys/un.h>

#include <string>

#include "qrb_sensor_client/sns_direct_channel_buffer.hpp"

#define PACK_NUM_MAX (1000)  // ring buffer len
#define SHARED_MEMORY_SIZE (PACK_NUM_MAX * sizeof(sensors_event_t))

namespace qrb
{
namespace sensor_client
{
class IMUSensor
{
public:
  /*
   * Two steps:
   * 1.Get the offset between accel and gyro.
   * 2.Read ring buffer accle and gyro, find latest sensor data return available
   * nums and start pointer.
   *
   *                      accel                           gyro
   *                   *---------*                    *---------*
   *                  0| sensor1 |                   0| sensor1 |
   *                   *---------*                    *---------*
   * count_accel_---> 1|         |                 ...|         |
   *                   *---------*       ---->        *---------*
   *                ...|         |    offset = 2     3|         | <---count_gyro_
   *                   *---------*                    *---------*
   *                999|         |                 999|         |
   *                   *---------*                    *---------*
   */
  bool get_data(sensors_event_t **& accel_ptr,
      sensors_event_t **& gyro_ptr,
      int32_t ** sample_count);

  void set_accel_fd(const int & fd) { accel_fd_ = fd; };
  void set_gyro_fd(const int & fd) { gyro_fd_ = fd; };

  void set_sample_rate(const int sample_rate) { adjusted_sample_rate_ = sample_rate; };

private:
  /*
   * acquire available sensors nums in ringbuffer.
   * sensor: ringbuffer start address.
   * index: last read position, range: [0, PACK_NUM_MAX-1].
   * last_count: last sensor data count. The newer sensor data count should
   * bigger than it.
   */
  int get_available_sensors_nums(char * sensor, int index, int last_count);

  int get_offset_of_data(char * sensor, int64_t target_timestamp);

  int get_sensors_ptr();

  int accel_fd_ = -1;
  int gyro_fd_ = -1;
  int adjusted_sample_rate_ = -1;
  char * accel_buffer_ptr_ = nullptr;
  char * gyro_buffer_ptr_ = nullptr;
  int count_accel_ = 0;  // accel current read position
  int count_gyro_ = 0;   // gyro current read position
  int accel_gyro_offset_ = -1;
  bool is_accel_gyro_offset_inited_ = false;

  std::string logger_ = "IMUSensor";
};
}  // namespace sensor_client
}  // namespace qrb
#endif