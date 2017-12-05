
#include <LiquidCrystal.h>

#include "RepStepper.h"
#include "WIFI8266.h"

LiquidCrystal lcd(16, 17, 23, 25, 27, 29);

// WIFI8266   wf(58,57); // RX,TX
WIFI8266  wcon(62, 57); // RX,TX

RepStepper motorX(400, 55, 54, 38 ); //   RepStepper(int number_of_steps, byte dir_pin, byte step_pin, byte enable_pin);
RepStepper motorY(400, 61, 60, 56 );
// RepStepper motorZ(400, 48, 46 , 62 );
RepStepper motorE1(400, 28, 26 , 24 );
RepStepper motorE2(400, 34, 36 , 30 );

// ##########################################################################
// ###  PIN DEFINE
// ##########################################################################

int pinBuzz = 37;

enum RotaryEncoder {
  pinRotaryEncoderA = 31,
  pinRotaryEncoderB = 33,
  pinRotaryEncoderButton = 35
};

int posCur = 0;
int arrCur = 0;
int value = 1000;
int prev = value;
int offset = 0;

int conId = 0;

#define MSIZE 512
char rxBuf[MSIZE] = "999 +IPD,4,38:G1 X10.111232 Y-20.224455 Z30.9366644";
char txBuf[MSIZE];
//char tmpBuf[100];
char msgBuf[MSIZE];


// ##############################################################
// ## BUF SUPPORT
// ##############################################################


  
int length(char buf[]) {
  for (int i = 0; i < 255; i++) {
    if ( buf[i] == 0 ) {
      return i;
    }
  }
  return 255;
}

int indexOf(char buf[], char findWord[]  ) {
  int size = length(findWord);
  int max = length(buf);
  if ( size > max ) {
    return -3;
  }
  for (int i = 0; i < max - size; i++ ) {
    if ( buf[i] == findWord[0] ) {
      if ( size == 1 ) {
        return i;
      } else {
        for (int j = 1; j < size; j++) {
          if ( buf[j + i] == findWord[j] ) {
            if ( j == (size - 1) ) return i;
          }
        } // for
      }
    }
  }
  return -1;
}

int sp = 0;

int scanInt(char debug[], char mark[]) {
  int pos = indexOf(&rxBuf[sp], mark);
  rxBuf[sp + pos] = 0;
  int value = atoi(&rxBuf[sp]);
  // sprintf(msgBuf, "\n <%s>=<int:%d> from <%s>", debug,  value, &rxBuf[sp]);
  // Serial.print(msgBuf);
  sp += pos + length(mark);
  return value;
}

float scanFloat(char debug[], char mark[]) {
  char buf2[10];
  int pos = indexOf(&rxBuf[sp], mark);
  rxBuf[sp + pos] = 0;
  float value = atof(&rxBuf[sp]);
  dtostrf(value, 9, 6, buf2);
  sprintf(msgBuf, "\n <%s>=<float:%s> from <%s> = freeRam =%d", debug,  buf2, &rxBuf[sp], freeRam());
  Serial.print(msgBuf);
  sp += pos + length(mark);
  return value;
}

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

boolean parsePDU(char buf[]) {
  int cp = indexOf(&rxBuf[0], "+IPD,");
  if ( cp > 0 ) {
    sp = cp + 5;
    conId = scanInt("ConID", ",");
    scanInt("Length", ":");
    sp ++;
    scanInt("G-Code", " ");
    sp ++;
    float x = scanFloat("X axis", " ");
    sp ++;
    float y = scanFloat("Y axis", " ");
    sp ++;
    float z =  scanFloat("Z axis", " ");
    lcd_print(1, "X ", x);
    lcd_print(2, "Y ", y);
    lcd_print(3, "Z ", z);
    moveY( y,   motorY);
    return true;
  } else {
    return false;
  }
}
//  ______ _          ____        _ _   ___   ___  __ ______ 
// |  ____(_)        |  _ \      | | | |__ \ / _ \/_ |____  |
// | |__   _ _ __ ___| |_) | __ _| | |    ) | | | || |   / / 
// |  __| | | '__/ _ \  _ < / _` | | |   / /| | | || |  / /  
// | |    | | | |  __/ |_) | (_| | | |  / /_| |_| || | / /   
// |_|    |_|_|  \___|____/ \__,_|_|_| |____|\___/ |_|/_/    

