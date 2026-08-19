import math

import rclpy
from rclpy.node import Node
import socket
from tf2_ros.static_transform_broadcaster import TransformBroadcaster
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Quaternion   # if you build the quaternion by hand
from geometry_msgs.msg import TransformStamped


class OdomPub(Node):

    def __init__(self):
        super().__init__('odom_pub')
        self.publisher_ = self.create_publisher(Odometry, 'odom', 10)
        self.UDP_IP = "192.168.4.51"
        self.UDP_PORT = 8886
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.UDP_IP, self.UDP_PORT))
        self.RG_Count_prev = 0
        self.LG_Count_prev = 0
        self.prev_time = self.get_clock().now().nanoseconds * 1e-9 
        self.theta = 0.0
        self.x = 0.0
        self.y = 0.0
        self.delta_time = 0.0
        self.tf_broadcaster = TransformBroadcaster(self)

    def odom_publish(self, linear_vel, angular_vel):
        msg = Odometry()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "odom"
        msg.child_frame_id = "base_link"

        msg.pose.pose.position.x = self.x
        msg.pose.pose.position.y = self.y
        msg.pose.pose.position.z = 0.0    
        qx = 0.0
        qy = 0.0
        qz = math.sin(self.theta / 2.0)
        qw = math.cos(self.theta / 2.0)
        msg.pose.pose.orientation = Quaternion(x=qx, y=qy, z=qz, w=qw)

        msg.twist.twist.linear.x = linear_vel   # v
        msg.twist.twist.angular.z = angular_vel # ω
        self.publisher_.publish(msg)


    def twist_calc(self, LG_Count, RG_Count):
        # Convert the string values to integers
        LG_Count = int(LG_Count)
        RG_Count = int(RG_Count)

        now = (self.get_clock().now().nanoseconds * 1e-9)
        self.delta_time = now - self.prev_time
        true_speed_left = ((LG_Count - self.LG_Count_prev)/408.0)*(math.pi*0.065)/self.delta_time
        true_speed_right = ((RG_Count - self.RG_Count_prev)/408.0)*(math.pi*0.065)/self.delta_time

        linear_vel = (true_speed_left + true_speed_right) / 2.0
        angular_vel = (true_speed_right - linear_vel)/(0.254/2.0)

        self.LG_Count_prev = LG_Count
        self.RG_Count_prev = RG_Count 
        self.prev_time = now
        return linear_vel, angular_vel

    def pose_calc(self, linear_vel, angular_vel, delta_time):
        distance = linear_vel * delta_time
        self.x += distance * math.cos(self.theta)
        self.y += distance * math.sin(self.theta)
        self.theta += angular_vel * delta_time

        if self.theta > math.pi:
            self.theta -= 2 * math.pi
        elif self.theta < -math.pi:
            self.theta += 2 * math.pi

    def transform_pub(self):
        t = TransformStamped()
        t.header.frame_id = "odom"        
        t.child_frame_id = "base_link"    
        t.transform.translation.x = self.x
        t.transform.translation.y = self.y
        t.header.stamp = self.get_clock().now().to_msg()
        qx = 0.0
        qy = 0.0
        qz = math.sin(self.theta / 2.0)
        qw = math.cos(self.theta / 2.0)
        t.transform.rotation = Quaternion(x=qx, y=qy, z=qz, w=qw)
        self.tf_broadcaster.sendTransform(t)
        # computed quaternion twice which may cause problems

        




def main(args=None):
    rclpy.init(args=args)

    odom_pub = OdomPub()
    while True:
        data, addr = odom_pub.sock.recvfrom(1024)
        if data:
            while True:
                data, addr = odom_pub.sock.recvfrom(1024)
                if data:
                    data = data.decode()
                    LG, RG = data.split(',')
                    linear_vel, angular_vel = odom_pub.twist_calc(LG, RG)
                    odom_pub.pose_calc(linear_vel, angular_vel, odom_pub.delta_time)
                    odom_pub.odom_publish(linear_vel, angular_vel)
                    odom_pub.transform_pub()




    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    odom_pub.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()