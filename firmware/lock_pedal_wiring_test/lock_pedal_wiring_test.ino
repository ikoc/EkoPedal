// Safe wiring test for two dry-contact pedals on an Arduino Leonardo.
// No keyboard, mouse or other HID reports are sent.
const byte NUM_PEDAL_PIN = 3;
const byte SCROLL_PEDAL_PIN = 4;

int previousNum = -1;
int previousScroll = -1;
unsigned long lastReport = 0;

void setup() {
  pinMode(NUM_PEDAL_PIN, INPUT_PULLUP);
  pinMode(SCROLL_PEDAL_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  const int num = digitalRead(NUM_PEDAL_PIN);
  const int scroll = digitalRead(SCROLL_PEDAL_PIN);
  digitalWrite(LED_BUILTIN, num == LOW || scroll == LOW);

  if (num != previousNum || scroll != previousScroll || millis() - lastReport >= 1000) {
    Serial.print("D3=");
    Serial.print(num == LOW ? "CLOSED" : "OPEN");
    Serial.print("  D4=");
    Serial.println(scroll == LOW ? "CLOSED" : "OPEN");
    previousNum = num;
    previousScroll = scroll;
    lastReport = millis();
  }
}
