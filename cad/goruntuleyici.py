"""B (Kama) için etkileşimli 3B görüntüleyici: cikti/EkoPedal_3B.html

Parçalar tessellate edilir (Int16, 0.01 mm), patlatma vektörleri ve bilgileriyle şablona gömülür.
Çalıştır: ../../AuvideaHailoH15/.venv_local/bin/python goruntuleyici.py
"""
import base64, json, math
from pathlib import Path
import numpy as np
import kama
from ortak import *

HERE = Path(__file__).parent
N = (0.0, -kama.SA, kama.CA)                       # panel normali (dünya, Z yukarı)


def nvec(d):
    return [N[0] * d, N[1] * d, N[2] * d]


def mesh(shape, tol=0.04):
    vs, tris = shape.tessellate(tol, 0.2)
    p = np.array([(v.x, v.y, v.z) for v in vs], dtype=np.float64)
    p = np.stack([p[:, 0], p[:, 2], -p[:, 1]], axis=1)   # Z yukarı -> three.js Y yukarı
    q = np.round(p * 100).astype(np.int16)
    idx = np.array(tris, dtype=np.uint32).ravel()
    idx = idx.astype(np.uint16) if len(vs) < 65536 else idx
    return dict(p=base64.b64encode(q.tobytes()).decode(), i=base64.b64encode(idx.tobytes()).decode(),
                i32=bool(idx.dtype == np.uint32), tris=len(tris))


def hexc(c):
    return "#%02x%02x%02x" % tuple(int(round(v * 255)) for v in c)


def build_items():
    parts, dums, local = kama.build()
    Pp = plane((0, 0, kama.H0), (1, 0, 0), (0, -kama.SA, kama.CA))
    PT = kama.PT
    ex, ey = kama.ENC_P
    items = []

    def add(key, shape, color, explode):
        items.append(dict(key=key, shape=shape, color=hexc(color), explode=explode))

    add("govde", parts["govde"][0], C_BODY, [0, 0, 0])
    add("panel", parts["panel"][0], C_LID, nvec(70))
    add("yazi", parts["yazi_inlay"][0], C_TEXT, nvec(92))
    add("tutucu", parts["enkoder_tutucu"][0], (0.85, 0.55, 0.15), nvec(25))
    # dummy sırası: leo(4) gx(3x3) anahtar(3) lcd(3) enkoder(4)
    for s, c in dums[0:4]:
        add("leo", s, c, [0, 0, 0])
    for k in range(3):
        for s, c in dums[4 + 3 * k: 7 + 3 * k]:
            add(f"gx{k+1}", s, c, [0, 45, 0])
    for s, c in dums[13:16]:
        add("anahtar", s, c, [35, 0, 0])
    for s, c in dums[16:19]:
        add("lcd", s, c, nvec(40))
    for s, c in dums[19:21]:
        add("enkoder", s, c, nvec(45))
    for s, c in dums[21:23]:
        add("topuz", s, c, nvec(100))

    brass, steel = (0.80, 0.62, 0.25), (0.62, 0.64, 0.67)
    ring = lambda z0, z1: cyl(2.5, z0, z1).cut(cyl(1.5, z0 - 1, z1 + 1))
    for x, y in kama.BOSS_P:                          # gövde kulelerindeki insertler + panel vidaları
        add("insert", put(mv(ring(-4.2, -0.2), x, y), Pp), brass, nvec(22))
        screw = U(cq.Solid.makeCone(1.5, 3.0, 1.6, V(0, 0, PT - 1.6)), cyl(1.5, PT - 8, PT - 1.6))
        add("vida", put(mv(screw, x, y), Pp), steel, nvec(135))
    zb = PT - ENC_SKIN - ENC_BODY_H                    # tutucu üst yüzü (panel yerel)
    for sx in (-1, 1):
        x = ex + sx * ENC_RET_X
        add("insert", put(mv(ring(zb + 0.3, zb + 4.3), x, ey), Pp), brass, nvec(62))
        screw = U(cyl(2.75, zb - ENC_RET_T - 2, zb - ENC_RET_T), cyl(1.5, zb - ENC_RET_T, zb - ENC_RET_T + 6))
        add("vida", put(mv(screw, x, ey), Pp), steel, nvec(6))
    return items


