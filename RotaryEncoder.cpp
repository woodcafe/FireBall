// Based on the work by DFRobot

#include "RotaryEncoder.h"
#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"
 

#else
#include "WProgram.h"
 

#endif
#include "Wire.h"




RotaryEncoder::RotaryEncoder(int pinA0, int pinB0, int pinBtn0)
{
  pinA = pinA0;
  pinB = pinB0;
  pinBtn = pinBtn0;
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(pinBtn, INPUT_PULLUP);
}

 


