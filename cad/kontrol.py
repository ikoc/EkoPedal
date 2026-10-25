"""İç çakışma kontrolü: bileşen yer tutucuları gövde/kapak/panel ile ve birbirleriyle çakışıyor mu."""
import itertools, kama
from ortak import C_TEXT

def check(tag, parts, dums):
    dums = [(d, c) for d, c in dums if c != C_TEXT and c != (0.20, 0.16, 0.12)]  # etiket görseli hariç
    bad = 0
    solids = [(n, s) for n, (s, _) in parts.items() if not n.startswith("yazi")]
    for i, (d, c) in enumerate(dums):
        for n, s in solids:
            v = d.intersect(s).Volume()
            if v > 1.0:
                print(f"  {tag}: dummy#{i} renk{c} x {n}: {v:.1f} mm3"); bad += 1
    for (i, (a, _)), (j, (b, _)) in itertools.combinations(enumerate(dums), 2):
        v = a.intersect(b).Volume()
        if v > 1.0:
            print(f"  {tag}: dummy#{i} x dummy#{j}: {v:.1f} mm3"); bad += 1
    print(f"{tag}: {'OK' if not bad else str(bad)+' cakisma'}")

pb, db, _ = kama.build()
check("B", pb, db)
