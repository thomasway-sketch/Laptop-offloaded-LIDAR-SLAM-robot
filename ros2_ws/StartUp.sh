sudo socat -d -d pty,raw,echo=0,link=/dev/ttyLIDAR udp-datagram:192.168.4.211:8890
sleep 1
sudo chmod 666 /dev/ttyLIDAR
ros2 launch robot_bringup robot.launch.py
ros2 launch robot_bringup rplidar.launch.py