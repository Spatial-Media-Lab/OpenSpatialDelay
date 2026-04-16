"""Generate Patreon graphics set.

Design system is borrowed from andrewrahman.com (dark void bg + 12-tap rainbow
palette sourced from Source/PluginEditor.cpp), so the tier cards feel continuous
with the landing page rather than being a separate aesthetic.

Tier cards are sized to Patreon's recommended 460x200 (landscape 2.3:1) and
each tier is mapped to one of the 12 taps as its accent colour. A thin 12-tap
rainbow strip runs along the bottom of every card with the active tap brightened;
the inactive taps stay at low opacity so the motif reads as a continuous thread
across the 4 tiers.

Run from repo root:
    python3 docs/phase-02-evidence/patreon-graphics/_generate.py
"""
from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).parent
ASSETS = Path(__file__).parents[2] / "assets"
FONTS = Path(__file__).parents[2] / ".." / "fonts"  # repo root /fonts
FONTS = (ROOT.parents[2] / "fonts").resolve()

# ---------------- Palette (Source/PluginEditor.cpp) ----------------
# Void background matches site --bg-void.
VOID = (3, 6, 11)
VOID_SOFT = (12, 16, 26)  # Used for subtle gradient stops

# 12 tap colours, indexed 1..12 for readability.
TAPS = [
    (0xED, 0x5E, 0x5E),  # 1  red
    (0xED, 0xA6, 0x5E),  # 2  orange
    (0xAC, 0xD4, 0x35),  # 3  lime
    (0x3B, 0xCE, 0x6C),  # 4  green
    (0x3C, 0xDD, 0xA7),  # 5  teal
    (0x52, 0xE0, 0xE0),  # 6  cyan
    (0x4D, 0xB3, 0xE6),  # 7  light blue
    (0x63, 0x90, 0xE9),  # 8  blue
    (0x67, 0x67, 0xE4),  # 9  indigo
    (0xA6, 0x67, 0xE4),  # 10 violet
    (0xE4, 0x67, 0xE4),  # 11 magenta
    (0xE9, 0x63, 0xA6),  # 12 rose
]

TEXT_PRIMARY = (240, 244, 250)
TEXT_DIM = (160, 168, 188)

DM_SANS_BOLD = FONTS / "DM_Sans-Bold.ttf"
DM_SANS_SEMI = FONTS / "DM_Sans-SemiBold.ttf"
DM_SANS_MED = FONTS / "DM_Sans-Medium.ttf"
JB_MONO = FONTS / "JetBrains_Mono-Medium.ttf"
JB_MONO_BOLD = FONTS / "JetBrains_Mono-Bold.ttf"


def load_font(path: Path, size: int) -> ImageFont.FreeTypeFont:
    try:
        return ImageFont.truetype(str(path), size=size)
    except Exception:
        return ImageFont.load_default()


# ---------------- Background helpers ----------------

def void_bg(size: tuple[int, int]) -> Image.Image:
    w, h = size
    img = Image.new("RGB", size, VOID)
    px = img.load()
    for y in range(h):
        t = (y / max(1, h - 1)) ** 1.4
        r = int(VOID[0] + (VOID_SOFT[0] - VOID[0]) * t)
        g = int(VOID[1] + (VOID_SOFT[1] - VOID[1]) * t)
        b = int(VOID[2] + (VOID_SOFT[2] - VOID[2]) * t)
        for x in range(w):
            px[x, y] = (r, g, b)
    return img


def sprinkle_stars(img: Image.Image, count: int, seed: int = 42, alpha_range=(90, 180)) -> None:
    rnd = random.Random(seed)
    w, h = img.size
    draw = ImageDraw.Draw(img, "RGBA")
    for _ in range(count):
        x = rnd.randint(0, w - 1)
        y = rnd.randint(0, h - 1)
        r = rnd.choice([0.4, 0.6, 0.9, 1.2, 1.6])
        a = rnd.randint(*alpha_range)
        draw.ellipse([x - r, y - r, x + r, y + r], fill=(255, 255, 255, a))


