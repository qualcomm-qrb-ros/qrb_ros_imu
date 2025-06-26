/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include "qrb_sensor_client/imu_sensor.hpp"

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <iostream>

namespace qrb
{
namespace sensor_client
{
int IMUSensor::get_available_sensors_nums(char * sensor, int index, int last_count)
{
  int count = 0;
  // if index == PACK_NUM_MAX - 1 means come to the buffer bottom, need reset it
  // to the buffer top.
  if (index == PACK_NUM_MAX - 1) {
    index = 0;
  }

  while (index < PACK_NUM_MAX) {
    sensors_event_t * next_data = (sensors_event_t *)sensor + index;
    if (next_data->reserved0 == 0 || next_data->reserved0 <= last_count) {
      break;
    }
    count++;
    index++;
  }
  return count;
}

int IMUSensor::get_offset_of_data(char * sensor, int64_t target_timestamp)
{
  if (adjusted_sample_rate_ < 0) {
    std::cout << "[ERROR] [" << logger_ << "]: please set the sample_rate at first!" << std::endl;
    return -1;
  }
  int index = 0;
  long ts_diff =
      1000000000L / adjusted_sample_rate_ / 2;  // 1s = 1000000000ns, rise and fall 1s / rate / 2
  while (index < PACK_NUM_MAX) {
    sensors_event_t * next_data = (sensors_event_t *)sensor + index;
    if (next_data->timestamp == 0 || (next_data->timestamp - ts_diff > target_timestamp)) {
      return -1;
    }
    if (target_timestamp <= ts_diff + next_data->timestamp &&
        target_timestamp > next_data->timestamp - ts_diff) {
      break;
    }
    index++;
  }
  return index;
}

bool IMUSensor::get_data(sensors_event_t **& accel_ptr,
    sensors_event_t **& gyro_ptr,
    int32_t ** sample_count)
{
  if (accel_buffer_ptr_ == nullptr || gyro_buffer_ptr_ == nullptr) {
    int ret = get_sensors_ptr();
    if (ret < 0) {
      std::cout << "[ERROR] [" << logger_ << "]: sensors_buffer_ptr ret < 0!" << std::endl;
      return false;
    }
  }
  sensors_event_t * data_accel = (sensors_event_t *)accel_buffer_ptr_ + count_accel_;
  sensors_event_t * data_gyro = (sensors_event_t *)gyro_buffer_ptr_ + count_gyro_;

  if (!is_accel_gyro_offset_inited_) {
    int gyro_offset = get_offset_of_data(gyro_buffer_ptr_, data_accel->timestamp);
    int accel_offset = get_offset_of_data(accel_buffer_ptr_, data_gyro->timestamp);

    if (gyro_offset == -1 && accel_offset == -1) {
      // not found.
      return false;
    }
    if (gyro_offset >= 0) {
      count_gyro_ = (count_gyro_ + gyro_offset) % PACK_NUM_MAX;
      accel_gyro_offset_ = gyro_offset;
    } else {
      count_accel_ = (count_accel_ + accel_offset) % PACK_NUM_MAX;
      accel_gyro_offset_ = 0 - accel_offset;
    }
    **sample_count = 1;
    is_accel_gyro_offset_inited_ = true;
  } else {
    int accel_available_nums =
        get_available_sensors_nums(accel_buffer_ptr_, count_accel_, data_accel->reserved0);
    if (accel_available_nums <= 0) {
      return false;
    }
    int gyro_available_nums =
        get_available_sensors_nums(gyro_buffer_ptr_, count_gyro_, data_gyro->reserved0);
    if (gyro_available_nums <= 0) {
      return false;
    }
    **sample_count =
        accel_available_nums < gyro_available_nums ? accel_available_nums : gyro_available_nums;
  }
  // update return value
  *accel_ptr = (sensors_event_t *)accel_buffer_ptr_ + (count_accel_ % PACK_NUM_MAX);
  *gyro_ptr = (sensors_event_t *)gyro_buffer_ptr_ + (count_gyro_ % PACK_NUM_MAX);
  // update current value
  count_accel_ = (count_accel_ + **sample_count) % PACK_NUM_MAX;
  count_gyro_ = (count_accel_ + accel_gyro_offset_ + PACK_NUM_MAX) % PACK_NUM_MAX;
  return true;
}

int IMUSensor::get_sensors_ptr()
{
  if (accel_fd_ < 0 || gyro_fd_ < 0) {
    std::cout << "[ERROR] [" << logger_ << "]: please set the fd at first!" << std::endl;
    return -1;
  }

  char * temp = (char *)(void *)mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, accel_fd_, 0);
  if (temp == nullptr) {
    std::cout << "[ERROR] [" << logger_ << "]: sensors_buffer_ptr is nullptr!" << std::endl;
    return -1;
  }
  accel_buffer_ptr_ = temp;

  temp = (char *)(void *)mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, gyro_fd_, 0);
  if (temp == nullptr) {
    std::cout << "[ERROR] [" << logger_ << "]: sensors_buffer_ptr is nullptr!" << std::endl;
    return -1;
  }
  gyro_buffer_ptr_ = temp;

  return 0;
}

}  // namespace sensor_client
}  // namespace qrb