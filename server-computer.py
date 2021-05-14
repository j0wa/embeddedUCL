import socket

udp_ip = "::"
udp_port = "60001"

socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
socket.bind((udp_ip, udp_port))

while True:
    data, addr = socket.recvfrom(1024)
    print("Message :", data, "; from:", addr)

# Once this part of code works, we can add function to communicate with the border router on Contiki.
#   1. Give ID to device
#   2. Get the list of available devices with their states
#   3. Interact with devices