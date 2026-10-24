// En basit kullanim: iki satira hizali yazi ve saniye sayaci
#include <LcdEkran.h>

LcdEkran lcd;  // 16x2, adres otomatik

void setup() {
  Serial.begin(115200);
  if (!lcd.begin()) {
    Serial.println("LCD bulunamadi, kablolari kontrol et");
    return;
  }
  Serial.print("LCD adresi: 0x");
  Serial.println(lcd.adres(), HEX);
  lcd.yaz(0, "Merhaba!", ORTA);
}

void loop() {
  lcd.yaz(1, "Sure: " + String(millis() / 1000) + " sn");
  delay(500);
}
