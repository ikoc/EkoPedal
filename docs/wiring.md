# Bağlantı Şeması

## Arduino Leonardo

- Arduino Leonardo USB kablosu ile Windows bilgisayara bağlanır.
- Acil durdurma butonu `D2` pini ile `GND` arasına bağlanır.
- Firmware `INPUT_PULLUP` kullandığı için harici direnç gerekmez.
- Buton bırakıldığında `D2` HIGH, basıldığında LOW okunur.
- Buton henüz bağlı değilse `INPUT_PULLUP` nedeniyle D2 HIGH kalır; firmware
  çalışır, ancak fiziksel acil durdurma kullanılamaz.

## Güvenlik davranışı

- Kart açıldığında HID pasiftir.
- Uygulama `ENABLE 1` göndermeden klavye/fare komutu çalışmaz.
- Heartbeat 3 saniyeden uzun süre kesilirse firmware tüm basılı tuşları bırakır ve HID'i kapatır.
- Acil durdurma butonu basılırsa tüm tuşlar bırakılır ve HID kapatılır.
- `STOP` komutu tüm basılı tuşları bırakır ve HID'i kapatır.

## İlk test

Butonu bağlamadan önce USB kablosunu çıkarın. Normalde açık, anlık butonun bir
ucunu D2'ye, diğer ucunu GND'ye bağlayın; 5V bağlantısı veya harici direnç
gerekmez. Yeniden bağladıktan sonra buton basılıyken `HID Etkinleştir`
komutunun `ERR EMERGENCY_STOP` döndürdüğünü doğrulayın. İlk tuş denemesini
oyun veya hassas bir uygulama yerine Not Defteri'nde yapın.
