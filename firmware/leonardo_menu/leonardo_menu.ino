#include <HID-Project.h>
#include <Wire.h>
#include <EEPROM.h>
#include <stddef.h>

// Firmware build: nested menus and eKO PEDAL splash.
// LCD edit labels identify the selected skill.
// Short pedal presses complete the two-key Minor sequence.
// Lock pedals trigger their macros directly for hosts without LED feedback.
// A second D4 press stops the direct-test loop.
// Default Minor timing is the fastest 25-minor preset.
// Minor labels map fastest-to-slowest: 25, 23, 21, 19.
// Ultra-fast timing: 1 ms key hold and 1 ms gap for 25 minor.
// Stable HID timing restored to 5 ms like the original working firmware.

const uint8_t FOOTSWITCH_PIN = 8;
const uint8_t MENU_BUTTON_PIN = 7;
const uint8_t ENCODER_A_PIN = 6;
const uint8_t ENCODER_B_PIN = 9;
const uint8_t NUM_LOCK_PEDAL_PIN = 5;
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
const unsigned long MOUSE_MOVE_TICK_MS = 12;
const unsigned long MOUSE_SETTLE_MS = 25;
const unsigned long ITEM_SETTLE_MS = 180;
const unsigned long MENU_HOLD_MS = 700;
const unsigned long MENU_DEBOUNCE_MS = 35;
const uint32_t SETTINGS_MAGIC = 0x4B4F4D31UL;
const uint8_t SETTINGS_VERSION = 9; // restore stable HID timing from original firmware

// Inventory positions in the supplied 1280x1024 screenshots.
const int BASE_SCREEN_WIDTH = 1280;
const int BASE_SCREEN_HEIGHT = 1024;
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

enum UiState { UI_IDLE, UI_MENU, UI_EDIT, UI_ACTIVE };
enum MenuItem { MENU_MINOR_GROUP, MENU_STYX_GROUP, MENU_BOW_GROUP, MENU_DEFAULTS, MENU_SAVE, MENU_COUNT, MENU_MINOR, MENU_MANA, MENU_HP, MENU_MINOR_DELAY, MENU_MOUSE, MENU_DISPLAY, MENU_FIVE_BOW, MENU_THREE_BOW, MENU_BOW_DELAY };
struct __attribute__((packed)) Settings {
  uint32_t magic;
  uint8_t version;
  uint8_t comboMode;  // 0=follow Caps Lock, 1=78, 2=28
  uint8_t resolution;
  uint8_t minorKey;
  uint8_t manaKey;
  uint8_t hpKey;
  uint8_t fiveBowKey;
  uint8_t threeBowKey;
  uint8_t minorDelayIndex;
  uint16_t mouseMoveMs;
  uint16_t styxDelayMs;
  uint16_t bowDelayMs;
  uint16_t checksum;
};
Settings settings = {SETTINGS_MAGIC, SETTINGS_VERSION, 0, 0, 8, 7, 2, 3, 4, 0, 40, 75, 0, 0};
UiState uiState = UI_IDLE;
uint8_t menuItem = MENU_MINOR;
uint8_t submenu = 0;
uint8_t submenuItem = 0;
bool settingsDirty = false;
bool menuRawPressed = false;
bool menuStablePressed = false;
bool menuLongDone = false;
unsigned long menuRawChangedMs = 0;
unsigned long menuPressedMs = 0;
bool comboMenuRaw = false;
bool comboMenuStable = false;
unsigned long comboMenuChangedMs = 0;
uint8_t encoderLastState = 0;
int8_t encoderQuarterSteps = 0;
uint8_t lcdAddress = 0;
bool lcdPresent = false;
char lcdWanted[2][17] = {{0}, {0}};
char lcdShown[2][17] = {{0}, {0}};
uint8_t lcdScanIndex = 0;

