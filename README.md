# QRB ROS IMU

## Overview

`qrb_ros_imu` is a package to publish the imu data from sensor service.
- This package uses Qualcomm Sensor See framework to get the latest imu data with high performance.
- The IMU data is widely used in robot localization, such as SLAM(Simultaneous localization and mapping).

## Quick Start

> **Note：**
> This document 's build & run is the latest.
> If it conflict with the online document, please follow this.

We provide two ways to use this package.

<details>
<summary>Docker</summary>

#### Setup
1. Please follow this [steps](https://github.com/qualcomm-qrb-ros/qrb_ros_docker?tab=readme-ov-file#quickstart) to setup docker env.
2. Download qrb_ros_imu and dependencies
    ```bash
    cd ${QRB_ROS_WS}/src

    git clone https://github.com/qualcomm-qrb-ros/lib_mem_dmabuf.git
    git clone https://github.com/qualcomm-qrb-ros/qrb_ros_imu.git
    git clone https://github.com/qualcomm-qrb-ros/qrb_ros_transport.git
    ```

#### Build
```bash
colcon build
```

#### Run
```bash
cd ${QRB_ROS_WS}/src

source install/local_setup.sh
ros2 run qrb_ros_imu imu_node
```

</details>
 

<details>
<summary>QIRP-SDK</summary>

#### Setup
1. Please follow this [steps](https://qualcomm-qrb-ros.github.io/getting_started/index.html) to setup qirp-sdk env.
2. Download qrb_ros_imu and dependencies
    ```bash
    mkdir -p <qirp_decompressed_workspace>/qirp-sdk/ros_ws
    cd <qirp_decompressed_workspace>/qirp-sdk/ros_ws

    git clone https://github.com/qualcomm-qrb-ros/qrb_ros_imu.git
    ```

#### Build
1. Build the project
    ```bash
    export AMENT_PREFIX_PATH="${OECORE_NATIVE_SYSROOT}/usr:${OECORE_TARGET_SYSROOT}/usr"
    export PYTHONPATH=${OECORE_NATIVE_SYSROOT}/usr/lib/python3.12/site-packages/:${OECORE_TARGET_SYSROOT}/usr/lib/python3.12/site-packages/

    colcon build --continue-on-error --cmake-args \
      -DCMAKE_TOOLCHAIN_FILE=${OE_CMAKE_TOOLCHAIN_FILE} \
      -DPYTHON_EXECUTABLE=${OECORE_NATIVE_SYSROOT}/usr/bin/python3 \
      -DPython3_NumPy_INCLUDE_DIR=${OECORE_NATIVE_SYSROOT}/usr/lib/python3.12/site-packages/numpy/core/include \
      -DCMAKE_MAKE_PROGRAM=/usr/bin/make \
      -DBUILD_TESTING=OFF
    ```
2. Install the package
    ```bash
    cd <qirp_decompressed_workspace>/qirp-sdk/ros_ws/install/qrb_ros_imu
    tar -czvf qrb_ros_imu.tar.gz include lib share
    scp qrb_ros_imu.tar.gz root@[ip-addr]:/home/
    cd <qirp_decompressed_workspace>/qirp-sdk/ros_ws/install/qrb_sensor_client
    tar -czvf qrb_sensor_client.tar.gz include lib share
    scp qrb_sensor_client.tar.gz root@[ip-addr]:/home/
    ssh root@[ip-addr]
    (ssh) mount -o remount rw /
    (ssh) tar --no-overwrite-dir --no-same-owner -zxf /home/qrb_ros_imu.tar.gz -C /usr/
    (ssh) tar --no-overwrite-dir --no-same-owner -zxf /home/qrb_sensor_client.tar.gz -C /usr/
    ```

#### Run
```bash
(ssh) export HOME=/home
(ssh) setenforce 0
(ssh) source /usr/bin/ros_setup.sh && source /usr/share/qirp-setup.sh
(ssh) ros2 run qrb_ros_imu imu_node
```

</details>

<br>

You can get more details from [here](https://qualcomm-qrb-ros.github.io/main/index.html).
## Contributing

We would love to have you as a part of the QRB ROS community. Whether you are helping us fix bugs, proposing new features, improving our documentation, or spreading the word, please refer to our [contribution guidelines](./CONTRIBUTING.md) and [code of conduct](./CODE_OF_CONDUCT.md).

- Bug report: If you see an error message or encounter failures, please create a [bug report](../../issues)
- Feature Request: If you have an idea or if there is a capability that is missing and would make development easier and more robust, please submit a [feature request](../../issues)


## Authors

* **Zhanye Lin** - *Initial work* - [zhanlin](https://github.com/quic-zhanlin)

See also the list of [contributors](https://github.com/qualcomm-qrb-ros/qrb_ros_imu/graphs/contributors) who participated in this project.


## License

Project is licensed under the [BSD-3-clause License](https://spdx.org/licenses/BSD-3-Clause.html). See [LICENSE](./LICENSE) for the full license text.

