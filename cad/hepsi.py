"""B (Kama) tasarımını üretir: STL (baskı yönünde) -> baski/B_kama/, render PNG -> cikti/.

Çalıştır (EkoPedal/cad içinden):
  ../../AuvideaHailoH15/.venv_local/bin/python hepsi.py
"""
import sys
from pathlib import Path
import cadquery as cq
from cadquery import Vector as V
import render
import kama

HERE = Path(__file__).parent
OUT = HERE / "cikti"
BASKI = HERE / "baski"
TMP = OUT / "_stl"


def stl(shape, path):
    cq.exporters.export(shape, str(path), tolerance=0.02, angularTolerance=0.15)


def shots(tag, items, view, cam2):
    TMP.mkdir(parents=True, exist_ok=True)
    paths, cols = [], []
    for i, (s, c) in enumerate(items):
        p = TMP / f"{tag}_{i}.stl"
        cq.exporters.export(view(s), str(p), tolerance=0.05, angularTolerance=0.2)
        paths.append(str(p)); cols.append(c)
    kw = dict(colors=cols, size=(1400, 1100), light_position=(200, -260, 300),
              fill_light_position=(-220, 150, -40), ambient=0.25)
    render.render(paths, str(OUT / f"{tag}_on.png"), cam=(0.8, -1.6, 0.45), zoom=1.2, **kw)
    render.render(paths, str(OUT / f"{tag}_arka.png"), cam=cam2, zoom=1.2, **kw)


def report(tag, parts):
    for n, (s, _) in parts.items():
        bb = s.BoundingBox()
        print(f"  {tag}/{n}: {bb.xlen:.1f} x {bb.ylen:.1f} x {bb.zlen:.1f} mm, "
              f"hacim {s.Volume()/1000:.1f} cm3, gecerli={s.isValid()}")


def run(which="B"):
    OUT.mkdir(exist_ok=True)
    if "B" in which:
        parts, dums, local = kama.build()
        report("B", parts)
        d = BASKI / "B_kama"; d.mkdir(parents=True, exist_ok=True)
        for n, (s, _) in parts.items():
            stl(kama.print_pose(n, s, local), d / f"B_{n}.stl")
        items = [(s, c) for s, c in parts.values()] + dums
        shots("B", items, lambda s: s, (-0.9, 1.5, 0.6))


if __name__ == "__main__":
    run(sys.argv[1] if len(sys.argv) > 1 else "B")