const char * const MENU_NAMES[] = {"MinorManaPot", "Styx ayarlari", "3lu5li okcu", "Varsayilan", "Kaydet/Geri"};
const char * const MINOR_NAMES[] = {"Minor tusu", "Mana tusu", "HP tusu", "Minor delay", "Kaydet/Geri"};
const char * const STYX_NAMES[] = {"Styx gecikme", "Cozunurluk", "Kaydet/Geri"};
const char * const BOW_NAMES[] = {"5li ok tusu", "3lu ok tusu", "Okcu delay", "Kaydet/Geri"};
const uint8_t MINOR_LABEL_VALUES[] = {25, 23, 21, 19};
const uint8_t MINOR_DELAY_MS[] = {5, 6, 7, 8};
const uint8_t RESOLUTION_COUNT = 13;
const uint16_t SCREEN_WIDTHS[RESOLUTION_COUNT] = {1024, 1280, 1280, 1280, 1366, 1440, 1600, 1680, 1920, 1920, 2048, 2560, 2560};
const uint16_t SCREEN_HEIGHTS[RESOLUTION_COUNT] = {768, 720, 800, 1024, 768, 900, 900, 1050, 1080, 1200, 1080, 1080, 1440};

bool armed = false;
bool wasPressed = false;
bool sequenceRunning = false;
bool pedalReleased = false;
bool keyHeld = false;
bool capsMode = false;
uint8_t stepIndex = 0;
unsigned long pressStartedMs = 0;
unsigned long releaseStartedMs = 0;
unsigned long keyPressedMs = 0;
unsigned long nextPressMs = 0;
bool numLockReady = false;
bool numLockWasOn = false;
bool scrollLockReady = false;
bool scrollLockWasOn = false;
bool scrollRunning = false;
uint8_t scrollIndex = 0;
unsigned long bootMs = 0;
unsigned long scrollNextMs = 0;
MacroStep macroStep = MACRO_IDLE;
MacroStep moveNextStep = MACRO_IDLE;
unsigned long macroNextMs = 0;
unsigned long moveStartedMs = 0;
unsigned long moveNextTickMs = 0;
int cursorX = BASE_SCREEN_WIDTH / 2;
int cursorY = BASE_SCREEN_HEIGHT / 2;
int moveStartX = 0;
int moveStartY = 0;
int moveTargetX = 0;
int moveTargetY = 0;
char pedalKey = 0;

void stopSequence();
void stopScrollSequence();
void showUi();
void startMacro(unsigned long now);
void startScrollAction(unsigned long now);

uint16_t settingsChecksum() {
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&settings);
  uint16_t sum = 0x4B4F;
  for (size_t i = 0; i < offsetof(Settings, checksum); ++i) {
    sum = static_cast<uint16_t>((sum << 5) | (sum >> 11)) ^ bytes[i];
  }
  return sum;
}

void setDefaultSettings() {
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  settings.comboMode = 0;
  settings.resolution = 0;
  settings.minorKey = 8;
  settings.manaKey = 7;
  settings.hpKey = 2;
  settings.fiveBowKey = 3;
  settings.threeBowKey = 4;
  settings.minorDelayIndex = 0;
  settings.mouseMoveMs = 40;
  settings.styxDelayMs = 75;
  settings.bowDelayMs = 0;
  settings.checksum = 0;
}

void loadSettings() {
  EEPROM.get(0, settings);
  if (settings.magic != SETTINGS_MAGIC || settings.version != SETTINGS_VERSION ||
      settings.comboMode > 2 || settings.resolution >= RESOLUTION_COUNT ||
      settings.minorKey > 9 || settings.manaKey > 9 || settings.hpKey > 9 ||
      settings.fiveBowKey > 9 || settings.threeBowKey > 9 ||
      settings.minorDelayIndex >= 4 ||
      settings.mouseMoveMs < 20 || settings.mouseMoveMs > 500 ||
      settings.styxDelayMs < 25 || settings.styxDelayMs > 500 ||
      settings.bowDelayMs > 200 ||
      settings.checksum != settingsChecksum()) {
    setDefaultSettings();
  }
}

