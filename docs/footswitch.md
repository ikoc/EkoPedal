# Standalone footswitch taslagi

Bu firmware bilgisayarda Python veya baska bir uygulama gerektirmez. Leonardo
USB klavye ve fare olarak calisir. Caps Lock durumu, bilgisayarin klavye LED raporundan
`BootKeyboard.getLeds()` ile okunur.

## Davranis

| Pedal | Caps Lock | Gonderilen tus dizisi |
| --- | --- | --- |
| Birakilmis | Fark etmez | Hicbir tus gonderilmez |
| Basili | Kapali | `7 8` tekrarlar |
| Basili | Acik | `2 8` tekrarlar |

- Caps Lock degisirse basili tus birakilir ve yeni dizi ilk tustan baslar.
- Pedal birakilinca basili tus hemen birakilir; dizi durur.
- USB takilirken pedal basiliysa, once birakilip yeniden basilmasi gerekir.
- Kullanici tarafindan dogrulanan kararli ayar: `78`/`28` sirasi, 5 ms tus
  basili / 5 ms ara. Oyunda iki pot arasinda 24-25 minor olculdu. Bu pedal
  dizisini ve zamanlamasini acik istek olmadan degistirmeyin; fare makrosu
  degisikliklerini bundan ayri tutun.
- `78`/`28` icin 0/0 ms denemesi oyunda belirgin bicimde kotulesti; onceki
  `7888`/`2888` sirasi da 0 ms ile 24 yerine 19 etkili minor vermisti.
- Basma 30 ms, birakma 50 ms kararlilik suresiyle algilanir. `L` LED'i pedal
  basiliyken yanar; pedal birakilinca tus hemen serbest birakilir.
- D2-GND arasindaki ikinci buton pedaldan bagimsizdir. Bir kez basinca `I` ile
  envanteri acar, sag ekipman yuvasindaki dagger'a sag tiklar, canta 1. goze
  (bow) sag tiklar, `0` skill tusunu gonderir, canta 1. ve 2. gozdeki
  dagger'lara sirasiyla sag tiklar ve `I` ile envanteri kapatir. Buton basili tutulursa dizi
  tekrarlanmaz; dizi surerken pedal da calismaya devam eder.
- D3-GND arasindaki yeni pedal her basista bilgisayara bir `Num Lock` tusu
  gonderir. Windows Num Lock durumunu degistirip Leonardo'ya LED raporu
  gonderdiginde mevcut envanter makrosu bir kez baslar. Basili tutma tusu
  tekrarlamaz; tekrar tetiklemek icin pedal birakilip yeniden basilmali.
- D4-GND arasindaki yeni pedal her basista bir `Scroll Lock` tusu gonderir.
  Windows LED raporunda Scroll Lock acilinca mevcut `3-W-4-S` dongusu baslar;
  ikinci basista kapanir. Pedal USB takilirken basiliysa once birakilip
  yeniden basilmasi gerekir. Mac'te LED raporu alinmadigi olculdugu icin
  bu iki pedalın makro sonucu Windows'ta dogrulanmalidir.
- Gecici deneme icin Num Lock durumunun her degisimi de ayni diziyi bir kez
  baslatir. USB acilisindan sonraki ilk 3 saniyede LED durumu yalnizca baslangic
  degeri olarak okunur. Dizi zaten calisiyorsa yeni tetik yok sayilir. Bu,
  fiziksel Num Lock tusunu dogrudan dinlemek degil, PC'nin klavye LED raporunu
  okumaktir. Hedef PC'de ek uygulama gerektirmez.
- Scroll Lock acilinca `3` (421 ms), bosluk (32 ms), `W` (29 ms),
  bosluk (0 ms), `4` (421 ms), bosluk (0 ms), `S` (29 ms), bosluk (32 ms)
  seklinde tekrarlar.
  Scroll Lock kapaninca o anda basili tusu birakip durur. Bu dizi
  pedal ve Num Lock makrosundan bagimsizdir. USB acilisinin ilk 3 saniyesinde
  Scroll Lock LED durumu sadece baslangic degeri olarak okunur; PC'de zaten
  aciksa once kapatip yeniden acmak gerekir. Gorseldeki `2-W-3-W` tuslari
  istenen `3-W-4-S` sirasi olarak uygulanmistir.
- PC'ye bagli bir farenin tusunu Leonardo dogrudan okuyamaz. Farenin kendi
  kalici profilinde yan tus Scroll Lock'a atanabiliyorsa bu dolayli yoldan
  calisabilir; hedef PC'de ek yazilim gerekmeden calisacagi fare modeline
  baglidir.
- Fare hedefleri 1280x1024 ekran goruntulerinden alinmistir. Mutlak HID fare
  konumu kullanilir; oyunun bunu kabul etmesi ve esya siralamasinin ayni
  kalmasi fiziksel testle dogrulanmalidir. Firmware ekrandaki esyalari okuyamaz.
