#!/usr/bin/env python3
"""
Annotate OpenSpatialDelay plugin screenshot with numbered callouts.
Generates screenshot_annotated.png with leader lines, callout circles, and a legend.

Round 3 (issue #168): source image includes the Global Drawer open and the
undo arrow highlighted (produced by the screenshot_tool `annotated-source`
mode). Callouts 9–13 now target each right-panel section header precisely.
Callouts 15/16 moved into the side margins so they no longer collide with the
legend block. All circles and leader lines are supersampled and downscaled so
they render with clean antialiased edges instead of pixelated ones.
"""

from PIL import Image, ImageDraw, ImageFont
import math

# --- Configuration ---
# Paths resolve relative to the repository root (parent of docs/).
import os
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO_ROOT, "docs/assets/screenshot_annotated_source.png")
DST = os.path.join(REPO_ROOT, "docs/assets/screenshot_annotated.png")
FONT_PATH = os.path.join(REPO_ROOT, "fonts/DM_Sans-Bold.ttf")

# Overlay drawing uses Pillow's built-in primitives, which do not antialias.
# To get clean circles and lines we render the overlay at this multiple of the
# final canvas size, then downscale with LANCZOS. 3x is a good balance of
# quality vs. memory.
SUPERSAMPLE = 3

CIRCLE_RADIUS = 18  # 36px diameter at final resolution
LINE_WIDTH = 2
TEXT_COLOR = (20, 14, 2)           # near-black for legibility on light hues
LEGEND_BG = (20, 24, 32, 220)      # dark semi-transparent
LEGEND_TEXT_COLOR = (225, 228, 235)
LEGEND_HEADING_COLOR = (245, 247, 252)

# Four section hues — chosen to stay high-contrast against both the dark plugin
# background and its cyan/purple/pink accent colours, and to differ enough from
# each other that grouping is obvious at a glance. Dark text works on all four.
SECTION_COLORS = {
    "HEADER":      (255, 176,  59),   # amber / gold — header row
    "MAP":         (148, 227, 120),   # spring green — spatial map + drawer
    "RIGHT PANEL": (255, 138, 101),   # coral — DELAY/MOD/TONE/MIX/OSC
    "PER-TAP":     (124, 200, 255),   # sky blue — tap selector + bottom panel
}
SECTION_ORDER = ["HEADER", "MAP", "RIGHT PANEL", "PER-TAP"]
SECTION_OUTLINES = {
    "HEADER":      ( 70,  40,   0),
    "MAP":         ( 24,  70,  16),
    "RIGHT PANEL": ( 80,  28,  18),
    "PER-TAP":     ( 18,  52,  86),
}

# Callout definitions. Tuple: (number, label, section, target_x, target_y, callout_x, callout_y)
# target = point on the UI element; callout = where the circle goes (outside the element)
# Coordinates are in source-image pixels (1640x1160 — scale=2 output).
#
# Right-panel targets (issue #168 r3 feedback: point to section CENTRES, not headers):
#   9  DELAY  → under the TIME knob value        (1376, 272)
#   10 MOD    → between AMOUNT and MORPH knobs   (1376, 474)
#   11 TONE   → centre of the filter graph       (1376, 692)
#   12 MIX    → between DRY/WET and OUTPUT       (1376, 954)
#   13 OSC    → between receive port and send    (1380, 1104)
# 15/16 numbers sit in the side margins at the bottom of the image so they
# don't occlude the per-tap knobs, and the leader lines do not cross pointer 13.
CALLOUTS = [
    # --- Header row ------------------------------------------------------
    ( 1, "SML Badge",                "HEADER",       80,   52,    80, -40),
    ( 2, "Preset Browser",           "HEADER",      625,   52,   625, -40),
    ( 3, "Undo / Redo",              "HEADER",      975,   52,   975, -40),
    ( 4, "Input Format",             "HEADER",     1098,   52,  1098, -40),
    ( 5, "Output Format",            "HEADER",     1248,   52,  1248, -40),
    ( 6, "Algorithm / HRTF Profile", "HEADER",     1430,   52,  1430, -40),
    # --- Map / Drawer ----------------------------------------------------
    ( 7, "Global Controls",          "MAP",         120,  420,   -50,  420),
    ( 8, "Spatial Map",              "MAP",         662,  572,   -50,  572),
    # --- Right panel sections — straight horizontal leaders from the right
    #     margin (callout Y matches target Y). Avoid diagonal lines.
    ( 9, "Delay controls",           "RIGHT PANEL",1376,  272,  1690,  272),
    (10, "Modulation controls",      "RIGHT PANEL",1376,  474,  1690,  474),
    (11, "Filter controls",          "RIGHT PANEL",1376,  692,  1690,  692),
    (12, "Output controls",          "RIGHT PANEL",1376,  954,  1690,  954),
    (13, "OSC Send/Receive",         "RIGHT PANEL",1380, 1104,  1690, 1104),
    # --- Bottom panel ----------------------------------------------------
    # 14 → the "1" digit on Tap 1 button.
    # 15 → exact knob-circle midpoint between ELEVATION and DISTANCE
    #      (measured from the rendered knob row: EL centre ≈ 330, DIST centre
    #      ≈ 475 → midpoint 402; vertical knob-circle centre ≈ 1052).
    # 16 → exact midpoint between TRAJ/DIR column and SPEED knob column
    #      (TRAJ centre ≈ 915, SPEED centre ≈ 1070 → midpoint 995;
    #      vertical midpoint of both columns ≈ 1052).
    # Callout numbers for 15/16 live in the far bottom corners of the canvas
    # (below the image, clear of the horizontally-centred legend at x=236–1604).
    (14, "Tap Selector",             "PER-TAP",      60,  958,   -50,  958),
    (15, "Per-Tap Controls",         "PER-TAP",     402, 1052,   -50, 1277),
    (16, "Trajectory Controls",      "PER-TAP",     995, 1052,  1690, 1277),
]


