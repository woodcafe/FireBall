#include <LiquidCrystal.h>
#include "RepStepper.h"

LiquidCrystal lcd(16, 17, 23, 25, 27, 29);


// RepStepper(int number_of_steps, byte dir_pin, byte step_pin, byte enable_pin);
RepStepper motorX(400, 55, 54, 38 );
RepStepper motorY(400, 61, 60, 56 );
RepStepper motorZ(400, 48, 46 , 62 );
RepStepper motorE1(400, 28, 26 , 24 );
RepStepper motorE2(400, 34, 36 , 30 );

// *********************************************************************
// ** PIN DEFINE
// *********************************************************************

int pinBuzz = 37;

enum RotaryEncoder {
  pinRotaryEncoderA = 31,
  pinRotaryEncoderB = 33,
  pinRotaryEncoderButton = 35
};

//const int motorSpeed = 250;//700

// 1000=250
//4pluse 1mill
//2.5

int posCur = 0;
int arrCur = 0;


void setup()
{
  lcd.begin(20, 4);
  lcd_print(0, "FireBALL 17B1015-1");

  Serial.begin(9600);
  Serial.println("Program Start()");
  pinMode(pinBuzz, OUTPUT);
  pinMode(pinRotaryEncoderA, INPUT_PULLUP);
  pinMode(pinRotaryEncoderB, INPUT_PULLUP);
  pinMode(pinRotaryEncoderButton, INPUT_PULLUP);
  motorE2.disable();
  motorX.disable();
  motorY.disable();
}

// ******************************************
// *** CONFIG
// ******************************************

int value = 1000;
int prev = value;
int offset = 0;

void loop()
{
  testMotor();
  //  lcd_print(1, value);
  value = readRotaryEncoder(value);
  //  lcd_print(1, value);
  offset = value - prev;
  prev = value;
}

// ******************************************
// ** TUNNING MOTORS
// ******************************************

// 800=150mm,

int tun_dist = 800  ;
void testMotor() {
  for ( int i = 0; i < 1000; i++ ) {
    lcd_print(1, "MOVER(1) ");
    motorMove( tun_dist    , motorX , motorE2, motorY);
    delay (2000);
     lcd_print(1, "MOVE(2) ");
    motorMove( tun_dist * -1,    motorX, motorE2, motorY);
    delay (2000);
  }
}

const  int SPEED_0 = 2000 ;
const  int SPEED_2 = 1500;
const  int SPEED_SCALE = 16; // 1/16 step
const  int SPEED_DISTANCE = 10 * SPEED_SCALE;
const  int SPEED_K   = ((SPEED_0 - SPEED_2) / SPEED_DISTANCE);

int speed_cal( int pos, long offset ) {
  int speed =0;
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

void motorMove(long offset, RepStepper a, RepStepper b, RepStepper c) {
  offset = offset * SPEED_SCALE;
  if ( offset > 0 ) {
    a.setDirection(RS_FORWARD);
    b.setDirection(RS_REVERSE);
    c.setDirection(RS_REVERSE);
  } else {
    a.setDirection(RS_REVERSE);
    b.setDirection(RS_FORWARD);
    c.setDirection(RS_FORWARD);
    offset *= -1;
  }

  for ( long s = 0; s < offset; s++ ) {
//    int r = s - ( offset - SPEED_DISTANCE)  ;
//    if ( s < SPEED_DISTANCE ) {
//      speed = SPEED_0 - (SPEED_K  * s) ;
//    } else if ( r > 0) {
//      speed = SPEED_2 + (SPEED_K * r );
//    }
//    if ( s >= SPEED_DISTANCE && r <= 0 ) {
//      speed = SPEED_2;
//    }
    a.pulse();
    b.pulse();
    c.pulse();
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
  dtostrf(value, 3, 1, buf2);
  sprintf (buf, "%5s = %5s mm", format, buf2);
  lcd.print(buf);
}

void debugf (char* format, int value ) {
  char buf[20];
  sprintf (buf,  format, value);
  Serial.println(buf);
}

void lcd_print(int row,  char* format , long value) {
  char buf[20];
  char buf2[20];
  lcd.setCursor(0, row);
  sprintf (buf, format,   value);
  lcd.print(buf);
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
        lcd_print(2, value);
        count = 0;
      }  else if ( !encoder_BTN) {
        //   debugf("count = %d", count);
        lcd_print(2, value);
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

