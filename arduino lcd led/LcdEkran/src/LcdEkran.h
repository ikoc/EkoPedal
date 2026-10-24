// LcdEkran: HW-61 (PCF8574) I2C adaptorlu karakter LCD icin kolay kullanim katmani.
// Baglanti (Uno/Nano): GND-GND, VCC-5V, SDA-A4, SCL-A5
#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

enum Hiza { SOL, ORTA, SAG };

class LcdEkran {
 public:
  LcdEkran(uint8_t sutun = 16, uint8_t satir = 2);

  // adres 0 ise once 0x27 ve 0x3F, sonra tum I2C hatti taranir.
  // Ekran bulunamazsa false doner.
  bool begin(uint8_t adres = 0);
  uint8_t adres() const { return _adres; }

  void temizle();
  void isik(bool acik);

  // Satiri tamamen yazar; metin kisaysa bosluklarla doldurur, uzunsa keser.
  void yaz(uint8_t satir, const char* metin, Hiza hiza = SOL);
  void yaz(uint8_t satir, const String& metin, Hiza hiza = SOL);

  // Bloklayan efektler: islem bitene kadar geri donmez.
  void yanipSon(uint8_t kez = 3, uint16_t ms = 300);
  void kayanYazi(uint8_t satir, const char* metin, uint16_t ms = 300);

  // Kutuphanede olmayan islemler icin alttaki LiquidCrystal_I2C nesnesi
  LiquidCrystal_I2C& ham() { return *_lcd; }

 private:
  static bool cevapVar(uint8_t adres);

  // Nesne begin() icinde bu alana kurulur (heap kullanilmaz)
  alignas(LiquidCrystal_I2C) uint8_t _depo[sizeof(LiquidCrystal_I2C)];
  LiquidCrystal_I2C* _lcd = nullptr;
  uint8_t _sutun;
  uint8_t _satir;
  uint8_t _adres = 0;
};