void saveSettings() {
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  settings.checksum = settingsChecksum();
  EEPROM.put(0, settings);
  settingsDirty = false;
  Serial.println("SETTINGS_SAVED");
}

void lcdWritePort(uint8_t value) {
  if (!lcdPresent) return;
  Wire.beginTransmission(lcdAddress);
  Wire.write(value | 0x08); // HW-61: P3 controls backlight.
  if (Wire.endTransmission() != 0) lcdPresent = false;
}

void lcdNibble(uint8_t nibble, bool rs) {
  const uint8_t value = static_cast<uint8_t>((nibble << 4) | (rs ? 1 : 0));
  lcdWritePort(value | 0x04);
  delayMicroseconds(1);
  lcdWritePort(value);
  delayMicroseconds(50);
}

void lcdSend(uint8_t value, bool rs) {
  lcdNibble(value >> 4, rs);
  lcdNibble(value & 0x0F, rs);
}

void lcdInit() {
  Wire.begin();
  const uint8_t preferred[] = {0x27, 0x3F};
  for (uint8_t i = 0; i < 2 && !lcdPresent; ++i) {
    Wire.beginTransmission(preferred[i]);
    if (Wire.endTransmission() == 0) {
      lcdAddress = preferred[i];
      lcdPresent = true;
    }
  }
  for (uint8_t addr = 0x20; addr <= 0x3F && !lcdPresent; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      lcdAddress = addr;
      lcdPresent = true;
    }
  }
  if (!lcdPresent) {
    Serial.println("LCD_NOT_FOUND");
    return;
  }
  Serial.print("LCD_ADDRESS=0x");
  Serial.println(lcdAddress, HEX);
  delay(50);
  lcdNibble(3, false); delay(5);
  lcdNibble(3, false); delayMicroseconds(150);
  lcdNibble(3, false); lcdNibble(2, false);
  lcdSend(0x28, false); // 4 bit, two rows.
  lcdSend(0x0C, false); // Display on, cursor off.
  lcdSend(0x06, false);
  lcdSend(0x01, false);
  delay(2);
  for (uint8_t row = 0; row < 2; ++row)
    for (uint8_t col = 0; col < 16; ++col) lcdShown[row][col] = '\0';
}

void lcdSetLine(uint8_t row, const char *text) {
  bool ended = false;
  for (uint8_t col = 0; col < 16; ++col) {
    if (!ended && text[col] != '\0') lcdWanted[row][col] = text[col];
    else { ended = true; lcdWanted[row][col] = ' '; }
  }
  lcdWanted[row][16] = '\0';
}

void lcdFlushOne() {
  if (!lcdPresent) return;
  for (uint8_t scanned = 0; scanned < 32; ++scanned) {
    const uint8_t index = static_cast<uint8_t>((lcdScanIndex + scanned) % 32);
    const uint8_t row = index / 16;
    const uint8_t col = index % 16;
    if (lcdShown[row][col] != lcdWanted[row][col]) {
      lcdSend(static_cast<uint8_t>(0x80 | (row ? 0x40 : 0) | col), false);
      lcdSend(lcdWanted[row][col], true);
      lcdShown[row][col] = lcdWanted[row][col];
      lcdScanIndex = static_cast<uint8_t>((index + 1) % 32);
      return;
    }
  }
}

