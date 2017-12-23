import serial
import time
from threading import Thread
from optparse import OptionParser

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
            self.ser.write('$x\n'.encode())
            self.ser.readlines();
        
    def write(self, msg):
        if not self.ser:
            self.open()
        self.ser.write(msg.encode())
        print(">>>"+msg)
        output = self.ser.readline();
        self.ser.write("?".encode())
        output = self.ser.readline();
        print("<<<"+str(output))
    
    def move(self, x=None, y=None, z=None, stay=0):
        print('cnc:move x=%s y=%s z=%s stay=%d' %(x, y, z, stay))
        msg = 'G0 '
        if x != None:
            msg += 'Z%d ' % (x*-1)
        if y is not None:
            msg += 'Y%d ' % (y)
        if z is not None:
            msg += 'X%d ' % (z)
        msg += '\n'
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
            if line.startswith('M'):
                if '30' in line: #down
                    super().move(z=-10)
                if '50' in line: #up
                    super().move(z=0)
                continue
            if line.startswith('G4'):
                continue
            line = line.replace('X-', 'Z')
            super().write(line)
            time.sleep(0.2)
    
def main():
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='COM3')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=9600)
    parser.add_option('-s', '--sand', action='store_true', dest='s', help='sand second')    
    parser.add_option('-g', '--gallery', action='store_true', dest='g', help='gallery hole')    
    parser.add_option('-f', '--file', action='store_true', dest='f', help='gcode file name')    
    options, args = parser.parse_args()
    
    cnc = Cnc()
    if options.s:
        cnc = Sander(options, args)
    elif options.g:
        cnc = Gallery()
    elif options.f:
        cnc = GReader(options, args)
    cnc.move(options.x, options.y, options.z)

if __name__ == "__main__":
    main()
