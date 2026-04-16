"""Generate Patreon graphics set: tier icons, cover banner, avatar crop, post-1 cover.

Run from repo root: python3 docs/phase-02-evidence/patreon-graphics/_generate.py
"""
from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).parent
ASSETS = Path(__file__).parents[2] / "assets"

BG_TOP = (10, 12, 28)
BG_BOTTOM = (26, 30, 64)
ACCENT = (232, 238, 255)
ACCENT_DIM = (170, 180, 220)
STAR = (255, 255, 255)


def dark_gradient(size: tuple[int, int]) -> Image.Image:
    w, h = size
    img = Image.new("RGB", size, BG_TOP)
    px = img.load()
    for y in range(h):
        t = y / max(1, h - 1)
        r = int(BG_TOP[0] + (BG_BOTTOM[0] - BG_TOP[0]) * t)
        g = int(BG_TOP[1] + (BG_BOTTOM[1] - BG_TOP[1]) * t)
        b = int(BG_TOP[2] + (BG_BOTTOM[2] - BG_TOP[2]) * t)
        for x in range(w):
            px[x, y] = (r, g, b)
    return img


def sprinkle_stars(img: Image.Image, count: int, seed: int = 42) -> None:
    rnd = random.Random(seed)
    w, h = img.size
    draw = ImageDraw.Draw(img, "RGBA")
    for _ in range(count):
        x = rnd.randint(0, w - 1)
        y = rnd.randint(0, h - 1)
        r = rnd.choice([0.5, 0.8, 1.0, 1.4, 1.8, 2.2])
        a = rnd.randint(120, 230)
        draw.ellipse([x - r, y - r, x + r, y + r], fill=(255, 255, 255, a))


def glow_circle(draw: ImageDraw.ImageDraw, cx: float, cy: float, r: float, color, alpha_outer=40, alpha_core=220):
    # Layered soft circles for glow
    for i in range(6, 0, -1):
        rr = r * (1 + i * 0.35)
        a = int(alpha_outer * (1 - i / 7))
        draw.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=(*color, max(a, 8)))
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(*color, alpha_core))


def draw_star(draw: ImageDraw.ImageDraw, cx: float, cy: float, r_outer: float, r_inner: float, color, points: int = 5):
    pts: list[tuple[float, float]] = []
    # Start at top
    start = -math.pi / 2
    for i in range(points * 2):
        ang = start + i * math.pi / points
        rr = r_outer if i % 2 == 0 else r_inner
        pts.append((cx + math.cos(ang) * rr, cy + math.sin(ang) * rr))
    draw.polygon(pts, fill=color)


def icon_canvas(size: int = 512) -> tuple[Image.Image, ImageDraw.ImageDraw]:
    img = dark_gradient((size, size))
    sprinkle_stars(img, count=55, seed=7)
    base = img.convert("RGBA")
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer, "RGBA")
    return Image.alpha_composite(base, layer), layer


def finalise(base: Image.Image, overlay: Image.Image, path: Path, size: int | None = None) -> None:
    combined = Image.alpha_composite(base.convert("RGBA"), overlay)
    if size is not None and combined.size != (size, size):
        combined = combined.resize((size, size), Image.LANCZOS)
    combined.convert("RGB").save(path, "PNG", optimize=True)


def rounded_square_icon(draw_fn, path: Path, size: int = 512):
    base, overlay = icon_canvas(size)
    draw = ImageDraw.Draw(overlay, "RGBA")
    draw_fn(draw, size)
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, size - 1, size - 1], radius=int(size * 0.18), fill=255)
    combined = Image.alpha_composite(base.convert("RGBA"), overlay)
    rounded = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    rounded.paste(combined, (0, 0), mask)
    rounded.convert("RGB").save(path, "PNG", optimize=True)


# ---------------- Tier icons ----------------

def tier_stargazer(draw: ImageDraw.ImageDraw, s: int):
    cx, cy = s / 2, s / 2 + s * 0.02
    # Big 5-point star
    draw_star(draw, cx, cy, r_outer=s * 0.28, r_inner=s * 0.12, color=(255, 248, 220, 255))
    # Bright center flare on star
    draw.ellipse([cx - s * 0.045, cy - s * 0.045, cx + s * 0.045, cy + s * 0.045], fill=(255, 255, 255, 255))
    # Surrounding small stars
    for (ox, oy, sz) in [(-0.32, -0.28, 0.04), (0.3, -0.22, 0.05), (-0.25, 0.3, 0.035), (0.28, 0.3, 0.045), (0.0, -0.38, 0.03)]:
        draw_star(draw, cx + ox * s, cy + oy * s, r_outer=sz * s, r_inner=sz * s * 0.42, color=STAR + (235,))


