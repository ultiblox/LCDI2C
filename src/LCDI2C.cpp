// Based on the work by DFRobot

#include "LCDI2C.h"
#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#define printIIC(args) Wire.write(args)
inline size_t LCDI2C::write(uint8_t value) {
    send(value, Rs);
    return 1;
}

#else
#include "WProgram.h"

#define printIIC(args) Wire.send(args)
inline void LCDI2C::write(uint8_t value) {
    send(value, Rs);
}

#endif
#include "Wire.h"

// When the display powers up, it is configured as follows:
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 

LCDI2C::LCDI2C(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) {
  _Addr = lcd_Addr;
  _cols = lcd_cols;
  _rows = lcd_rows;
  _backlightval = LCD_NOBACKLIGHT;
}

void LCDI2C::init(){
    init_priv();
}

void LCDI2C::init_priv() {
    Wire.begin();
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(_cols, _rows);  
}

void LCDI2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;

    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    delay(50); 

    expanderWrite(_backlightval);    
    delay(1000);

   write4bits(0x03 << 4);
   delayMicroseconds(4500); 

   write4bits(0x03 << 4);
   delayMicroseconds(4500); 

   write4bits(0x03 << 4); 
   delayMicroseconds(150);
   
   write4bits(0x02 << 4); 

    command(LCD_FUNCTIONSET | _displayfunction);  

    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    clear();

    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    command(LCD_ENTRYMODESET | _displaymode);

    home();
}

void LCDI2C::clear(){
    command(LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LCDI2C::home(){
    command(LCD_RETURNHOME);  
    delayMicroseconds(2000);
}

void LCDI2C::setCursor(uint8_t col, uint8_t row){
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if ( row > _numlines ) {
        row = _numlines-1;
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCDI2C::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDI2C::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDI2C::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDI2C::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDI2C::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDI2C::blink() {
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDI2C::scrollDisplayLeft(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C::scrollDisplayRight(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LCDI2C::leftToRight(void) {
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LCDI2C::rightToLeft(void) {
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LCDI2C::autoscroll(void) {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LCDI2C::noAutoscroll(void) {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LCDI2C::createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7; 
    command(LCD_SETCGRAMADDR | (location << 3));
    for (int i=0; i<8; i++) {
        write(charmap[i]);
    }
}

void LCDI2C::noBacklight(void) {
    _backlightval=LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void LCDI2C::backlight(void) {
    _backlightval=LCD_BACKLIGHT;
    expanderWrite(0);
}

inline void LCDI2C::command(uint8_t value) {
    send(value, 0);
}

void LCDI2C::send(uint8_t value, uint8_t mode) {
    uint8_t highnib=value&0xf0;
    uint8_t lownib=(value<<4)&0xf0;
    write4bits((highnib)|mode);
    write4bits((lownib)|mode); 
}

void LCDI2C::write4bits(uint8_t value) {
    expanderWrite(value);
    pulseEnable(value);
}

void LCDI2C::expanderWrite(uint8_t _data){                                        
    Wire.beginTransmission(_Addr);
    printIIC((int)(_data) | _backlightval);
    Wire.endTransmission();
}

void LCDI2C::pulseEnable(uint8_t _data){
    expanderWrite(_data | En);   
    delayMicroseconds(1);

    expanderWrite(_data & ~En);   
    delayMicroseconds(50);
}

void LCDI2C::cursor_on(){
    cursor();
}

void LCDI2C::cursor_off(){
    noCursor();
}

void LCDI2C::blink_on(){
    blink();
}

void LCDI2C::blink_off(){
    noBlink();
}

void LCDI2C::load_custom_character(uint8_t char_num, uint8_t *rows){
    createChar(char_num, rows);
}

void LCDI2C::setBacklight(uint8_t new_val){
    if(new_val){
        backlight();
    }else{
        noBacklight();
    }
}

void LCDI2C::printstr(const char c[]){
    print(c);
}

void LCDI2C::setDelayTime(int t){
  delayTime = t;
}
