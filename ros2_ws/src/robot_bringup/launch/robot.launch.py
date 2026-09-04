from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='robot_bridge',
            executable='robot_bridge',
            name='bridge',
        ),
        Node(
            package='odom_pub',
            executable='odom_pub',
            name='odom',
        ),
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='laser',
            arguments=['--x', '0', '--y', '0', '--z', '0.1',
           '--frame-id', 'base_link', '--child-frame-id', 'laser']
        )
    ])