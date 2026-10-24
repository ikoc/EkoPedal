"""EkoPedal ortak ölçüler, kesiciler ve görsel yer tutucular (dummy).

Kesici/dummy fonksiyonları YEREL koordinatta çalışır:
  panelin dış yüzü z=0, malzeme z<0 tarafında, dışa bakan yön +Z.
`put(shape, plane)` ile duvar/panel üzerine taşınır.
Ölçüler tipik değerlerdir -> parçaları kumpasla doğrula (README tablosu).
"""
import math
from functools import reduce
import cadquery as cq
from cadquery import Vector as V

# ---------------- Bileşen ölçüleri (mm) ----------------
LCD_PCB = (80.0, 36.0, 1.6)                 # 1602 LCD kartı
LCD_HOLE_PITCH = (74.0, 30.0)               # merkezden merkeze: dış 77 x 33 - Ø3 (ölçüldü 24 Eki)
LCD_HOLES = [(sx * LCD_HOLE_PITCH[0] / 2, sy * LCD_HOLE_PITCH[1] / 2) for sx in (-1, 1) for sy in (-1, 1)]
LCD_BEZEL = (71.5, 25.5)                    # siyah çerçeve (ölçüldü 24 Eki)
LCD_BEZEL_H = 9.6                           # çerçevenin PCB üstünden yüksekliği (ölçüldü)
LCD_VIEW = (64.5, 16.0)                     # görünür alan
LCD_SKIN = 1.6                              # pencere çevresinde kalan panel et kalınlığı

ENC_HOLE = 7.4                              # EC11 M7 burç
KNOB_D = 14.5                               # metal topuz (ölçüldü)
KNOB_H = 16.5
ENC_SQ = (12.0, 12.2)                       # enkoder gövdesi (ölçüldü)
ENC_BODY_H = 6.5                            # 23.0 - 16.5
ENC_LEGS = 3.5                              # 26.5 - 23.0
ENC_SKIN = 1.6                              # enkoder yerinde panel kalınlığı (topuz boşluğu için)

SW_BODY = (13.0, 8.0, 17.0)                 # mini rocker gövdesi en x boy x derinlik (ölçüldü 24 Eki)
SW_CUT = (SW_BODY[0] + 0.2, SW_BODY[1] + 0.3)  # pano deliği 13.2 x 8.3
SW_TAB = 16.0                               # 13 mm kenarda tırnaklar açıkken (ölçüldü)
SW_SKIN = 1.6                               # tırnakların tutması için duvar inceltme

GX_HOLE = 16.2                              # GX16 havacılık konnektörü
USB_CUT = (13.0, 9.0)                       # micro-USB fiş gövdesi geçsin

LEO = (68.5, 54.0)                          # Arduino Leonardo (ölçüldü)
LEO_HOLES = [(14.0, 2.5), (15.3, 50.7), (66.1, 7.6), (66.1, 35.5)]
LEO_USB_Y = 54.0 - 16.0                     # micro-USB merkezi: karşı köşeden 12.0..20.0 ölçüldü
LEO_STANDOFF = 5.0
LEO_USB_Z = LEO_STANDOFF + 1.6 + 1.4        # kart tabanından USB merkezine (standoff dahil)

INSERT_R = 2.2                              # M3x4x5 insert: X2D kuponunda en iyi Ø4.4 (Auvidea v12f)
INSERT_L = 6.5
M3_R = 1.7

# Renkler
C_BODY = (0.17, 0.18, 0.20)
C_LID = (0.26, 0.27, 0.29)
C_LCD = (0.15, 0.40, 0.95)
C_PCB = (0.10, 0.45, 0.25)
C_LEO = (0.05, 0.45, 0.55)
C_METAL = (0.78, 0.79, 0.81)
C_BLACK = (0.05, 0.05, 0.06)
C_TEXT = (0.92, 0.72, 0.30)


# ---------------- Temel yardımcılar ----------------
def U(*shapes):
    shapes = [s for s in shapes if s is not None]
    return reduce(lambda a, b: a.fuse(b), shapes).clean()


def box(x0, x1, y0, y1, z0, z1):
    return cq.Solid.makeBox(x1 - x0, y1 - y0, z1 - z0, V(x0, y0, z0))


def cyl(r, z0, z1, x=0.0, y=0.0):
    return cq.Solid.makeCylinder(r, z1 - z0, V(x, y, z0))