void showUi() {
  char top[17];
  char bottom[17];
  if (uiState == UI_IDLE) {
    snprintf(top, sizeof(top), "SECIM BEKLENIYOR");
    snprintf(bottom, sizeof(bottom), "BTN TUT: MENU");
  } else if (uiState == UI_ACTIVE) {
    snprintf(top, sizeof(top), "eKO PEDAL");
    snprintf(bottom, sizeof(bottom), "HAZIR");
  } else if (uiState == UI_MENU) {
    if (submenu == 1) {
      snprintf(top, sizeof(top), "MINOR %u/5", submenuItem + 1);
      snprintf(bottom, sizeof(bottom), "%s", MINOR_NAMES[submenuItem]);
    } else if (submenu == 2) {
      snprintf(top, sizeof(top), "STYX %u/3", submenuItem + 1);
      snprintf(bottom, sizeof(bottom), "%s", STYX_NAMES[submenuItem]);
    } else if (submenu == 3) {
      snprintf(top, sizeof(top), "OKCU %u/4", submenuItem + 1);
      snprintf(bottom, sizeof(bottom), "%s", BOW_NAMES[submenuItem]);
    } else {
      snprintf(top, sizeof(top), "MENU %u/%u", menuItem + 1, MENU_COUNT);
      snprintf(bottom, sizeof(bottom), "%s", MENU_NAMES[menuItem]);
    }
  } else {
    if (menuItem == MENU_MINOR) snprintf(top, sizeof(top), "MINOR TUSU");
    else if (menuItem == MENU_MANA) snprintf(top, sizeof(top), "MANA TUSU");
    else if (menuItem == MENU_HP) snprintf(top, sizeof(top), "HP TUSU");
    else if (menuItem == MENU_FIVE_BOW) snprintf(top, sizeof(top), "5LI OK TUSU");
    else if (menuItem == MENU_THREE_BOW) snprintf(top, sizeof(top), "3LU OK TUSU");
    else if (menuItem == MENU_MINOR_DELAY) snprintf(top, sizeof(top), "MINOR DELAY");
    else if (menuItem == MENU_BOW_DELAY) snprintf(top, sizeof(top), "OKCU DELAY");
    else if (menuItem == MENU_MOUSE) snprintf(top, sizeof(top), "STYX GECIKME");
    else snprintf(top, sizeof(top), "COZUNURLUK");
    if (menuItem == MENU_MINOR)
      snprintf(bottom, sizeof(bottom), "TUS: %u", settings.minorKey);
    else if (menuItem == MENU_MANA)
      snprintf(bottom, sizeof(bottom), "TUS: %u", settings.manaKey);
    else if (menuItem == MENU_HP)
      snprintf(bottom, sizeof(bottom), "TUS: %u", settings.hpKey);
    else if (menuItem == MENU_FIVE_BOW)
      snprintf(bottom, sizeof(bottom), "TUS: %u", settings.fiveBowKey);
    else if (menuItem == MENU_THREE_BOW)
      snprintf(bottom, sizeof(bottom), "TUS: %u", settings.threeBowKey);
    else if (menuItem == MENU_MINOR_DELAY)
      snprintf(bottom, sizeof(bottom), "%u minor", MINOR_LABEL_VALUES[settings.minorDelayIndex]);
    else if (menuItem == MENU_MOUSE)
      snprintf(bottom, sizeof(bottom), "%u ms", settings.mouseMoveMs);
    else if (menuItem == MENU_BOW_DELAY)
      snprintf(bottom, sizeof(bottom), "+%u ms", settings.bowDelayMs);
    else
      snprintf(bottom, sizeof(bottom), "%ux%u", SCREEN_WIDTHS[settings.resolution], SCREEN_HEIGHTS[settings.resolution]);
  }
  lcdSetLine(0, top);
  lcdSetLine(1, bottom);
  Serial.print("UI: "); Serial.print(top); Serial.print(" | "); Serial.println(bottom);
}

bool capsLockOn() {
  if (settings.comboMode == 1) return false;
  if (settings.comboMode == 2) return true;
  return (BootKeyboard.getLeds() & LED_CAPS_LOCK) != 0;
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
  pedalReleased = false;
  stepIndex = 0;
}

void enterMenu() {
  stopSequence();
  stopScrollSequence();
  BootKeyboard.releaseAll();
  if (macroStep != MACRO_IDLE) AbsoluteMouse.release(MOUSE_RIGHT);
  numLockPedal.keyHeld = false;
  scrollLockPedal.keyHeld = false;
  macroStep = MACRO_IDLE;
  uiState = UI_MENU;
  menuItem = MENU_MINOR_GROUP;
  submenu = 0;
  submenuItem = 0;
  setPedalLed(false);
  showUi();
}

