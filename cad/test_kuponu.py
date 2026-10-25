"""B için test kuponu (5. tur): sadece LCD vida deliği konumu – hızlı baskı.

1. tur (24 Eki): insert Ø4.4 OK, GX16 OK, enkoder + tutucu OK, anahtar 13.2 dar -> 14.0 x 8.3.
2. tur: anahtar 14.0 OK; LCD bölümü tablaya tutmadı (ön yüz girintisi) -> düz ön yüz.
3. tur: LCD yüksekliği/cep OK; delik aralığı 74 x 30 olmadı -> 75 x 31.
5. tur: sadece delik konumu: 2 mm düz U çerçeve (üst uzun kenar yok), ortada çerçeve (bezel) açıklığı, 75 x 31 aralıklı 4 x Ø2.5 delik.
   LCD'yi çerçeve yüzü açıklıktan geçecek şekilde koy; 4 delik kart deliklerine denk gelmeli, M2.5 vida geçmeli.
Çıktı: baski/B_kama/B_test_kuponu.stl
Çalıştır: ../../AuvideaHailoH15/.venv_local/bin/python test_kuponu.py
"""
from pathlib import Path
import cadquery as cq
import render
import kama
from ortak import *

HERE = Path(__file__).parent
PT = kama.PT
RING = (84.0, 40.0, 2.0)                   # dış en, boy, kalınlık
OPEN = (LCD_BEZEL[0] + 1.0, LCD_BEZEL[1] + 1.0)   # bezel geçsin: 72.5 x 26.5


def build():
    ring = rrect(RING[0], RING[1], 3, 0, RING[2]).cut(rrect(OPEN[0], OPEN[1], 1, -1, RING[2] + 1))
    # U: üst uzun kenar kaldırılır; kesik deliklerin iç tarafında 1.75 mm et bırakır
    xin = LCD_HOLES[3][0] - 1.25 - 1.75
    ring = ring.cut(box(-xin, xin, OPEN[1] / 2 - 0.5, RING[1], -1, RING[2] + 1))
    return ring.cut(U(*[cyl(1.25, -1, RING[2] + 1, x, y) for x, y in LCD_HOLES]))


if __name__ == "__main__":
    k = build()
    pr = k
    bb = pr.BoundingBox()
    pr = pr.translate(V(-bb.center.x, -bb.center.y, -bb.zmin))
    out = HERE / "baski" / "B_kama" / "B_test_kuponu.stl"
    cq.exporters.export(pr, str(out), tolerance=0.02, angularTolerance=0.15)
    b = pr.BoundingBox()
    print(f"kupon {b.xlen:.1f} x {b.ylen:.1f} x {b.zlen:.1f} mm, {k.Volume()/1000:.1f} cm3, gecerli={k.isValid()} -> {out}")
    tmp = HERE / "cikti" / "_stl"; tmp.mkdir(parents=True, exist_ok=True)
    cq.exporters.export(pr, str(tmp / "kupon.stl"), tolerance=0.03, angularTolerance=0.15)
    render.render([str(tmp / "kupon.stl")], str(HERE / "cikti" / "B_test_kuponu.png"), colors=[(0.85, 0.55, 0.15)],
                  cam=(0.6, -1.0, 1.1), size=(1400, 1000), zoom=1.2, light_position=(200, -200, 400),
                  fill_light_position=(-200, 150, 100), ambient=0.3)