def rrect(w, h, r, z0, z1, cx=0.0, cy=0.0):
    wp = cq.Workplane("XY").workplane(offset=z0).center(cx, cy).rect(w, h).extrude(z1 - z0)
    if r > 0:
        wp = wp.edges("|Z").fillet(r)
    return wp.val()


def plane(origin, xdir, normal):
    return cq.Plane(V(*origin), V(*xdir), V(*normal))


def put(shape, pl):
    return shape.transformShape(pl.rG)


def mv(shape, x=0.0, y=0.0, z=0.0):
    return shape.translate(V(x, y, z))


def text_solid(s, size, depth, x=0.0, y=0.0):
    t = cq.Workplane("XY").text(s, size, -depth, kind="bold", halign="center", valign="center",
                                combine=False).val()
    return mv(t, x, y, 0.0)


# ---------------- Kesiciler (yerel) ----------------
def lcd_cut(t):
    w, h = LCD_VIEW[0] + 1.5, LCD_VIEW[1] + 1.5
    c = 0.8
    win = rrect(w, h, 0.6, -t - 1, 1)
    cham = (cq.Workplane("XY").workplane(offset=-c).rect(w, h)
            .workplane(offset=c + 0.02).rect(w + 2 * c + 0.04, h + 2 * c + 0.04).loft().val())
    pocket = rrect(LCD_BEZEL[0] + 1.2, LCD_BEZEL[1] + 0.5, 0.5, -t - 1, -LCD_SKIN)
    return U(win, cham, pocket)


def lcd_bosses(t):
    """(eklenecek, kesilecek) – LCD kartını tutan 4 kule, M2.5/M3 vida."""
    front = -LCD_SKIN - LCD_BEZEL_H
    add = U(*[cyl(2.5, front, -t + 0.3, x, y) for x, y in LCD_HOLES])
    # çerçeveye bakan yüz traşlanır: çerçeve ile kule arasında 0.3 mm boşluk
    add = add.cut(box(-LCD_BEZEL[0] / 2 - 0.3, LCD_BEZEL[0] / 2 + 0.3,
                      -LCD_BEZEL[1] / 2 - 0.3, LCD_BEZEL[1] / 2 + 0.3, front - 1, 1))
    cut = U(*[cyl(1.25, front - 0.1, front + 6, x, y) for x, y in LCD_HOLES])
    return add, cut


def enc_cut(t):
    """Mil deliği + alttan cep: gövde panele 1.6 mm kalana kadar girer (topuz 2 mm yükselir)."""
    w, h = ENC_SQ[0] + 0.4, ENC_SQ[1] + 0.4
    return U(cyl(ENC_HOLE / 2, -t - 1, 1), box(-w / 2, w / 2, -h / 2, h / 2, -t - 10, -ENC_SKIN))


def enc_cradle(t):
    """Panel altına 4 köşe (L) direği: enkoder gövdesini ortalar, yan tırnak/bacaklar boşlukta kalır."""
    w, h, k, arm = ENC_SQ[0] + 0.4, ENC_SQ[1] + 0.4, 1.8, 3.5
    z0, z1 = -ENC_SKIN - ENC_BODY_H + 0.5, -t + 0.3
    ring = box(-w / 2 - k, w / 2 + k, -h / 2 - k, h / 2 + k, z0, z1)
    ring = ring.cut(box(-w / 2, w / 2, -h / 2, h / 2, z0 - 1, z1 + 1))
    ring = ring.cut(box(-w / 2 + arm, w / 2 - arm, -50, 50, z0 - 1, z1 + 1))
    ring = ring.cut(box(-50, 50, -h / 2 + arm, h / 2 - arm, z0 - 1, z1 + 1))
    return ring


ENC_RET_X = 13.5                            # tutucu vidalarının enkoder merkezine uzaklığı
ENC_RET_T = 3.0                             # tutucu plaka kalınlığı


def enc_retainer_bosses(t):
    """(eklenecek, kesilecek): tutucu plaka için 2 kule + M3 insert deliği.
    Kule alt yüzü enkoder gövdesinin altından 0.3 yukarıda -> vida sıkınca gövdeyi bastırır."""
    zb = -ENC_SKIN - ENC_BODY_H + 0.3
    add = U(*[cyl(4.0, zb, -t + 0.3, sx * ENC_RET_X) for sx in (-1, 1)])
    # 5.0 derin: insert 4 + 1 pay; ön yüzde 2.8 mm (oluk altında 2.2) kalır -> ısıl insert yüzü kabartmaz. Vida M3x6.
    cut = U(*[cyl(INSERT_R, zb - 0.1, zb + 5.0, sx * ENC_RET_X) for sx in (-1, 1)])
    return add, cut


