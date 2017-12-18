# -*- coding: utf8 -*-

 
from socket import *
from select import select
import sys
import time


clientSocket = socket(AF_INET, SOCK_STREAM)
ADDR = ("172.30.1.5",80)
try:
    clientSocket.connect(ADDR)
except Exception as e:
    print("can't open %s %s" % ADDR)
    sys.exit()
print("ok %s %s" % ADDR)

for i in range(1,100000):
    print(" =======> %d " % i)
    msg = "G1 X11.111232 Y100.224455 Z"+str(i)+".9366644 END"
    clientSocket.send(str.encode(msg));
    print("send %s %s" % ADDR)
    while(1):
        data = clientSocket.recv(512)
        if ( len(data) > 0 ):
            print(data)
            break
        else :
            time.sleep(0.1)