def tier_astronaut(draw: ImageDraw.ImageDraw, s: int):
    cx, cy = s / 2, s / 2 + s * 0.05
    # Backpack/life-support box behind helmet
    bw, bh = s * 0.36, s * 0.22
    draw.rounded_rectangle([cx - bw / 2, cy + s * 0.05, cx + bw / 2, cy + s * 0.05 + bh],
                           radius=s * 0.03, fill=(210, 215, 230, 255))
    # Neck + shoulders suggestion
    draw.polygon([(cx - s * 0.30, cy + s * 0.36),
                  (cx + s * 0.30, cy + s * 0.36),
                  (cx + s * 0.16, cy + s * 0.18),
                  (cx - s * 0.16, cy + s * 0.18)], fill=(240, 244, 255, 255))
    # Helmet outer (bright white)
    r_out = s * 0.30
    draw.ellipse([cx - r_out, cy - r_out, cx + r_out, cy + r_out], fill=(245, 248, 255, 255))
    # Visor — deep blue-navy, distinct from space background
    r_in = s * 0.24
    # Visor gradient (layered darker blues inside)
    for i, col in enumerate([(35, 60, 120, 255), (24, 44, 95, 255), (14, 28, 70, 255)]):
        rr = r_in - i * s * 0.018
        draw.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=col)
    # Visor highlight (curved crescent, upper-left)
    h_layer = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    hd = ImageDraw.Draw(h_layer, "RGBA")
    hd.chord([cx - r_in * 0.9, cy - r_in * 0.9, cx + r_in * 0.3, cy + r_in * 0.1],
             start=200, end=330, fill=(210, 225, 255, 170))
    h_layer = h_layer.filter(ImageFilter.GaussianBlur(radius=3))
    base_rgba = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    base_rgba = Image.alpha_composite(base_rgba, h_layer)
    # Composite highlight onto overlay via draw's image target
    target = draw.im  # PIL's core image for the draw target
    # Fall back: just draw the crescent directly (less blurred but reliable)
    draw.chord([cx - r_in * 0.85, cy - r_in * 0.85, cx + r_in * 0.25, cy + r_in * 0.05],
               start=205, end=325, fill=(210, 225, 255, 130))
    # Small reflected star on visor
    draw_star(draw, cx - r_in * 0.32, cy - r_in * 0.32, r_outer=s * 0.035, r_inner=s * 0.015, color=(255, 255, 255, 240))
    # Antenna + red light
    draw.rectangle([cx - s * 0.012, cy - r_out - s * 0.07, cx + s * 0.012, cy - r_out + s * 0.005],
                   fill=(220, 225, 240, 255))
    draw.ellipse([cx - s * 0.022, cy - r_out - s * 0.095, cx + s * 0.022, cy - r_out - s * 0.05],
                 fill=(255, 95, 95, 255))


