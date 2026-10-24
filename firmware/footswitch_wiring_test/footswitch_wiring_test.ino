// Temporary contact test: no USB keyboard or serial output.
const byte PEDAL_PIN = 8;

void setup() {
  pinMode(PEDAL_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, digitalRead(PEDAL_PIN) == LOW ? HIGH : LOW);
}
