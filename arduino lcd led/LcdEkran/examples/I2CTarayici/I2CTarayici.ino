// I2C hattindaki cihazlarin adresini seri porta yazar (115200 baud)
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
}

void loop() {
  int bulunan = 0;
  for (byte adres = 1; adres < 127; adres++) {
    Wire.beginTransmission(adres);
    if (Wire.endTransmission() == 0) {
      Serial.print("BULUNDU 0x");
      if (adres < 16) Serial.print("0");
      Serial.println(adres, HEX);
      bulunan++;
    }
  }
  if (bulunan == 0) Serial.println("CIHAZ YOK");
  Serial.println("---");
  delay(2000);
}