def tier_commander(draw: ImageDraw.ImageDraw, s: int):
    cx, cy = s / 2, s / 2 + s * 0.06
    # 3 chevrons (rank insignia), bottom-to-top widening
    bar_h = s * 0.06
    gap = s * 0.04
    widths = [s * 0.42, s * 0.36, s * 0.30]
    top_offsets = [0.18, 0.05, -0.08]  # fraction of s above cy
    for w_frac, off in zip(widths, top_offsets):
        y_peak = cy + off * s
        half = w_frac / 2
        angle_drop = s * 0.08
        pts = [
            (cx - half, y_peak + angle_drop),
            (cx, y_peak),
            (cx + half, y_peak + angle_drop),
            (cx + half, y_peak + angle_drop + bar_h),
            (cx, y_peak + bar_h),
            (cx - half, y_peak + angle_drop + bar_h),
        ]
        draw.polygon(pts, fill=ACCENT + (255,))
    # Star above chevrons
    draw_star(draw, cx, cy - s * 0.24, r_outer=s * 0.085, r_inner=s * 0.035, color=(255, 220, 140, 255))
    # Orbit arc around star (subtle)
    arc_box = [cx - s * 0.14, cy - s * 0.30, cx + s * 0.14, cy - s * 0.17]
    draw.arc(arc_box, start=200, end=340, fill=(255, 220, 140, 120), width=max(2, s // 180))


def tier_mission_control(draw: ImageDraw.ImageDraw, s: int):
    cx, cy = s / 2, s / 2 + s * 0.06
    # Earth sphere bottom
    r_earth = s * 0.26
    earth_cy = cy + s * 0.14
    # Earth gradient-ish via layered circles
    draw.ellipse([cx - r_earth, earth_cy - r_earth, cx + r_earth, earth_cy + r_earth], fill=(40, 110, 170, 255))
    # continents (abstract blobs)
    for (bx, by, br) in [(-0.06, -0.05, 0.08), (0.05, 0.03, 0.06), (-0.10, 0.08, 0.04), (0.10, -0.08, 0.05)]:
        draw.ellipse([cx + (bx - br) * s, earth_cy + (by - br) * s, cx + (bx + br) * s, earth_cy + (by + br) * s],
                     fill=(74, 150, 98, 230))
    # Orbit ring (ellipse) behind a rising rocket — drawn as arc
    orbit_box = [cx - s * 0.40, earth_cy - s * 0.10, cx + s * 0.40, earth_cy + s * 0.10]
    draw.arc(orbit_box, start=0, end=360, fill=(200, 215, 255, 150), width=max(2, s // 220))
    # Rocket silhouette rising from top
    rx = cx
    ry = cy - s * 0.28
    body_w = s * 0.08
    body_h = s * 0.24
    # Body
    draw.rectangle([rx - body_w / 2, ry, rx + body_w / 2, ry + body_h], fill=(240, 244, 255, 255))
    # Nose cone
    draw.polygon([(rx - body_w / 2, ry), (rx + body_w / 2, ry), (rx, ry - s * 0.09)], fill=(240, 244, 255, 255))
    # Fins
    draw.polygon([(rx - body_w / 2, ry + body_h * 0.7), (rx - body_w / 2 - s * 0.05, ry + body_h),
                  (rx - body_w / 2, ry + body_h)], fill=ACCENT_DIM + (255,))
    draw.polygon([(rx + body_w / 2, ry + body_h * 0.7), (rx + body_w / 2 + s * 0.05, ry + body_h),
                  (rx + body_w / 2, ry + body_h)], fill=ACCENT_DIM + (255,))
    # Window
    draw.ellipse([rx - body_w * 0.3, ry + body_h * 0.25, rx + body_w * 0.3, ry + body_h * 0.55],
                 fill=(60, 120, 200, 255))
    # Exhaust flame
    draw.polygon([(rx - body_w * 0.45, ry + body_h), (rx + body_w * 0.45, ry + body_h),
                  (rx, ry + body_h + s * 0.11)], fill=(255, 170, 60, 240))
    draw.polygon([(rx - body_w * 0.25, ry + body_h), (rx + body_w * 0.25, ry + body_h),
                  (rx, ry + body_h + s * 0.07)], fill=(255, 230, 120, 255))


# ---------------- Avatar (crop headshot to 500x500 square, center-biased to face) ----------------

def make_avatar():
    src = ROOT / "avatar-andrew-rahman.jpg"
    out = ROOT / "avatar-500x500.png"
    if not src.exists():
        print(f"skip avatar (source missing): {src}")
        return
    img = Image.open(src).convert("RGB")
    w, h = img.size
    # Face likely top-third — crop to square anchored near top
    side = min(w, h)
    left = (w - side) // 2
    top = int((h - side) * 0.20)  # bias upward
    top = max(0, min(top, h - side))
    sq = img.crop((left, top, left + side, top + side))
    sq = sq.resize((500, 500), Image.LANCZOS)
    sq.save(out, "PNG", optimize=True)
    print(f"wrote {out.name} 500x500")


# ---------------- Cover banner 1600x400 ----------------

def make_cover():
    W, H = 1600, 400
    img = dark_gradient((W, H))
    sprinkle_stars(img, count=280, seed=3)
    base = img.convert("RGBA")
    overlay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(overlay, "RGBA")
    # Nebula glow blobs
    for (cx, cy, r, col) in [
        (W * 0.18, H * 0.55, 220, (80, 60, 160, 70)),
        (W * 0.82, H * 0.45, 260, (60, 130, 200, 70)),
        (W * 0.55, H * 0.2, 180, (180, 80, 140, 55)),
    ]:
        blob = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        bd = ImageDraw.Draw(blob, "RGBA")
        bd.ellipse([cx - r, cy - r, cx + r, cy + r], fill=col)
        blob = blob.filter(ImageFilter.GaussianBlur(radius=70))
        overlay = Image.alpha_composite(overlay, blob)
        d = ImageDraw.Draw(overlay, "RGBA")

    # Tagline text
    try:
        title_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Futura.ttc", 72)
        sub_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 28)
    except Exception:
        title_font = ImageFont.load_default()
        sub_font = ImageFont.load_default()

    title = "SPATIAL MEDIA LIBRARY"
    sub = "Spatial audio tools for musicians and sound designers"
    # Center text
    tw = d.textlength(title, font=title_font)
    sw = d.textlength(sub, font=sub_font)
    d.text(((W - tw) / 2, H * 0.32), title, font=title_font, fill=(240, 244, 255, 255))
    d.text(((W - sw) / 2, H * 0.65), sub, font=sub_font, fill=(200, 210, 240, 230))

    combined = Image.alpha_composite(base, overlay)
    out = ROOT / "cover-1600x400.png"
    combined.convert("RGB").save(out, "PNG", optimize=True)
    print(f"wrote {out.name} 1600x400")


# ---------------- Post 1 cover 1200x675 (plugin screenshot + branded overlay) ----------------

def make_post1_cover():
    shot = ASSETS / "screenshot_full.png"
    out = ROOT / "post-01-cover-1200x675.png"
    W, H = 1200, 675
    if shot.exists():
        img = Image.open(shot).convert("RGB")
        # Cover-fit to 1200x675
        src_w, src_h = img.size
        target_ratio = W / H
        src_ratio = src_w / src_h
        if src_ratio > target_ratio:
            # too wide → crop sides
            new_w = int(src_h * target_ratio)
            left = (src_w - new_w) // 2
            img = img.crop((left, 0, left + new_w, src_h))
        else:
            new_h = int(src_w / target_ratio)
            top = (src_h - new_h) // 2
            img = img.crop((0, top, src_w, top + new_h))
        img = img.resize((W, H), Image.LANCZOS)
        # Dark gradient overlay at bottom for text legibility
        overlay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        od = ImageDraw.Draw(overlay, "RGBA")
        for y in range(H):
            a = int(180 * max(0, (y - H * 0.55) / (H * 0.45)) ** 2)
            od.rectangle([0, y, W, y + 1], fill=(10, 12, 28, a))
        combined = Image.alpha_composite(img.convert("RGBA"), overlay)
    else:
        combined = dark_gradient((W, H)).convert("RGBA")

    # Title text
    try:
        title_font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Futura.ttc", 54)
    except Exception:
        title_font = ImageFont.load_default()
    td = ImageDraw.Draw(combined, "RGBA")
    title = "WELCOME TO THE SPATIAL MEDIA LIBRARY"
    tw = td.textlength(title, font=title_font)
    td.text(((W - tw) / 2, H * 0.78), title, font=title_font, fill=(240, 244, 255, 255))
    combined.convert("RGB").save(out, "PNG", optimize=True)
    print(f"wrote {out.name} 1200x675")


# ---------------- main ----------------

def main():
    # Tier icons 512x512
    rounded_square_icon(tier_stargazer, ROOT / "tier-1-stargazer.png", size=512)
    print("wrote tier-1-stargazer.png 512x512")
    rounded_square_icon(tier_astronaut, ROOT / "tier-2-astronaut.png", size=512)
    print("wrote tier-2-astronaut.png 512x512")
    rounded_square_icon(tier_commander, ROOT / "tier-3-commander.png", size=512)
    print("wrote tier-3-commander.png 512x512")
    rounded_square_icon(tier_mission_control, ROOT / "tier-4-mission-control.png", size=512)
    print("wrote tier-4-mission-control.png 512x512")
    make_avatar()
    make_cover()
    make_post1_cover()


if __name__ == "__main__":
    main()
