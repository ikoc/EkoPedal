# LCD menulu Leonardo surumu

Guncel durum ve Windows'ta devam adimlari icin `docs/PROJECT_STATE.md` dosyasini
esas alin. Bu belge ilk wiring calismalarinin notlarini da icerir.

Firmware: `firmware/leonardo_menu/leonardo_menu.ino`. Daha onceki calisan
surum `firmware/leonardo_footswitch/leonardo_footswitch.ino` olarak sakli.

## Pinler

Güncel ve doğrulanmış liste: [`PIN_HARITASI.md`](PIN_HARITASI.md).

| Parca | Leonardo | Diger uc |
| --- | --- | --- |
| 78/28 kombo pedali | D8 | GND |
| Num Lock pedali | **D5** (eski D3 kablosu tasinacak) | GND |
| Scroll Lock pedali | D4 | GND |
| Encoder basma kontagi | D7 | GND |
| Encoder A donus kontagi | D6 | - |
| Encoder B donus kontagi | D9 | - |
| Encoder ortak donus kontagi | GND | - |
| HW-61 LCD GND | GND | - |
| HW-61 LCD VCC | 5V | - |
| HW-61 LCD SDA | D2 / SDA | - |
| HW-61 LCD SCL | D3 / SCL | - |
| Acma/kapama anahtari (yazilimsal) | A0 | GND |

Leonardo'nun D2 ve D3 pinleri ayni zamanda donanimsal I2C SDA/SCL
pinleridir. Eski D2 makro butonu bu surumde kullanilmaz. Onu D2'den ayirin;
menu butonu D7'ye baglanacak. D3'teki Num Lock pedali D5'e tasinmadan LCD
baglamayin. LCD ile pedal ve buton **sebekeye degil**, yalniz USB ile beslenen
Leonardo'ya baglanir. Her kablo degisiminden once USB'yi cikarin.

Metal butonun fotografinda anahtar ve varsa LED bacaklari ayirt edilemiyor.
Anahtar cifti, buton serbestken acik ve basiliyken kapali olan iki bacaktir.
Multimetre ile veya `firmware/menu_button_wiring_test/menu_button_wiring_test.ino`
ile bulunmadan bacak numarasi varsayilmaz. LED bacaklari kullanilmaz.

LCD backpack PCF8574 icin yaygin HW-61 baglanti sirasi kullanilir. Yazilim
0x27 ve 0x3F adreslerini once dener, sonra 0x20-0x3F araligini tarar.
Ekran gorunmezse ilk once gercek pin etiketlerini, 5V/GND'yi ve mavi
kontrast potunu kontrol edin. Farkli pin eslemeli backpack ayrica ayar ister.

## Kullanım

- Acilista **SECIM BEKLENIYOR**: HID komutlari gonderilmez.
- Menu butonunu 700 ms basili tut: menuyu ac. Aktif modda ayni hareket
  kombo/scroll/makroyu durdurur ve menuyu acar.
- Menude butona kisa bas veya D5 pedalina bas: sonraki oge.
- D8 pedalina bas: onceki oge.
- Encoder'i saat yonunde veya tersine cevir: menu oge/ayar degistir.
- Butona uzun bas veya D4 pedalina bas: ogeye gir/onayla.
- Bir ayar icinde D8 azaltir, D5 artirir. D4 veya uzun buton kaydeder ve
  menuye doner.
- `Baslat/Durdur` ogesini secmek aktif modu baslatir. `Kaydet/Cik` secmek
  pasif ana ekrana doner. Aktif moddaki pedallar onceki islevlerini yapar.

## Ayarlar

| Menu | Degerler | Etki |
| --- | --- | --- |
| Skill bar | Auto Caps / Sadece 78 / Sadece 28 | D8 dizisinin 7 veya 2 ile baslayacagini secer; 5 ms basma ve 5 ms aralik korunur. |
| Mouse delay | 20-500 ms, 10 ms adim | Envanter makrosundaki imlec hareket suresi. Varsayilan 40 ms. |
| Styx delay | 25-500 ms, 25 ms adim | Dagger adimlari oncesi bekleme. Varsayilan 75 ms. |
| Ekran ayari | 1280x1024 / 1920x1080 / 2560x1440 | Sag/ust kenara gore hedef konumlarini yeniden hesaplar. |

Ayarlar yalniz onaylandiginda EEPROM'a yazilir. Magic, surum ve saglama
toplami dogrulanir; gecersiz kayitta varsayilanlara donulur. Ekran cozunurlugu
hesabi, oyundaki envanterin sag/ust kenara ayni piksel uzakliginda kalmasini
varsayar. Oyun arayuzu farkli olceklenirse imlec hedefleri fiziksel olarak
kalibre edilmelidir. Mac'te fareyle envanter islevinin oyundaki dogrulugu
test edilemez; ilk testi bos ve guvenli bir ekranda yapin.
