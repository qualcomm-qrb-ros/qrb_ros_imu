/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qrb_sensor_client/sensor_client.hpp"

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

#define PACK_NUM_MAX (1000)  // ring buffer len

#ifndef DIRECT_CHANNEL_SHARED_MEMORY_SIZE
#define DIRECT_CHANNEL_SHARED_MEMORY_SIZE (PACK_NUM_MAX * sizeof(sensors_event_t))
#endif

namespace qrb
{
namespace sensor_client
{
int count_accel = 0;  // accel current read position
int count_gyro = 0;   // gyro current read position
int accel_gyro_offset = -1;
bool is_accel_gyro_offset_inited = false;
std::vector<int> sensors_fd;
std::vector<char *> sensors_buffer_ptr;
int msg_socket[1] = { -1 };
int original_sample_rate = 240;
int adjusted_sample_rate = 240;

int get_sensors_fd(int socket_fd, int fd_number)
{
  msghdr msg;
  cmsghdr * cm;
  int * fds = (int *)malloc(fd_number * sizeof(int));
  char iov_buf[1];
  char buf[100];
  memset(buf, '\0', sizeof(buf));

  iovec iov[1];
  iov[0].iov_base = iov_buf;
  iov[0].iov_len = 1;

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_name = nullptr;
  msg.msg_namelen = 0;

  msg.msg_control = &buf;
  msg.msg_controllen = sizeof(buf);

  if (recvmsg(socket_fd, &msg, 0) < 0) {
    std::cout << "recvmsg failed " << std::endl;
    return -1;
  }

  cm = CMSG_FIRSTHDR(&msg);

  memcpy(fds, (int *)CMSG_DATA(cm), fd_number * sizeof(int));
  for (int i = 0; i < fd_number; i++) {
    sensors_fd.emplace_back(fds[i]);
  }
  free(fds);
  return 0;
}

int get_sensors_ptr(int client_fd)
{
  int ret = get_sensors_fd(client_fd, 2);
  if (ret < 0) {
    return -1;
  }

  for (int i = 0; i < (int)sensors_fd.size(); i++) {
    char * temp = (char *)(void *)mmap(
        NULL, DIRECT_CHANNEL_SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, sensors_fd[i], 0);
    if (temp == nullptr) {
      std::cout << "sensors_buffer_ptr is nullptr!" << std::endl;
      return -1;
    }
    sensors_buffer_ptr.emplace_back(temp);
  }
  return 0;
}

bool SensorClient::CreateConnection()
{
  _client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (_client_fd < 0) {
    std::cout << "sensor client: create socket failed" << std::endl;
    return false;
  }

  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, SOCKET_PATH);

  int ret = connect(_client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ret < 0) {
    std::cout << "please check if the sensor service 's config is right." << std::endl;
    std::cout << "or please check if the sensor service is running." << std::endl;
    close(_client_fd);
    _client_fd = 0;
    return false;
  }
  int msg[2] = { -1, -1 };
  int len = recv(_client_fd, &msg, sizeof(msg), 0);
  if (len <= 0) {
    std::cout << "sensor client: recv sample_rate failed " << std::endl;
  }
  std::cout << "sensor client recv from msg. User set sample_rate: " << msg[0]
            << " adjusted sample_rate: " << msg[1] << " len: " << len << std::endl;
  original_sample_rate = msg[0];
  adjusted_sample_rate = msg[1];

