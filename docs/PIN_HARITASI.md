# EkoPedal – Pin haritası (doğrulandı)

Son doğrulama: **25 Ekim 2026**, `firmware/baglanti_testi` ile, tüm parçalar aynı anda bağlıyken.
Kart: **Arduino Leonardo** (ATmega32U4). Firmware: `firmware/leonardo_menu/leonardo_menu.ino`.

Tüm girişler `INPUT_PULLUP`: basılı / kapalı = LOW. Harici direnç gerekmez.

## Leonardo pinleri

| Leonardo | Parça | Parçadaki uç | Firmware'deki adı |
|---|---|---|---|
| **D2 (SDA)** | LCD 1602 + HW-61 | SDA | I2C |
| **D3 (SCL)** | LCD 1602 + HW-61 | SCL | I2C |
| **D4** | Pedal 3 – okçu (Scroll Lock) | GX16-3 sinyal | `SCROLL_LOCK_PEDAL_PIN` |
| **D5** | Pedal 2 – Styx (Num Lock) | GX16-2 sinyal | `NUM_LOCK_PEDAL_PIN` |
| **D6** | Döner enkoder EC11 | A (3'lü taraf, sol) | `ENCODER_A_PIN` |
| **D7** | Enkoder butonu | 2'li taraf, herhangi bir bacak | `MENU_BUTTON_PIN` |
| **D8** | Pedal 1 – Minor | GX16-1 sinyal | `FOOTSWITCH_PIN` |
| **D9** | Döner enkoder EC11 | B (3'lü taraf, sağ) | `ENCODER_B_PIN` |
| **D10** | (opsiyonel) aktif LED çıkışı | – | `ACTIVE_LED_PIN` |
| **A0** | Açma/kapama anahtarı (mini rocker) | herhangi bir bacak | `POWER_SWITCH_PIN` |
| **5V** | LCD | HW-61 VCC | – |
| **GND** | Ortak GND hattı | aşağıdaki 7 uç | – |

**5V'a yalnız LCD VCC bağlanır.** D3 LCD'nin SCL hattıdır; pedal veya başka bir şey bağlama.

Boş pinler: D0, D1, D11, D12, D13 (dahili LED), A1–A5.

## Ortak GND hattı (7 uç → tek nokta → Leonardo GND)

| # | Parça | Uç |
|---|---|---|
| 1 | LCD | HW-61 GND |
| 2 | Enkoder | 3'lü taraf **orta** bacak (C) |
| 3 | Enkoder butonu | 2'li tarafın diğer bacağı |
| 4 | Açma/kapama | diğer bacak |
| 5 | Pedal 1 | GX16-1 GND |
| 6 | Pedal 2 | GX16-2 GND |
| 7 | Pedal 3 | GX16-3 GND |

## Döner enkoder (EC11, basmalı, düz mil) bacakları

Mili üstten görerek, **3 bacaklı taraf kendine doğru**:

```text
        (2'li taraf: SW ve SW2, karşıda)
          ┌───────────┐
          │     ●     │  ← mil
          └───────────┘
            │   │   │
            A   C   B
           D6  GND  D9
```

- 2'li taraf basit bir buton: yönü yok, bir bacak D7, diğeri GND.
- Saat yönünde çevirince sayaç **artar** (doğrulandı). Ters sayarsa D6 ↔ D9 değiştir.
- Yan metal tırnaklar montaj içindir, bağlanmaz.

## Pedal konnektörleri (GX16, 3 pin)

- Her pedalın iki teli (anahtar kontağı) konnektörün iki pinine gelir, o iki pinin karşılığı Leonardo'da
  **sinyal pini** ve **GND**'ye gider. Üçüncü pin boş.
- Üç konnektörde **aynı pin düzenini** kullan (ör. pin 1 = sinyal, pin 3 = GND); pedallar birbirinin yerine takılabilir.
- Pedalda 3 tel varsa (COM / NO / NC): **COM ve NO** kullanılır, NC boş. Multimetre bip modunda: serbestken ötmeyen,
  basınca öten çift COM–NO'dur.
- Kutunun arka duvarındaki 1-2-3 yazıları: GX16-1 = Pedal 1 (D8), GX16-2 = Pedal 2 (D5), GX16-3 = Pedal 3 (D4).

## Pedal davranışı (ana firmware)

| Pedal | Pin | Basınca |
|---|---|---|
| Pedal 1 | D8 | Basılı tuttukça Mana/HP + Minor dizisi: Caps kapalı `7878…`, Caps açık `2828…` |
| Pedal 2 | D5 | Styx makrosu: `i`, fare ile sağ tıklar, `0`, `i` (≈1–2 sn, kendiliğinden biter) |
| Pedal 3 | D4 | İlk basış `3w4s` döngüsünü başlatır, ikinci basış durdurur |

## Açma/kapama anahtarı (yazılımsal, A0)

| Konum | Davranış | LCD |
|---|---|---|
| **I** (kontak kapalı) | Normal çalışma | `eKO PEDAL / HAZIR` |
| **O** (kontak açık) | Çalışan makro durur, basılı tuş/fare bırakılır, pedal/menü hiçbir şey göndermez | `eKO PEDAL / KAPALI` |

Açılınca o an basılı pedal önce bırakılmadan tuş göndermez. Anahtar bağlı değilse A0 HIGH kalır, cihaz KAPALI görünür.

## LCD (1602 + HW-61)

- I2C adresi **0x27** (firmware 0x27, 0x3F, sonra 0x20–0x3F tarar).
- Üst satırda dolu kareler + yazı yok = LCD güç alıyor ama komut almıyor. 25 Eki'de sebep **HW-61 ile LCD arasındaki
  lehim**di (I2C taramasında 0x27 görünmesine rağmen). Lehim yenilenince düzeldi.
- Kontrast: HW-61 arkasındaki mavi vida.

## Test

`firmware/baglanti_testi/baglanti_testi.ino` – **hiç tuş göndermez**; LCD'de canlı durum gösterir:

```text
1:X 2:- 3:- ACIK      ← pedal 1/2/3 (X = basılı), anahtar ACIK/KAPL
ENC:+003 BTN:-        ← enkoder adım sayısı, enkoder butonu
```

Her değişiklik seri porta da yazılır (115200): `P1 BASILDI`, `ENC +1 (saat yonu)`, `ANAHTAR KAPALI` …
Lehimden sonra ve kutu kapatılmadan önce bununla kontrol et; sonra `leonardo_menu` yüklenir.

## Kablolama önerileri

- Leonardo'ya lehim yapma: kablo uçlarına erkek dupont pin, Leonardo'nun dişi soketlerine tak.
- Lehim parçaların ucunda (GX16 kulakları, anahtar, enkoder); her noktaya makaron.
- Paneldeki parçalara (LCD, enkoder) ~15 cm kablo bırak; panel açıkken yanda durabilsin.
- Renk: kırmızı = 5V, siyah = GND, sinyaller ayrı renkler.
