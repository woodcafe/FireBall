
# -*- coding: utf8 -*-

import serial
import time
from threading import Thread
from optparse import OptionParser

CNC = serial.Serial(port='COM3', baudrate="9600",
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS, timeout=0)

def sendG(msg):
    msg2 = 'G0 '+msg+" \n"
    print("CODE :::::::: "+msg2)
    CNC.write(msg2.encode())
    
def init():
    print("init")
    time.sleep(3)
    if CNC.isOpen():
        print("opened")
        CNC.write('?\n\r'.encode())
        CNC.write('$x\n'.encode())
        for y in range(2,20,2):
            move(y)
            drill()
            time.sleep(3)
        sendG("Z-10")
        sendG("Y0")
        move(0)
        sendG('X0 Y0 Z0 \n' )
        print("end")
        time.sleep(20)
        CNC.close()
        
def drill():
    CNC.write('G0 Z-10\n'.encode())
    CNC.write('G0 Z0\n'.encode())
    
def move (y):
    print("MOVE ....")
    sendG('Z0')
    sendG('Y'+str(y))
  
        
def readThread(ser):
    line = []
    print("readThrea run")
    while CNC.isOpen():
        c =  CNC.readline()
        if ( len(c) > 0 ):
            print(str(c))
           #     del line[:]            
    print("readThrea end")      

class Cnc(object):
    def __init__(self, port='COM3', baud=9600):
        self.port = port
        self.baud = baud
        self.ser = None
        
    def open(self):
        self.ser = serial.Serial(port=self.port, baudrate=self.baud,
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS, timeout=0)
        time.sleep(1.5)
        if self.ser.isOpen():
            print("COM OPENED")
            self.read()
            self.ser.write('$x\n'.encode())
            self.read()
        
    def read(self, ok=False):
        time.sleep(0.1)
        while(True):
            output = self.ser.readline().decode().strip()
            if not output:
                break
            if ok and output != 'ok':
                time.sleep(0.5)
                continue
            print("<<<"+output)
        
    def write(self, msg):
        if not self.ser:
            self.open()
        self.ser.write((msg+'\n').encode())
        print(">>>"+msg)
        self.read(True)
        self.ser.write("?".encode())
        self.read(True)
    
    def move(self, x=None, y=None, z=None, stay=0):
        print('cnc:move x=%s y=%s z=%s stay=%d' %(x, y, z, stay))
        msg = 'G0 '
        if x != None:
            msg += 'X%d ' % (x)
        if y is not None:
            msg += 'Y%d ' % (y)
        if z is not None:
            msg += 'Z%d ' % (z)
        self.write(msg)
        time.sleep(stay)

class Sander(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self)
        self.stay = 5
        if args:
            self.stay = int(args[0])
            
    def move(self, x=0, y=0, z=0, xstep=10, ystep=50):
        print('Sander:move x=%d y=%d z=%d xs=%d ys=%d' %(x, y, z, xstep, ystep))
        for v in range(0, y+1, ystep):
            print("v=%d"%v)
            super().move(y=v)
            for u in range(0, x, xstep):
                super().move(x=u, stay=self.stay)
            for u in range(x, 0, xstep*-1):
                super().move(x=u, stay=self.stay)

class Gallery(Cnc):
    def move(self, x=0, y=0, z=0):
        xx = 30
        yy = 40
        print('Gallery:move x=%s y=%s z=%s' %(x, y, z))
        for i in range(int(y/yy)):
            super().move(xx, yy*(i+1))
            super().move(z=10)
            super().move(0)
            super().move(z=0)
            
class GReader(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self)
        if args:
            self.fname = args[0]
            
    def move(self, x=0, y=0, z=0):
        f = open(self.fname)
        for line in f:
            super().write(line)

class Test(Cnc):
    def __init__(self, opts, args):
        Cnc.__init__(self)
            
    def move(self, x=0, y=0, z=0):
        for i in range(100):
            super().move(x, y, z)
            time.sleep(5)
            super().move(x*-1, y*-1, z*-1)
            time.sleep(5)
    
def main():
    thread = Thread(target=readThread, args=(CNC,))
    thread.start()
    print("start")
    init()
 


if __name__ == "__main__":
    main()