def draw_tap_strip(img: Image.Image, active_idx: int, y_top: int, height: int = 4,
                   side_pad: int = 0, bar_gap: int = 4) -> None:
    """Draw the 12-tap rainbow strip with the active tap brightened.

    active_idx is 1-based.
    """
    w, _ = img.size
    inner_w = w - 2 * side_pad
    total_gap = bar_gap * (len(TAPS) - 1)
    bar_w = (inner_w - total_gap) / len(TAPS)
    draw = ImageDraw.Draw(img, "RGBA")
    for i, col in enumerate(TAPS, start=1):
        x0 = side_pad + (i - 1) * (bar_w + bar_gap)
        x1 = x0 + bar_w
        alpha = 255 if i == active_idx else 70
        draw.rectangle([x0, y_top, x1, y_top + height], fill=(*col, alpha))


def add_glow(layer_size: tuple[int, int], draw_fn, blur: int = 18, intensity: float = 1.0) -> Image.Image:
    """Run draw_fn(layer, draw) on a transparent layer then blur it."""
    layer = Image.new("RGBA", layer_size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer, "RGBA")
    draw_fn(layer, d)
    blurred = layer.filter(ImageFilter.GaussianBlur(radius=blur))
    if intensity != 1.0:
        # Boost alpha
        r, g, b, a = blurred.split()
        a = a.point(lambda p: min(255, int(p * intensity)))
        blurred = Image.merge("RGBA", (r, g, b, a))
    return blurred


# ---------------- Shape primitives ----------------

def draw_star(draw: ImageDraw.ImageDraw, cx: float, cy: float, r_outer: float, r_inner: float,
              color, points: int = 5, rotation: float = -math.pi / 2) -> None:
    pts: list[tuple[float, float]] = []
    for i in range(points * 2):
        ang = rotation + i * math.pi / points
        rr = r_outer if i % 2 == 0 else r_inner
        pts.append((cx + math.cos(ang) * rr, cy + math.sin(ang) * rr))
    draw.polygon(pts, fill=color)


# ---------------- Tier icon draws (scaled for ~120-140px tall) ----------------

def icon_stargazer(img: Image.Image, cx: float, cy: float, s: float, accent: tuple[int, int, int]):
    d = ImageDraw.Draw(img, "RGBA")
    # Soft glow behind big star
    glow = add_glow(img.size, lambda layer, gd: gd.ellipse(
        [cx - s * 0.28, cy - s * 0.28, cx + s * 0.28, cy + s * 0.28],
        fill=(*accent, 200)), blur=22)
    img.alpha_composite(glow)
    # Main 5-point star
    draw_star(d, cx, cy, r_outer=s * 0.30, r_inner=s * 0.13, color=(*accent, 255))
    # Bright core
    d.ellipse([cx - s * 0.045, cy - s * 0.045, cx + s * 0.045, cy + s * 0.045], fill=(255, 255, 255, 240))
    # Satellite stars — constellation around
    satellites = [(-0.48, -0.32, 0.045), (0.44, -0.30, 0.04), (-0.38, 0.34, 0.035),
                  (0.40, 0.32, 0.05), (0.0, -0.48, 0.03), (-0.55, 0.02, 0.028), (0.52, 0.05, 0.032)]
    for (ox, oy, sz) in satellites:
        draw_star(d, cx + ox * s, cy + oy * s, r_outer=sz * s, r_inner=sz * s * 0.42,
                  color=(230, 240, 255, 230))
    # Thin constellation lines between 3 satellites and the main star (subtle)
    for (ox, oy, _) in satellites[:3]:
        d.line([(cx, cy), (cx + ox * s, cy + oy * s)], fill=(*accent, 40), width=1)


