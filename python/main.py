import ntplib
import os
from time import ctime
import datetime
from contextlib import closing
from socket import socket, AF_INET, SOCK_DGRAM, error
import struct
import time

def ntp_client(host = "ua.pool.ntp.org"):
    try:
        server = ntplib.NTPClient()
        response = server.request(host, version=3)
        now = datetime.datetime.now()
        offset = datetime.timedelta(seconds = response.offset)
        correct_time = now + offset
        os.system("sudo date -s '%s'" % correct_time)
        print("Offset:", response.offset)
    except ntplib.NTPException:
        print("ntplib timeout raised and caught.")
    

def ntp_socket(host="ua.pool.ntp.org", port=123):
    NTP_PACKET_FORMAT = "!12I"
    NTP_QUERY = b'\x1b' + 47 * b'\0'  
    with closing(socket(AF_INET, SOCK_DGRAM)) as s:
        s.settimeout(5.0)
        s.sendto(NTP_QUERY, (host, port))
        data, address = s.recvfrom(1024)
        print("add", address)
    t = struct.unpack(NTP_PACKET_FORMAT, data)[10]
    t -= 2208988800 # 1970-01-01 00:00:00
    os.system("sudo date -s '%s'" % time.ctime(t))
    
    

if __name__ == "__main__":
    ntp_client()
    ntp_socket()

   


