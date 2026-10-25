# EkoPedal - devam notu

Bu dosya, projeyi Windows bilgisayarda Codex ile açtığımızda kaldığımız noktayı
hızlıca anlamak için tutulur.

## Son çalışan firmware

Ana firmware:

`firmware/leonardo_menu/leonardo_menu.ino`

Son dokümantasyon commit'i: `63a6d0f`. Firmware'in güncel kaynak dosyası her
zaman `main` dalındaki `firmware/leonardo_menu/leonardo_menu.ino` dosyasıdır.
5 ms HID zamanlamasını geri getiren firmware değişikliği `6548a97` commit'inde
bulunur; sonraki commit'ler bu sürümün menü ve test düzeltmelerini içerir.

Arduino hedefi `Arduino Leonardo (arduino:avr:leonardo)` ve kart ATmega32U4'tür.
Firmware USB HID klavye/fare olarak çalışır; masaüstü Python uygulaması gerekli
değildir.

## Mevcut varsayılanlar

- Minor tuşu: `8`
- Mana tuşu: `7`
- HP tuşu: `2`
- 5'li ok tuşu: `3`
- 3'lü ok tuşu: `4`
- Minor profili: `25 minor` (en hızlı profil)
- Caps davranışı: Caps LED açıksa HP (`2`), kapalıysa Mana (`7`)

Minor zamanlamasında oyun tarafından güvenilir algılanan eski 5 ms HID tutma
zamanlaması korunmuştur. 1 ms denemesi oyun tarafından tuş kaçırdığı için geri
alınmıştır.

## Pin haritası

| İşlev | Leonardo pini |
|---|---|
| Minor pedal | D8 - GND |
| Num Lock pedalı / Styx makrosu | D5 - GND |
| Scroll Lock pedalı / 3w4s okçu | D4 - GND |
| Encoder basma | D7 - GND |
| Encoder A | D6 |
| Encoder B | D9 |
| Encoder ortak (3'lü döner grubun orta pini) | GND |
| LCD SDA | D2 / SDA |
| LCD SCL | D3 / SCL |
| LCD VCC/GND | 5V / GND |

Kablo değiştirirken USB gücü kesilmelidir. Pedal girişleri `INPUT_PULLUP` kullanır;
harici direnç gerekmez.

## LCD ve menü

Açılış ekranı `eKO PEDAL / HAZIR` olur ve cihaz doğrudan aktif moda geçer.
Menü testleri şimdilik ertelenmiştir; breadboard/encoder teması kararsızdır.
Ana menü yapısı:

```text
MinorManaPot
  Minor tuşu / Mana tuşu / HP tuşu / Minor delay / Kaydet-Geri
Styx ayarları
  Styx gecikme / Çözünürlük / Kaydet-Geri
3lu5li okcu
  5li ok tuşu / 3lu ok tuşu / Okcu delay / Kaydet-Geri
Varsayilan
Kaydet-Geri
```

LCD düzenleme ekranları hangi tuşun değiştirildiğini açıkça yazar (`MINOR TUSU`,
`3LU OK TUSU` vb.).

## Test geçmişi ve bilinen durumlar

- D8 Minor pedalı Mac TextEdit'te `787878...` olarak doğrulandı.
- D4 okçu makrosu Mac'te `3w4s` üretiyor; Türkçe Mac klavye düzeni bazı
  karakterleri farklı gösterebilir.
- Mac, Num Lock/Scroll Lock LED geri bildirimini Arduino'ya vermediği için D4/D5
  makroları doğrudan pedal basışında başlatılacak şekilde düzenlendi.
- D4 ikinci kez basıldığında doğrudan test döngüsünü durdurur.
- Windows oyun testi henüz tamamlanmadı. Bir sonraki adım Windows'ta 25 minor
  hızını ve Caps açıkken `2-8` dizisini ölçmektir.

## Windows'ta devam prosedürü

1. Depoyu Windows'ta aç ve `docs/PROJECT_STATE.md` dosyasını oku.
2. `firmware/leonardo_menu/leonardo_menu.ino` dosyasını Arduino IDE'de aç.
3. Board: **Arduino Leonardo**, doğru COM portunu seç.
4. Firmware'i yükle; kart yeniden bağlanınca LCD'de `eKO PEDAL / HAZIR` görülmeli.
5. Önce güvenli bir metin alanında D8, D4 ve D5'i ayrı test et.
6. Sonra oyunda 25 minor hızını ve Caps Lock davranışını test et.

## Önemli dosyalar

- `firmware/leonardo_menu/leonardo_menu.ino`: güncel firmware
- `docs/wiring.md`: genel bağlantı notları
- `docs/lcd_menu.md`: LCD/pin notları (tarihsel; bu dosyadaki güncel haritayı esas al)
- `firmware/leonardo_footswitch/leonardo_footswitch.ino`: ilk çalışan standalone firmware
- `tests/`: donanımsız Python testleri

