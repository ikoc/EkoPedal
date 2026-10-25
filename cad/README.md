# EkoPedal kontrol kutusu – B Kama (Bambu Lab X2D)

Pedallar hazır ayak pedalı; bu klasör **kontrol kutusunu** modelliyor (LCD + enkoder + açma/kapama anahtarı + 3 × GX16 + Leonardo).

```bash
# EkoPedal/cad içinden
../../AuvideaHailoH15/.venv_local/bin/python hepsi.py      # STL -> baski/, render -> cikti/
../../AuvideaHailoH15/.venv_local/bin/python kontrol.py    # iç çakışma kontrolü ("B: OK" beklenir)
../../AuvideaHailoH15/.venv_local/bin/python test_kuponu.py # test kuponu STL
../../AuvideaHailoH15/.venv_local/bin/python goruntuleyici.py # cikti/EkoPedal_3B.html
```

| | B – Kama |
|---|---|
| Dış ölçü | 112 × 100 × 59 mm, panel 20° eğik |
| Parçalar | gövde, panel, enkoder tutucu (hepsi tek renk) |
| Konnektörler | arka duvar, 3 × GX16 (1-2-3 yazılı) |
| Anahtar | sağ yan duvar, alt (LCD'nin altı) |
| USB | sol yan |
| Ön yüz | düz; LCD penceresi + 0.8 mm çerçeve oluğu, enkoder halkası. "eKO PEDAL" yazısı 25 Eki'de kaldırıldı |

A (Kule) ve C (Yatay) alternatifleri 24 Eki'de silindi; git geçmişinde (`2f6258f`) duruyor.

## Baskı (X2D, PLA+, Textured PEI)

| Parça | Yön | Destek |
|---|---|---|
| Gövde | taban tablada | yok |
| Panel | dış yüz tablada | yok |
| Enkoder tutucu | düz | yok |

Önerilen: 0.2 mm katman, 3 duvar, %15 gyroid. GX16 delikleri yatay; üst kenarda hafif sarkma flanşın altında kalır.

## ✅ Doğrulanmış montaj ölçüleri (25 Eki 2026, X2D + PLA, gerçek baskıda denendi)

Tekrar kullanılabilir: başka kutularda da aynen al (`ortak.py`).

| Bileşen | Doğrulanan tasarım |
|---|---|
| **LCD 1602 + HW-61** | Çerçeve 71.5 × 25.5 × 9.6 (PCB üstü). Panel 3 mm; pencere arkasında çerçeve cebi (çerçeve + 1.2 × + 0.5), cepte 1.6 mm et. 4 kule **8.2 mm**, delik aralığı **75 × 31**, Ø2.5 pilot, M2.5×6 |
| **Döner enkoder (EC11, düz mil, basmalı)** | Gövde 12.0 × 12.2 × 6.5. Panelde alttan cep (12.4 × 12.6), enkoder yerinde panel **1.6 mm** → topuz panelden ~2 mm yukarıda. 4 köşe L direği (1.8 kalın, 3.5 kol). Tutucu plaka 3 mm, artı boşluk (7.5 genişlik), 2 × M3×6 → panel altı kulelerde Ø4.4 × 5.0 insert. Mil deliği Ø7.4 |
| **M3 ısıl insert (M3×4×5)** | Delik **Ø4.4**, kule Ø8–9 |
| **Mini rocker anahtar (13 × 8 × 17)** | Delik **14.0 × 8.3**, duvar içten 1.6 mm'ye inceltilir (cep ≥ 20.2 × 14.3) |
| **GX16** | Delik **Ø16.2** |
| **Arduino Leonardo** | Delik konumları doğru (68.5 × 54.0 kart, standart Uno deseni). Ayak Ø6 × 5 mm. Pilot delik Ø2.6 dardı → **Ø2.9 × 6.6** (M3 dişini açar; LCD ile aynı yöntem) – yeni değer henüz basılmadı |

## Montaj malzemesi

- B için **6 × M3×4×5 ısıl insert** → delik **Ø4.4** (X2D/PLA kuponunda en iyi oturan, Auvidea v12f)
  - 4 tanesi gövde kulelerinde: panel 4 × M3×8 havşa vidayla
  - 2 tanesi panel altında: enkoder tutucu plaka 2 × **M3×6** vidayla (delik 5.0 derin, ön yüz kabarmasın diye)
- LCD: 4 × M2.5×6; Leonardo: 4 × **M3×8** (Ø2.9 pilot, vida kendi dişini açar)
- 4 × Ø10 lastik ayak

## Ölçüler (24 Eki 2026, kumpasla ölçüldü → `ortak.py`)

| Parça | Ölçü | Modelde |
|---|---|---|
| LCD çerçeve | 71.5 × 25.5, karttan 9.6 yüksek | LCD kuleleri 8.2 mm |
| LCD vida delikleri | Ø3; ölçüm 74 × 30 kuponda olmadı → **75 × 31** (25 Eki) | 4 kule Ø5, Ø2.5 pilot, M2.5×6 |
| Enkoder | gövde 12.0 × 12.2 × 6.5, bacak 3.5; topuz Ø14.5 × 16.5 | düz mil (dişli boyun yok) → panel altında cep + 4 köşe direği + vidalı tutucu plaka (artı boşluktan bacaklar geçer); topuz panelden 2 mm yukarıda |
| GX16 | Ø16.0, arkada 15 | Ø16.2 delik |
| Leonardo | 68.5 × 54.0; USB karşı köşeden 12.0–20.0 | USB merkezi köşeden 38.0 |
| Anahtar (mini rocker) | gövde 13 × 8, derinlik 17; tırnaklar açık 16 | delik **14.0** × 8.3 (kupon 1'de 13.2 dar geldi), duvar 1.6 mm'ye inceltilmiş |
| Henüz ölçülmedi | I2C kartı derinliği (12 kabul) | – |

## B test kuponu (`baski/B_kama/B_test_kuponu.stl`, 84 × 40 × 2 mm, birkaç dakika)

Ön yüz tablada, desteksiz.

| Tur | Sonuç |
|---|---|
| 1 (24 Eki) | Insert Ø4.4 ✅ · GX16 ✅ · enkoder + tutucu ✅ · anahtar 13.2 dar ❌ · LCD tekrar denenecek |
| 2 | Anahtar 14.0 × 8.3 ✅ · LCD bölümü tablaya tutmadı ❌: ön yüzdeki 0.6 mm girinti yüzünden ilk 0.6 mm havadaydı |
| 3 | LCD tablaya tuttu ✅ · yükseklik ✅ · delik aralığı 74 × 30 olmadı ❌ → her yön +1 mm |
| 5 | LCD delikleri 75 × 31 ✅ (25 Eki). Sadece delik konumu: 2 mm düz U çerçeve (bir uzun kenar yok), 75 × 31 aralıklı 4 × Ø2.5 delik |

**Kural:** Ön yüz tablada basılan parçada ön yüze geniş girinti konmaz (tabanı havada kalır). Sadece ince oluk,
gravür yazı ve delik olur. B panelindeki LCD girintisi 0.8 mm'lik çerçeve oluğuna çevrildi.
