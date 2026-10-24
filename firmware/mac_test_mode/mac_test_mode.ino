#include <HID-Project.h>

// Temporary Mac test build. LED states are simulated because macOS does not
// report its lock LEDs to this Leonardo. Inventory mouse actions are disabled.

const uint8_t FOOTSWITCH_PIN = 8;
const uint8_t MACRO_BUTTON_PIN = 2;
const uint8_t NUM_LOCK_PEDAL_PIN = 3;
const uint8_t SCROLL_LOCK_PEDAL_PIN = 4;
const uint8_t ACTIVE_LED_PIN = 10;

const unsigned long PRESS_DEBOUNCE_MS = 30;
const unsigned long RELEASE_DEBOUNCE_MS = 50;
const unsigned long KEY_HOLD_MS = 5;
const unsigned long KEY_GAP_MS = 5;
const unsigned long MACRO_DEBOUNCE_MS = 30;
const unsigned long LOCK_PEDAL_DEBOUNCE_MS = 30;
const unsigned long LOCK_KEY_HOLD_MS = 30;
const unsigned long NUM_LOCK_STARTUP_MS = 3000;
const unsigned long SCROLL_LOCK_STARTUP_MS = 3000;
const unsigned long MACRO_KEY_HOLD_MS = 30;
const unsigned long INVENTORY_OPEN_MS = 300;
const unsigned long MOUSE_CLICK_HOLD_MS = 40;
const unsigned long MOUSE_MOVE_MS = 40;
const unsigned long MOUSE_MOVE_TICK_MS = 12;
const unsigned long MOUSE_SETTLE_MS = 25;
const unsigned long ITEM_SETTLE_MS = 180;
const unsigned long DAGGER_WAIT_MS = 75;

// Inventory positions in the supplied 1280x1024 screenshots.
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 1024;
const int EQUIPPED_RIGHT_X = 1230;
const int EQUIPPED_RIGHT_Y = 273;
const int BAG_SLOT_1_X = 936;
const int BAG_SLOT_1_Y = 441;
const int BAG_SLOT_2_X = 985;
const int BAG_SLOT_2_Y = 441;

const char SCROLL_STEP_KEYS[] = {'3', 0, 'w', 0, '4', 0, 's', 0};
const uint16_t SCROLL_STEP_MS[] = {421, 32, 29, 0, 421, 0, 29, 32};

enum MacroStep {
  MACRO_IDLE,
  MACRO_PRESS_OPEN,
  MACRO_RELEASE_OPEN,
  MACRO_MOVE_EQUIPPED,
  MACRO_CLICK_EQUIPPED,
  MACRO_RELEASE_EQUIPPED,
  MACRO_MOVE_BOW,
  MACRO_CLICK_BOW,
  MACRO_RELEASE_BOW,
  MACRO_PRESS_ZERO,
  MACRO_RELEASE_ZERO,
  MACRO_MOVE_DAGGER_1,
  MACRO_CLICK_DAGGER_1,
  MACRO_RELEASE_DAGGER_1,
  MACRO_MOVE_DAGGER_2,
  MACRO_CLICK_DAGGER_2,
  MACRO_RELEASE_DAGGER_2,
  MACRO_PRESS_CLOSE,
  MACRO_RELEASE_CLOSE,
  MACRO_MOVING
};

struct LockPedal {
  uint8_t pin;
  KeyboardKeycode key;
  bool rawPressed;
  bool stablePressed;
  bool armed;
  bool keyHeld;
  unsigned long rawChangedMs;
  unsigned long keyPressedMs;
};

LockPedal numLockPedal = {NUM_LOCK_PEDAL_PIN, KEY_NUM_LOCK, false, false, false, false, 0, 0};
LockPedal scrollLockPedal = {SCROLL_LOCK_PEDAL_PIN, KEY_SCROLL_LOCK, false, false, false, false, 0, 0};