void startActive() {
  BootKeyboard.releaseAll();
  armed = false;
  wasPressed = false;
  releaseStartedMs = millis();
  numLockPedal.armed = !numLockPedal.stablePressed;
  scrollLockPedal.armed = !scrollLockPedal.stablePressed;
  numLockWasOn = (BootKeyboard.getLeds() & LED_NUM_LOCK) != 0;
  scrollLockWasOn = (BootKeyboard.getLeds() & LED_SCROLL_LOCK) != 0;
  numLockReady = scrollLockReady = true;
  uiState = UI_ACTIVE;
  showUi();
}

void adjustSetting(int8_t delta) {
  if (menuItem == MENU_MINOR) settings.minorKey = static_cast<uint8_t>((settings.minorKey + 10 + delta) % 10);
  else if (menuItem == MENU_MANA) settings.manaKey = static_cast<uint8_t>((settings.manaKey + 10 + delta) % 10);
  else if (menuItem == MENU_HP) settings.hpKey = static_cast<uint8_t>((settings.hpKey + 10 + delta) % 10);
  else if (menuItem == MENU_FIVE_BOW) settings.fiveBowKey = static_cast<uint8_t>((settings.fiveBowKey + 10 + delta) % 10);
  else if (menuItem == MENU_THREE_BOW) settings.threeBowKey = static_cast<uint8_t>((settings.threeBowKey + 10 + delta) % 10);
  else if (menuItem == MENU_DISPLAY) settings.resolution = static_cast<uint8_t>((settings.resolution + RESOLUTION_COUNT + delta) % RESOLUTION_COUNT);
  else if (menuItem == MENU_MINOR_DELAY) settings.minorDelayIndex = static_cast<uint8_t>((settings.minorDelayIndex + 4 + delta) % 4);
  else if (menuItem == MENU_BOW_DELAY) {
    const int next = static_cast<int>(settings.bowDelayMs) + 10 * delta;
    settings.bowDelayMs = static_cast<uint16_t>(constrain(next, 0, 200));
  }
  else if (menuItem == MENU_MOUSE) {
    const int next = static_cast<int>(settings.mouseMoveMs) + 10 * delta;
    settings.mouseMoveMs = static_cast<uint16_t>(constrain(next, 20, 500));
  }
  settingsDirty = true;
  showUi();
}

void selectMenu() {
  if (uiState == UI_EDIT) {
    saveSettings();
    uiState = UI_MENU;
    showUi();
  } else if (uiState == UI_MENU) {
    if (submenu != 0) {
      const uint8_t itemCount = submenu == 1 ? 5 : (submenu == 2 ? 3 : 4);
      if (submenuItem == itemCount - 1) {
        if (settingsDirty) saveSettings();
        submenu = 0;
      } else if (submenu == 1 && submenuItem == 0) {
        menuItem = MENU_MINOR;
        uiState = UI_EDIT;
      } else if (submenu == 1 && submenuItem == 1) {
        menuItem = MENU_MANA;
        uiState = UI_EDIT;
      } else if (submenu == 1 && submenuItem == 2) {
        menuItem = MENU_HP;
        uiState = UI_EDIT;
      } else if (submenu == 1 && submenuItem == 3) {
        menuItem = MENU_MINOR_DELAY;
        uiState = UI_EDIT;
      } else if (submenu == 2 && submenuItem == 0) {
        menuItem = MENU_MOUSE;
        uiState = UI_EDIT;
      } else if (submenu == 2 && submenuItem == 1) {
        menuItem = MENU_DISPLAY;
        uiState = UI_EDIT;
      } else if (submenu == 3 && submenuItem == 0) {
        menuItem = MENU_FIVE_BOW;
        uiState = UI_EDIT;
      } else if (submenu == 3 && submenuItem == 1) {
        menuItem = MENU_THREE_BOW;
        uiState = UI_EDIT;
      } else if (submenu == 3 && submenuItem == 2) {
        menuItem = MENU_BOW_DELAY;
        uiState = UI_EDIT;
      }
      showUi();
    }
    else if (menuItem == MENU_MINOR_GROUP || menuItem == MENU_STYX_GROUP || menuItem == MENU_BOW_GROUP) {
      submenu = menuItem == MENU_MINOR_GROUP ? 1 : (menuItem == MENU_STYX_GROUP ? 2 : 3);
      submenuItem = 0;
      showUi();
    }
    else if (menuItem == MENU_DEFAULTS) {
      setDefaultSettings();
      settingsDirty = true;
      showUi();
    }
    else if (menuItem == MENU_SAVE) {
      if (settingsDirty) saveSettings();
      startActive();
    } else {
      uiState = UI_EDIT;
      showUi();
    }
  }
}

