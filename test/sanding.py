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
    parser = OptionParser()
    parser.add_option('-x', '--x', type='int', dest='x', help='x mm length', default=0)
    parser.add_option('-y', '--y', type='int', dest='y', help='y mm length', default=0)
    parser.add_option('-z', '--z', type='int', dest='z', help='z mm length', default=0)
    parser.add_option('-p', '--port', dest='port', help='serial port', default='COM3')
    parser.add_option('-b', '--baud', type='int', dest='baud', help='baud rate', default=9600)
    parser.add_option('-s', '--sand', action='store_true', dest='s', help='sand second')    
    parser.add_option('-g', '--gallery', action='store_true', dest='g', help='gallery hole')    
    parser.add_option('-f', '--file', action='store_true', dest='f', help='gcode file name')    
    parser.add_option('-t', '--test', action='store_true', dest='t', help='test')    
    options, args = parser.parse_args()
    
    cnc = Cnc()
    if options.s:
        cnc = Sander(options, args)
    elif options.g:
        cnc = Gallery()
    elif options.f:
        cnc = GReader(options, args)
    elif options.t:
        cnc = Test(options, args)
    cnc.move(options.x, options.y, options.z)

if __name__ == "__main__":
    main()
