
# -*- coding: utf8 -*-

import serial
import time
from threading import Thread
from optparse import OptionParser

CNC = serial.Serial(port='COM4', baudrate="9600",
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS, timeout=0)

okCount = 0;

#도미노, 나사구명.
#박스조인트

def waitLoop(okSize):
    waitCount = 0
 #   print("wait in okCount=%d okSize=%d" % ( okCount, okSize))
    while ( okCount != okSize ):
        time.sleep(0.1)
        waitCount += 1
#    print("wait out okCount=%d okSize=%d waitCount=%d" % ( okCount, okSize, waitCount))

def sendCmd(msg):
    print("CMD: "+msg)
    msg =  msg+" \n"
    CNC.write(msg.encode())
    waitLoop(okCount+1)
    
def init():
    print("init")
    time.sleep(3)
    if CNC.isOpen():
        print("opened")
        CNC.write('?\n'.encode())
        waitLoop(okCount+1)
        sendCmd('?')
        sendCmd('$x')
        drill()
        sendCmd('G0 X0 Y0 Z0 ' )
        for y in range(0,21,2):
            drill()
            move(y)
    CNC.close()

def drill():
    sendCmd('G1 Z-10 F200' )
    sendCmd('G0 Z0' )
    
def move (y):
    sendCmd('G0 Y'+str(y))
        
def readThread(ser):
    global okCount 
    global rxloop
    line = []
    while True:
        try:
            for c in CNC.read():
    #             print("Rx.C= " + chr(c))
    #             print("Rx.C= %d" %   c  )
                line.append(chr(c))
                if c == 10:
                    msg = str(''.join(line))
                    if ( msg.find("ok") >= 0):
    #                    print("RX2:"+msg)
                        print("--------------------------------------------------------------> ok")
                        okCount += 1
                        line = []
        except:
            break
               
    print("Serial Port Receiver End...")      

    
def main():
    thread = Thread(target=readThread, args=(CNC,))
    thread.start()
    print("start")
    init()
 


if __name__ == "__main__":
    main()

