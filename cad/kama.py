"""Tasarım B – KAMA: eğik ön panelli masa konsolu.

Koordinat: X genişlik (merkez 0), Y derinlik (ön 0 -> arka DP), Z yukarı.
Gövde = taban + duvarlar (tabanı tablada basılır, destek yok).
Panel = düz plaka (dış yüz tablada basılır), 4 x M3 havşa vida -> eğik kulelerdeki ısıl insertlere.
"""
import math
import cadquery as cq
from ortak import *

W, DP, H0, H1 = 112.0, 100.0, 20.0, 56.0       # 24 Eki: 150x100x73 -> 112x100x59
WALL, BASE, PT, R = 2.4, 2.4, 3.0, 6.0
ANG = math.atan2(H1 - H0, DP)
SA, CA = math.sin(ANG), math.cos(ANG)
L = DP / CA                                          # panel boyu (eğim boyunca)

LCD_P = (0, 71)                                      # panel yerel (x, eğim boyunca y)
ENC_P = (34, 30)
BOSS_P = [(sx * (W / 2 - 7), y) for sx in (-1, 1) for y in (7, L - 7)]
GX_X = (-27, 0, 27)
GX_Z = 16
SW_P = (62, 16)                                      # sağ duvar (Y, Z): LCD'nin altında
LEO_X0, LEO_Y0 = -W / 2 + WALL + 3, 22


def build():
    hull = (cq.Workplane("YZ").polyline([(0, 0), (DP, 0), (DP, H1), (0, H0)]).close()
            .extrude(W / 2, both=True).edges("|Z").fillet(R).val())
    inner = rrect(W - 2 * WALL, DP - 2 * WALL, R - WALL, BASE, 200, 0, DP / 2)
    Pp = plane((0, 0, H0), (1, 0, 0), (0, -SA, CA))  # panel alt yüzü = gövde üst düzlemi
    bosses = U(*[put(cyl(4.5, -150, 0, x, y), Pp) for x, y in BOSS_P])
    tub = hull.cut(inner).fuse(bosses).intersect(hull)

    cuts, dums = [], []
    cuts += [put(cyl(INSERT_R, -INSERT_L, 0.5, x, y), Pp) for x, y in BOSS_P]
    # Leonardo standoffları (tabanda)
    so = [(LEO_X0 + hx, LEO_Y0 + hy) for hx, hy in LEO_HOLES]
    tub = tub.fuse(U(*[cyl(LEO_STANDOFF_R, BASE - 0.1, BASE + LEO_STANDOFF, x, y) for x, y in so]))
    cuts += [cyl(LEO_PILOT_R, LEO_PILOT_FLOOR, BASE + LEO_STANDOFF + 0.1, x, y) for x, y in so]   # M3, 6.6 derin
    dums += [(mv(s, LEO_X0, LEO_Y0, BASE + LEO_STANDOFF), c) for s, c in leo_dummy()]
    # Lastik ayak yuvaları
    cuts += [cyl(5.25, -1, 1.0, x, y) for x in (-W / 2 + 14, W / 2 - 14) for y in (12, 88)]
    # Arka duvar: 3 x GX16 + numaralar
    Pb = plane((0, DP, 0), (-1, 0, 0), (0, 1, 0))    # yerel (-X, Z)
    for i, x in enumerate(GX_X):
        cuts.append(put(mv(gx_cut(WALL), x, GX_Z), Pb))
        cuts.append(put(mv(text_solid(str(i + 1), 7, 0.6, x, GX_Z + 15), 0, 0, 0.01), Pb))
        dums += place_dummy(gx_dummy(), Pb, x, GX_Z)
    # Sağ duvar: anahtar
    Pr = plane((W / 2, 0, 0), (0, 1, 0), (1, 0, 0))
    cuts.append(put(mv(sw_cut(WALL), *SW_P), Pr))
    dums += place_dummy(sw_dummy(), Pr, *SW_P)
    # Sol duvar: USB
    Pl = plane((-W / 2, 0, 0), (0, -1, 0), (-1, 0, 0))
    cuts.append(put(mv(usb_cut(WALL), -(LEO_Y0 + LEO_USB_Y), BASE + LEO_USB_Z), Pl))
    tub = tub.cut(U(*cuts))

    # ---- Panel (yerel: z=0 alt yüz, z=PT dış yüz) ----
    panel = (cq.Workplane("XY").center(0, L / 2).rect(W, L).extrude(PT)
             .edges("|Z").fillet(R).faces(">Z").edges().chamfer(1.0).val())
    a, c = lcd_bosses(PT)
    panel = panel.fuse(mv(a, LCD_P[0], LCD_P[1], PT), mv(enc_cradle(PT), ENC_P[0], ENC_P[1], PT))
    ra, rc = enc_retainer_bosses(PT)
    panel = panel.fuse(mv(ra, ENC_P[0], ENC_P[1], PT))
    pc = [mv(c, LCD_P[0], LCD_P[1], PT), mv(lcd_cut(PT), LCD_P[0], LCD_P[1], PT),
          # LCD çerçeve OLUĞU (0.8 geniş, 0.6 derin). Eskiden 88x48 düz girintiydi: ön yüz tablada basılınca
          # girinti tabanı 0.6 mm havada kalıyordu (kupon 2'de LCD bölümü tablaya tutmadı).
          mv(rrect(88, 48, 4, -0.6, 1, *LCD_P).cut(rrect(86.4, 46.4, 3.2, -1, 2, *LCD_P)), 0, 0, PT),
          mv(enc_cut(PT), ENC_P[0], ENC_P[1], PT),
          mv(rc, ENC_P[0], ENC_P[1], PT),
          mv(cyl(KNOB_D / 2 + 4.8, -0.6, 1).cut(cyl(KNOB_D / 2 + 4.0, -1, 2)), ENC_P[0], ENC_P[1], PT)]
    for x, y in BOSS_P:                              # M3 havşa
        pc += [cyl(M3_R, -1, PT + 1, x, y),
               cq.Solid.makeCone(M3_R, 3.3, 1.6, V(x, y, PT - 1.6)), cyl(3.3, PT - 0.01, PT + 1, x, y)]
    panel = panel.cut(U(*pc))

    pd = []
    pd += [(mv(sh, LCD_P[0], LCD_P[1], PT), col) for sh, col in lcd_dummy()]
    pd += [(mv(sh, ENC_P[0], ENC_P[1], PT), col) for sh, col in knob_dummy()]
    dums += [(put(sh, Pp), col) for sh, col in pd]

    ret = mv(enc_retainer(), ENC_P[0], ENC_P[1], PT)
    parts = {"govde": (tub, C_BODY), "panel": (put(panel, Pp), C_LID),
             "enkoder_tutucu": (put(ret, Pp), C_LID)}
    local = {"panel": panel, "enkoder_tutucu": ret}
    return parts, dums, local


def print_pose(name, shape, local):
    """Baskı yönü: gövde taban tablada, panel ön yüz tablada, tutucu düz."""
    if name == "enkoder_tutucu":
        shape = local[name]
        bb = shape.BoundingBox()
        return shape.translate(V(-bb.center.x, -bb.center.y, -bb.zmin))
    if name == "panel":
        flip = lambda s: s.rotate(V(0, 0, 0), V(1, 0, 0), 180)
        bb = flip(local["panel"]).BoundingBox()
        return flip(local[name]).translate(V(-bb.center.x, -bb.center.y, -bb.zmin))
    bb = shape.BoundingBox()
    return shape.translate(V(-bb.center.x, -bb.center.y, -bb.zmin))
