#!/usr/bin/env python3
"""
OpenSpatialDelay — Signal Flow Diagram (Pass 4: Orbital Horizon).

Design intent
-------------
Canvas is 2400 x 900 (aspect 8:3). The DOCX manual renders the image
at 480 x 180 pt via `generate_manual.js`, which is exactly 8:3 — so
the diagram displays without horizontal distortion in the manual.
Passes 1-3 drew at ~16:9 and got squashed inside the page.

Layout is horizontal left-to-right with a 12-tap radial fan at the
compositional centre, honouring the "Orbital Cartography" design
philosophy: every layout radiates from a centre. The dry path
(AMBER) and feedback loop (VIOLET) are drawn as cubic Bezier arcs,
not right-angle routes — "computed trajectories, not decorative
swooshes".

Labels are capability-level only. No DSP jargon on a public surface.
"""

from PIL import Image, ImageDraw, ImageFont
import math
import os


# ---------------------------------------------------------------------------
# Canvas & palette
# ---------------------------------------------------------------------------
W, H = 2400, 900

BG          = (10, 10, 20)
PANEL       = (14, 14, 28)
PANEL_2     = (20, 20, 38)
GRID        = (20, 20, 35, 90)
GRID_MAJOR  = (26, 30, 46, 140)
RING        = (46, 78, 102, 220)
RING_FAINT  = (30, 52, 70, 150)
RING_MAJOR  = (60, 100, 130, 240)

CYAN        = (128, 216, 255)
CYAN_MID    = (72, 140, 180)
CYAN_DIM    = (54, 94, 120)
CYAN_FAINT  = (32, 54, 72)

VIOLET      = (133, 112, 215)   # WCAG-AA adjusted
VIOLET_DIM  = (80, 62, 150)
VIOLET_FNT  = (48, 36, 90)

AMBER       = (240, 166, 70)
AMBER_DIM   = (146, 100, 42)
AMBER_FNT   = (74, 52, 24)

WHITE       = (225, 229, 234)
DIM         = (152, 160, 176)
DIM_2       = (98, 104, 120)

# ---------------------------------------------------------------------------
# Geometry budget
# ---------------------------------------------------------------------------
CENTRE_Y    = 500

IN_X,   IN_W   = 80,   260          # INPUT block
GAIN_X, GAIN_W = 440,  170          # INPUT GAIN
SUM_X          = 680                # feedback summer ⊕
BUF_X,  BUF_W  = 720,  190          # DELAY BUFFER
FAN_ORIGIN     = (BUF_X + BUF_W, CENTRE_Y)   # (910, 500)
FAN_END_X      = 1300
DSP_X,  DSP_W  = 1340, 280          # PER-TAP DSP panel
SPAT_X, SPAT_W = 1660, 320          # SPATIAL RENDER panel
MIX_X,  MIX_W  = 2000, 170          # MIX block
OUT_X,  OUT_W  = 2200, 140          # OUTPUT block


# ---------------------------------------------------------------------------
# File IO
# ---------------------------------------------------------------------------
FONT_DIR = os.path.join(os.path.dirname(__file__), "..", "fonts")
OUT_DIR  = os.path.join(os.path.dirname(__file__), "assets")
OUT_PATH = os.path.join(OUT_DIR, "signal-flow.png")


def load_font(name, size):
    return ImageFont.truetype(os.path.join(FONT_DIR, name), size)