- Ilk fare hedefi sagdaki takili dagger'dir; imlec dogrudan oraya gider.
  Sonraki hedeflere 40 ms boyunca adimlarla ilerler. Arduino PC'deki gercek
  imlec konumunu okuyamaz. Envanter acilmasi icin 300 ms, hedefe gittikten
  sonra 25 ms bekler. Sag tik 40 ms basili kalir. Sag dagger'dan bow'a ve
  bow'dan `0`'a geciste 180 ms bekler; iki dagger oncesindeki beklemeler 75 ms'dir.
  Son dagger'dan kapanis `I` tusuna geciste 180 ms bekler. Bu hizli ayarlar
  oyunda dogrulanmalidir.

## Baglanti (KEK FS-1)

- Etiket `WHITE=COM`, `RED=NO`, `BLACK=NC` diyor; ancak paylasilan fotografta
  acik kablo renkleri bu uc renkle net olarak eslesmiyor. Renge bakarak
  baglanti yapmayin.
- USB ve baska bir guc bagli degilken multimetre sureklilik modunda `COM-NO`
  cifti pedal birakilmisken acik, basiliyken kapali olmalidir.
- Dogrulanan `COM` ucu `GND` pinine, `NO` ucu `D8` pinine baglanir. `NC`
  kullanilmaz; acik ucu ayri olarak yalitilir.
- Bu pedalda yesil-kirmizi ciftinin basinca kapandigi LED testiyle dogrulandi.
  Iki ucun `D8`/`GND` sirasi fark etmez; sari uc baglanmaz ve yalitilir.
- `INPUT_PULLUP` kullanilir; harici direnc gerekmez.
- Ikinci butonun birbirine basinca baglanan iki ucunu `D2` ve `GND` arasina
  baglayin. Dort bacakli butonda ayni taraftaki bacaklar genellikle zaten
  baglidir; iki farkli tarafi kullanin. USB cikarilmisken baglayin.
- Num Lock pedali: normalde acik `COM` ucunu `GND`'ye, `NO` ucunu `D3`'e
  baglayin. Scroll Lock pedali: `COM` ucunu `GND`'ye, `NO` ucunu `D4`'e
  baglayin. `NC` uclari kullanilmaz; ayri ayri yalitilir. Iki telli normalde
  acik pedalda tel sirasi fark etmez. Birden fazla `GND` gerektiginde
  breadboard eksi hatti veya guvenilir bir dagitici kullanin; `5V`'a baglamayin.
- Bu iki pedal icin firmware `INPUT_PULLUP` kullanir. Anlik (yayla geri
  donen) pedal gerekir; kilitlenen tipte tekrar basma ancak kontagin yeniden
  acilmasindan sonra calisir. Her kablo degisiminden once USB'yi cikarin.
- Kart uzerindeki LED pedal basiliyken yanar. Istege bagli harici LED icin `D10 ->
  330 ohm direnc -> LED anodu`, LED katodu `GND`.

Bu baglanti yalnizca Arduino'nun dusuk gerilimli girisidir; pedala sebekeye
ait hicbir kablo baglamayin. Kablolama sirasinda USB'yi cikarin. Pedal
birakildiginda eski konumuna dondugunu fiziksel olarak kontrol edin.

### Multimetre yoksa: gecici LED testi

`firmware/footswitch_wiring_test/footswitch_wiring_test.ino` yalnizca kartin
`L` LED'ini kullanir; bilgisayara tus gondermez. Bu test yazilimi ayri bir
yukleme gerektirir ve kullanici onayi olmadan karta yuklenmez. Yukleme mevcut
firmware'i gecici olarak degistirir; sonrasinda asagidaki asil firmware tekrar
yuklenir.

Pedal herhangi bir sebekeye veya baska cihaza bagli olmamalidir. Her kablo
degisiminden once Arduino USB'sini cikarin. Uc tellerin ikisini `D8` ve `GND`
arasina baglayip ucuncu ciplak ucu ayri ayri yalitin; ciplak teller birbirine
ve kartin baska pinlerine degmesin. USB'yi takip `L` LED'ine bakin:

| Pedal birakilmisken | Pedal basiliyken | Anlam |
| --- | --- | --- |
| Sonuk | Yanik | Dogru, normalde acik (NO) cift |
| Yanik | Sonuk | Normalde kapali (NC) cift; kullanmayin |
| Sonuk | Sonuk | Birbirine baglanmayan iki uc; baska cifti deneyin |

Sonuc belirsizse veya baglanti sabit/yalitimli yapilamiyorsa testi durdurun;
multimetre ya da uygun klemens/jumper ile devam edin. Dogru NO ciftinin iki
ucundan hangisinin `D8` veya `GND` oldugu fark etmez.

## Derleme ve ilk test

Gelistirme bilgisayarinda Arduino AVR Boards ve HID-Project 2.8.4 kutuphanesi
gereklidir. Baska bir PC'ye takildiginda bu kutuphanenin kurulmasi gerekmez.
Asil firmware karta yuklendi; fiziksel tekrar ve Caps Lock davranisi Not Defteri
testiyle dogrulanmalidir.

Ilk denemeyi Not Defteri'nde, kisa pedal basislariyla yapin. Caps Lock kapali
ve acik durumlarini ayri ayri deneyin; pedal birakildiginda girisin durdugunu
kontrol edin.

Kaynak: https://github.com/NicoHood/HID/blob/master/examples/Keyboard/KeyboardLed/KeyboardLed.ino
