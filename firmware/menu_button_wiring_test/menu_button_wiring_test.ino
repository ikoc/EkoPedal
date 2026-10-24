// Find the normally open contact pair of the momentary menu button.
// Connect only one candidate pair between D7 and GND, with USB disconnected.
// No keyboard, mouse or LCD activity is produced.
const byte MENU_TEST_PIN = 7;
int previous = -1;

void setup() {
  pinMode(MENU_TEST_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  const int current = digitalRead(MENU_TEST_PIN);
  digitalWrite(LED_BUILTIN, current == LOW);
  if (current != previous) {
    Serial.println(current == LOW ? "BUTTON=CLOSED" : "BUTTON=OPEN");
    previous = current;
  }
}