  ret = get_sensors_ptr(_client_fd);
  if (ret < 0) {
    std::cout << "sensor client: get_sensors_ptr failed " << std::endl;
    DisconnectServer();
    return false;
  }
  return true;
}

/*
 * acquire available sensors nums in ringbuffer.
 * sensor: ringbuffer start address.
 * index: last read position, range: [0, PACK_NUM_MAX-1].
 * last_count: last sensor data count. The newer sensor data count should bigger than it.
 */
int get_available_sensors_nums(char * sensor, int index, int last_count)
{
  int count = 0;
  // if index + 1 == PACK_NUM_MAX means come to the buffer bottom, need reset it to the buffer top.
  if (index + 1 == PACK_NUM_MAX) {
    index = -1;
  }

  while (index + 1 < PACK_NUM_MAX) {
    sensors_event_t * next_data = (sensors_event_t *)sensor + index + 1;
    if (next_data->reserved0 == 0 || next_data->reserved0 <= last_count) {
      break;
    }
    count++;
    index++;
  }
  return count;
}

int get_offset_of_data(char * sensor, int64_t target_timestamp)
{
  int index = 0;
  long ts_diff =
      1000000000L / adjusted_sample_rate / 2;  // 1s = 1000000000ns, rise and fall 1s / rate / 2
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

/*
 * Two steps:
 * 1.Get the offset between accel and gyro.
 * 2.Read ring buffer accle and gyro, find latest sensor data return available nums and start
 * pointer.
 *
 *                     accel                           gyro
 *                  *---------*                    *---------*
 *                 0| sensor1 |                   0| sensor1 |
 *                  *---------*                    *---------*
 * count_accel---> 1|         |                 ...|         |
 *                  *---------*       ---->        *---------*
 *               ...|         |    offset = 2     3|         | <---count_gyro
 *                  *---------*                    *---------*
 *               999|         |                 999|         |
 *                  *---------*                    *---------*
 */
bool SensorClient::GetImuData(sensors_event_t ** accel_ptr,
    sensors_event_t ** gyro_ptr,
    int32_t * sample_count)
{
  static int OFFSET =
      0;  // if found accel_gyro_offset, after each read need to move pointer to next.
  sensors_event_t * data_accel = (sensors_event_t *)sensors_buffer_ptr[0] + count_accel;
  sensors_event_t * data_gyro = (sensors_event_t *)sensors_buffer_ptr[1] + count_gyro;

  if (!is_accel_gyro_offset_inited) {
    int gyro_offset = get_offset_of_data(sensors_buffer_ptr[1], data_accel->timestamp);
    int accel_offset = get_offset_of_data(sensors_buffer_ptr[0], data_gyro->timestamp);

    if (gyro_offset == -1 && accel_offset == -1) {
      // not found.
      return false;
    }
    if (gyro_offset >= 0) {
      count_gyro = (count_gyro + gyro_offset) % PACK_NUM_MAX;
      accel_gyro_offset = gyro_offset;
    } else {
      count_accel = (count_accel + accel_offset) % PACK_NUM_MAX;
      accel_gyro_offset = 0 - accel_offset;
    }
    *sample_count = 1;
    is_accel_gyro_offset_inited = true;
  } else {
    OFFSET = 1;  // find the offset between accel and gyro
    int accel_available_nums =
        get_available_sensors_nums(sensors_buffer_ptr[0], count_accel, data_accel->reserved0);
    if (accel_available_nums <= 0) {
      return false;
    }
    int gyro_available_nums =
        get_available_sensors_nums(sensors_buffer_ptr[1], count_gyro, data_gyro->reserved0);
    if (gyro_available_nums <= 0) {
      return false;
    }
    *sample_count =
        accel_available_nums < gyro_available_nums ? accel_available_nums : gyro_available_nums;
  }
  // update return value
  *accel_ptr = (sensors_event_t *)sensors_buffer_ptr[0] + ((count_accel + OFFSET) % PACK_NUM_MAX);
  *gyro_ptr = (sensors_event_t *)sensors_buffer_ptr[1] + ((count_gyro + OFFSET) % PACK_NUM_MAX);
  // update current value
  count_accel = (count_accel + *sample_count) % PACK_NUM_MAX;
  count_gyro = (count_accel + accel_gyro_offset + PACK_NUM_MAX) % PACK_NUM_MAX;
  return true;
}

void end_connect(int client_fd)
{
  msg_socket[0] = STOP;
  int send_len = send(client_fd, &msg_socket, sizeof(msg_socket), 0);
  if (send_len < 0) {
    std::cout << "sensor client: socket send failed" << std::endl;
  }
  close(client_fd);
}

void SensorClient::DisconnectServer()
{
  end_connect(_client_fd);
  shutdown(_client_fd, SHUT_RDWR);
  _client_fd = 0;
}

int SensorClient::ReadMsg(char * buffer, int len)
{
  if (_client_fd < 0)
    return -1;
  return read(_client_fd, buffer, len);
}

}  // namespace sensor_client
}  // namespace qrb
