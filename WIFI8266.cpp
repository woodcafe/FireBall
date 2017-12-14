
#include "WIFI8266.h"

#include <HardwareSerial.h>

// AT+RST, AT+GMR :: version
// AT+CWJAP="SSID”,”12345678″
// AT+CIFSR :: show my ip
// AT+CIUPDATE
// AT+CWLAP : AP LIST
// AP+   : MY IP

SoftwareSerial *softSerial;
#define MEMSIZE 512
int top = 0;
char espBuffer[MEMSIZE];
char dstBuf[MEMSIZE];
char espTxBuffer[MEMSIZE];
char debugBuffer[MEMSIZE];

void  _printf(char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  //  char buf[vsnprintf(NULL, 0, fmt, va) + 1];
  vsprintf(debugBuffer, fmt, va);
  Serial.println(debugBuffer);
  va_end(va);
}

WIFI8266::WIFI8266(int rxpin, int txpin)
{
  pinMode(txpin, OUTPUT);
  pinMode(rxpin, INPUT_PULLUP);
  softSerial  = new SoftwareSerial(rxpin, txpin);
}

void WIFI8266:: begin() {
  Serial.begin(9600);
  Serial.println("ESP8266 WIFI Enabled.");
  softSerial->begin(9600);
  softSerial->println("AT+RST\n\n");
  sendData("AT+RST\r\n", 1000, true);
  sendData("AT+GMR\r\n", 5000, true);
  sendData( "AT+CIFSR\r\n" , 1000, true);
  sendData( "AT+CIPMUX=1\r\n" , 1000, true);        // configure for multiple connections
  sendData( "AT+CIPSERVER=1,80\r\n" , 1000, true);  // turn on server on port 80
}


// #######################################################################
// ###  TERMINAL MODE
// #######################################################################

void WIFI8266::testTerminal() {
  // Read Serial
  int count = pushBuf(500);
  if ( count > 0 ) {
    Serial.print(espBuffer);
    top = 0;
    espBuffer[top] = 0;
  }
  String s = String();
  while (Serial.available() ) {
    char c = Serial.read();
    s += c;
    Serial.write(c);
  }
  // Write WIFI
  if ( s.length() > 0) {
    softSerial->write(s.c_str()); // send to wifi
    Serial.print("\n[WIFI]> ");
    Serial.flush();
  }
}







// #######################################################################
// ###  TX FROM EPS
// #######################################################################

int WIFI8266:: pushBuf(int timeout) {
  int oldTop = top;
  int  dispose = 0;
  long int time = millis();
  while ( (time + timeout) > millis() ) {
    while (softSerial->available()) {
      char c = softSerial->read();  // read the next character
      if ( top < MEMSIZE - 1 ) {
        espBuffer[top++] = c;
      } else {
        dispose ++;
      }
    }
  }
  if ( dispose > 0 ) {
    _printf("[WARNING] dispsose = %d", dispose);
  }
  espBuffer[top] = 0;
  return top;
}


void  WIFI8266::txPDU (String msg, int conId) {
  _printf("\ntxPDU length=%d, conid=%d msg=%s free = %d", msg.length(), conId, msg.c_str(), 0);
  //  sprintf(espTxBuffer, " AT+CIPSEND=%d,%d\r\n", conId, xx);
  String cipSend = "AT+CIPSEND=";
  cipSend += conId;
  cipSend += ",";
  cipSend += msg.length();
  cipSend += "\r\n";
  // _printf("\n KILL = %s",cipSend.c_str());
  sendData(cipSend.c_str(), 1000, true);
  sendData(msg.c_str(), 500, true);
}

void  WIFI8266:: sendData(String cmd, const int timeout, boolean debug) {
  softSerial->print(cmd);
  {
    int n = pushBuf(timeout);
    if ( n <= 0 ) {
      _printf("[ERROR] ESP8266 is not responsed cmd = %s" , cmd.c_str() );
      return;
    }
    _printf("### cmdResponse::espBuffer[%d]=<%s> ", n, espBuffer);
    int pos = indexAfter(0, "OK\n\n", 4);
    if ( pos > 0 ) {
      copyBuf( dstBuf, pos + 4);
    } else {
      pos = indexAfter(0, "ERROR\r\n", 7);
      if ( pos > 0 ) {
        copyBuf( dstBuf, pos + 7);
      }
    }
  }
}
// #######################################################################
// ## RX FROM EPS
// #######################################################################

int WIFI8266:: rxPDU(char rxBuf[], int timeout) {
  int posIPD = -1;
  int posEND = -1;
  while ( posIPD < 0 || posEND < 0 ) {
    int n = pushBuf(timeout);
    //   _printf("rxPDU_NOK::rxCount=%d, bufTop=%d buf=%s", n, top, espBuffer);
    if ( top > 0  ) {
      int pos = indexAfter(0, "OK", 2);
      posIPD  = indexAfter(0, "IPD,", 4);
      posEND = indexAfter(0, "END", 3);
      if (    posIPD > 0 && posEND > 0 ) {
        n = copyBuf(rxBuf, top);
        _printf("rxPDU_OK::espBuf[%d] : %s", n, dstBuf);
        return n;
      }
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











