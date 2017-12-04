


#ifndef WIFI8266_h
#define WIFI8266_h

#include <WString.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
//#include <HardwareSerial.h>

class WIFI8266 {
  public:
    WIFI8266(int rxpin, int txpin);
    void begin();
    void testTerminal();
    void txPDU(String msg, int conId );
    int  rxPDU(char dstBuf[], int size);
    void clear();
    int  copyBuf(char dstBuf[], int size);
  
    void sendData(String cmd, const int timeout, boolean debug);
    void clearEspBuffer();
    

  private:
    int top;
    int capacity;
    int pushBuf( int timeout );
    int indexAfter( int sp, char wd[], int size );
    String substring( int sp, int ep );

};

#endif