def draw_circle_with_number(draw, cx, cy, number, font, section, ss):
    """Draw a filled section-coloured circle with a dark number centered inside.

    All coordinates and sizes are multiplied by `ss` (supersample factor) so
    the overlay renders at higher resolution and antialiases when downscaled.
    """
    r = CIRCLE_RADIUS * ss
    bbox = [cx - r, cy - r, cx + r, cy + r]
    fill = SECTION_COLORS[section]
    outline = SECTION_OUTLINES[section]
    # Outer glow / shadow for visibility against the dark background
    for offset in range(4 * ss, 0, -ss):
        glow_bbox = [bbox[0] - offset, bbox[1] - offset,
                     bbox[2] + offset, bbox[3] + offset]
        alpha = int(40 * (5 * ss - offset) / (4 * ss))
        draw.ellipse(glow_bbox, fill=(0, 0, 0, alpha))
    draw.ellipse(bbox, fill=fill, outline=outline, width=2 * ss)

    # Center the number text
    text = str(number)
    text_bbox = font.getbbox(text)
    tw = text_bbox[2] - text_bbox[0]
    th = text_bbox[3] - text_bbox[1]
    tx = cx - tw / 2 - text_bbox[0]
    ty = cy - th / 2 - text_bbox[1]
    draw.text((tx, ty), text, fill=TEXT_COLOR, font=font)


def draw_leader_line(draw, x1, y1, x2, y2, section, ss):
    """Draw a thin section-coloured leader line from callout circle to target point."""
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx * dx + dy * dy)
    r = CIRCLE_RADIUS * ss
    if dist < r + 4 * ss:
        return

    start_x = x1 + (dx / dist) * (r + 2 * ss)
    start_y = y1 + (dy / dist) * (r + 2 * ss)

    hue = SECTION_COLORS[section]
    line_col = (hue[0], hue[1], hue[2], 210)
    draw.line([(start_x, start_y), (x2, y2)], fill=line_col, width=LINE_WIDTH * ss)
    dot_r = 3 * ss
    draw.ellipse([x2 - dot_r, y2 - dot_r, x2 + dot_r, y2 + dot_r], fill=line_col)


