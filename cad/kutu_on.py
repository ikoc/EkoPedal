"""Önden panelli kutu: Tasarım A (Kule) ve C (Yatay).

Koordinat: X genişlik (merkez 0), Y yükseklik (alt kenar 0), Z derinlik (arka 0, ön yüz z=D).
Baskı: gövde ÖN YÜZ tablada (destek yok), kapak dış yüz tablada.
"""
import cadquery as cq
from ortak import *

WALL, TF, LID = 2.4, 2.8, 3.0


def build(p):
    W, H, D, R, FIL = p["W"], p["H"], p["D"], p["R"], p["FIL"]
    zin = D - TF                                    # ön panelin iç yüzü
    zc = (LID + zin) / 2                            # iç derinlik ortası
    outer = (cq.Workplane("XY").center(0, H / 2).rect(W, H).extrude(D)
             .edges("|Z").fillet(R).faces(">Z").edges().fillet(FIL).val())
    inner = rrect(W - 2 * WALL, H - 2 * WALL, R - WALL, LID - 1, zin, 0, H / 2)
    back = box(-W, W, -1, H + 1, LID, D + 1)

    bpos = [(sx * (W / 2 - p["BOSS"]), y) for sx in (-1, 1) for y in (p["BOSS"], H - p["BOSS"])]
    bosses = U(*[cyl(4.5, LID, zin + 0.5, x, y) for x, y in bpos])
    shell = outer.intersect(back).cut(inner).fuse(bosses).intersect(outer).intersect(back)

    adds, cuts, dums = [], [], []
    Pf = plane((0, 0, D), (1, 0, 0), (0, 0, 1))
    Pb = plane((0, 0, 0), (1, 0, 0), (0, -1, 0))      # alt duvar, yerel (X, Z)
    Pt = plane((0, H, 0), (1, 0, 0), (0, 1, 0))       # üst duvar, yerel (X, -Z)
    Pr = plane((W / 2, 0, 0), (0, 1, 0), (1, 0, 0))   # sağ duvar, yerel (Y, Z)
    Pl = plane((-W / 2, 0, 0), (0, -1, 0), (-1, 0, 0))  # sol duvar, yerel (-Y, Z)

    # Ön panel
    lx, ly = p["LCD"]
    a, c = lcd_bosses(TF)
    adds.append(put(mv(a, lx, ly), Pf)); cuts.append(put(mv(c, lx, ly), Pf))
    cuts.append(put(mv(lcd_cut(TF), lx, ly), Pf))
    dums += place_dummy(lcd_dummy(), Pf, lx, ly)
    ex, ey = p["ENC"]
    cuts.append(put(mv(enc_cut(TF), ex, ey), Pf))
    dums += place_dummy(knob_dummy(), Pf, ex, ey)
    for (w, h, r, cx, cy, d) in p.get("RECESS", []):
        cuts.append(put(rrect(w, h, r, -d, 1, cx, cy), Pf))
    if p.get("RING"):
        cuts.append(put(mv(cyl(KNOB_D / 2 + 4.8, -0.6, 1).cut(cyl(KNOB_D / 2 + 4.0, -1, 2)), ex, ey), Pf))
        for i in range(11):                         # ölçek çizgileri
            ang = -135 + i * 27
            t = box(-0.5, 0.5, KNOB_D / 2 + 6, KNOB_D / 2 + 9, -0.6, 1)
            cuts.append(put(mv(t.rotate(V(0, 0, 0), V(0, 0, 1), ang), ex, ey), Pf))
    inlay = None
    if p.get("STICKER"):                             # yalnız render: basılı etiket görünümü
        w, h, cx, cy = p["STICKER"]
        dums.append((put(rrect(w - 1, h - 1, 2.5, -0.5, -0.1, cx, cy), Pf), (0.20, 0.16, 0.12)))
        dums.append((put(mv(text_solid("EKO PEDAL", 11, 0.3, cx, cy), 0, 0, 0.1), Pf), C_TEXT))
    if p.get("TEXT"):
        s, size, tx, ty = p["TEXT"]
        cuts.append(put(mv(text_solid(s, size, 0.6, tx, ty), 0, 0, 0.01), Pf))
        inlay = put(text_solid(s, size, 0.6, tx, ty), Pf)

    # Alt duvar: 3 x GX16
    for x in p["GX_X"]:
        cuts.append(put(mv(gx_cut(WALL), x, zc), Pb))
        dums += place_dummy(gx_dummy(), Pb, x, zc)

    # Anahtar
    side, u = p["SW"]
    Psw, loc = {"right": (Pr, (u, zc)), "top": (Pt, (u, -zc))}[side]
    swc = sw_cut(WALL)
    swd = sw_dummy()
    cuts.append(put(mv(swc, *loc), Psw))
    dums += place_dummy(swd, Psw, *loc)

    # Leonardo (kapakta)
    X0, Y0, rot = p["LEO"]                           # rot=True -> 180° (USB sağ duvarda)
    def leo_xy(hx, hy):
        return (X0 - hx, Y0 - hy) if rot else (X0 + hx, Y0 + hy)
    usb_y = leo_xy(0, LEO_USB_Y)[1]
    usb_z = LID + LEO_USB_Z
    if rot:
        cuts.append(put(mv(usb_cut(WALL), usb_y, usb_z), Pr))
    else:
        cuts.append(put(mv(usb_cut(WALL), -usb_y, usb_z), Pl))
    leo = [(s.rotate(V(0, 0, 0), V(0, 0, 1), 180) if rot else s, col) for s, col in leo_dummy()]
    dums += [(mv(s, X0, Y0, LID + LEO_STANDOFF), col) for s, col in leo]

    shell = shell.fuse(*adds).clean()
    shell = shell.cut(U(*cuts))
    shell = shell.cut(U(*[cyl(INSERT_R, LID - 0.1, LID + INSERT_L, x, y) for x, y in bpos]))

    # ---- Kapak ----
    lid = outer.intersect(box(-W, W, -1, H + 1, 0, LID))
    lip = rrect(W - 2 * WALL - 0.5, H - 2 * WALL - 0.5, R - WALL - 0.25, LID - 0.1, LID + 2, 0, H / 2).cut(
        rrect(W - 2 * WALL - 3.3, H - 2 * WALL - 3.3, R - WALL - 1.6, LID - 1, LID + 3, 0, H / 2))
    lip = lip.cut(U(*[cyl(5.5, 0, 10, x, y) for x, y in bpos]))
    so = [leo_xy(hx, hy) for hx, hy in LEO_HOLES]
    stand = U(*[cyl(3.0, LID - 0.1, LID + LEO_STANDOFF, x, y) for x, y in so])
    lid = lid.fuse(lip, stand).clean()
    lcuts = [cyl(1.3, LID + 0.6, LID + LEO_STANDOFF + 0.1, x, y) for x, y in so]
    for x, y in bpos:
        lcuts += [cyl(M3_R, -1, LID + 1, x, y), cyl(3.1, -1, 1.6, x, y)]
    for x, y in p.get("FEET", []):
        lcuts.append(cyl(5.25, -1, 1.0, x, y))
    for x, y in p.get("KEYHOLE", []):               # duvara asma
        lid = lid.fuse(box(x - 7, x + 7, y - 11, y + 11, LID - 0.1, LID + 5)
                       .cut(box(x - 5.4, x + 5.4, y - 9.4, y + 9.4, LID - 1, LID + 3.4)))
        lcuts += [cyl(3.75, -1, LID + 0.5, x, y - 3), box(x - 1.9, x + 1.9, y - 3, y + 5, -1, LID + 0.5)]
    lid = lid.cut(U(*lcuts))

    parts = {"govde": (shell, C_BODY), "kapak": (lid, C_LID)}
    if inlay is not None:
        parts["yazi_inlay"] = (inlay, C_TEXT)
    if p.get("WEDGE"):
        wl, wt0, wt1, wx = p["WEDGE"]
        w = (cq.Workplane("YZ").polyline([(0, 0), (wl, 0), (wl, wt1), (0, wt0)]).close()
             .extrude(6, both=True).edges("|X").fillet(1.0).val())
        y0 = (H - wl) / 2
        for i, x in enumerate((-wx, wx)):
            parts[f"egim_ayagi_{i+1}"] = (w.mirror("XY").translate(V(x, y0, 0)), C_LID)
    return parts, dums


