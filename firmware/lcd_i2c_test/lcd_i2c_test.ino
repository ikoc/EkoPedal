#include <Wire.h>

uint8_t address = 0;

void writePort(uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(value | 0x08);
  Wire.endTransmission();
}

void nibble(uint8_t value, bool rs) {
  uint8_t out = (value << 4) | (rs ? 1 : 0);
  writePort(out | 4);
  delayMicroseconds(1);
  writePort(out);
  delayMicroseconds(50);
}

void command(uint8_t value) { nibble(value >> 4, false); nibble(value & 15, false); }
void data(uint8_t value) { nibble(value >> 4, true); nibble(value & 15, true); }

void setup() {
  Serial.begin(115200);
  Wire.begin();
  for (uint8_t a = 1; a < 127; ++a) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { address = a; break; }
  }
  if (!address) { Serial.println("LCD_NOT_FOUND"); return; }
  Serial.print("LCD=0x"); Serial.println(address, HEX);
  delay(50);
  nibble(3, false); delay(5); nibble(3, false); delayMicroseconds(150);
  nibble(3, false); nibble(2, false);
  command(0x28); command(0x0C); command(0x06); command(0x01); delay(2);
  command(0x80); const char *line1 = "LCD TEST OK";
  for (const char *p = line1; *p; ++p) data(*p);
  command(0xC0); const char *line2 = "ENCODER D7";
  for (const char *p = line2; *p; ++p) data(*p);
}

void loop() {}
