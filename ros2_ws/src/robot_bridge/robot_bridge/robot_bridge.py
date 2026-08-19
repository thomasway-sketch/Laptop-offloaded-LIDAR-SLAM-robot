import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped
import socket


class RobotBridge(Node):

    def __init__(self):
        super().__init__('robot_bridge')
        self.subscription = self.create_subscription(
            TwistStamped,
            'cmd_vel',
            self.bridge_publish,
            10)
        self.subscription  # prevent unused variable warning
        self.UDP_IP = "192.168.4.211"
        self.UDP_PORT = 8889
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    def bridge_publish(self, msg):
        data = f"{msg.twist.linear.x}, {msg.twist.angular.z}"
        self.sock.sendto(data.encode(), (self.UDP_IP, self.UDP_PORT))

        



def main(args=None):
    rclpy.init(args=args)

    robot_bridge = RobotBridge()

    rclpy.spin(robot_bridge)

    robot_bridge.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()