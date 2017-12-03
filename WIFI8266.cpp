
#include "WIFI8266.h"

#include <HardwareSerial.h>

// AT+RST, AT+GMR :: version
// AT+CWJAP="SSID”,”12345678″
// AT+CIFSR :: show my ip
// AT+CIUPDATE
// AT+CWLAP : AP LIST
// AP+   : MY IP

SoftwareSerial *softSerial;

int top = 0;
int _conId = 0;
char espBuffer[1024];
char dstBuf[1024];

WIFI8266::WIFI8266(int rxpin, int txpin)
{
  pinMode(txpin, OUTPUT);
  pinMode(rxpin, INPUT);
  softSerial  = new SoftwareSerial(rxpin, txpin);
}

void WIFI8266:: begin() {
  Serial.begin(9600);
  Serial.println("ESP8266 WIFI Enabled.");
  softSerial->begin(9600);
  softSerial->println("AT+RST\n\n");
  sendData("AT+RST\r\n", 1000, true);
  sendData("AT+GMR\r\n", 5000, true);
  sendData(F("AT+CIFSR\r\n"), 1000, true);
  sendData(F("AT+CIPMUX=1\r\n"), 1000, true);        // configure for multiple connections
  sendData(F("AT+CIPSERVER=1,80\r\n"), 1000, true);  // turn on server on port 80
}

void WIFI8266::testTerminal() {
  int count = flushEspBuffer(500);
  if ( count > 0 ) {
    Serial.print(espBuffer);
  }
  count = 0;
  while (Serial.available() ) {
    softSerial->write(Serial.read());
    count++;
  }
  if ( count > 0 ) {
    Serial.print("[WIFI] >>");
  }
}



void  _printf(char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  char buf[vsnprintf(NULL, 0, fmt, va) + 1];
  vsprintf(buf, fmt, va);
  Serial.println(buf);
  va_end(va);
}



// #######################################################################
// ## TX FROM EPS
// #######################################################################

int WIFI8266:: pushBuf(int timeout) {
  int oldTop = top;
  long int time = millis();
  while ( (time + timeout) > millis() ) {
    while (softSerial->available()) {
      char c = softSerial->read();  // read the next character
      espBuffer[top++] = c;
    }
  }
  espBuffer[top] = 0;
  return top;
}



void  WIFI8266:: sendData(String command, const int timeout, boolean debug) {
  softSerial->print(command);
  {
    pushBuf(timeout);
    int pos = indexAfter(0, "OK\n\n", 4);
    if ( pos > 0 ) {
         copyBuf( dstBuf, pos + 4);
    }
    pos = indexAfter(0,"ERROR\r\n",7);
    if ( pos > 0 ) {
       copyBuf( dstBuf, pos + 7);
    }
    //   _printf("Total Read : espBuf[%d] indexOf[%d] ,<%s>", top, pos,  espBuffer );
    //   _printf("Rest  Read : espBuf[%d]             ,dstBuf = <%s>", top,   dstBuf );
    _printf("###rxCmdRsp::%s###%s", command.c_str(), dstBuf);
  }
}

void  WIFI8266::txPDU (String d, int length, int conId) {
  String cipSend = " AT+CIPSEND=";
  cipSend += conId;
  cipSend += ",";
  cipSend += d.length();
  cipSend += "\r\n";
  sendData(cipSend, 1000, true);
  sendData(d, 500, true);
}
// #######################################################################
// ## RX FROM EPS
// #######################################################################

int WIFI8266:: rxPDU(char dstBuf[], int timeout) {
  int n = pushBuf(timeout);
  if ( top > 0  ) {
    int pos = indexAfter(0, "OK", 2);
      int pos2 = indexAfter(0, "IPD,", 2);
    if ( pos < 0 && pos2 > 0) {
      n = copyBuf(dstBuf, top);
      _printf("rxPDU::espBuf[%d] : %s", n, dstBuf);
      return n;
    }
  }
  return 0;
}

int WIFI8266:: copyBuf( char dstBuf[], int size ) {
  for (int i = 0; i < size; i++ ) {
    dstBuf[i] = espBuffer[i];
  }
  dstBuf[size] = 0;
  for ( int i = size; i < top; i++ ) {
    espBuffer[i - size] = espBuffer[i];
  }
  espBuffer[top - size] = 0;
  top -= size;
  return size;
}

int WIFI8266:: indexAfter(int sp, char findWord[] , int size) {
  if ( top < size ) {
    //   _printf("[ERROR] buffer is short than find word. buffer size = %d", top );
    return -3;
  }
  for (int i = sp; i < top; i++ ) {
    if ( espBuffer[i] == findWord[0] ) {
      if ( size == 1 ) {
        return i;
      } else {
        for (int j = 1; j < size; j++) {
          if ( espBuffer[j + i] == findWord[j] ) {
            if ( j == (size - 1) ) return i;
          }
        } // for
      }
    }
  }
  return -2;
}

String WIFI8266:: substring(int sp, int ep) {
  String s = String();
  for (int i = sp; i < ep; i++ ) {
    s += espBuffer[i];
  }
  return s;
}










