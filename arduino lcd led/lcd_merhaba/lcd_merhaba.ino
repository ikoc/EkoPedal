// 16x2 LCD + HW-61 (PCF8574) I2C adaptör, adres 0x27
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Merhaba!");
}

void loop() {
  // Alt satırda açılıştan beri geçen saniye
  lcd.setCursor(0, 1);
  lcd.print("Sure: ");
  lcd.print(millis() / 1000);
  lcd.print(" sn   ");
  delay(500);
}
