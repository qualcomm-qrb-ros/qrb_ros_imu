/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include "qrb_sensor_client/sensor_client.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace qrb::sensor_client;

int main()
{
  SensorClient sensor_client;
  bool ret = sensor_client.CreateConnection();
  if (!ret) {
    std::cout << "sensor_client_test: CreateConnection failed" << std::endl;
    return -1;
  }
  int num = 0;
  sensors_event_t * accel_ptr;
  sensors_event_t * gyro_ptr;
  sensors_event_t data_accel;
  sensors_event_t data_gyro;

  while (1) {
    bool success = sensor_client.GetImuData(&accel_ptr, &gyro_ptr, &num);
    if (!success) {
      std::this_thread::sleep_for(std::chrono::microseconds(5 * 1000));
      continue;
    }
    for (int i = 0; i < num; i++) {
      data_accel = *accel_ptr;
      data_gyro = *gyro_ptr;
      accel_ptr += 1;
      gyro_ptr += 1;
      std::cout << "[sensor_client_test]accel time: " << data_accel.timestamp
                << " accel reserved0: " << data_accel.reserved0
                << " gyro time: " << data_gyro.timestamp
                << " gyro reserved0: " << data_gyro.reserved0 << std::endl;
      if (data_accel.timestamp == 0 || data_gyro.timestamp == 0) {
        break;
      }
    }
  }
  return 0;
}
