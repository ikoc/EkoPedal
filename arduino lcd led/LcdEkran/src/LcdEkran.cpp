#include "LcdEkran.h"
#include <Wire.h>
#include <new.h>

LcdEkran::LcdEkran(uint8_t sutun, uint8_t satir) : _sutun(sutun), _satir(satir) {}

bool LcdEkran::cevapVar(uint8_t adres) {
  Wire.beginTransmission(adres);
  return Wire.endTransmission() == 0;
}

bool LcdEkran::begin(uint8_t adres) {
  Wire.begin();
  if (adres == 0) {
    if (cevapVar(0x27)) adres = 0x27;
    else if (cevapVar(0x3F)) adres = 0x3F;
    else {
      for (uint8_t a = 1; a < 127 && adres == 0; a++) {
        if (cevapVar(a)) adres = a;
      }
    }
  }
  if (adres == 0 || !cevapVar(adres)) return false;

  // LiquidCrystal_I2C adresi yalnizca kurucuda aldigi icin nesne burada kurulur.
  // Kaynak tutmadigindan eskisinin uzerine yeniden kurmak guvenli.
  if (_lcd == nullptr || adres != _adres) {
    _lcd = new (_depo) LiquidCrystal_I2C(adres, _sutun, _satir);
  }
  _adres = adres;
  _lcd->init();
  _lcd->backlight();
  _lcd->clear();
  return true;
}

void LcdEkran::temizle() {
  if (_lcd) _lcd->clear();
}

void LcdEkran::isik(bool acik) {
  if (!_lcd) return;
  if (acik) _lcd->backlight();
  else _lcd->noBacklight();
}

void LcdEkran::yaz(uint8_t satir, const char* metin, Hiza hiza) {
  if (!_lcd || satir >= _satir) return;
  uint8_t uzunluk = min((size_t)_sutun, strlen(metin));
  uint8_t bas = 0;
  if (hiza == ORTA) bas = (_sutun - uzunluk) / 2;
  else if (hiza == SAG) bas = _sutun - uzunluk;

  _lcd->setCursor(0, satir);
  for (uint8_t i = 0; i < _sutun; i++) {
    bool metinde = i >= bas && i < bas + uzunluk;
    _lcd->write(metinde ? metin[i - bas] : ' ');
  }
}

void LcdEkran::yaz(uint8_t satir, const String& metin, Hiza hiza) {
  yaz(satir, metin.c_str(), hiza);
}

void LcdEkran::yanipSon(uint8_t kez, uint16_t ms) {
  for (uint8_t i = 0; i < kez; i++) {
    isik(false);
    delay(ms);
    isik(true);
    delay(ms);
  }
}

void LcdEkran::kayanYazi(uint8_t satir, const char* metin, uint16_t ms) {
  if (!_lcd || satir >= _satir) return;
  // Metin sagdan girip soldan tamamen cikana kadar kaydirilir
  int uzunluk = strlen(metin);
  for (int kayma = -(int)_sutun; kayma <= uzunluk; kayma++) {
    _lcd->setCursor(0, satir);
    for (uint8_t i = 0; i < _sutun; i++) {
      int j = kayma + i;
      _lcd->write(j >= 0 && j < uzunluk ? metin[j] : ' ');
    }
    delay(ms);
  }
}