void menuNext() {
  if (uiState == UI_MENU) {
    if (submenu != 0) {
      const uint8_t itemCount = submenu == 1 ? 5 : (submenu == 2 ? 3 : 4);
      submenuItem = static_cast<uint8_t>((submenuItem + 1) % itemCount);
    }
    else menuItem = static_cast<uint8_t>((menuItem + 1) % MENU_COUNT);
    showUi();
  } else if (uiState == UI_EDIT) adjustSetting(1);
}

void menuPrevious() {
  if (uiState == UI_MENU) {
    if (submenu != 0) {
      const uint8_t itemCount = submenu == 1 ? 5 : (submenu == 2 ? 3 : 4);
      submenuItem = static_cast<uint8_t>((submenuItem + itemCount - 1) % itemCount);
    }
    else menuItem = static_cast<uint8_t>((menuItem + MENU_COUNT - 1) % MENU_COUNT);
    showUi();
  } else if (uiState == UI_EDIT) adjustSetting(-1);
}

void updateEncoder() {
  const uint8_t state = static_cast<uint8_t>((digitalRead(ENCODER_A_PIN) << 1) | digitalRead(ENCODER_B_PIN));
  if (state == encoderLastState) return;
  const uint8_t transition = static_cast<uint8_t>((encoderLastState << 2) | state);
  encoderLastState = state;
  int8_t direction = 0;
  if (transition == 0b0001 || transition == 0b0111 || transition == 0b1110 || transition == 0b1000) direction = 1;
  if (transition == 0b0010 || transition == 0b1011 || transition == 0b1101 || transition == 0b0100) direction = -1;
  if (!direction || (uiState != UI_MENU && uiState != UI_EDIT)) return;
  encoderQuarterSteps += direction;
  if (encoderQuarterSteps >= 4) { encoderQuarterSteps = 0; menuNext(); }
  if (encoderQuarterSteps <= -4) { encoderQuarterSteps = 0; menuPrevious(); }
}

void onMenuPedal(uint8_t pin) {
  if (uiState != UI_MENU && uiState != UI_EDIT) return;
  if (pin == FOOTSWITCH_PIN) menuPrevious();
  else if (pin == NUM_LOCK_PEDAL_PIN) menuNext();
  else if (pin == SCROLL_LOCK_PEDAL_PIN) selectMenu();
}

int16_t mouseCoordinate(int pixel, int size) {
  return static_cast<int16_t>((static_cast<int32_t>(pixel) * 65535L) / (size - 1) - 32768L);
}

void moveMouseTo(int x, int y) {
  const int width = SCREEN_WIDTHS[settings.resolution];
  const int height = SCREEN_HEIGHTS[settings.resolution];
  // Presets assume the inventory stays at the same right/top pixel offsets.
  const int targetX = constrain(width - (BASE_SCREEN_WIDTH - x), 0, width - 1);
  const int targetY = constrain(y, 0, height - 1);
  AbsoluteMouse.moveTo(mouseCoordinate(targetX, width), mouseCoordinate(targetY, height));
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
  if (elapsed >= settings.mouseMoveMs) {
    moveMouseTo(moveTargetX, moveTargetY);
    cursorX = moveTargetX;
    cursorY = moveTargetY;
    macroStep = moveNextStep;
    macroNextMs = now + MOUSE_SETTLE_MS;
    return;
  }

  const int32_t progress = (elapsed * 256L) / settings.mouseMoveMs;
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
  macroStep = MACRO_PRESS_OPEN;
  macroNextMs = now;
}

