/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qrb_sensor_client/sensor_client.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace qrb
{
namespace sensor_client
{
int SensorClient::get_sensors_fd(int fd_number, std::vector<int> & sensors_fd)
{
  msghdr msg;
  cmsghdr * cm;
  int * fds = (int *)malloc(fd_number * sizeof(int));

  char buf[2];
  char rec_buf[CMSG_SPACE(fd_number * sizeof(int))];

  iovec iov[1];
  iov[0].iov_base = buf;
  iov[0].iov_len = 2;

  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_name = nullptr;
  msg.msg_namelen = 0;

  msg.msg_control = &rec_buf;
  msg.msg_controllen = sizeof(rec_buf);
  if (recvmsg(_client_fd, &msg, 0) < 0) {
    perror("recvmsg failed");
    return -1;
  }

  std::cout << "[INFO] [" << logger_ << "]: recvmsg success " << std::endl;
  cm = CMSG_FIRSTHDR(&msg);

  memcpy(fds, (int *)CMSG_DATA(cm), fd_number * sizeof(int));
  for (int i = 0; i < fd_number; i++) {
    sensors_fd.emplace_back(fds[i]);
  }
  free(fds);
  return 0;
}

bool SensorClient::create_connection()
{
  _client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (_client_fd < 0) {
    std::cout << "[ERROR] [" << logger_ << "]: sensor client: create socket failed" << std::endl;
    return false;
  }

  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  strlcpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path));

  int ret = connect(_client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ret < 0) {
    std::cout << "[ERROR] [" << logger_
              << "]: please check if the sensor service 's config is right." << std::endl;
    std::cout << "[ERROR] [" << logger_ << "]: or please check if the sensor service is running."
              << std::endl;
    close(_client_fd);
    _client_fd = 0;
    return false;
  }
  return true;
}

bool SensorClient::get_config_data(SensorType type)
{
  int sensor_type;
  int fd_number;

  if (type == SensorType::IMU_TYPE) {
    sensor_type = IMU;
    fd_number = 2;
  } else {
    return false;
  }

  // send get config request
  int requets_msg[2] = { GETCONFIG, sensor_type };
  int send_len = send(_client_fd, &requets_msg, sizeof(requets_msg), 0);
  if (send_len <= 0) {
    std::cout << "[ERROR] [" << logger_ << "]: send get config failed " << std::endl;
  }

  // receive the config
  int rec_msg[2] = { -1, -1 };
  int len = recv(_client_fd, &rec_msg, sizeof(rec_msg), 0);
  if (len <= 0) {
    std::cout << "[ERROR] [" << logger_ << "]: recv sample_rate failed " << std::endl;
    return false;
  }

  std::cout << "[INFO] [" << logger_
            << "]: sensor client recv from msg. User set sample_rate: " << rec_msg[0]
            << " adjusted sample_rate: " << rec_msg[1] << " len: " << len << std::endl;

  sensor.set_sample_rate(rec_msg[1]);

  // send start request
  requets_msg[0] = START;
  send_len = send(_client_fd, &requets_msg, sizeof(requets_msg), 0);
  if (send_len <= 0) {
    std::cout << "[ERROR] [" << logger_ << "]: send get config failed " << std::endl;
    return false;
  }

  std::cout << "[INFO] [" << logger_ << "]: send get config success " << std::endl;
  std::vector<int> sensors_fd;
  auto ret = get_sensors_fd(fd_number, sensors_fd);
  if (ret < 0) {
    return false;
  }

  if (type == SensorType::IMU_TYPE) {
    sensor.set_accel_fd(sensors_fd[0]);
    sensor.set_gyro_fd(sensors_fd[1]);
    sensor_statue[IMU] = true;
  } else {
    return false;
  }

  return true;
}

bool SensorClient::get_imu_data(sensors_event_t ** accel_ptr,
    sensors_event_t ** gyro_ptr,
    int32_t * sample_count)
{
  mtx_.lock();
  if (!sensor_statue[IMU]) {
    bool ret = get_config_data(SensorType::IMU_TYPE);
    if (!ret) {
      std::cout << "[ERROR] [" << logger_ << "]: get config failed" << std::endl;
      return false;
    }
  }
  bool ret = sensor.get_data(accel_ptr, gyro_ptr, &sample_count);
  mtx_.unlock();
  return ret;
}

void end_connect(int client_fd)
{
  int requets_msg[2] = { STOP, SENSOR_SIZE };
  int send_len = send(client_fd, &requets_msg, sizeof(requets_msg), 0);
  if (send_len < 0) {
    std::cout << "[ERROR] [SensorClient]: socket send failed" << std::endl;
  }
  close(client_fd);
}

void SensorClient::disconnect_server()
{
  end_connect(_client_fd);
  shutdown(_client_fd, SHUT_RDWR);
  _client_fd = 0;
}

}  // namespace sensor_client
}  // namespace qrb
