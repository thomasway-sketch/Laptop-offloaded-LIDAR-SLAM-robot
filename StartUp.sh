trap 'kill $(jobs -p)' EXIT
socat pty,raw,echo=0,link=$HOME/ttyLIDAR udp-datagram:192.168.4.211:8890,bind=:8890 &
sleep 1
ros2 launch robot_bringup robot.launch.py &
ros2 launch robot_bringup rplidar.launch.py
