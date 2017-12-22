import serial
import time
from threading import Thread


ser = serial.Serial(
    port='COM3',\
    baudrate=9600,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
        timeout=0)

print(ser.portstr)


def receiving(ser):
  buffer = "xx"
  while True:
    if ( ser.inWaiting()):
        c = ser.readline();
        print(c)
    time.sleep(0.5)
    
#Thread(target=receiving, args=(ser,)).start()
time.sleep(1.5)

if ser.isOpen():
    print("COM OPENED")
    ser.write('$x\n'.encode()) 
#    ser.flush() 

def sendCmd(x,y,z):
    msg = 'G0 X'+str(z)+' Y'+str(y)+' Z'+str(x)
    ser.write((msg+' F8\n').encode())

def sendCmd2(s):
    ser.write((s+'\n').encode())
    
def draw1():    
    for x in range(100,300, 10):
       sendCmd(0,0,0)
       sendCmd(x,300-x,-3);

def draw():    
    for x in range(1,330,50):
       sendCmd(x,0,0)     
       sendCmd(x,550,0);
    
draw()   


   
#time.sleep(3)
ser.close()  


