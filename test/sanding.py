import serial
import time
from threading import Thread
from optparse import OptionParser

class Sander:
    def __init__(self, port='COM3', baud=9600):
        self.ser = serial.Serial(port=port, baudrate=baud,
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS, timeout=0)
        time.sleep(1.5)
        if self.ser.isOpen():
            print("COM OPENED")
            self.ser.write('$x\n'.encode()) 
        
    def write(self, msg):
        self.ser.write(msg.encode())
        print(">>>"+msg)
        output = self.ser.readline();
        print("<<<"+output)
    
    def move(self, x=0, y=0, z=0, sleep=5):
        if x:
            msg = 'G0 Z%d\n' % (x)
            self.write(msg)
        if y:
            msg = 'G0 Y%d\n' % (y)
            self.write(msg)
        if z:
            msg = 'G0 X%d\n' % (z)
            self.write(msg)
        time.sleep(sleep)
    
    def go(self, x=0, y=0, z=0, xstep=10, ystep=100):
        for v in range(0, y, ystep):
            for u in range(0, x, xstep):
                self.move(x=u)
            for u in range(x, 0-xstep, xstep*-1):
                self.move(x=u)
            self.move(y=v)

def main():
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='COM3')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=9600)
    parser.add_option('-s', '--speed', type='int', dest='s', help='speed', default=4)    
    options, args = parser.parse_args()
    
    cnc = Sander(options.port, options.baud)
    cnc.go(options.x, options.y, options.z)
    

if __name__ == "__main__":
    main()