# ---------------------------------------------------------------------------
# Bezier sampler (cubic)
# ---------------------------------------------------------------------------
def cubic_bezier(p0, p1, p2, p3, steps=240):
    pts = []
    for i in range(steps + 1):
        t = i / steps
        mt = 1 - t
        x = (mt ** 3) * p0[0] + 3 * (mt ** 2) * t * p1[0] \
            + 3 * mt * (t ** 2) * p2[0] + (t ** 3) * p3[0]
        y = (mt ** 3) * p0[1] + 3 * (mt ** 2) * t * p1[1] \
            + 3 * mt * (t ** 2) * p2[1] + (t ** 3) * p3[1]
        pts.append((x, y))
    return pts


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # Fonts — title large, coordinate/annotation mono small
    dm_b_40   = load_font("DM_Sans-Bold.ttf",       40)
    dm_b_30   = load_font("DM_Sans-Bold.ttf",       30)
    dm_b_26   = load_font("DM_Sans-Bold.ttf",       26)
    dm_b_22   = load_font("DM_Sans-Bold.ttf",       22)
    dm_b_20   = load_font("DM_Sans-Bold.ttf",       20)
    dm_sb_20  = load_font("DM_Sans-SemiBold.ttf",   20)
    dm_sb_18  = load_font("DM_Sans-SemiBold.ttf",   18)
    dm_m_18   = load_font("DM_Sans-Medium.ttf",     18)
    dm_m_16   = load_font("DM_Sans-Medium.ttf",     16)
    dm_r_14   = load_font("DM_Sans-Regular.ttf",    14)
    jb_m_14   = load_font("JetBrains_Mono-Medium.ttf",  14)
    jb_m_12   = load_font("JetBrains_Mono-Medium.ttf",  12)
    jb_r_13   = load_font("JetBrains_Mono-Regular.ttf", 13)
    jb_r_12   = load_font("JetBrains_Mono-Regular.ttf", 12)
    jb_r_11   = load_font("JetBrains_Mono-Regular.ttf", 11)
    jb_r_10   = load_font("JetBrains_Mono-Regular.ttf", 10)

    img = Image.new("RGBA", (W, H), BG + (255,))
    d   = ImageDraw.Draw(img, "RGBA")

    # ======================================================================
    # Local draw helpers
    # ======================================================================
    def rect(x, y, w, h, r, fill, bd, bw=2):
        d.rounded_rectangle([x, y, x + w, y + h], radius=r,
                            fill=fill, outline=bd, width=bw)

    def text_size(txt, font):
        bb = font.getbbox(txt)
        return bb[2] - bb[0], bb[3] - bb[1], bb[1]  # w, h, top_offset

    def centre_text(cx, cy, txt, font, color):
        tw, th, toff = text_size(txt, font)
        d.text((cx - tw // 2, cy - th // 2 - toff), txt,
               fill=color, font=font)

    def left_text(x, y, txt, font, color):
        _, _, toff = text_size(txt, font)
        d.text((x, y - toff), txt, fill=color, font=font)

    def block(x, y, w, h, label, sub=None, *, fill=PANEL, border=CYAN_DIM,
              bw=2, radius=12, title_font=dm_b_22, title_color=CYAN,
              sub_font=jb_r_12, sub_color=DIM):
        rect(x, y, w, h, radius, fill, border, bw)
        if sub:
            tw, th, toff = text_size(label, title_font)
            tx = x + (w - tw) // 2
            ty = y + h // 2 - th - 2 - toff
            d.text((tx, ty), label, fill=title_color, font=title_font)
            sw, sh, soff = text_size(sub, sub_font)
            sx = x + (w - sw) // 2
            sy = y + h // 2 + 6 - soff
            d.text((sx, sy), sub, fill=sub_color, font=sub_font)
        else:
            centre_text(x + w // 2, y + h // 2, label, title_font,
                        title_color)

    def arrow_h(x1, y, x2, color=CYAN, width=3, head=11):
        d.line([(x1, y), (x2 - head + 1, y)], fill=color, width=width)
        d.polygon([
            (x2, y),
            (x2 - head, y - head // 2),
            (x2 - head, y + head // 2),
        ], fill=color)

    def arrow_tip(tip, prev, color, head=12):
        dx, dy = tip[0] - prev[0], tip[1] - prev[1]
        L = math.sqrt(dx * dx + dy * dy)
        if L == 0:
            return
        ux, uy = dx / L, dy / L
        base = (tip[0] - ux * head, tip[1] - uy * head)
        perp = (-uy * head * 0.55, ux * head * 0.55)
        d.polygon([
            tip,
            (base[0] + perp[0], base[1] + perp[1]),
            (base[0] - perp[0], base[1] - perp[1]),
        ], fill=color)

    def pill(cx, cy, txt, font, fg=DIM, bg=BG, bd=(44, 48, 62),
             padx=10, pady=4):
        tw, th, toff = text_size(txt, font)
        w = tw + padx * 2
        h = th + pady * 2 + 2
        x = cx - w // 2
        y = cy - h // 2
        rect(x, y, w, h, 5, bg, bd, 1)
        d.text((x + padx, y + pady - toff), txt, fill=fg, font=font)

    def dot(cx, cy, r, fill):
        d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=fill)

    def ring_outline(cx, cy, r, color, width=1):
        d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=color,
                  width=width)

    # ======================================================================
    # Background grid + orbital field
    # ======================================================================
    for gx in range(0, W, 40):
        d.line([(gx, 0), (gx, H)], fill=GRID, width=1)
    for gy in range(0, H, 40):
        d.line([(0, gy), (W, gy)], fill=GRID, width=1)
    for gx in range(0, W, 200):
        d.line([(gx, 0), (gx, H)], fill=GRID_MAJOR, width=1)
    for gy in range(0, H, 200):
        d.line([(0, gy), (W, gy)], fill=GRID_MAJOR, width=1)

    # Orbital rings at fan origin — astronomical chart aesthetic
    ox, oy = FAN_ORIGIN
    for r in [200, 320, 440]:
        ring_outline(ox, oy, r, RING, width=1)
    for r in [110, 260, 380]:
        ring_outline(ox, oy, r, RING_FAINT, width=1)
    # Major outer ring at fan radius — slightly stronger
    ring_outline(ox, oy, 500, RING_MAJOR, width=1)

    # Radial tick markers — symmetric pair in clear canvas regions
    for ang_deg in (-30, 30):
        a = math.radians(ang_deg)
        rx1 = ox + 494 * math.cos(a)
        ry1 = oy + 494 * math.sin(a)
        rx2 = ox + 512 * math.cos(a)
        ry2 = oy + 512 * math.sin(a)
        d.line([(rx1, ry1), (rx2, ry2)], fill=RING_MAJOR, width=1)
        # tiny angle label outside the ring
        lx = ox + 530 * math.cos(a)
        ly = oy + 530 * math.sin(a)
        sign = "+" if ang_deg > 0 else ("−" if ang_deg < 0 else " ")
        label = f"{sign}{abs(ang_deg):02d}°"
        centre_text(int(lx), int(ly), label, jb_r_10, DIM_2)

    # Corner registration marks (chart-style crosshairs)
    crosshair_color = (42, 46, 60)
    def crosshair(cx, cy, inset=28, length=18):
        d.line([(cx - length, cy), (cx + length, cy)],
               fill=crosshair_color, width=1)
        d.line([(cx, cy - length), (cx, cy + length)],
               fill=crosshair_color, width=1)
    for (cx, cy) in [(28, 28), (W - 28, 28), (28, H - 28), (W - 28, H - 28)]:
        crosshair(cx, cy)

    # ======================================================================
    # TITLE (top-left) — generous scale, then clinical mono metadata
    # ======================================================================
    left_text(60, 42, "OpenSpatialDelay", dm_b_40, CYAN)
    left_text(60, 90, "Signal Flow", dm_sb_20, WHITE)
    left_text(60, 118, "per-sample audio path   ·   v1.0",
              jb_r_12, DIM_2)

    # ======================================================================
    # LEGEND (top-right) — horizontal strip, three swatches
    # ======================================================================
    lg_w, lg_h = 560, 42
    lg_x = W - 60 - lg_w
    lg_y = 44
    rect(lg_x, lg_y, lg_w, lg_h, 6, (14, 14, 28, 220),
         (44, 50, 66), 1)
    seg_w = lg_w // 3
    for i, (col, label) in enumerate([
        (CYAN,   "Signal"),
        (AMBER,  "Dry path"),
        (VIOLET, "Feedback"),
    ]):
        bx = lg_x + seg_w * i + 18
        d.line([(bx, lg_y + 22), (bx + 40, lg_y + 22)],
               fill=col, width=4)
        left_text(bx + 54, lg_y + 14, label, dm_m_16, WHITE)

    # ======================================================================
    # DRY PATH — cubic Bezier arc (amber)
    #   splits after INPUT, apexes above the centre row, enters MIX top
    # ======================================================================
    dry_p0 = (380, CENTRE_Y)
    dry_p1 = (660, 40)
    dry_p2 = (MIX_X + MIX_W // 2, 50)
    dry_p3 = (MIX_X + MIX_W // 2, MIX_Y_TOP := CENTRE_Y - 46)  # just above MIX top

    dry_pts = cubic_bezier(dry_p0, dry_p1, dry_p2, dry_p3, steps=260)
    for i in range(len(dry_pts) - 1):
        d.line([dry_pts[i], dry_pts[i + 1]], fill=AMBER, width=4)
    arrow_tip(dry_pts[-1], dry_pts[-2], AMBER, head=14)

    # Dry arc apex label cluster
    apex_i = len(dry_pts) // 2
    apex = (int(dry_pts[apex_i][0]), int(dry_pts[apex_i][1]))
    pill(apex[0], apex[1] - 14, "DRY PATH", dm_b_20,
         fg=AMBER, bd=AMBER_DIM, padx=16, pady=5)
    pill(apex[0], apex[1] + 20,
         "2048-sample latency compensation",
         jb_r_13, fg=DIM, bd=(44, 48, 62), padx=10, pady=4)

    # ======================================================================
    # FEEDBACK PATH — cubic Bezier arc (violet)
    #   tap from DELAY BUFFER bottom, sweeps left below the main row,
    #   re-enters the summer ⊕ from below. Pre-DSP, pre-spatialisation.
    # ======================================================================
    fb_tap_x = BUF_X + BUF_W - 40        # under BUFFER, right-of-centre
    buf_bottom = CENTRE_Y - 56 + 112     # = 556
    fb_p0 = (fb_tap_x, buf_bottom)       # starts exactly at buffer bottom
    fb_p1 = (fb_tap_x, 860)
    fb_p2 = (SUM_X, 860)
    fb_p3 = (SUM_X, CENTRE_Y + 22)       # arrives just under summer from below

    fb_pts = cubic_bezier(fb_p0, fb_p1, fb_p2, fb_p3, steps=260)
    for i in range(len(fb_pts) - 1):
        d.line([fb_pts[i], fb_pts[i + 1]], fill=VIOLET, width=4)
    arrow_tip(fb_pts[-1], fb_pts[-2], VIOLET, head=14)

    # Tap marker on the buffer underside — small violet dot
    dot(fb_tap_x, buf_bottom, 5, VIOLET)

    # Feedback arc apex label cluster — position slightly above arc apex
    apex_i = len(fb_pts) // 2
    fb_apex = (int(fb_pts[apex_i][0]), int(fb_pts[apex_i][1]))
    pill(fb_apex[0], fb_apex[1] - 16, "FEEDBACK", dm_b_20,
         fg=VIOLET, bd=VIOLET_DIM, padx=16, pady=5)
    pill(fb_apex[0], fb_apex[1] + 18,
         "filter   ·   saturate   ·   gain",
         jb_r_13, fg=DIM, bd=(44, 48, 62), padx=10, pady=4)

    # ======================================================================
    # 12-TAP ORBITAL FAN — radiates from delay-buffer output
    # ======================================================================
    fan_span = 340
    fan_top = CENTRE_Y - fan_span // 2
    fan_bot = CENTRE_Y + fan_span // 2
    tap_ys = []

    # Rays
    for n in range(12):
        t = (n + 0.5) / 12.0
        ry = int(fan_top + t * fan_span)
        tap_ys.append(ry)
        # Subtle curvature via cubic bezier
        ctl1 = (ox + 120, oy)
        ctl2 = (FAN_END_X - 80, ry)
        pts = cubic_bezier((ox, oy), ctl1, ctl2, (FAN_END_X, ry), steps=40)
        # Alternating intensities give the fan texture
        if n in (0, 5, 6, 11):
            color = CYAN_MID
            width = 2
        elif n in (2, 9):
            color = CYAN_DIM
            width = 2
        else:
            color = CYAN_FAINT
            width = 1
        for i in range(len(pts) - 1):
            d.line([pts[i], pts[i + 1]], fill=color, width=width)
        dot(FAN_END_X, ry, 3, CYAN_MID)

    # Outer tap-number ticks: 1, 6, 12 only (overload avoidance)
    for n, label in [(0, "1"), (5, "6"), (11, "12")]:
        ry = tap_ys[n]
        d.line([(FAN_END_X + 6, ry), (FAN_END_X + 16, ry)],
               fill=DIM_2, width=1)
        left_text(FAN_END_X + 22, ry, label, jb_m_12, DIM)

    # Fan header cluster — "12 TAPS / (n+1) × delay time"
    fan_hdr_x = (ox + FAN_END_X) // 2
    centre_text(fan_hdr_x, fan_top - 48, "12 TAPS", dm_b_26, CYAN)
    centre_text(fan_hdr_x, fan_top - 22,
                "(n + 1) × delay time", jb_r_12, DIM)

    # ======================================================================
    # MAIN ROW BLOCKS
    # ======================================================================
    # INPUT
    block(IN_X, CENTRE_Y - 56, IN_W, 112,
          "INPUT", "stereo  ·  L / R",
          fill=PANEL_2, border=CYAN, bw=2, radius=14,
          title_font=dm_b_30, title_color=CYAN,
          sub_font=jb_r_12, sub_color=DIM)

    # Arrow INPUT → split dot (dry branches up from here)
    split_x = 380
    arrow_h(IN_X + IN_W + 2, CENTRE_Y, split_x - 4, color=CYAN, width=3)
    dot(split_x, CENTRE_Y, 7, CYAN)
    # Continue cyan to INPUT GAIN
    arrow_h(split_x + 8, CENTRE_Y, GAIN_X - 2, color=CYAN, width=3)

    # INPUT GAIN
    block(GAIN_X, CENTRE_Y - 28, GAIN_W, 56,
          "INPUT GAIN",
          fill=PANEL, border=CYAN_DIM, bw=2, radius=10,
          title_font=dm_sb_18, title_color=WHITE)

    # GAIN → summer
    arrow_h(GAIN_X + GAIN_W + 2, CENTRE_Y, SUM_X - 18, color=CYAN, width=3)

    # Feedback summer ⊕
    ring_outline(SUM_X, CENTRE_Y, 18, CYAN, width=2)
    dot(SUM_X, CENTRE_Y, 17, PANEL_2)
    d.line([(SUM_X - 10, CENTRE_Y), (SUM_X + 10, CENTRE_Y)],
           fill=CYAN, width=2)
    d.line([(SUM_X, CENTRE_Y - 10), (SUM_X, CENTRE_Y + 10)],
           fill=CYAN, width=2)

    # Summer → buffer
    arrow_h(SUM_X + 20, CENTRE_Y, BUF_X - 2, color=CYAN, width=3)

    # DELAY BUFFER
    block(BUF_X, CENTRE_Y - 56, BUF_W, 112,
          "DELAY BUFFER", "two channels  ·  circular",
          fill=PANEL_2, border=CYAN, bw=2, radius=14,
          title_font=dm_b_26, title_color=CYAN,
          sub_font=jb_r_12, sub_color=DIM)

    # Bus-bar at fan end — aggregates the 12 rays
    d.line([(FAN_END_X, fan_top - 4), (FAN_END_X, fan_bot + 4)],
           fill=CYAN_MID, width=2)

    # Bus-bar → PER-TAP DSP
    arrow_h(FAN_END_X + 2, CENTRE_Y, DSP_X - 2, color=CYAN, width=3)

    # PER-TAP DSP panel
    dsp_h = 300
    dsp_y = CENTRE_Y - dsp_h // 2
    rect(DSP_X, dsp_y, DSP_W, dsp_h, 14, (14, 14, 28, 220),
         CYAN_DIM, 2)
    left_text(DSP_X + 22, dsp_y + 22, "PER-TAP DSP",
              dm_b_26, CYAN)
    left_text(DSP_X + 22, dsp_y + 58,
              "12 parallel voices", jb_r_12, DIM)
    sub_items = [
        ("Pitch Shift",    "± 12 st"),
        ("Doppler",        "velocity"),
        ("Air Absorption", "distance"),
        ("Filter",         "HP  ·  LP"),
    ]
    it_y0 = dsp_y + 92
    it_h  = 42
    it_gap = 10
    for i, (name, annot) in enumerate(sub_items):
        by = it_y0 + i * (it_h + it_gap)
        rect(DSP_X + 18, by, DSP_W - 36, it_h, 8,
             (22, 22, 42), CYAN_DIM, 1)
        left_text(DSP_X + 32, by + it_h // 2, name, dm_m_16, WHITE)
        aw, _, _ = text_size(annot, jb_r_11)
        left_text(DSP_X + DSP_W - 22 - aw, by + it_h // 2,
                  annot, jb_r_11, DIM)

    # DSP → SPATIAL
    arrow_h(DSP_X + DSP_W + 2, CENTRE_Y, SPAT_X - 2, color=CYAN, width=3)

    # SPATIAL RENDER panel
    spat_h = 300
    spat_y = CENTRE_Y - spat_h // 2
    rect(SPAT_X, spat_y, SPAT_W, spat_h, 14, (14, 14, 28, 220),
         CYAN_DIM, 2)
    left_text(SPAT_X + 22, spat_y + 22, "SPATIAL RENDER",
              dm_b_26, CYAN)
    left_text(SPAT_X + 22, spat_y + 58,
              "per-tap  ·  azimuth / elevation / distance",
              jb_r_12, DIM)
    spat_items = [
        ("HRTF",       "6 profiles"),
        ("Woodworth",  "simple (low CPU)"),
        ("Stereo pair", "5 modes"),
        ("Ambisonics", "1 – 6 OA"),
        ("Surround",   "7 algorithms"),
    ]
    si_y0 = spat_y + 92
    si_h  = 36
    si_gap = 6
    for i, (name, annot) in enumerate(spat_items):
        by = si_y0 + i * (si_h + si_gap)
        rect(SPAT_X + 18, by, SPAT_W - 36, si_h, 7,
             (22, 22, 42), CYAN_DIM, 1)
        left_text(SPAT_X + 32, by + si_h // 2, name, dm_m_16, WHITE)
        aw, _, _ = text_size(annot, jb_r_11)
        left_text(SPAT_X + SPAT_W - 22 - aw, by + si_h // 2,
                  annot, jb_r_11, DIM)

    # SPATIAL → MIX
    arrow_h(SPAT_X + SPAT_W + 2, CENTRE_Y, MIX_X - 2, color=CYAN, width=3)

    # MIX block
    block(MIX_X, CENTRE_Y - 44, MIX_W, 88,
          "MIX", "equal-power  dry / wet",
          fill=PANEL_2, border=CYAN, bw=2, radius=12,
          title_font=dm_b_26, title_color=CYAN,
          sub_font=jb_r_12, sub_color=DIM)

    # "limiter on wet only" annotation below MIX
    pill(MIX_X + MIX_W // 2, CENTRE_Y + 72,
         "limiter · wet path only", jb_r_11,
         fg=DIM, bd=(44, 48, 62), padx=9, pady=4)

    # MIX → OUTPUT
    arrow_h(MIX_X + MIX_W + 2, CENTRE_Y, OUT_X - 2, color=CYAN, width=3)

    # OUTPUT block
    block(OUT_X, CENTRE_Y - 56, OUT_W, 112,
          "OUTPUT", "up to 49 channels",
          fill=PANEL_2, border=CYAN, bw=2, radius=14,
          title_font=dm_b_30, title_color=CYAN,
          sub_font=jb_r_12, sub_color=DIM)

    # ======================================================================
    # FOOTER — minimal, two clinical mono lines
    # ======================================================================
    left_text(60, H - 40, "OpenSpatialDelay  ·  Spatial Media Lab",
              jb_r_12, DIM_2)
    phrase = "github.com/Spatial-Media-Lab/openspatialdelay"
    pw, _, _ = text_size(phrase, jb_r_12)
    left_text(W - 60 - pw, H - 40, phrase, jb_r_12, DIM_2)

    # ======================================================================
    # SAVE
    # ======================================================================
    rgb = Image.new("RGB", (W, H), BG)
    rgb.paste(img, (0, 0), img)
    rgb.save(OUT_PATH, "PNG", quality=95)
    print(f"Saved {OUT_PATH}  ·  {os.path.getsize(OUT_PATH) / 1024:.1f} KB")


if __name__ == "__main__":
    main()
