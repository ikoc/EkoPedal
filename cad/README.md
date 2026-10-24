# EkoPedal kontrol kutusu – 3 tasarım (Bambu Lab X2D)

Pedallar hazır ayak pedalı; bu klasör **kontrol kutusunu** modelliyor (LCD + enkoder + açma/kapama anahtarı + 3 × GX16 + Leonardo).

```bash
# EkoPedal/cad içinden
../../AuvideaHailoH15/.venv_local/bin/python hepsi.py      # STL -> baski/, render -> cikti/
../../AuvideaHailoH15/.venv_local/bin/python kontrol.py    # iç çakışma kontrolü (A/B/C: OK beklenir)
```

| | A – Kule | B – Kama | C – Yatay |
|---|---|---|---|
| Dış ölçü | 104 × 172 × 44 | 112 × 100 × 59 (24 Eki küçültüldü) | 180 × 90 × 46 |
| Parçalar | gövde, kapak | gövde, panel, yazı inlay | gövde, kapak, yazı inlay, 2 eğim ayağı |
| Kullanım | masaya yatık / duvara asma (2 anahtar deliği) | masa üstü, 20° eğik panel | masa üstü, ayaklarla ~10° eğik |
| Konnektörler | alt kenar | arka duvar (1-2-3 yazılı) | alt kenar |
| Anahtar | sağ yan | sağ yan, alt (LCD altı) | üst kenar |
| USB | sol yan | sol yan | sağ yan |
| Ön yüz | etiket yuvası 88×68×0.5 (basılı sticker) | gravür + AMS renkli yazı | gravür + AMS renkli yazı, enkoder ölçeği |

## Baskı (X2D, PLA+, Textured PEI)

| Parça | Yön | Destek |
|---|---|---|
| Gövde (A, C) | ön yüz tablada | yok |
| Gövde (B) | taban tablada | yok |
| Panel (B), kapak | dış yüz tablada | yok |
| Enkoder tutucu (B) | düz | yok |
| Yazı inlay | gövde/panel ile birlikte seç → "tek nesne, çok parça" → 2. renk | – |

Önerilen: 0.2 mm katman, 3 duvar, %15 gyroid. GX16 delikleri yatay; üst kenarda hafif sarkma flanşın altında kalır.

## Montaj malzemesi

- B için **6 × M3×4×5 ısıl insert** → delik **Ø4.4** (X2D/PLA kuponunda en iyi oturan, Auvidea v12f)
  - 4 tanesi gövde kulelerinde: panel 4 × M3×8 havşa vidayla
  - 2 tanesi panel altında: enkoder tutucu plaka 2 × **M3×6** vidayla (delik 5.0 derin, ön yüz kabarmasın diye)
- LCD: 4 × M2.5×6 veya M3 sac vidası; Leonardo: 4 × M3×6 sac vidası (Ø2.6 pilot)
- 4 × Ø10 lastik ayak (A, B)

## Ölçüler (24 Eki 2026, kumpasla ölçüldü → `ortak.py`)

| Parça | Ölçü | Modelde |
|---|---|---|
| LCD çerçeve | 71.5 × 25.5, karttan 9.6 yüksek | LCD kuleleri 8.2 mm |
| LCD vida delikleri | Ø3, dış kenarlar 77 × 33 → merkez 74 × 30 | 4 kule Ø5 (çerçeve tarafı 0.3 boşlukla traşlı), Ø2.5 pilot, M2.5×6 |
| Enkoder | gövde 12.0 × 12.2 × 6.5, bacak 3.5; topuz Ø14.5 × 16.5 | düz mil (dişli boyun yok) → panel altında cep + 4 köşe direği + vidalı tutucu plaka (artı boşluktan bacaklar geçer); topuz panelden 2 mm yukarıda |
| GX16 | Ø16.0, arkada 15 | Ø16.2 delik |
| Leonardo | 68.5 × 54.0; USB karşı köşeden 12.0–20.0 | USB merkezi köşeden 38.0 |
| Anahtar (mini rocker) | gövde 13 × 8, derinlik 17 | delik 13.2 × 8.3, duvar 1.6 mm'ye inceltilmiş |
| Henüz ölçülmedi | I2C kartı derinliği (12 kabul) | – |

## B test kuponu (`baski/B_kama/B_test_kuponu.stl`, 111 × 86 mm, ~25 dk)

Ön yüz tablada, desteksiz. Enkoder tutucu aynı STL'de, kuponun yanında ayrı parça (montajda arkadan vidalandığı için panelle birleşik değil). Insert çapı Ø4.4 sabit (Auvidea v12f).

| Ne denenir | Nasıl | Sonuca göre |
|---|---|---|
| Dik insert (Ø4.4) | Enkoder tutucusunun 2 kulesi | Gevşek/sıkı ise `INSERT_R` |
| Eğik insert (gövde) | 20° eğik kule, Ø4.4 (ön yüzde çizgi) | Havyayı kule eksenine paralel tut |
| Enkoder | Cebe otur, tutucuyu 2 × M3×6 ile vidala, topuzu tak | Gevşekse / sıkıysa `ENC_SQ` |
| Anahtar | Tık sesiyle oturmalı, sallanmamalı | Gevşekse `SW_CUT` −0.1 |
| GX16 | Somunla sıkılmalı | Ø16.2 |
| Ön yüz | Insert sonrası enkoder çevresinde kabarma olmamalı | – |
| LCD | Çerçeve cebe girmeli, kart 4 kuleye oturmalı, 4 × M2.5×6 delikleri karşılamalı; ekran pencereden tam görünmeli | `LCD_HOLE_PITCH`, `LCD_BEZEL`, `LCD_BEZEL_H` |
