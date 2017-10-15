// Based on the work by DFRobot

#include "RotaryEncoder.h"
#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#define printIIC(args)	Wire.write(args)
inline size_t StepMotor::write(uint8_t value) {
  send(value, Rs);
  return 1;
}

#else
#include "WProgram.h"

#define printIIC(args)	Wire.send(args)
inline void StepMotor::write(uint8_t value) {
  send(value, Rs);
}

#endif
#include "Wire.h"




StepMotor::StepMotor(int pinA0, int pinB0, int pinBtn0)
{
  pinA = pinA0;
  pinB = pinB0;
  pinBtn = pinBtn0;
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(pinBtn, INPUT_PULLUP);
}

int StepMotor::move(int speed ) {
  pinMode(stepPin, OUTPUT);
  digitalWrite(stepPin, HIGH); // Output high
  delayMicroseconds(speed); // Wait 1/2 a ms (변경하여 속도 조절 가능) =500
  digitalWrite(stepPin, LOW); // Output low
  delayMicroseconds(speed); // Wait 1/2 a ms
  return stepPin;
}

void StepMotor::setDirection(int dir) {
  digitalWrite(dirPin, dir); // Output high
}

void StepMotor::init_priv()
{
  Wire.begin();
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  begin(_cols, _rows);
}

void StepMotor::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delay(50);

  // Now we pull both RS and R/W low to begin commands
  expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
  delay(1000);

  //put the LCD into 4 bit mode
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46

  // we start in 8bit mode, try to set 4 bit mode
  write4bits(0x03 << 4);
  delayMicroseconds(4500); // wait min 4.1ms

  // second try
  write4bits(0x03 << 4);
  delayMicroseconds(4500); // wait min 4.1ms

  // third go!
  write4bits(0x03 << 4);
  delayMicroseconds(150);

  // finally, set to 4-bit interface
  write4bits(0x02 << 4);


  // set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for roman languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

  home();

}

/********** high level commands, for the user! */
void StepMotor::clear() {
  command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void StepMotor::home() {
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void StepMotor::setCursor(uint8_t col, uint8_t row) {
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) {
    row = _numlines - 1;  // we count rows starting w/0
  }
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void StepMotor::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void StepMotor::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void StepMotor::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void StepMotor::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void StepMotor::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void StepMotor::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void StepMotor::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void StepMotor::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void StepMotor::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void StepMotor::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void StepMotor::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void StepMotor::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void StepMotor::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++) {
    write(charmap[i]);
  }
}

//createChar with PROGMEM input
void StepMotor::createChar(uint8_t location, const char *charmap) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++) {
    write(pgm_read_byte_near(charmap++));
  }
}

// Turn the (optional) backlight off/on
void StepMotor::noBacklight(void) {
  _backlightval = LCD_NOBACKLIGHT;
  expanderWrite(0);
}

void StepMotor::backlight(void) {
  _backlightval = LCD_BACKLIGHT;
  expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

inline void StepMotor::command(uint8_t value) {
  send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void StepMotor::send(uint8_t value, uint8_t mode) {
  uint8_t highnib = value & 0xf0;
  uint8_t lownib = (value << 4) & 0xf0;
  write4bits((highnib) | mode);
  write4bits((lownib) | mode);
}

void StepMotor::write4bits(uint8_t value) {
  expanderWrite(value);
  pulseEnable(value);
}

void StepMotor::expanderWrite(uint8_t _data) {
  Wire.beginTransmission(_Addr);
  printIIC((int)(_data) | _backlightval);
  Wire.endTransmission();
}

void StepMotor::pulseEnable(uint8_t _data) {
  expanderWrite(_data | En);	// En high
  delayMicroseconds(1);		// enable pulse must be >450ns

  expanderWrite(_data & ~En);	// En low
  delayMicroseconds(50);		// commands need > 37us to settle
}


// Alias functions

void StepMotor::cursor_on() {
  cursor();
}

void StepMotor::cursor_off() {
  noCursor();
}

void StepMotor::blink_on() {
  blink();
}

void StepMotor::blink_off() {
  noBlink();
}

void StepMotor::load_custom_character(uint8_t char_num, uint8_t *rows) {
  createChar(char_num, rows);
}

void StepMotor::setBacklight(uint8_t new_val) {
  if (new_val) {
    backlight();		// turn backlight on
  } else {
    noBacklight();		// turn backlight off
  }
}

void StepMotor::printstr(const char c[]) {
  //This function is not identical to the function used for "real" I2C displays
  //it's here so the user sketch doesn't have to be changed
  print(c);
}


// unsupported API functions
void StepMotor::off() {}
void StepMotor::on() {}
void StepMotor::setDelay (int cmdDelay, int charDelay) {}
uint8_t StepMotor::status() {
  return 0;
}
uint8_t StepMotor::keypad () {
  return 0;
}
uint8_t StepMotor::init_bargraph(uint8_t graphtype) {
  return 0;
}
void StepMotor::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end) {}
void StepMotor::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end) {}
void StepMotor::setContrast(uint8_t new_val) {}