bool armed = false;
bool wasPressed = false;
bool sequenceRunning = false;
bool keyHeld = false;
bool capsMode = false;
uint8_t stepIndex = 0;
unsigned long pressStartedMs = 0;
unsigned long releaseStartedMs = 0;
unsigned long keyPressedMs = 0;
unsigned long nextPressMs = 0;
bool macroRawPressed = false;
bool macroStablePressed = false;
bool macroArmed = false;
bool numLockReady = false;
bool numLockWasOn = false;
bool scrollLockReady = false;
bool scrollLockWasOn = false;
bool testCapsOn = false;
bool testNumOn = false;
bool testScrollOn = false;
bool scrollRunning = false;
uint8_t scrollIndex = 0;
unsigned long macroRawChangedMs = 0;
unsigned long bootMs = 0;
unsigned long scrollNextMs = 0;
MacroStep macroStep = MACRO_IDLE;
MacroStep moveNextStep = MACRO_IDLE;
unsigned long macroNextMs = 0;
unsigned long moveStartedMs = 0;
unsigned long moveNextTickMs = 0;
int cursorX = SCREEN_WIDTH / 2;
int cursorY = SCREEN_HEIGHT / 2;
int moveStartX = 0;
int moveStartY = 0;
int moveTargetX = 0;
int moveTargetY = 0;
char pedalKey = 0;

bool capsLockOn() {
  return testCapsOn;
}

void setPedalLed(bool pressed) {
  digitalWrite(LED_BUILTIN, pressed ? HIGH : LOW);
  digitalWrite(ACTIVE_LED_PIN, pressed ? HIGH : LOW);
}

void stopSequence() {
  if (keyHeld) {
    BootKeyboard.release(pedalKey);
  }
  keyHeld = false;
  sequenceRunning = false;
  stepIndex = 0;
}

int16_t mouseCoordinate(int pixel, int size) {
  return static_cast<int16_t>((static_cast<int32_t>(pixel) * 65535L) / (size - 1) - 32768L);
}

void moveMouseTo(int x, int y) {
  AbsoluteMouse.moveTo(mouseCoordinate(x, SCREEN_WIDTH),
                       mouseCoordinate(y, SCREEN_HEIGHT));
}

void startMouseMove(int x, int y, MacroStep nextStep, unsigned long now) {
  moveStartX = cursorX;
  moveStartY = cursorY;
  moveTargetX = x;
  moveTargetY = y;
  moveNextStep = nextStep;
  moveStartedMs = now;
  moveNextTickMs = now;
  macroStep = MACRO_MOVING;
}

void updateMouseMove(unsigned long now) {
  if (static_cast<int32_t>(now - moveNextTickMs) < 0) {
    return;
  }
  const unsigned long elapsed = now - moveStartedMs;
  if (elapsed >= MOUSE_MOVE_MS) {
    moveMouseTo(moveTargetX, moveTargetY);
    cursorX = moveTargetX;
    cursorY = moveTargetY;
    macroStep = moveNextStep;
    macroNextMs = now + MOUSE_SETTLE_MS;
    return;
  }

  const int32_t progress = (elapsed * 256L) / MOUSE_MOVE_MS;
  const int32_t eased = (progress * progress * (768L - 2L * progress)) / 65536L;
  const int x = moveStartX + (static_cast<int32_t>(moveTargetX - moveStartX) * eased) / 256L;
  const int y = moveStartY + (static_cast<int32_t>(moveTargetY - moveStartY) * eased) / 256L;
  moveMouseTo(x, y);
  moveNextTickMs = now + MOUSE_MOVE_TICK_MS;
}

void startRightClick(MacroStep releaseStep, unsigned long now) {
  AbsoluteMouse.press(MOUSE_RIGHT);
  macroStep = releaseStep;
  macroNextMs = now + MOUSE_CLICK_HOLD_MS;
}

void startMacro(unsigned long now) {
  if (macroStep != MACRO_IDLE) {
    return;
  }
  // The inventory macro uses Windows game coordinates and right-clicks.
  // Confirm the trigger without moving or clicking the Mac pointer.
  Serial.println("INVENTORY_MACRO_TRIGGERED (mouse actions disabled)");
}