void updateMenuButton(unsigned long now) {
  const bool pressed = digitalRead(MENU_BUTTON_PIN) == LOW;
  if (pressed != menuRawPressed) {
    menuRawPressed = pressed;
    menuRawChangedMs = now;
  }
  if (now - menuRawChangedMs >= MENU_DEBOUNCE_MS && pressed != menuStablePressed) {
    menuStablePressed = pressed;
    if (pressed) {
      menuPressedMs = now;
      menuLongDone = false;
    } else if (!menuLongDone && (uiState == UI_MENU || uiState == UI_EDIT)) {
      menuNext();
    }
  }
  if (menuStablePressed && !menuLongDone && now - menuPressedMs >= MENU_HOLD_MS) {
    menuLongDone = true;
    if (uiState == UI_IDLE || uiState == UI_ACTIVE) enterMenu();
    else selectMenu();
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
      if (uiState != UI_ACTIVE) {
        onMenuPedal(pedal.pin);
        return;
      }
      BootKeyboard.press(pedal.key);
      Serial.print(pedal.pin == NUM_LOCK_PEDAL_PIN ? "D5 NUM_LOCK" : "D4 SCROLL_LOCK");
      Serial.println(" DOWN");
      pedal.keyHeld = true;
      pedal.keyPressedMs = now;
      if (pedal.pin == NUM_LOCK_PEDAL_PIN) startMacro(now);
      else if (pedal.pin == SCROLL_LOCK_PEDAL_PIN) {
        if (scrollRunning) stopScrollSequence();
        else startScrollAction(now);
      }
    }
  }

  if (pedal.keyHeld && now - pedal.keyPressedMs >= LOCK_KEY_HOLD_MS) {
    BootKeyboard.release(pedal.key);
    Serial.print(pedal.pin == NUM_LOCK_PEDAL_PIN ? "D5 NUM_LOCK" : "D4 SCROLL_LOCK");
    Serial.println(" UP");
    pedal.keyHeld = false;
  }
}

void updateNumLock(unsigned long now) {
  const bool on = (BootKeyboard.getLeds() & LED_NUM_LOCK) != 0;
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

char selectedScrollKey() {
  if (scrollIndex == 0) return static_cast<char>('0' + settings.fiveBowKey);
  if (scrollIndex == 4) return static_cast<char>('0' + settings.threeBowKey);
  return SCROLL_STEP_KEYS[scrollIndex];
}

void setScrollStep(bool pressed) {
  const char key = selectedScrollKey();
  if (key == 0) {
    return;
  }
  if (pressed) {
    BootKeyboard.press(key);
  } else {
    BootKeyboard.release(key);
  }
}

void startScrollAction(unsigned long now) {
  if (scrollRunning) return;
  scrollIndex = 0;
  setScrollStep(true);
  scrollNextMs = now + SCROLL_STEP_MS[scrollIndex] + settings.bowDelayMs;
  scrollRunning = true;
}

void stopScrollSequence() {
  if (scrollRunning) {
    setScrollStep(false);
    scrollRunning = false;
  }
}

void updateScrollLock(unsigned long now) {
  const bool on = (BootKeyboard.getLeds() & LED_SCROLL_LOCK) != 0;
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
      startScrollAction(now);
    } else {
      stopScrollSequence();
    }
  }

  if (scrollRunning && static_cast<int32_t>(now - scrollNextMs) >= 0) {
    setScrollStep(false);
    scrollIndex = (scrollIndex + 1) % 8;
    setScrollStep(true);
    scrollNextMs = now + SCROLL_STEP_MS[scrollIndex] + settings.bowDelayMs;
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
      macroNextMs = now + settings.styxDelayMs;
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
      macroNextMs = now + settings.styxDelayMs;
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
  loadSettings();
  pinMode(FOOTSWITCH_PIN, INPUT_PULLUP);
  pinMode(MENU_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  initLockPedal(numLockPedal);
  initLockPedal(scrollLockPedal);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ACTIVE_LED_PIN, OUTPUT);
  setPedalLed(false);
  menuRawPressed = menuStablePressed = digitalRead(MENU_BUTTON_PIN) == LOW;
  menuRawChangedMs = millis();
  encoderLastState = static_cast<uint8_t>((digitalRead(ENCODER_A_PIN) << 1) | digitalRead(ENCODER_B_PIN));
  comboMenuRaw = comboMenuStable = digitalRead(FOOTSWITCH_PIN) == LOW;
  comboMenuChangedMs = millis();
  bootMs = millis();
  BootKeyboard.begin();
  BootKeyboard.releaseAll();
  lcdInit();
  lcdSetLine(0, "eKO PEDAL");
  lcdSetLine(1, "Hazirlaniyor");
  delay(1200);
  startActive();
}