INFO = {
    "govde": ("Gövde", "Basılır", f"{kama.W:.0f} × {kama.DP:.0f} × {kama.H1:.0f} mm · PLA+ · taban tablada, desteksiz. 4 eğik kule (insert), Leonardo ayakları, 3 × GX16, anahtar ve USB delikleri, 4 lastik ayak yuvası."),
    "panel": ("Üst panel", "Basılır", f"{kama.W:.0f} × {kama.L:.0f} × 3 mm, {math.degrees(kama.ANG):.0f}° eğik. Dış yüz tablada basılır. LCD penceresi, enkoder cebi, 4 × M3 havşa delik."),
    "yazi": ("eKO PEDAL yazısı", "Basılır", "0.6 mm dolgu yazı. Bambu Studio'da panelle 'tek nesne, çok parça' yükle, 2. renk ata (AMS)."),
    "tutucu": ("Enkoder tutucu", "Basılır", "36 × 22 × 3 mm. Enkoder gövdesini köşelerinden panele bastırır; artı boşluktan bacaklar geçer. 2 × M3×8."),
    "lcd": ("LCD 1602 + HW-61", "Elektronik", "Çerçeve 71.5 × 25.5 × 9.6, delik aralığı 74 × 30 (ölçüldü). Panel altındaki 4 kuleye (8.2 mm, Ø2.5 pilot) M2.5×6 ile."),
    "enkoder": ("Enkoder (EC11)", "Elektronik", "Gövde 12.0 × 12.2 × 6.5, bacak 3.5 (ölçüldü). Düz mil, cebe oturur."),
    "topuz": ("Metal topuz", "Elektronik", "Ø14.5 × 16.5 (ölçüldü). Panelden ~2 mm yukarıda kalacak şekilde mile tak."),
    "leo": ("Arduino Leonardo", "Elektronik", "68.5 × 54.0 (ölçüldü). Tabandaki 4 ayağa M3×6 sac vidası; micro-USB sol duvardan çıkar."),
    "gx1": ("GX16 – Pedal 1", "Elektronik", "Ø16.2 delik, arka duvar. Arkada ~15 mm yer kaplar."),
    "gx2": ("GX16 – Pedal 2", "Elektronik", "Ø16.2 delik, arka duvar."),
    "gx3": ("GX16 – Pedal 3", "Elektronik", "Ø16.2 delik, arka duvar."),
    "anahtar": ("Açma/kapama anahtarı", "Elektronik", "Mini rocker, gövde 13 × 8 × 17 (ölçüldü). Sağ yan duvarın altında, 13.2 × 8.3 delik; duvar 1.6 mm'ye inceltilmiş."),
    "insert": ("M3 × 4 × 5 insert (6)", "Bağlantı", "Ø4.4 delik (X2D/PLA kuponunda en iyi oturan). 4 gövdede, 2 panel altında."),
    "vida": ("M3 vidalar (6)", "Bağlantı", "4 × M3×8 havşa başlı (panel), 2 × M3×6 silindir başlı (enkoder tutucu)."),
}
GROUPS = [("Basılan parçalar", ["govde", "panel", "yazi", "tutucu"]),
          ("Elektronik", ["lcd", "enkoder", "topuz", "leo", "gx1", "gx2", "gx3", "anahtar"]),
          ("Bağlantı", ["insert", "vida"])]


def main():
    items = build_items()
    out, total = [], 0
    for it in items:
        m = mesh(it["shape"])
        total += m["tris"]
        e = it["explode"]
        out.append(dict(key=it["key"], color=it["color"], explode=[e[0], e[2], -e[1]], **m))
    data = dict(items=out, info=INFO, groups=GROUPS,
                center=[0, (kama.H1 + kama.PT) / 2, -kama.DP / 2], W=kama.W)
    tpl = (HERE / "goruntuleyici_sablon.html").read_text()
    hud = (f"{kama.W:.0f} × {kama.DP:.0f} × {kama.H1 + kama.PT:.0f} mm · panel {math.degrees(kama.ANG):.0f}° · X2D / PLA+")
    html = tpl.replace("__HUD__", hud).replace("__DATA__", json.dumps(data, ensure_ascii=False))
    dst = HERE / "cikti" / "EkoPedal_3B.html"
    dst.write_text(html)
    print(f"{len(out)} mesh, {total} üçgen, {dst.stat().st_size/1e6:.2f} MB -> {dst}")


if __name__ == "__main__":
    main()