void updateMacroButton(unsigned long now) {
  const bool pressed = digitalRead(MACRO_BUTTON_PIN) == LOW;
  if (pressed != macroRawPressed) {
    macroRawPressed = pressed;
    macroRawChangedMs = now;
  }
  if (now - macroRawChangedMs < MACRO_DEBOUNCE_MS || pressed == macroStablePressed) {
    return;
  }
  macroStablePressed = pressed;
  if (!pressed) {
    macroArmed = true;
  } else if (macroArmed && macroStep == MACRO_IDLE) {
    macroArmed = false;
    startMacro(now);
  } else {
    macroArmed = false;
  }
}

void initLockPedal(LockPedal &pedal) {
  pinMode(pedal.pin, INPUT_PULLUP);
  pedal.rawPressed = pedal.stablePressed = digitalRead(pedal.pin) == LOW;
  pedal.armed = !pedal.stablePressed;
  pedal.rawChangedMs = millis();
}

void updateLockPedal(LockPedal &pedal, unsigned long now) {
  const bool pressed = digitalRead(pedal.pin) == LOW;
  if (pressed != pedal.rawPressed) {
    pedal.rawPressed = pressed;
    pedal.rawChangedMs = now;
  }

  if (now - pedal.rawChangedMs >= LOCK_PEDAL_DEBOUNCE_MS && pressed != pedal.stablePressed) {
    pedal.stablePressed = pressed;
    if (!pressed) {
      pedal.armed = true;
    } else if (pedal.armed) {
      pedal.armed = false;
      BootKeyboard.press(pedal.key);
      Serial.print(pedal.pin == NUM_LOCK_PEDAL_PIN ? "D3 NUM_LOCK" : "D4 SCROLL_LOCK");
      Serial.println(" DOWN");
      if (pedal.pin == NUM_LOCK_PEDAL_PIN) {
        testNumOn = !testNumOn;
        Serial.println(testNumOn ? "TEST_NUM=ON" : "TEST_NUM=OFF");
      } else {
        testScrollOn = !testScrollOn;
        Serial.println(testScrollOn ? "TEST_SCROLL=ON" : "TEST_SCROLL=OFF");
      }
      pedal.keyHeld = true;
      pedal.keyPressedMs = now;
    }
  }

  if (pedal.keyHeld && now - pedal.keyPressedMs >= LOCK_KEY_HOLD_MS) {
    BootKeyboard.release(pedal.key);
    Serial.print(pedal.pin == NUM_LOCK_PEDAL_PIN ? "D3 NUM_LOCK" : "D4 SCROLL_LOCK");
    Serial.println(" UP");
    pedal.keyHeld = false;
  }
}

void updateNumLock(unsigned long now) {
  const bool on = testNumOn;
  if (!numLockReady) {
    numLockWasOn = on;
    if (now - bootMs >= NUM_LOCK_STARTUP_MS) {
      numLockReady = true;
    }
    return;
  }
  if (on != numLockWasOn) {
    numLockWasOn = on;
    startMacro(now);
  }
}

void setScrollStep(bool pressed) {
  const char key = SCROLL_STEP_KEYS[scrollIndex];
  if (key == 0) {
    return;
  }
  if (pressed) {
    BootKeyboard.press(key);
  } else {
    BootKeyboard.release(key);
  }
}

void stopScrollSequence() {
  if (scrollRunning) {
    setScrollStep(false);
    scrollRunning = false;
  }
}

