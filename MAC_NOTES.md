# Mac kurulum ve çalışma notu

Bu klasör Windows'tan aktarılan projenin temiz kopyasıdır. Firmware kaynakları
değiştirilmedi.

## Bu Mac'e kurulanlar (24 Ekim 2026)

- Arduino IDE 2.3.10: `~/Applications/Arduino IDE.app`
- Arduino CLI 1.5.1: `~/.local/bin/arduino-cli`
- Leonardo için Arduino AVR Boards 1.8.8 ve HID-Project 2.8.4
- Miniforge ve projeye özel Python 3.12.15 ortamı:
  `~/.local/miniforge3/envs/ko-pedal/`
- Bu Python ortamında Tk 8.6 ve PySerial 3.5

Arduino IDE ve CLI resmi yayınlarından indirildi, indirme dosyalarının SHA-256
değerleri doğrulandı. CLI ile `leonardo_footswitch` Leonardo hedefi için
başarıyla derlendi. Python ortamında 9 birim testi geçti.

## Projenin iki parçası

- `firmware/leonardo_footswitch/`: Leonardo üzerinde tek başına çalışan pedal,
  tuş ve fare makroları. Masaüstü Python uygulamasını kullanmaz.
- `desktop/` ve `firmware/leonardo_hid_safe/`: seri haberleşmeli eski kontrol
  prototipi. Masaüstü paneli bu firmware'in `HELLO` protokolünü bekler; güncel
  bağımsız pedal firmware'ine bağlanmak için tasarlanmamıştır.

## Geliştirme komutları

Proje klasöründe Terminal açtıktan sonra:

```sh
~/.local/bin/arduino-cli compile --fqbn arduino:avr:leonardo firmware/leonardo_footswitch
~/.local/miniforge3/envs/ko-pedal/bin/python -m unittest discover -s tests -v
```

Eski masaüstü panelini donanım olmadan açmak için `run_fake_mac.command`
dosyasına çift tıklayın. Terminal'den aynı işlem:

```sh
~/.local/miniforge3/envs/ko-pedal/bin/python desktop/main.py --fake
```

Arduino IDE ile düzenlemek için
`firmware/leonardo_footswitch/leonardo_footswitch.ino` dosyasını açın ve kart
olarak **Arduino Leonardo** seçin.

## Doğrulama sınırı

24 Ekim 2026'da `leonardo_footswitch` firmware'i Arduino IDE üzerinden bağlı
Leonardo'ya yüklendi; IDE `Done uploading` bildirdi ve kart yeniden seri portta
göründü. D3 Num Lock ve D4 Scroll Lock pedal girişlerini ekleyen sürüm de
derlenip yüklendi; yeni pedallar henüz fiziksel olarak sınanmadı. Pedalın `78`
gönderdiği kullanıcı tarafından doğrulandı. Komut aracının macOS oturumunda Tk
penceresi açma denemesi pencere sunucusu hatası verdi. Python, Tk ve PySerial
kurulu; masaüstü arayüzünün kullanıcının etkileşimli oturumunda açılması ayrıca
kontrol edilmeli.

## Caps Lock tanısı

Mac'te Caps Lock açıkken Leonardo'ya yüklenen `led_report_diagnostic` taslağı
seri monitörde sürekli `LEDS=0 CAPS=0` gösterdi. Bu Mac, mevcut HID arayüzüne
Caps Lock LED durumunu iletmediği için bağımsız firmware `78` dizisinden `28`
dizisine geçemiyor. Tanıdan sonra `leonardo_footswitch` firmware'i karta yeniden
yüklendi; pedalın çalışan sürümü geri geldi. Bu durum Mac'e özgü ölçümdür;
Windows'taki davranış hakkında yeni bir sonuç çıkarmaz.