def icon_astronaut(img: Image.Image, cx: float, cy: float, s: float, accent: tuple[int, int, int]):
    d = ImageDraw.Draw(img, "RGBA")
    # Soft outer glow
    glow = add_glow(img.size, lambda layer, gd: gd.ellipse(
        [cx - s * 0.42, cy - s * 0.42, cx + s * 0.42, cy + s * 0.42],
        fill=(*accent, 100)), blur=26)
    img.alpha_composite(glow)
    # Backpack + torso
    bw, bh = s * 0.50, s * 0.32
    d.rounded_rectangle([cx - bw / 2, cy + s * 0.10, cx + bw / 2, cy + s * 0.10 + bh],
                        radius=int(s * 0.05), fill=(210, 215, 230, 255))
    # Shoulders (trapezoid)
    d.polygon([(cx - s * 0.40, cy + s * 0.45),
               (cx + s * 0.40, cy + s * 0.45),
               (cx + s * 0.22, cy + s * 0.22),
               (cx - s * 0.22, cy + s * 0.22)], fill=(240, 244, 255, 255))
    # Helmet outer — white
    r_out = s * 0.36
    d.ellipse([cx - r_out, cy - r_out, cx + r_out, cy + r_out], fill=(245, 248, 255, 255))
    # Visor — use the accent colour as the visor tint so the tier accent is carried
    r_in = s * 0.29
    for i, factor in enumerate([1.0, 0.85, 0.65]):
        rr = r_in - i * s * 0.022
        a = 255 if i == 0 else 255
        tinted = tuple(int(c * factor) for c in accent)
        d.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=(*tinted, a))
    # Accent glossy highlight arc
    d.chord([cx - r_in * 0.85, cy - r_in * 0.85, cx + r_in * 0.30, cy + r_in * 0.05],
            start=205, end=330, fill=(220, 232, 255, 120))
    # Pinpoint star on visor
    draw_star(d, cx - r_in * 0.32, cy - r_in * 0.32, r_outer=s * 0.04, r_inner=s * 0.017,
              color=(255, 255, 255, 245))
    # Antenna + red tip
    d.rectangle([cx - s * 0.012, cy - r_out - s * 0.06, cx + s * 0.012, cy - r_out + s * 0.005],
                fill=(210, 215, 230, 255))
    d.ellipse([cx - s * 0.022, cy - r_out - s * 0.085, cx + s * 0.022, cy - r_out - s * 0.04],
              fill=(*TAPS[0], 255))  # tap 1 red for the antenna light


def icon_commander(img: Image.Image, cx: float, cy: float, s: float, accent: tuple[int, int, int]):
    d = ImageDraw.Draw(img, "RGBA")
    # Glow
    glow = add_glow(img.size, lambda layer, gd: gd.rectangle(
        [cx - s * 0.35, cy - s * 0.25, cx + s * 0.35, cy + s * 0.30], fill=(*accent, 80)), blur=22)
    img.alpha_composite(glow)
    # 3 chevrons, bottom-to-top widening
    bar_h = s * 0.08
    widths = [s * 0.46, s * 0.38, s * 0.30]
    top_offsets = [0.22, 0.05, -0.12]  # fraction of s from cy
    for idx, (w_frac, off) in enumerate(zip(widths, top_offsets)):
        y_peak = cy + off * s
        half = w_frac / 2
        drop = s * 0.10
        pts = [
            (cx - half, y_peak + drop),
            (cx, y_peak),
            (cx + half, y_peak + drop),
            (cx + half, y_peak + drop + bar_h),
            (cx, y_peak + bar_h),
            (cx - half, y_peak + drop + bar_h),
        ]
        # Slight alpha falloff for depth
        alpha = 255 if idx == 0 else 240 - idx * 25
        d.polygon(pts, fill=(*accent, alpha))
    # Command star above
    draw_star(d, cx, cy - s * 0.32, r_outer=s * 0.10, r_inner=s * 0.042,
              color=(255, 220, 140, 255))
    # Orbit arc around star
    d.arc([cx - s * 0.18, cy - s * 0.40, cx + s * 0.18, cy - s * 0.22],
          start=200, end=340, fill=(255, 220, 140, 150), width=2)


