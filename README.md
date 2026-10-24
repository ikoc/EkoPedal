# Arduino Leonardo Footswitch

Yeni hedef, bilgisayar uygulamasi olmadan calisan bir USB footswitch'tir.
`firmware/leonardo_footswitch/` taslagi D8 pedal girisini ve bilgisayarin
Caps Lock LED durumunu kullanir. Pedal basiliyken Caps Lock kapaliysa `7 8 8 8`,
aciksa `2 8 8 8` dizisini tekrarlar. Baglanti ve test notlari icin
`docs/footswitch.md` dosyasina bakin.

Asagidaki eski Aşama 1 seri uygulama prototipi korunuyor; yeni standalone
firmware'in calismasi icin masaustu uygulamasi gerekmiyor.

Bu depo Aşama 1 için güvenli bir temel kurar:

- Arduino Leonardo üzerinde USB HID klavye/fare yeteneği olan, varsayılan olarak pasif firmware.
- Windows tarafında Tkinter tabanlı basit kontrol paneli.
- PySerial ile COM port bağlantısı.
- Donanım olmadan test edilebilen sahte Arduino haberleşme katmanı.
- Not Defteri gibi zararsız bir uygulamada denenebilecek tek tuş basma komutu.

Bu sürüm anti-cheat atlatma, donanım kimliği gizleme veya tespit mekanizması aşma içermez.

## Klasörler

- `firmware/` Arduino Leonardo firmware kodu
- `desktop/` Windows masaüstü uygulaması
- `profiles/` ileride JSON makro profilleri
- `tests/` Python birim testleri
- `docs/` bağlantı, protokol ve kullanım notları
- `requirements.txt` Python bağımlılıkları

## Gereksinimler

- Windows
- Arduino Leonardo veya ATmega32U4 tabanlı uyumlu kart
- Python 3.10+
- Arduino IDE veya Arduino CLI
- Python paketi: `pyserial`

Python bağımlılığını kurmak için:

```powershell
py -m pip install -r requirements.txt
```

Bu bilgisayarda Python 3.13 ve PySerial 3.5 doğrulandı. Arduino CLI proje çalışma
alanında yerel olarak hazırlandı. Firmware Leonardo hedefi için derlendi ve
onayla karta yüklendi. Kart yeniden bağlandığında COM numarası değişebilir.

## Masaüstü uygulamasını çalıştırma

```powershell
cd C:\Arduino-Knight-Macro
py desktop\main.py
```

Geliştirme ve donanımsız deneme için:

```powershell
py desktop\main.py --fake
```

## Güvenli ilk test

1. `docs\wiring.md` dosyasındaki acil durdurma bağlantısını incele.
2. Kartın firmware yüklü olduğundan emin ol. Yeniden yükleme için ayrıca onay gerekir.
3. Windows'ta Not Defteri'ni aç.
4. Uygulamada COM portu seç veya otomatik algılamayı kullan.
5. `Bağlan` düğmesine bas.
6. `HID Etkinleştir` düğmesine bas.
7. Test tuşu alanına örneğin `1` yaz.
8. `Tek Tuş Gönder` düğmesine bas; 5 saniyelik geri sayım başlar.
9. Bu sürede Not Defteri penceresini odakla. Test tuşu yalnızca bir kez gönderilir.
10. Herhangi bir sorun olursa `DURDUR` veya fiziksel acil durdurma butonunu kullan.

Bu bilgisayardaki ilk gerçek donanım testinde tek bir `1` Not Defteri'ne ulaştı.
Fiziksel buton henüz bağlanmadığı için acil durdurma davranışı test edilmedi.

## Testler

Donanım gerektirmeyen testler:

```powershell
py -m unittest discover -s tests
```

## Aşama 1 kapsamı

Bu sürüm yalnızca güvenli temel bağlantı, HID etkinleştirme/devre dışı bırakma, durdurma ve tek tuş gönderme işlevlerini içerir. Gelişmiş zamanlama, fare makroları ve JSON profil düzenleme Aşama 2'ye bırakıldı.
