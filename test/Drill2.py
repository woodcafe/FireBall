
# -*- coding: utf8 -*-

import serial
import time
from threading import Thread
from optparse import OptionParser

CNC = serial.Serial(port='COM3', baudrate="9600",
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
        print("**** opened *****")
        sendCmd('?')
        sendCmd('$x')
        sendCmd('Z-10')
#         drillBox(5,4,2,2) #2,18
#         drillBox(5,12,2,2) #6,20
#         for y in range(0,201,5):
#              drill()
#              move(y)
# #         moveDrill(0)
        
    CNC.close()
    
def drillBox2(width, height): 
    for z in range(-1,-10,-2):
        drillMove(0,height,z)   
        drillMove(width,height,z)
        drillMove(width,0,z)
        drillMove(0,0,z)
    sendCmd('G0 Z0' )
    
def drillMove(x,y,z):
    sendCmd('G1 Z'+str(z)+' F200' )
    sendCmd('G1 X'+str(x)+' Y'+str(y)+" F200")
    
# ////////////////////////////////////////////////////
   
def drillBox(px, py, width, height):
    drillLineY(px, py, py+height)
    drillLineX(py+height, px, px+width )
    drillLineY(px+width, py+height, py)
    drillLineX(py , px+width, px )

def drillLineY(px, y0, y1):
    gap = 2
    if ( y0 > y1 ):
        gap *= -1;
    for y in range(y0, y1, gap):
        sendCmd('G0 X'+str(px)+' Y'+str(y))
        drill()
        
def drillLineX(py, x0, x1 ):
    gap = 2
    if ( x0 > x1 ):
        gap *= -1;
    for x in range(x0, x1, gap):
        sendCmd('G0 X'+str(x)+ 'Y'+str(py))
        drill()

def drill():
    sendCmd('G1 Z-12 F300' )
    sendCmd('G0 Z0' )
    
def move (y):
    sendCmd('G0 Y'+str(y))
    
def moveDrill(y):
    sendCmd('G1 Z-10 F100' )
    sendCmd('G1 Y'+str(y)+" F100")
    sendCmd('G0 Z0' )
        
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