void updateScrollLock(unsigned long now) {
  const bool on = testScrollOn;
  if (!scrollLockReady) {
    scrollLockWasOn = on;
    if (now - bootMs >= SCROLL_LOCK_STARTUP_MS) {
      scrollLockReady = true;
    }
    return;
  }

  if (on != scrollLockWasOn) {
    scrollLockWasOn = on;
    if (on) {
      Serial.println("SCROLL_SEQUENCE_STARTED");
      scrollIndex = 0;
      setScrollStep(true);
      scrollNextMs = now + SCROLL_STEP_MS[scrollIndex];
      scrollRunning = true;
    } else {
      stopScrollSequence();
      Serial.println("SCROLL_SEQUENCE_STOPPED");
    }
  }

  if (scrollRunning && static_cast<int32_t>(now - scrollNextMs) >= 0) {
    setScrollStep(false);
    scrollIndex = (scrollIndex + 1) % 8;
    setScrollStep(true);
    scrollNextMs = now + SCROLL_STEP_MS[scrollIndex];
  }
}

void updateMacro(unsigned long now) {
  if (macroStep == MACRO_MOVING) {
    updateMouseMove(now);
    return;
  }
  if (macroStep == MACRO_IDLE || static_cast<int32_t>(now - macroNextMs) < 0) {
    return;
  }

  switch (macroStep) {
    case MACRO_PRESS_OPEN:
      BootKeyboard.press('i');
      macroStep = MACRO_RELEASE_OPEN;
      macroNextMs = now + MACRO_KEY_HOLD_MS;
      break;
    case MACRO_RELEASE_OPEN:
      BootKeyboard.release('i');
      macroStep = MACRO_MOVE_EQUIPPED;
      macroNextMs = now + INVENTORY_OPEN_MS;
      break;
    case MACRO_MOVE_EQUIPPED:
      AbsoluteMouse.begin();
      moveMouseTo(EQUIPPED_RIGHT_X, EQUIPPED_RIGHT_Y);
      cursorX = EQUIPPED_RIGHT_X;
      cursorY = EQUIPPED_RIGHT_Y;
      macroStep = MACRO_CLICK_EQUIPPED;
      macroNextMs = now + MOUSE_SETTLE_MS;
      break;
    case MACRO_CLICK_EQUIPPED:
      startRightClick(MACRO_RELEASE_EQUIPPED, now);
      break;
    case MACRO_RELEASE_EQUIPPED:
      AbsoluteMouse.release(MOUSE_RIGHT);
      macroStep = MACRO_MOVE_BOW;
      macroNextMs = now + ITEM_SETTLE_MS;
      break;
    case MACRO_MOVE_BOW:
      startMouseMove(BAG_SLOT_1_X, BAG_SLOT_1_Y, MACRO_CLICK_BOW, now);
      break;
    case MACRO_CLICK_BOW:
      startRightClick(MACRO_RELEASE_BOW, now);
      break;
    case MACRO_RELEASE_BOW:
      AbsoluteMouse.release(MOUSE_RIGHT);
      macroStep = MACRO_PRESS_ZERO;
      macroNextMs = now + ITEM_SETTLE_MS;
      break;
    case MACRO_PRESS_ZERO:
      BootKeyboard.press('0');
      macroStep = MACRO_RELEASE_ZERO;
      macroNextMs = now + MACRO_KEY_HOLD_MS;
      break;
    case MACRO_RELEASE_ZERO:
      BootKeyboard.release('0');
      macroStep = MACRO_MOVE_DAGGER_1;
      macroNextMs = now + DAGGER_WAIT_MS;
      break;
    case MACRO_MOVE_DAGGER_1:
      startMouseMove(BAG_SLOT_1_X, BAG_SLOT_1_Y, MACRO_CLICK_DAGGER_1, now);
      break;
    case MACRO_CLICK_DAGGER_1:
      startRightClick(MACRO_RELEASE_DAGGER_1, now);
      break;
    case MACRO_RELEASE_DAGGER_1:
      AbsoluteMouse.release(MOUSE_RIGHT);
      macroStep = MACRO_MOVE_DAGGER_2;
      macroNextMs = now + DAGGER_WAIT_MS;
      break;
    case MACRO_MOVE_DAGGER_2:
      startMouseMove(BAG_SLOT_2_X, BAG_SLOT_2_Y, MACRO_CLICK_DAGGER_2, now);
      break;
    case MACRO_CLICK_DAGGER_2:
      startRightClick(MACRO_RELEASE_DAGGER_2, now);
      break;
    case MACRO_RELEASE_DAGGER_2:
      AbsoluteMouse.release(MOUSE_RIGHT);
      macroStep = MACRO_PRESS_CLOSE;
      macroNextMs = now + ITEM_SETTLE_MS;
      break;
    case MACRO_PRESS_CLOSE:
      BootKeyboard.press('i');
      macroStep = MACRO_RELEASE_CLOSE;
      macroNextMs = now + MACRO_KEY_HOLD_MS;
      break;
    case MACRO_RELEASE_CLOSE:
      BootKeyboard.release('i');
      macroStep = MACRO_IDLE;
      break;
    case MACRO_IDLE:
    case MACRO_MOVING:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FOOTSWITCH_PIN, INPUT_PULLUP);
  pinMode(MACRO_BUTTON_PIN, INPUT_PULLUP);
  initLockPedal(numLockPedal);
  initLockPedal(scrollLockPedal);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ACTIVE_LED_PIN, OUTPUT);
  setPedalLed(false);
  macroRawPressed = macroStablePressed = digitalRead(MACRO_BUTTON_PIN) == LOW;
  macroArmed = !macroStablePressed;
  macroRawChangedMs = millis();
  bootMs = millis();
  BootKeyboard.begin();
  BootKeyboard.releaseAll();
  Serial.println("MAC_TEST_READY: D3 toggles Num, D4 toggles Scroll; send c for Caps");
}

