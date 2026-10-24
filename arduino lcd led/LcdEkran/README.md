# LcdEkran

HW-61 (PCF8574) I2C adaptörlü 16x2 karakter LCD için kolay kullanım katmanı. `LiquidCrystal I2C` kütüphanesine dayanır.

## Bağlantı (Uno / Nano)

| Adaptör | Arduino |
|---|---|
| GND | GND |
| VCC | 5V |
| SDA | A4 |
| SCL | A5 |

## Kullanım

```cpp
#include <LcdEkran.h>

LcdEkran lcd;            // 16x2; 20x4 icin LcdEkran lcd(20, 4);

void setup() {
  lcd.begin();           // adresi kendisi bulur (0x27 / 0x3F / tarama)
  lcd.yaz(0, "Merhaba!", ORTA);
}

void loop() {}
```

| Fonksiyon | İş |
|---|---|
| `begin(adres = 0)` | Ekranı başlatır; 0 ise adresi bulur. Ekran yoksa `false` |
| `adres()` | Bulunan I2C adresi |
| `yaz(satir, metin, hiza)` | Satırı baştan sona yazar (`SOL` / `ORTA` / `SAG`), artanı siler |
| `temizle()` | Ekranı siler |
| `isik(acik)` | Arka ışığı açar / kapatır |
| `yanipSon(kez, ms)` | Arka ışığı yakıp söndürür (bloklar) |
| `kayanYazi(satir, metin, ms)` | Metni sağdan sola kaydırır (bloklar) |
| `ham()` | Alttaki `LiquidCrystal_I2C` nesnesi (özel karakter vb. için) |

## Örnekler

Arduino IDE → Dosya → Örnekler → LcdEkran: `Merhaba`, `Efektler`, `I2CTarayici`.

## Kurulum

Bu klasörü `~/Documents/Arduino/libraries/LcdEkran` altına koy (veya sembolik bağ ver) ve
`LiquidCrystal I2C` kütüphanesini kur:

```bash
arduino-cli lib install "LiquidCrystal I2C"
```