def draw_legend(draw, font_label, font_num, font_heading,
                img_width, start_y, ss):
    """Draw the legend split into per-section subsection blocks.

    Returns the bottom-Y of the legend block so the caller can size the canvas.
    """
    padding = 24 * ss
    line_height = 30 * ss
    heading_height = 34 * ss

    grouped = {name: [] for name in SECTION_ORDER}
    for c in CALLOUTS:
        grouped[c[2]].append(c)

    num_cols = len(SECTION_ORDER)
    col_width = 330 * ss
    legend_width = col_width * num_cols + padding * 2

    max_items = max(len(grouped[name]) for name in SECTION_ORDER)
    legend_height = padding + heading_height + max_items * line_height + padding

    legend_x = (img_width - legend_width) // 2
    legend_y = start_y + 16 * ss

    draw.rounded_rectangle(
        [legend_x, legend_y, legend_x + legend_width, legend_y + legend_height],
        radius=12 * ss, fill=LEGEND_BG
    )

    for col_idx, section in enumerate(SECTION_ORDER):
        col_x = legend_x + padding + col_idx * col_width
        heading_y = legend_y + padding
        fill = SECTION_COLORS[section]

        draw.text((col_x, heading_y), section,
                  fill=fill, font=font_heading)

        for row, (num, label, sec, *_rest) in enumerate(grouped[section]):
            row_y = heading_y + heading_height + row * line_height
            small_r = 11 * ss
            cx = col_x + small_r
            cy = row_y + small_r
            draw.ellipse(
                [cx - small_r, cy - small_r, cx + small_r, cy + small_r],
                fill=fill, outline=SECTION_OUTLINES[section], width=1 * ss,
            )
            text = str(num)
            tb = font_num.getbbox(text)
            tw = tb[2] - tb[0]
            th = tb[3] - tb[1]
            draw.text((cx - tw / 2 - tb[0], cy - th / 2 - tb[1]),
                      text, fill=TEXT_COLOR, font=font_num)
            draw.text((col_x + small_r * 2 + 10 * ss, row_y + 2 * ss), label,
                      fill=LEGEND_TEXT_COLOR, font=font_label)

    return legend_y + legend_height


def main():
    src = Image.open(SRC).convert("RGBA")
    src_w, src_h = src.size
    print(f"Source image: {src_w}x{src_h}")

    margin_top = 60
    margin_right = 120
    margin_left = 80

    # Size the bottom margin to fit the legend without clipping.
    padding = 24
    line_height = 30
    heading_height = 34
    max_items_per_section = max(
        sum(1 for c in CALLOUTS if c[2] == s) for s in SECTION_ORDER
    )
    legend_block_height = padding + heading_height + max_items_per_section * line_height + padding
    margin_bottom = 4 + 16 + legend_block_height + 24

    canvas_w = src_w + margin_left + margin_right
    canvas_h = src_h + margin_top + margin_bottom

    canvas = Image.new("RGBA", (canvas_w, canvas_h), (14, 17, 23, 255))
    canvas.paste(src, (margin_left, margin_top), src)

    # Supersampled overlay: all drawing happens at SUPERSAMPLE× resolution and
    # the result is downscaled with LANCZOS so circles and leader lines look
    # antialiased (Pillow's primitives have no native AA).
    ss = SUPERSAMPLE
    overlay_hi = Image.new(
        "RGBA", (canvas_w * ss, canvas_h * ss), (0, 0, 0, 0)
    )
    draw = ImageDraw.Draw(overlay_hi)

    font_number = ImageFont.truetype(FONT_PATH, 20 * ss)
    font_legend_num = ImageFont.truetype(FONT_PATH, 14 * ss)
    font_legend_label = ImageFont.truetype(FONT_PATH, 16 * ss)
    font_legend_heading = ImageFont.truetype(FONT_PATH, 14 * ss)

    # Adjust callout coords into canvas space, then scale to the supersampled
    # overlay.
    adjusted_callouts = []
    for num, label, section, tx, ty, cx, cy in CALLOUTS:
        adj_tx = (tx + margin_left) * ss
        adj_ty = (ty + margin_top) * ss
        adj_cx = (cx + margin_left) * ss
        adj_cy = (cy + margin_top) * ss
        adjusted_callouts.append(
            (num, label, section, adj_tx, adj_ty, adj_cx, adj_cy)
        )

    # Draw legend FIRST so that leader lines from 15/16 (which sit in the
    # bottom canvas corners and diagonally cross the legend on their way to
    # the bottom-panel knobs) render on top of the legend box rather than
    # being cut off by it.
    legend_start_y = (src_h + margin_top + 4) * ss
    draw_legend(
        draw,
        font_legend_label,
        font_legend_num,
        font_legend_heading,
        canvas_w * ss,
        legend_start_y,
        ss,
    )

    for num, label, section, tx, ty, cx, cy in adjusted_callouts:
        draw_leader_line(draw, cx, cy, tx, ty, section, ss)

    for num, label, section, tx, ty, cx, cy in adjusted_callouts:
        draw_circle_with_number(draw, cx, cy, num, font_number, section, ss)

    overlay = overlay_hi.resize((canvas_w, canvas_h), Image.LANCZOS)

    canvas = Image.alpha_composite(canvas, overlay)
    canvas.save(DST, "PNG")
    print(f"Saved annotated screenshot: {DST}")
    print(f"Output size: {canvas.size[0]}x{canvas.size[1]}")


if __name__ == "__main__":
    main()
