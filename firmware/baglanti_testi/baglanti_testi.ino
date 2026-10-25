// EkoPedal bağlantı testi: HİÇBİR TUŞ GÖNDERMEZ. Her girişin durumunu LCD'de ve seri portta gösterir.
//
// LCD satır 1:  1:X 2:- 3:- ACIK     (pedal basılı = X, anahtar ACIK/KAPL)
// LCD satır 2:  ENC:+003 BTN:X       (enkoder adım sayısı, enkoder butonu)
// Her değişiklik seri porta da yazılır (115200): "P1 BASILDI", "ENC +1", "ANAHTAR KAPALI" ...
//
// Pinler (firmware/leonardo_menu ile aynı):
//   Pedal 1 D8, Pedal 2 D5, Pedal 3 D4, açma/kapama A0, enkoder A D6 / B D9 / buton D7,
//   LCD SDA D2 / SCL D3. Hepsinin diğer ucu GND (INPUT_PULLUP: basılı/kapalı = LOW).
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <new.h>

const uint8_t PIN_P1 = 8, PIN_P2 = 5, PIN_P3 = 4;
const uint8_t PIN_SW = A0;
const uint8_t PIN_ENC_A = 6, PIN_ENC_B = 9, PIN_ENC_BTN = 7;
const unsigned long DEBOUNCE_MS = 20;

struct Giris { uint8_t pin; const char *ad; bool ham; bool kararli; unsigned long degisti; };
Giris girisler[] = {
  {PIN_P1, "P1", false, false, 0},
  {PIN_P2, "P2", false, false, 0},
  {PIN_P3, "P3", false, false, 0},
  {PIN_SW, "ANAHTAR", false, false, 0},
  {PIN_ENC_BTN, "ENC BUTON", false, false, 0},
};
const uint8_t GIRIS_SAYISI = sizeof(girisler) / sizeof(girisler[0]);

LiquidCrystal_I2C *lcd = nullptr;
uint8_t lcdDepo[sizeof(LiquidCrystal_I2C)];
uint8_t encEski = 0;
int8_t encCeyrek = 0;
int16_t encAdim = 0;
bool ekranKirli = true;

uint8_t lcdAdresiBul() {
  const uint8_t once[] = {0x27, 0x3F};
  for (uint8_t a : once) { Wire.beginTransmission(a); if (Wire.endTransmission() == 0) return a; }
  for (uint8_t a = 0x20; a < 0x40; a++) { Wire.beginTransmission(a); if (Wire.endTransmission() == 0) return a; }
  return 0;
}

bool basili(uint8_t pin) { return digitalRead(pin) == LOW; }

void ekraniYaz() {
  char ust[17], alt[17];
  snprintf(ust, sizeof(ust), "1:%c 2:%c 3:%c %s",
           girisler[0].kararli ? 'X' : '-', girisler[1].kararli ? 'X' : '-',
           girisler[2].kararli ? 'X' : '-', girisler[3].kararli ? "ACIK" : "KAPL");
  snprintf(alt, sizeof(alt), "ENC:%+04d BTN:%c", encAdim, girisler[4].kararli ? 'X' : '-');
  if (lcd) {
    lcd->setCursor(0, 0); lcd->print(ust);
    lcd->setCursor(0, 1); lcd->print(alt); lcd->print("  ");
  }
  ekranKirli = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  for (uint8_t i = 0; i < GIRIS_SAYISI; i++) {
    pinMode(girisler[i].pin, INPUT_PULLUP);
    girisler[i].ham = girisler[i].kararli = basili(girisler[i].pin);
  }
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  encEski = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);

  Wire.begin();
  const uint8_t adr = lcdAdresiBul();
  if (adr) {
    lcd = new (lcdDepo) LiquidCrystal_I2C(adr, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->setCursor(0, 0); lcd->print("BAGLANTI TESTI");
    lcd->setCursor(0, 1); lcd->print("tus gondermez");
    delay(1200);
    lcd->clear();
  }
  Serial.print("BAGLANTI TESTI, LCD adresi 0x"); Serial.println(adr, HEX);
}

void loop() {
  const unsigned long simdi = millis();

  // Pedallar, anahtar, enkoder butonu
  for (uint8_t i = 0; i < GIRIS_SAYISI; i++) {
    Giris &g = girisler[i];
    const bool b = basili(g.pin);
    if (b != g.ham) { g.ham = b; g.degisti = simdi; }
    if (simdi - g.degisti >= DEBOUNCE_MS && b != g.kararli) {
      g.kararli = b;
      ekranKirli = true;
      Serial.print(g.ad);
      if (g.pin == PIN_SW) Serial.println(b ? " ACIK" : " KAPALI");
      else Serial.println(b ? " BASILDI" : " BIRAKILDI");
    }
  }

  // Enkoder dönüşü (4 çeyrek adım = 1 tık)
  const uint8_t enc = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  if (enc != encEski) {
    const uint8_t gecis = (encEski << 2) | enc;
    encEski = enc;
    int8_t yon = 0;
    if (gecis == 0b0001 || gecis == 0b0111 || gecis == 0b1110 || gecis == 0b1000) yon = 1;
    if (gecis == 0b0010 || gecis == 0b1011 || gecis == 0b1101 || gecis == 0b0100) yon = -1;
    encCeyrek += yon;
    if (encCeyrek >= 4) { encCeyrek = 0; encAdim++; ekranKirli = true; Serial.println("ENC +1 (saat yonu)"); }
    if (encCeyrek <= -4) { encCeyrek = 0; encAdim--; ekranKirli = true; Serial.println("ENC -1 (ters)"); }
  }

  bool herhangi = false;
  for (uint8_t i = 0; i < 3; i++) herhangi |= girisler[i].kararli;
  digitalWrite(LED_BUILTIN, herhangi || girisler[4].kararli);

  if (ekranKirli) ekraniYaz();
}