void setup()
{
  Serial.begin(9600);
  Serial.println("Program Start()");

  lcd.begin(20, 4);
  lcd_print(0, "FireBall V1.00 17W48");

  wcon.begin();
  pinMode(pinBuzz, OUTPUT);
  pinMode(pinRotaryEncoderA, INPUT_PULLUP);
  pinMode(pinRotaryEncoderB, INPUT_PULLUP);
  pinMode(pinRotaryEncoderButton, INPUT_PULLUP);
  motorE2.disable();
  motorX.disable();
  motorY.disable();

}

void loop()
{

  if ( true ) {
    wcon.testTerminal();
    return;
  }
  int count = wcon.rxPDU(rxBuf, 200);
  if ( count > 0 ) {
    if ( parsePDU(rxBuf)) {
      String s = String("hello !!!!");
      wcon.txPDU(s, conId);
    }
    //    moveY( y,   motorY);
  }
  return;
  lcd_print(1, "1) TEMINAL MODE");
  return;
  testMotor();
  value = readRotaryEncoder(value);
  offset = value - prev;
  prev = value;
}

// ##############################################################
// ### TUN MOTOR
// ##############################################################

// 800 = 160mm, 5 = 10mm

int tun_dist = 10 * 50  ;//
void testMotor() {
  for ( int x = 0; x < 50; x++) {
    lcd_print(1, "x = %03d", x);
    for ( int y = 0; y < 50; y++ ) {
      lcd_print(2, "y = %03d", y);
      moveY( 10 * 5,   motorY);
      drawBox(0, 0);
    }
    moveX( 10 * 5, motorX , motorE2 );
    moveY( 1000 * 5 * -1,   motorY);
  }
}

void drawBox(int x, int y ) {
  int size = 9 * 5;
  moveX( size  ,  motorX, motorE2 );
  moveY( size,   motorY);
  moveX( size * -1,  motorX, motorE2 );
  moveY( size * -1,  motorY);
}

const  int SPEED_0 = 2000 ;
const  int SPEED_2 = 1500;
const  int SPEED_SCALE = 16; // 1/16 step
const  int SPEED_DISTANCE = 10 * SPEED_SCALE;
const  int SPEED_K   = ((SPEED_0 - SPEED_2) / SPEED_DISTANCE);

int speed_cal( int pos, long offset ) {
  int speed = 0;
  int r = pos - ( offset - SPEED_DISTANCE)  ;
  if ( pos < SPEED_DISTANCE ) {
    speed = SPEED_0 - (SPEED_K  * pos) ;
  } else if ( r > 0) {
    speed = SPEED_2 + (SPEED_K * r );
  }
  if ( pos >= SPEED_DISTANCE && r <= 0 ) {
    speed = SPEED_2;
  }
  return speed;
}

void moveX(long offset, RepStepper a, RepStepper b) {
  offset = offset * SPEED_SCALE;
  if ( offset > 0 ) {
    a.setDirection(RS_FORWARD);
    b.setDirection(RS_REVERSE);
  } else {
    a.setDirection(RS_REVERSE);
    b.setDirection(RS_FORWARD);
    offset *= -1;
  }
  for ( long s = 0; s < offset; s++ ) {
    a.pulse();
    b.pulse();
    delayMicroseconds(speed_cal(s, offset));
  }
}

void moveY(long offset, RepStepper a ) {
  offset = offset * SPEED_SCALE;
  if ( offset > 0 ) {
    a.setDirection(RS_FORWARD);
  } else {
    a.setDirection(RS_REVERSE);
    offset *= -1;
  }
  for ( long s = 0; s < offset; s++ ) {
    a.pulse();
    delayMicroseconds(speed_cal(s, offset));
  }
}


// ***********************************************************
// ****************   readPins()
// ***********************************************************


