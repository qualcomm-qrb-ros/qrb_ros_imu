# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import launch
from launch_ros.actions import ComposableNodeContainer, Node
from launch_ros.descriptions import ComposableNode

@pytest.mark.rostest
def generate_test_description():
    imu_node = ComposableNode(
        name='imu',
        package='qrb_ros_imu',
        plugin='qrb_ros::imu::ImuComponent'
    )

    container = ComposableNodeContainer(
        name='imu_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[imu_node],
        output='screen',
        arguments=['--ros-args', '--log-level', 'info']
    )
    return (launch.LaunchDescription([container]))