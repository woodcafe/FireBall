import serial
import time
from threading import Thread
from optparse import OptionParser

class Cnc:
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
        print(msg)
    
    def go(self, x=0, y=0, z=0, s=0):
        msg = 'G0 X%d Y%d Z%d F%d\n' % (z, y, x*-1, s)
        self.write(msg)
        time.sleep(1)
        msg = 'G0 X%d Y%d Z%d F%d\n' % (0,0,0, s)
        self.write(msg)
        
    def sendCmd(self, x,y,z):
        msg = 'G0 X'+str(z)+' Y'+str(y)+' Z'+str(x)
        self.ser.write((msg+' F8\n').encode())

def main():
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='COM3')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=9600)
    parser.add_option('-s', '--speed', type='int', dest='s', help='speed', default=4)    
    options, args = parser.parse_args()
    
#    cnc = Cnc()
    cnc = Cnc(options.port, options.baud)
    cnc.go(options.x, options.y, options.z, options.s)
    

if __name__ == "__main__":
    main()
