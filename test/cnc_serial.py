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

def sendCmd(s):
    ser.write((s+'\n').encode())
y=0
for x in range(100,300, 10):
    sendCmd('G0 X0 Y0 F2')
    t = str(x)
    sendCmd('G0 X0 Y'+t+' F2')
    
    


   
#time.sleep(3)
ser.close()  