def print_pose(name, shape):
    """Baskı yönü: gövde ön yüz tabla; kapak olduğu gibi; ayak yan yatık."""
    if name == "govde" or name == "yazi_inlay":
        shape = shape.rotate(V(0, 0, 0), V(1, 0, 0), 180)
    elif name.startswith("egim"):
        shape = shape.rotate(V(0, 0, 0), V(0, 1, 0), 90)
    bb = shape.BoundingBox()
    return shape.translate(V(-bb.center.x, -bb.center.y, -bb.zmin))


def to_view(shape):
    """Sunum: Y yukarı -> Z yukarı, ön yüz kameraya (-Y)."""
    return shape.rotate(V(0, 0, 0), V(1, 0, 0), 90)


# ---------------- Parametre setleri ----------------
A = dict(  # KULE – referansa en yakın dikey kutu
    W=104, H=172, D=44, R=10, FIL=2.5, BOSS=7,
    LCD=(0, 140), ENC=(0, 26),
    RECESS=[(94, 52, 4, 0, 140, 0.6), (88, 68, 3, 0, 78, 0.5)],   # LCD çerçevesi + etiket yuvası
    STICKER=(88, 68, 0, 78),
    GX_X=(-28, 0, 28), SW=("right", 100),
    LEO=(-104 / 2 + 2.4 + 3, 50, False),
    FEET=[(-38, 20), (38, 20), (-38, 152), (38, 152)],
    KEYHOLE=[(-30, 150), (30, 150)],
)
C = dict(  # YATAY – geniş, alçak, masa üstü + eğim ayağı
    W=180, H=90, D=46, R=16, FIL=3.0, BOSS=8,
    LCD=(-40, 50), ENC=(55, 47), RING=True,
    RECESS=[(92, 50, 6, -40, 50, 0.6)],
    TEXT=("EKO PEDAL", 7.5, -40, 15.5),
    GX_X=(-40, 0, 40), SW=("top", -55),
    LEO=(180 / 2 - 2.4 - 3, 75, True),
    WEDGE=(70, 3, 16, 62),
)