void updatePedal(unsigned long now) {
  const bool pressed = digitalRead(FOOTSWITCH_PIN) == LOW;
  if (uiState != UI_ACTIVE) {
    setPedalLed(false);
    if (pressed != comboMenuRaw) {
      comboMenuRaw = pressed;
      comboMenuChangedMs = now;
    }
    if (now - comboMenuChangedMs >= PRESS_DEBOUNCE_MS && pressed != comboMenuStable) {
      comboMenuStable = pressed;
      if (pressed) onMenuPedal(FOOTSWITCH_PIN);
    }
    return;
  }
  setPedalLed(pressed);

  if (!pressed) {
    if (wasPressed) {
      releaseStartedMs = now;
    }
    wasPressed = false;
    if (now - releaseStartedMs >= RELEASE_DEBOUNCE_MS) {
      armed = true;
    }
    pedalReleased = true;
    if (keyHeld) {
      BootKeyboard.release(pedalKey);
      keyHeld = false;
      nextPressMs = now + MINOR_DELAY_MS[settings.minorDelayIndex];
    }
    if (sequenceRunning && !keyHeld && stepIndex != 0 && static_cast<int32_t>(now - nextPressMs) >= 0) {
      pedalKey = static_cast<char>('0' + settings.minorKey);
      BootKeyboard.press(pedalKey);
      keyHeld = true;
      keyPressedMs = now;
      stepIndex = 0;
    }
    if (keyHeld && now - keyPressedMs >= KEY_HOLD_MS) {
      BootKeyboard.release(pedalKey);
      keyHeld = false;
    }
    if (sequenceRunning && stepIndex == 0 && !keyHeld) {
      stopSequence();
    }
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
    pedalReleased = false;
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
      nextPressMs = now + MINOR_DELAY_MS[settings.minorDelayIndex];
    }
    return;
  }

  if (static_cast<int32_t>(now - nextPressMs) >= 0) {
    const uint8_t selectedKey = stepIndex == 0
      ? (capsMode ? settings.hpKey : settings.manaKey)
      : settings.minorKey;
    pedalKey = static_cast<char>('0' + selectedKey);
    BootKeyboard.press(pedalKey);
    keyHeld = true;
    keyPressedMs = now;
    stepIndex = (stepIndex + 1) % 2;
  }
}

void loop() {
  const unsigned long now = millis();
  updateMenuButton(now);
  updateEncoder();
  updatePedal(now);
  updateLockPedal(numLockPedal, now);
  updateLockPedal(scrollLockPedal, now);
  if (uiState == UI_ACTIVE) {
    updateNumLock(now);
    updateScrollLock(now);
    updateMacro(now);
  }
  // Keep I2C transfers out of the validated 5 ms combo and macro timings.
  if (!sequenceRunning && !scrollRunning && macroStep == MACRO_IDLE) lcdFlushOne();
}