def icon_mission_control(img: Image.Image, cx: float, cy: float, s: float, accent: tuple[int, int, int]):
    d = ImageDraw.Draw(img, "RGBA")
    # Launch exhaust glow
    glow = add_glow(img.size, lambda layer, gd: gd.ellipse(
        [cx - s * 0.22, cy + s * 0.35, cx + s * 0.22, cy + s * 0.72],
        fill=(*accent, 150)), blur=24)
    img.alpha_composite(glow)
    # Earth sphere
    r_earth = s * 0.30
    earth_cy = cy + s * 0.20
    d.ellipse([cx - r_earth, earth_cy - r_earth, cx + r_earth, earth_cy + r_earth],
              fill=(*TAPS[7], 255))  # blue tap 8
    # Continents
    for (bx, by, br) in [(-0.07, -0.06, 0.10), (0.06, 0.04, 0.07), (-0.12, 0.10, 0.05), (0.12, -0.09, 0.06)]:
        d.ellipse([cx + (bx - br) * s, earth_cy + (by - br) * s,
                   cx + (bx + br) * s, earth_cy + (by + br) * s],
                  fill=(*TAPS[3], 220))  # green tap 4
    # Orbit ring (ellipse arc)
    d.arc([cx - s * 0.46, earth_cy - s * 0.12, cx + s * 0.46, earth_cy + s * 0.12],
          start=0, end=360, fill=(200, 215, 255, 140), width=2)
    # Rocket silhouette ascending
    rx = cx
    ry = cy - s * 0.32
    body_w = s * 0.10
    body_h = s * 0.30
    # Body
    d.rectangle([rx - body_w / 2, ry, rx + body_w / 2, ry + body_h], fill=(245, 248, 255, 255))
    # Nose
    d.polygon([(rx - body_w / 2, ry), (rx + body_w / 2, ry), (rx, ry - s * 0.12)],
              fill=(245, 248, 255, 255))
    # Fins
    d.polygon([(rx - body_w / 2, ry + body_h * 0.66), (rx - body_w / 2 - s * 0.07, ry + body_h),
               (rx - body_w / 2, ry + body_h)], fill=(*accent, 255))
    d.polygon([(rx + body_w / 2, ry + body_h * 0.66), (rx + body_w / 2 + s * 0.07, ry + body_h),
               (rx + body_w / 2, ry + body_h)], fill=(*accent, 255))
    # Window
    d.ellipse([rx - body_w * 0.30, ry + body_h * 0.25, rx + body_w * 0.30, ry + body_h * 0.55],
              fill=(*TAPS[6], 255))  # light blue tap 7
    # Exhaust flames
    d.polygon([(rx - body_w * 0.50, ry + body_h),
               (rx + body_w * 0.50, ry + body_h),
               (rx, ry + body_h + s * 0.15)],
              fill=(*accent, 255))
    d.polygon([(rx - body_w * 0.28, ry + body_h),
               (rx + body_w * 0.28, ry + body_h),
               (rx, ry + body_h + s * 0.09)],
              fill=(*TAPS[1], 240))  # orange tap 2


# ---------------- Tier card builder (460x200) ----------------

def fit_font_width(text: str, max_w: int, path: Path, start_size: int, min_size: int = 16) -> ImageFont.FreeTypeFont:
    """Return the largest font size in [min_size, start_size] whose rendered width <= max_w."""
    size = start_size
    tmp_img = Image.new("RGBA", (1, 1))
    td = ImageDraw.Draw(tmp_img)
    while size > min_size:
        font = load_font(path, size)
        w = td.textlength(text, font=font)
        if w <= max_w:
            return font
        size -= 1
    return load_font(path, min_size)


def build_tier_card(tier_name: str, price_label: str, accent_tap: int, icon_fn, out_path: Path):
    W, H = 460, 200
    img = void_bg((W, H)).convert("RGBA")
    sprinkle_stars(img, count=75, seed=hash(tier_name) & 0xFF, alpha_range=(70, 170))

    accent = TAPS[accent_tap - 1]
    # Left accent vertical bar (very subtle, 3px)
    ld = ImageDraw.Draw(img, "RGBA")
    ld.rectangle([0, 0, 3, H], fill=(*accent, 200))

    # Icon region — left 175px wide, full height
    icon_cx = 90
    icon_cy = H / 2 - 6  # nudge up to account for bottom strip
    icon_scale = 150  # bounding box size for the icon primitives
    icon_fn(img, icon_cx, icon_cy, icon_scale, accent)

    # Text region
    text_x = 190
    # Tier name — fit to available width
    name_max_w = W - text_x - 20
    title_font = fit_font_width(tier_name.upper(), int(name_max_w), DM_SANS_BOLD, start_size=44, min_size=22)
    # Price (mono caps)
    price_font = load_font(JB_MONO, 15)

    td = ImageDraw.Draw(img, "RGBA")
    # Title baseline — vertically centered over (tier + price) block
    title_bbox = td.textbbox((0, 0), tier_name.upper(), font=title_font)
    title_h = title_bbox[3] - title_bbox[1]
    price_bbox = td.textbbox((0, 0), price_label, font=price_font)
    price_h = price_bbox[3] - price_bbox[1]
    block_h = title_h + 10 + price_h
    block_top = (H - 16) / 2 - block_h / 2  # -16 accounts for bottom strip area
    td.text((text_x, block_top - title_bbox[1]), tier_name.upper(), font=title_font, fill=TEXT_PRIMARY)
    td.text((text_x, block_top + title_h + 10 - price_bbox[1]), price_label, font=price_font, fill=TEXT_DIM)

    # 12-tap rainbow strip at bottom
    strip_h = 5
    strip_pad = 14
    strip_y = H - strip_h - 12
    draw_tap_strip(img, active_idx=accent_tap, y_top=strip_y, height=strip_h,
                   side_pad=strip_pad, bar_gap=4)
    # Label under strip: "TAP NN" in mono — ties it to the plugin's tap metaphor
    tap_label_font = load_font(JB_MONO, 9)
    label = f"TAP {accent_tap:02d}"
    lw = td.textlength(label, font=tap_label_font)
    td.text((W - lw - strip_pad, strip_y - 14), label, font=tap_label_font, fill=TEXT_DIM)

    img.convert("RGB").save(out_path, "PNG", optimize=True)
    print(f"wrote {out_path.name} {W}x{H}")


