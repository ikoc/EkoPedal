#include <Keyboard.h>
#include <Mouse.h>

// Safe USB HID bridge for Arduino Leonardo / ATmega32U4.
// HID is disabled on boot and only accepts commands after ENABLE 1 plus heartbeat.

const uint8_t EMERGENCY_STOP_PIN = 2;
const unsigned long HEARTBEAT_TIMEOUT_MS = 3000;
const unsigned long MAX_KEY_HOLD_MS = 1000;
const size_t COMMAND_BUFFER_SIZE = 64;

char commandBuffer[COMMAND_BUFFER_SIZE];
size_t commandLength = 0;
bool commandOverflow = false;
bool hidEnabled = false;
unsigned long lastHeartbeatMs = 0;

void releaseAllInputs() {
  Keyboard.releaseAll();
  Mouse.release(MOUSE_LEFT);
  Mouse.release(MOUSE_RIGHT);
  Mouse.release(MOUSE_MIDDLE);
}

bool emergencyStopPressed() {
  return digitalRead(EMERGENCY_STOP_PIN) == LOW;
}

void disableHid(const char *reason) {
  releaseAllInputs();
  hidEnabled = false;
  Serial.print(F("OK STOPPED "));
  Serial.println(reason);
}

bool heartbeatValid() {
  if (!hidEnabled) {
    return false;
  }
  return millis() - lastHeartbeatMs <= HEARTBEAT_TIMEOUT_MS;
}

int parseKeyToken(const char *token) {
  if (token == NULL || token[0] == '\0') {
    return 0;
  }

  if (token[1] == '\0') {
    return token[0];
  }

  if (strcmp(token, "SPACE") == 0) return ' ';
  if (strcmp(token, "ENTER") == 0) return KEY_RETURN;
  if (strcmp(token, "ESC") == 0) return KEY_ESC;
  if (strcmp(token, "TAB") == 0) return KEY_TAB;
  if (strcmp(token, "BACKSPACE") == 0) return KEY_BACKSPACE;
  if (strcmp(token, "UP") == 0) return KEY_UP_ARROW;
  if (strcmp(token, "DOWN") == 0) return KEY_DOWN_ARROW;
  if (strcmp(token, "LEFT") == 0) return KEY_LEFT_ARROW;
  if (strcmp(token, "RIGHT") == 0) return KEY_RIGHT_ARROW;
  if (strcmp(token, "F1") == 0) return KEY_F1;
  if (strcmp(token, "F2") == 0) return KEY_F2;
  if (strcmp(token, "F3") == 0) return KEY_F3;
  if (strcmp(token, "F4") == 0) return KEY_F4;
  if (strcmp(token, "F5") == 0) return KEY_F5;
  if (strcmp(token, "F6") == 0) return KEY_F6;
  if (strcmp(token, "F7") == 0) return KEY_F7;
  if (strcmp(token, "F8") == 0) return KEY_F8;
  if (strcmp(token, "F9") == 0) return KEY_F9;
  if (strcmp(token, "F10") == 0) return KEY_F10;
  if (strcmp(token, "F11") == 0) return KEY_F11;
  if (strcmp(token, "F12") == 0) return KEY_F12;

  return 0;
}

bool parseHoldMs(const char *token, unsigned long *holdMs) {
  if (token == NULL || token[0] == '\0') {
    *holdMs = 50;
    return true;
  }

  if (token[0] < '0' || token[0] > '9') return false;
  char *endPtr = NULL;
  unsigned long value = strtoul(token, &endPtr, 10);
  if (endPtr == token || *endPtr != '\0') {
    return false;
  }
  *holdMs = value > MAX_KEY_HOLD_MS ? MAX_KEY_HOLD_MS : value;
  return true;
}

void holdKeySafely(int keyCode, unsigned long holdMs) {
  Keyboard.press(keyCode);
  unsigned long start = millis();
  while (millis() - start < holdMs) {
    if (emergencyStopPressed()) {
      releaseAllInputs();
      hidEnabled = false;
      Serial.println(F("ERR EMERGENCY_STOP"));
      return;
    }
    delay(5);
  }
  Keyboard.release(keyCode);
  Serial.println(F("OK KEY_PRESSED"));
}

void handleCommand(char *line) {
  char *command = strtok(line, " ");
  if (command == NULL) {
    Serial.println(F("ERR EMPTY"));
    return;
  }

  if (strcmp(command, "HELLO") == 0) {
    Serial.println(F("OK HELLO LEONARDO_HID_SAFE 1"));
    return;
  }

  if (strcmp(command, "PING") == 0) {
    lastHeartbeatMs = millis();
    Serial.println(F("OK PONG"));
    return;
  }

  if (strcmp(command, "ENABLE") == 0) {
    char *value = strtok(NULL, " ");
    if (value == NULL) {
      Serial.println(F("ERR MISSING_ENABLE_VALUE"));
      return;
    }
    if (strcmp(value, "1") == 0) {
      if (emergencyStopPressed()) {
        releaseAllInputs();
        hidEnabled = false;
        Serial.println(F("ERR EMERGENCY_STOP"));
        return;
      }
      hidEnabled = true;
      lastHeartbeatMs = millis();
      Serial.println(F("OK ENABLED"));
      return;
    }
    if (strcmp(value, "0") == 0) {
      disableHid("DISABLED");
      return;
    }
    Serial.println(F("ERR BAD_ENABLE_VALUE"));
    return;
  }

  if (strcmp(command, "STOP") == 0) {
    disableHid("APP_STOP");
    return;
  }

  if (strcmp(command, "RELEASE") == 0) {
    releaseAllInputs();
    Serial.println(F("OK RELEASED"));
    return;
  }

  if (strcmp(command, "KP") == 0) {
    if (!heartbeatValid()) {
      releaseAllInputs();
      hidEnabled = false;
      Serial.println(F("ERR HID_NOT_ENABLED_OR_HEARTBEAT_TIMEOUT"));
      return;
    }
    if (emergencyStopPressed()) {
      releaseAllInputs();
      hidEnabled = false;
      Serial.println(F("ERR EMERGENCY_STOP"));
      return;
    }

    char *keyToken = strtok(NULL, " ");
    char *holdToken = strtok(NULL, " ");
    unsigned long holdMs = 50;
    int keyCode = parseKeyToken(keyToken);
    if (keyCode == 0 || strtok(NULL, " ") != NULL || !parseHoldMs(holdToken, &holdMs)) {
      Serial.println(F("ERR BAD_KEY_OR_HOLD"));
      return;
    }
    holdKeySafely(keyCode, holdMs);
    return;
  }

  Serial.println(F("ERR UNKNOWN_COMMAND"));
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      if (commandOverflow) {
        Serial.println(F("ERR COMMAND_TOO_LONG"));
      } else {
        commandBuffer[commandLength] = '\0';
        handleCommand(commandBuffer);
      }
      commandLength = 0;
      commandOverflow = false;
      return;
    }
    if (commandOverflow) {
      continue;
    }
    if (commandLength < COMMAND_BUFFER_SIZE - 1) {
      commandBuffer[commandLength++] = c;
    } else {
      commandLength = 0;
      commandOverflow = true;
      releaseAllInputs();
    }
  }
}

void setup() {
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  Keyboard.begin();
  Mouse.begin();
  releaseAllInputs();
}

void loop() {
  if (hidEnabled && emergencyStopPressed()) {
    disableHid("EMERGENCY_STOP");
  }

  if (hidEnabled && millis() - lastHeartbeatMs > HEARTBEAT_TIMEOUT_MS) {
    disableHid("HEARTBEAT_TIMEOUT");
  }

  readSerialCommands();
}
