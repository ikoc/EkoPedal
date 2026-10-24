#include <HID-Project.h>

const uint8_t FOOTSWITCH_PIN = 8;
const uint8_t EMERGENCY_STOP_PIN = 2;

void setup() {
  pinMode(FOOTSWITCH_PIN, INPUT_PULLUP);
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  BootKeyboard.begin();
  BootKeyboard.releaseAll();
}

void loop() {
  if (digitalRead(EMERGENCY_STOP_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, (millis() / 150) % 2 ? HIGH : LOW);
    return;
  }

  digitalWrite(LED_BUILTIN, digitalRead(FOOTSWITCH_PIN) == LOW ? HIGH : LOW);
}
