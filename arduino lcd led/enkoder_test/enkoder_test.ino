// EC11 doner enkoder testi: cevirince sayi artar/azalir, basinca sifirlanir.
// Baglanti: A-D2, orta-GND, B-D3, SW-D4, SW diger-GND. LCD: SDA-A4, SCL-A5.
#include <LcdEkran.h>

const uint8_t PIN_A = 2, PIN_B = 3, PIN_SW = 4;
const int8_t ADIM_BOLEN = 4;  // bir "tik"teki gecis sayisi; sayi 2'ser atlarsa 2 yap

LcdEkran lcd;
volatile int16_t gecis = 0;   // ham dortlu gecis sayaci
volatile uint8_t onceki = 0;

// Gecerli gecisler +1/-1, zipzip (gurultu) 0
const int8_t TABLO[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

void enkoderOku() {
  uint8_t simdi = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);
  gecis += TABLO[(onceki << 2) | simdi];
  onceki = simdi;
}

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);
  onceki = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);
  attachInterrupt(digitalPinToInterrupt(PIN_A), enkoderOku, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), enkoderOku, CHANGE);

  Serial.begin(115200);
  if (!lcd.begin()) Serial.println("LCD bulunamadi");
  lcd.yaz(0, "Enkoder testi", ORTA);
}

void loop() {
  static int16_t gosterilen = 32767;
  static bool butonEski = HIGH;
  static uint32_t butonZaman = 0;

  noInterrupts();
  int16_t g = gecis;
  interrupts();
  int16_t deger = g / ADIM_BOLEN;

  // Buton: basinca sifirla (30 ms parazit filtresi)
  bool buton = digitalRead(PIN_SW);
  if (buton != butonEski && millis() - butonZaman > 30) {
    butonZaman = millis();
    butonEski = buton;
    if (buton == LOW) {
      noInterrupts();
      gecis = 0;
      interrupts();
      deger = 0;
      gosterilen = 32767;  // ekrani yenilemeye zorla
      Serial.println("Buton: sifirlandi");
    }
  }

  if (deger != gosterilen) {
    gosterilen = deger;
    lcd.yaz(1, "Deger: " + String(deger), ORTA);
    Serial.println(deger);
  }
}
