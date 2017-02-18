import socket
import fcntl
import struct

def getIpAddress(ssid):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('www.google.com',0))

    print(s.getsockname()[0])
    s.close()
    
getIpAddress('JF2')
