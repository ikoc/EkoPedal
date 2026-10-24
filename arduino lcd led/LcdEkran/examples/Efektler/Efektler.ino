// Kayan yazi ve arka isik yanip sonme
#include <LcdEkran.h>

LcdEkran lcd;

void setup() {
  lcd.begin();
}

void loop() {
  lcd.yaz(0, "LcdEkran", ORTA);
  lcd.kayanYazi(1, "Kayan yazi ornegi", 250);
  lcd.yanipSon(3, 300);
  lcd.yaz(0, "Sol");
  lcd.yaz(1, "Sag", SAG);
  delay(2000);
}