# ---------------- Avatar crop from headshot ----------------

def make_avatar():
    src = ROOT / "avatar-andrew-rahman.jpg"
    out = ROOT / "avatar-500x500.png"
    if not src.exists():
        print(f"skip avatar (source missing): {src}")
        return
    img = Image.open(src).convert("RGB")
    w, h = img.size
    side = min(w, h)
    left = (w - side) // 2
    top = int((h - side) * 0.20)  # face bias upward
    top = max(0, min(top, h - side))
    sq = img.crop((left, top, left + side, top + side))
    sq = sq.resize((500, 500), Image.LANCZOS)
    sq.save(out, "PNG", optimize=True)
    print(f"wrote {out.name} 500x500")


# ---------------- Cover banner 1600x400 (refreshed to match site) ----------------

def make_cover():
    W, H = 1600, 400
    img = void_bg((W, H)).convert("RGBA")
    sprinkle_stars(img, count=260, seed=3, alpha_range=(50, 160))

    # Single soft nebula blob in the rose accent (matches site's Patreon accent)
    blob_layer = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    bd = ImageDraw.Draw(blob_layer, "RGBA")
    bd.ellipse([W * 0.55 - 260, H * 0.35 - 200, W * 0.55 + 260, H * 0.35 + 200],
               fill=(*TAPS[11], 40))  # rose tap 12 at low opacity
    blob_layer = blob_layer.filter(ImageFilter.GaussianBlur(radius=80))
    img = Image.alpha_composite(img, blob_layer)

    # Second nebula in the stellar/cyan zone (matches --accent-stellar)
    blob2 = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    b2d = ImageDraw.Draw(blob2, "RGBA")
    b2d.ellipse([W * 0.25 - 260, H * 0.65 - 200, W * 0.25 + 260, H * 0.65 + 200],
                fill=(*TAPS[6], 35))  # light blue tap 7
    blob2 = blob2.filter(ImageFilter.GaussianBlur(radius=80))
    img = Image.alpha_composite(img, blob2)

    d = ImageDraw.Draw(img, "RGBA")
    # Title — DM Sans Bold, big
    title_font = load_font(DM_SANS_BOLD, 84)
    sub_font = load_font(JB_MONO, 22)
    label_font = load_font(JB_MONO, 14)

    title = "Spatial Media Library"
    sub = "SPATIAL AUDIO TOOLS  ·  MUSICIANS + SOUND DESIGNERS  ·  OPEN SOURCE"
    label = "ANDREWRAHMAN.COM  /  PATREON"

    tw = d.textlength(title, font=title_font)
    sw = d.textlength(sub, font=sub_font)
    lw = d.textlength(label, font=label_font)
    d.text(((W - tw) / 2, H * 0.30), title, font=title_font, fill=TEXT_PRIMARY)
    d.text(((W - sw) / 2, H * 0.58), sub, font=sub_font, fill=TEXT_DIM)
    d.text(((W - lw) / 2, H * 0.75), label, font=label_font, fill=(*TAPS[6], 230))

    # Full 12-tap rainbow strip across the bottom, all active
    strip_h = 8
    strip_y = H - strip_h - 22
    side_pad = 120
    inner_w = W - 2 * side_pad
    bar_gap = 10
    total_gap = bar_gap * (len(TAPS) - 1)
    bar_w = (inner_w - total_gap) / len(TAPS)
    for i, col in enumerate(TAPS, start=1):
        x0 = side_pad + (i - 1) * (bar_w + bar_gap)
        x1 = x0 + bar_w
        d.rectangle([x0, strip_y, x1, strip_y + strip_h], fill=(*col, 235))

    out = ROOT / "cover-1600x400.png"
    img.convert("RGB").save(out, "PNG", optimize=True)
    print(f"wrote {out.name} {W}x{H}")


