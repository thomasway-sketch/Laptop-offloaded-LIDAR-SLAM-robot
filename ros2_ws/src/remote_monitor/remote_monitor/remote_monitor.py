import rclpy
from rclpy.node import Node
import socket


class RemoteMonitor(Node):

    def __init__(self):
        super().__init__('remote_monitor')
        self.UDP_IP = "192.168.4.51"
        self.UDP_PORT = 8887
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.UDP_IP, self.UDP_PORT))

        

def main(args=None):
    rclpy.init(args=args)

    remote_monitor = RemoteMonitor()

    while True:
      data, addr = remote_monitor.sock.recvfrom(1024)
      print(data.decode(), flush=True)


    remote_monitor.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()