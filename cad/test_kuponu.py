"""B için test kuponu: kritik geçmeleri ~25 dk'lık tek baskıda dener.

İçerik (panel ile aynı 3 mm kalınlık, aynı fonksiyonlar):
  - Enkoder cebi + mil deliği + köşe tutucu + tutucu plaka kuleleri (Ø4.4 insert)  -> gerçek B_enkoder_tutucu ile dene
  - Anahtar deliği 13.2 x 8.3 + 1.6 mm inceltme
  - GX16 deliği Ø16.2
  - Gövdedeki gibi 20° eğik insert kulesi Ø4.4 (ön yüzde çizgi)
  - Enkoder tutucu plaka (ayrı gövde, aynı STL; vidalayıp enkoderi sıkmayı dene)
  - LCD bölümü: panelin birebir kopyası (pencere, çerçeve cebi, 0.6 girinti, 74 x 30 aralıklı 4 kule)
Insert çapı Ø4.4 sabit (Auvidea v12f X2D kuponu); tutucu kuleleri de Ø4.4 -> dik insert de denenmiş olur.
Çıktı: baski/B_kama/B_test_kuponu.stl (ön yüz tablada)
Çalıştır: ../../AuvideaHailoH15/.venv_local/bin/python test_kuponu.py
"""
from pathlib import Path
import cadquery as cq
import render
import kama
from ortak import *

HERE = Path(__file__).parent
PT = kama.PT
ENC = (-6, 11)
SW = (8, -12)
GX = (-16, -12)
TB = (22, 13)                                             # eğik kule
PLATE = (58, 48, 1, 1)                                    # en, boy, merkez x, y
LCD = (1, 43)                                             # LCD bölümü merkezi
LCD_PLATE = (84, 40)                                      # pencere + cep + kuleleri taşır
TILT = math.degrees(kama.ANG)


def build():
    plate = rrect(PLATE[0], PLATE[1], 4, 0, PT, PLATE[2], PLATE[3])
    plate = plate.fuse(rrect(LCD_PLATE[0], LCD_PLATE[1], 4, 0, PT, *LCD))
    add, cut = [], []
    ex, ey = ENC
    add.append(mv(enc_cradle(PT), ex, ey, PT))
    ra, rc = enc_retainer_bosses(PT)
    add.append(mv(ra, ex, ey, PT)); cut.append(mv(rc, ex, ey, PT))
    cut.append(mv(enc_cut(PT), ex, ey, PT))
    cut.append(mv(cyl(KNOB_D / 2 + 4.8, -0.6, 1).cut(cyl(KNOB_D / 2 + 4.0, -1, 2)), ex, ey, PT))
    cut.append(mv(sw_cut(PT), SW[0], SW[1], PT))
    cut.append(mv(gx_cut(PT), GX[0], GX[1], PT))
    tb = cyl(4.5, -10.0, 0.3).fuse(cq.Solid.makeCone(4.5, 6.0, 1.5, V(0, 0, -1.2)))
    th = cyl(INSERT_R, -10.1, -10.0 + INSERT_L)
    tb = tb.rotate(V(0, 0, 0), V(1, 0, 0), -TILT).translate(V(TB[0], TB[1], 0))
    th = th.rotate(V(0, 0, 0), V(1, 0, 0), -TILT).translate(V(TB[0], TB[1], 0))
    add.append(tb.intersect(box(-60, 60, -60, 60, -20, 0.3)))
    cut += [th, box(TB[0] - 3, TB[0] + 3, TB[1] - 0.4, TB[1] + 0.4, PT - 0.5, PT + 1)]
    # LCD bölümü (panelde olduğu gibi: üstte 0.6 girinti, cep, pencere, kuleler)
    la, lc = lcd_bosses(PT)
    add.append(mv(la, LCD[0], LCD[1], PT))
    cut += [mv(lc, LCD[0], LCD[1], PT), mv(lcd_cut(PT), LCD[0], LCD[1], PT),
            box(LCD[0] - 50, LCD[0] + 50, LCD[1] - LCD_PLATE[1] / 2 + 3, LCD[1] + 30, PT - 0.6, PT + 1)]
    return plate.fuse(*add).clean().cut(U(*cut))


if __name__ == "__main__":
    k = build()
    pr = k.rotate(V(0, 0, 0), V(1, 0, 0), 180)
    bb = pr.BoundingBox()
    pr = pr.translate(V(-bb.center.x, -bb.center.y, -bb.zmin))
    # Enkoder tutucu: ayrı parça (montajda arkadan vidalanır), aynı STL'de kuponun yanında
    ret = enc_retainer()
    rb = ret.BoundingBox()
    ret = ret.translate(V(-rb.center.x, -rb.center.y, -rb.zmin)).rotate(V(0, 0, 0), V(0, 0, 1), 90)
    pb = pr.BoundingBox()
    ret = ret.translate(V(pb.xmax + 5 + ret.BoundingBox().xlen / 2, pb.ymin + 20, 0))
    pr = cq.Compound.makeCompound([pr, ret])
    out = HERE / "baski" / "B_kama" / "B_test_kuponu.stl"
    cq.exporters.export(pr, str(out), tolerance=0.02, angularTolerance=0.15)
    b = pr.BoundingBox()
    print(f"kupon {b.xlen:.1f} x {b.ylen:.1f} x {b.zlen:.1f} mm, {k.Volume()/1000:.1f} cm3, gecerli={k.isValid()} -> {out}")
    tmp = HERE / "cikti" / "_stl"; tmp.mkdir(parents=True, exist_ok=True)
    cq.exporters.export(pr, str(tmp / "kupon.stl"), tolerance=0.03, angularTolerance=0.15)
    render.render([str(tmp / "kupon.stl")], str(HERE / "cikti" / "B_test_kuponu.png"), colors=[(0.85, 0.55, 0.15)],
                  cam=(0.6, -1.0, 1.1), size=(1400, 1000), zoom=1.2, light_position=(200, -200, 400),
                  fill_light_position=(-200, 150, 100), ambient=0.3)