# ---------------- Post 1 cover 1200x675 ----------------

def make_post1_cover():
    shot = ASSETS / "screenshot_full.png"
    out = ROOT / "post-01-cover-1200x675.png"
    W, H = 1200, 675
    if shot.exists():
        img = Image.open(shot).convert("RGB")
        src_w, src_h = img.size
        target_ratio = W / H
        src_ratio = src_w / src_h
        if src_ratio > target_ratio:
            new_w = int(src_h * target_ratio)
            left = (src_w - new_w) // 2
            img = img.crop((left, 0, left + new_w, src_h))
        else:
            new_h = int(src_w / target_ratio)
            top = (src_h - new_h) // 2
            img = img.crop((0, top, src_w, top + new_h))
        img = img.resize((W, H), Image.LANCZOS)
        # Dark gradient at bottom
        overlay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        od = ImageDraw.Draw(overlay, "RGBA")
        for y in range(H):
            a = int(200 * max(0, (y - H * 0.50) / (H * 0.50)) ** 2)
            od.rectangle([0, y, W, y + 1], fill=(VOID[0], VOID[1], VOID[2], a))
        combined = Image.alpha_composite(img.convert("RGBA"), overlay)
    else:
        combined = void_bg((W, H)).convert("RGBA")

    td = ImageDraw.Draw(combined, "RGBA")
    title_font = load_font(DM_SANS_BOLD, 62)
    sub_font = load_font(JB_MONO, 18)
    title = "Spatial Media Library"
    sub = "FUNDING THE PIPELINE  ·  OPENSPATIALDELAY IS FIRST"
    tw = td.textlength(title, font=title_font)
    sw = td.textlength(sub, font=sub_font)
    td.text(((W - tw) / 2, H * 0.73), title, font=title_font, fill=TEXT_PRIMARY)
    td.text(((W - sw) / 2, H * 0.89), sub, font=sub_font, fill=TEXT_DIM)

    # 12-tap rainbow strip at bottom
    strip_h = 5
    strip_pad = 120
    strip_y = H - strip_h - 22
    draw_tap_strip(combined, active_idx=0, y_top=strip_y, height=strip_h,
                   side_pad=strip_pad, bar_gap=6)
    # Draw all taps at full alpha for the post cover (no active/inactive split)
    d = ImageDraw.Draw(combined, "RGBA")
    inner_w = W - 2 * strip_pad
    bar_gap = 6
    total_gap = bar_gap * (len(TAPS) - 1)
    bar_w = (inner_w - total_gap) / len(TAPS)
    for i, col in enumerate(TAPS, start=1):
        x0 = strip_pad + (i - 1) * (bar_w + bar_gap)
        x1 = x0 + bar_w
        d.rectangle([x0, strip_y, x1, strip_y + strip_h], fill=(*col, 255))

    combined.convert("RGB").save(out, "PNG", optimize=True)
    print(f"wrote {out.name} {W}x{H}")


# ---------------- Main ----------------

TIER_SPECS = [
    ("Stargazer", "$3 / MONTH", 7, icon_stargazer, "tier-1-stargazer.png"),
    ("Astronaut", "$10 / MONTH", 6, icon_astronaut, "tier-2-astronaut.png"),
    ("Commander", "$25 / MONTH", 10, icon_commander, "tier-3-commander.png"),
    ("Mission Control", "$100 / MONTH", 12, icon_mission_control, "tier-4-mission-control.png"),
]


def main():
    for (name, price, tap, icon_fn, fname) in TIER_SPECS:
        build_tier_card(name, price, tap, icon_fn, ROOT / fname)
    make_avatar()
    make_cover()
    make_post1_cover()


if __name__ == "__main__":
    main()
