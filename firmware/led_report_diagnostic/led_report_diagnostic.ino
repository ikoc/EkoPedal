#include <HID-Project.h>

uint8_t previousLeds = 0xff;
unsigned long lastReportMs = 0;

void setup() {
  Serial.begin(115200);
  BootKeyboard.begin();
  BootKeyboard.releaseAll();
}

void loop() {
  const uint8_t leds = BootKeyboard.getLeds();
  const unsigned long now = millis();
  if (leds != previousLeds || now - lastReportMs >= 1000) {
    Serial.print("LEDS=");
    Serial.print(leds, HEX);
    Serial.print(" CAPS=");
    Serial.print((leds & LED_CAPS_LOCK) != 0);
    Serial.print(" NUM=");
    Serial.print((leds & LED_NUM_LOCK) != 0);
    Serial.print(" SCROLL=");
    Serial.println((leds & LED_SCROLL_LOCK) != 0);
    previousLeds = leds;
    lastReportMs = now;
  }
}
