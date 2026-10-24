# Seri Komut Protokolü

Baud rate: `115200`

Her komut ASCII satır olarak gönderilir ve `\n` ile biter. Firmware her geçerli komuta `OK ...`, hatalı komuta `ERR ...` yanıtı verir.

## Komutlar

- `HELLO`
  - Yanıt: `OK HELLO LEONARDO_HID_SAFE 1`
- `PING`
  - Heartbeat zamanını yeniler.
  - Yanıt: `OK PONG`
- `ENABLE 1`
  - HID komutlarını etkinleştirir.
  - Yanıt: `OK ENABLED`
- `ENABLE 0`
  - Tüm basılı tuşları bırakır, HID'i kapatır.
  - Yanıt: `OK STOPPED DISABLED`
- `STOP`
  - Acil yazılımsal durdurma.
  - Yanıt: `OK STOPPED APP_STOP`
- `RELEASE`
  - Tüm basılı tuşları bırakır.
  - Yanıt: `OK RELEASED`
- `KP <KEY> <HOLD_MS>`
  - Tek tuşa basar, belirtilen süre sonra bırakır.
  - `HOLD_MS` firmware tarafında en fazla `1000` ms kabul edilir.
  - Örnek: `KP 1 50`

## Tuşlar

Tek ASCII karakterler ve şu özel tuşlar desteklenir:

`SPACE`, `ENTER`, `ESC`, `TAB`, `BACKSPACE`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `F1` ... `F12`
