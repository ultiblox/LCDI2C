#include<LCDI2C.h>
#include<Wire.h>

LCDI2C lcd(0x3F,16,2); //LCD 클래스 초기화

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Hello World!");
}

void loop() {
}