def enc_retainer():
    """Tutucu plaka (yerel, panel dış yüzü z=0). Artı boşluk: bacak/tırnak/kablo geçer, köşeler bastırır."""
    zt = -ENC_SKIN - ENC_BODY_H
    p = rrect(2 * ENC_RET_X + 9, 22, 4, zt - ENC_RET_T, zt)
    p = p.cut(box(-8, 8, -3.75, 3.75, zt - 5, zt + 1)).cut(box(-3.75, 3.75, -8, 8, zt - 5, zt + 1))
    p = p.cut(U(*[cyl(M3_R, zt - 5, zt + 1, sx * ENC_RET_X) for sx in (-1, 1)]))
    return p


def sw_cut(t):
    c = rrect(SW_CUT[0], SW_CUT[1], 0.4, -t - 1, 1)
    if t > SW_SKIN:
        # iç cep: duvar 1.6'ya incelir; tırnakların açılacağı yönde en az SW_TAB + 2
        c = U(c, rrect(max(SW_CUT[0] + 6, SW_TAB + 2), SW_CUT[1] + 6, 1.0, -t - 1, -SW_SKIN))
    return c


def gx_cut(t):
    return cyl(GX_HOLE / 2, -t - 1, 1)


def usb_cut(t):
    return rrect(USB_CUT[0], USB_CUT[1], 2.0, -t - 1, 1)


# ---------------- Görsel yer tutucular (yerel) ----------------
def lcd_dummy():
    f = -LCD_SKIN - LCD_BEZEL_H
    return [
        (rrect(LCD_BEZEL[0], LCD_BEZEL[1], 0.3, f, -LCD_SKIN - 0.05), C_LCD),
        (box(-40, 40, -18, 18, f - 1.6, f), C_PCB),
        (box(-21, 21, 0, 18, f - 1.6 - 11, f - 1.6), C_BLACK),   # HW-61 I2C sırt kartı
    ]


def knob_dummy():
    return [
        (box(-ENC_SQ[0] / 2, ENC_SQ[0] / 2, -ENC_SQ[1] / 2, ENC_SQ[1] / 2,
             -ENC_SKIN - ENC_BODY_H, -ENC_SKIN - 0.05), C_METAL),        # EC11 gövdesi
        (box(-4, 4, -0.5, 0.5, -ENC_SKIN - ENC_BODY_H - ENC_LEGS, -ENC_SKIN - ENC_BODY_H), C_METAL),
        (cyl(KNOB_D / 2, 0.4, 0.4 + KNOB_H - 1.5), C_METAL),
        (cyl(KNOB_D / 2 - 1.5, 0.4 + KNOB_H - 1.5, 0.4 + KNOB_H), (0.6, 0.61, 0.63)),
    ]


def gx_dummy():
    return [
        (cyl(9.75, 0, 2.0), C_METAL),
        (cyl(8.0, 2.0, 11.0), C_METAL),
        (cyl(8.0, -16, -2.5), C_METAL),                 # arka gövde + lehim kulakları
    ]


def sw_dummy():
    return [
        (rrect(SW_BODY[0] + 2, SW_BODY[1] + 2, 0.6, 0, 1.5), C_BLACK),          # ön çerçeve (yaklaşık)
        (rrect(SW_BODY[0] - 1.5, SW_BODY[1] - 1.5, 0.6, 1.5, 4.0), C_BLACK),    # rocker
        (box(-SW_BODY[0] / 2, SW_BODY[0] / 2, -SW_BODY[1] / 2, SW_BODY[1] / 2, -SW_BODY[2], -0.1), C_BLACK),
    ]


def leo_dummy():
    """Kart, kart tabanı z=0, kart x/y ekseninde (USB kenarı x=0)."""
    return [
        (box(0, LEO[0], 0, LEO[1], 0, 1.6), C_LEO),
        (box(-1.0, 6.0, LEO_USB_Y - 4, LEO_USB_Y + 4, 1.6, 4.4), C_METAL),
        (box(18, 66, 50.5, 53, 1.6, 10), C_BLACK),
        (box(26, 66, 0.3, 2.8, 1.6, 10), C_BLACK),
    ]


def place_dummy(items, pl, x=0.0, y=0.0, z=0.0):
    return [(put(mv(s, x, y, z), pl), c) for s, c in items]