void manualTestMotor() {

  int pos = 0;
  int scale = 10;
  boolean motorFlag = false;

  //  lcd_print (1, "scale", scale / (float)10);
  //  lcd_print (2, "posi", pos / (float)100);
  while (1) {

    int u = readRotaryEncoder(100);
    switch ( u ) {
      case 0 : // Control Scale
        scale = scale / 10;
        if (scale <= 0.1   ) {
          scale = 1000;
        }
        //        lcd_print (1, "scale", scale / (float)10);
        break;
      case 2: // Control Motor
        if ( motorFlag ) {
          motorFlag = false;
          lcd.setCursor(0, 3); lcd.print("motor = OFF");
        } else {
          motorFlag = true;
          lcd.setCursor(0, 3); lcd.print("motor = ON ");
        }
        break;
      case 3: // escape
        break;
      case 1:
        pos += (10 * scale);
        //        lcd_print (2, "posi",  pos / (float)100);
        //        if ( motorFlag )
        //          moveMotor(10  * scale , true);
        break;
      case -1:
        pos +=  (-10 * scale);
        //        lcd_print (2, "posi", pos / (float)100);
        //        if ( motorFlag)
        //          moveMotor(-10 * scale , true );
        break;
    }
  }
}

unsigned int encoderPos = 0;
unsigned int lastReportedPos = 1;

// *********************************************************************
// *********************************************************************
// *** BASIC LIB for DISPLAY
// *********************************************************************
// *********************************************************************

void beep(int duration) {
  //  digitalWrite(pinBuzz, HIGH);
  //  delay(duration);
  //  digitalWrite(pinBuzz, LOW);
}

void lcd_print(int row, char* msg ) {
  lcd.setCursor(0, row);
  lcd.print(msg);
}

void lcd_print(int row, char* format, float value) {
  char buf[20];
  char buf2[20];
  lcd.setCursor(0, row);
  dtostrf(value, 10, 6, buf2);
  sprintf (buf, "%-3s = %9s mm", format, buf2);
  lcd.print(buf);
}

void lcd_print(int row,  char* format , long value) {
  char buf[20];
  char buf2[20];
  lcd.setCursor(0, row);
  sprintf (buf, format,   value);
  lcd.print(buf);
}

void lcd_print(int row,  char* format , int value) {
  char buf[20];
  char buf2[20];
  lcd.setCursor(0, row);
  sprintf (buf, format,   value);
  lcd.print(buf);
}

void debugf (char* format, int value ) {
  char buf[20];
  sprintf (buf,  format, value);
  Serial.println(buf);
}


int readRotaryEncoder(int value) {
  unsigned long currentTime;
  unsigned long loopTime = 0;
  unsigned long timeOnlyButton = 0;
  unsigned char encoder_A;
  unsigned char encoder_B;
  unsigned char encoder_BTN;
  unsigned char encoder_A_prev = 0;

  static byte rPos = 3;
  int count = 0;
  int inc;
  // beep(10);
  while (true) {
    encoder_A = digitalRead(pinRotaryEncoderA);
    encoder_B = digitalRead(pinRotaryEncoderB);
    encoder_BTN = digitalRead(pinRotaryEncoderButton);
    currentTime = millis();
    if ( currentTime >= (loopTime + 5)) {

      if ( (!encoder_A) && (encoder_A_prev)) {
        if ( !encoder_BTN) {
          inc = 10;
        } else {
          inc = 1;
        }
        if ( encoder_B ) {
          value += inc;
        } else {
          value -= inc;
        }
        beep(1);
        //      lcd_print(2, value);
        count = 0;
      }  else if ( !encoder_BTN) {
        //   debugf("count = %d", count);
        //     lcd_print(2, value);
        if ( count++ > 50) {
          beep(50);
          break;
        }
      }
      encoder_A_prev = encoder_A;
      loopTime = currentTime;
    }
  }

  return value;
}

// *********************************************************************
// *********************************************************************
// *** MOTER
// *********************************************************************
// *********************************************************************



int motorStep99(int x, int SPEED_0, int SPEED_2 ) {
  int speed = 0;
  int step = x / 10;
  if ( step < 80) {
    speed =  700 - ( step * 5 );
  } else {
    speed = 300;
  }
  speed += 500;
  if (SPEED_2 != 0 ) {
    digitalWrite(SPEED_2, 1); // Output high
  }
  digitalWrite(SPEED_0, HIGH); // Output high
  delayMicroseconds(speed); // Wait 1/2 a ms (변경하여 속도 조절 가능) =500
  digitalWrite(SPEED_0, LOW); // Output low
  if (SPEED_2 != 0 ) {
    digitalWrite(SPEED_2, 0); // Output high
  }
  delayMicroseconds(speed); // Wait 1/2 a ms
}


