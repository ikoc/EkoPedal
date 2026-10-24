#include <HID-Project.h>

const uint8_t FOOTSWITCH_PIN = 8;
const unsigned long PRESS_DEBOUNCE_MS = 30;
const unsigned long RELEASE_DEBOUNCE_MS = 50;

bool armed = false;
bool wasPressed = false;
unsigned long pressStartedMs = 0;
unsigned long releaseStartedMs = 0;

void setup() {
  pinMode(FOOTSWITCH_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  BootKeyboard.begin();
  BootKeyboard.releaseAll();
}

void loop() {
  const bool pressed = digitalRead(FOOTSWITCH_PIN) == LOW;
  const unsigned long now = millis();
  digitalWrite(LED_BUILTIN, pressed ? HIGH : LOW);

  if (!pressed) {
    if (wasPressed) {
      releaseStartedMs = now;
    }
    wasPressed = false;
    if (now - releaseStartedMs >= RELEASE_DEBOUNCE_MS) {
      armed = true;
    }
    return;
  }

  if (!wasPressed) {
    pressStartedMs = now;
  }
  wasPressed = true;
  if (armed && now - pressStartedMs >= PRESS_DEBOUNCE_MS) {
    armed = false;
    BootKeyboard.write('7');
  }
}