void updateTestCommands() {
  while (Serial.available() > 0) {
    const char command = Serial.read();
    if (command == 'c' || command == 'C') {
      testCapsOn = !testCapsOn;
      Serial.println(testCapsOn ? "TEST_CAPS=ON (D8 sends 28)" : "TEST_CAPS=OFF (D8 sends 78)");
    }
  }
}

void updatePedal(unsigned long now) {
  const bool pressed = digitalRead(FOOTSWITCH_PIN) == LOW;
  setPedalLed(pressed);

  if (!pressed) {
    if (wasPressed) {
      releaseStartedMs = now;
    }
    wasPressed = false;
    if (now - releaseStartedMs >= RELEASE_DEBOUNCE_MS) {
      armed = true;
    }
    stopSequence();
    return;
  }

  if (!wasPressed) {
    pressStartedMs = now;
  }
  wasPressed = true;
  if (!sequenceRunning) {
    if (!armed || now - pressStartedMs < PRESS_DEBOUNCE_MS) {
      return;
    }
    armed = false;
    sequenceRunning = true;
    capsMode = capsLockOn();
    stepIndex = 0;
    nextPressMs = now;
  }

  const bool newCapsMode = capsLockOn();
  if (newCapsMode != capsMode) {
    if (keyHeld) {
      BootKeyboard.release(pedalKey);
    }
    keyHeld = false;
    capsMode = newCapsMode;
    stepIndex = 0;
    nextPressMs = now;
  }

  if (keyHeld) {
    if (now - keyPressedMs >= KEY_HOLD_MS) {
      BootKeyboard.release(pedalKey);
      keyHeld = false;
      nextPressMs = now + KEY_GAP_MS;
    }
    return;
  }

  if (static_cast<int32_t>(now - nextPressMs) >= 0) {
    pedalKey = stepIndex == 0 ? (capsMode ? '2' : '7') : '8';
    BootKeyboard.press(pedalKey);
    keyHeld = true;
    keyPressedMs = now;
    stepIndex = (stepIndex + 1) % 2;
  }
}

void loop() {
  const unsigned long now = millis();
  updateTestCommands();
  updatePedal(now);
  updateMacroButton(now);
  updateLockPedal(numLockPedal, now);
  updateLockPedal(scrollLockPedal, now);
  updateNumLock(now);
  updateScrollLock(now);
  updateMacro(now);
}
