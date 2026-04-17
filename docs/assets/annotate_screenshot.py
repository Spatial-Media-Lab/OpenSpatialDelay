#!/usr/bin/env python3
"""
Annotate OpenSpatialDelay plugin screenshot with numbered callouts.
Generates screenshot_annotated.png with leader lines, callout circles, and a legend.

Round 2 (issue #168): callouts are coloured by UI section, and the legend is
broken into subsections that mirror the callout colour groups.
"""

from PIL import Image, ImageDraw, ImageFont
import math

# --- Configuration ---
# Paths resolve relative to the repository root (parent of docs/).
import os
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO_ROOT, "docs/assets/screenshot.png")
DST = os.path.join(REPO_ROOT, "docs/assets/screenshot_annotated.png")
FONT_PATH = os.path.join(REPO_ROOT, "fonts/DM_Sans-Bold.ttf")

CIRCLE_RADIUS = 18  # 36px diameter
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
CALLOUTS = [
    # --- Header row ------------------------------------------------------
    ( 1, "SML Badge",           "HEADER",       80,   52,    80, -40),
    ( 2, "Preset Browser",      "HEADER",      625,   52,   625, -40),
    ( 3, "Undo / Redo",         "HEADER",      975,   52,   975, -40),
    ( 4, "Input Format",        "HEADER",     1098,   52,  1098, -40),
    ( 5, "Output Format",       "HEADER",     1248,   52,  1248, -40),
    ( 6, "Algorithm",           "HEADER",     1430,   52,  1430, -40),
    # --- Map / Drawer ----------------------------------------------------
    ( 7, "Global Drawer",       "MAP",          30,  420,   -50,  420),
    ( 8, "Spatial Map",         "MAP",         500,  450,   -50,  560),
    # --- Right panel sections --------------------------------------------
    ( 9, "DELAY Section",       "RIGHT PANEL",1350,  200,  1690,  160),
    (10, "MOD Section",         "RIGHT PANEL",1350,  380,  1690,  340),
    (11, "TONE Section",        "RIGHT PANEL",1350,  560,  1690,  520),
    (12, "MIX Section",         "RIGHT PANEL",1350,  800,  1690,  740),
    (13, "OSC Section",         "RIGHT PANEL",1350,  950,  1690,  930),
    # --- Bottom panel ----------------------------------------------------
    (14, "Tap Selector",        "PER-TAP",     200,  950,   -50,  950),
    (15, "Per-Tap Controls",    "PER-TAP",     500, 1060,   420, 1210),
    (16, "Trajectory Controls", "PER-TAP",    1000, 1060,  1000, 1210),
]


def draw_circle_with_number(draw, cx, cy, number, font, section):
    """Draw a filled section-coloured circle with a dark number centered inside."""
    bbox = [cx - CIRCLE_RADIUS, cy - CIRCLE_RADIUS,
            cx + CIRCLE_RADIUS, cy + CIRCLE_RADIUS]
    fill = SECTION_COLORS[section]
    outline = SECTION_OUTLINES[section]
    # Outer glow / shadow for visibility against the dark background
    for offset in range(4, 0, -1):
        glow_bbox = [bbox[0] - offset, bbox[1] - offset,
                     bbox[2] + offset, bbox[3] + offset]
        alpha = int(40 * (5 - offset) / 4)
        draw.ellipse(glow_bbox, fill=(0, 0, 0, alpha))
    draw.ellipse(bbox, fill=fill, outline=outline, width=2)

    # Center the number text
    text = str(number)
    text_bbox = font.getbbox(text)
    tw = text_bbox[2] - text_bbox[0]
    th = text_bbox[3] - text_bbox[1]
    tx = cx - tw / 2 - text_bbox[0]
    ty = cy - th / 2 - text_bbox[1]
    draw.text((tx, ty), text, fill=TEXT_COLOR, font=font)


def draw_leader_line(draw, x1, y1, x2, y2, section):
    """Draw a thin section-coloured leader line from callout circle to target point."""
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx * dx + dy * dy)
    if dist < CIRCLE_RADIUS + 4:
        return

    start_x = x1 + (dx / dist) * (CIRCLE_RADIUS + 2)
    start_y = y1 + (dy / dist) * (CIRCLE_RADIUS + 2)

    hue = SECTION_COLORS[section]
    line_col = (hue[0], hue[1], hue[2], 210)
    draw.line([(start_x, start_y), (x2, y2)], fill=line_col, width=LINE_WIDTH)
    dot_r = 3
    draw.ellipse([x2 - dot_r, y2 - dot_r, x2 + dot_r, y2 + dot_r], fill=line_col)


def draw_legend(draw, font_label, font_num, font_heading,
                img_width, start_y):
    """Draw the legend split into per-section subsection blocks.

    Returns the bottom-Y of the legend block so the caller can size the canvas.
    """
    padding = 24
    line_height = 30
    heading_height = 34
    block_gap = 14

    # Group callouts by section, preserving the SECTION_ORDER
    grouped = {name: [] for name in SECTION_ORDER}
    for c in CALLOUTS:
        grouped[c[2]].append(c)

    # Compute column layout: one column per section, equal widths
    num_cols = len(SECTION_ORDER)
    col_width = 330
    legend_width = col_width * num_cols + padding * 2

    # Tallest column determines block height
    max_items = max(len(grouped[name]) for name in SECTION_ORDER)
    legend_height = padding + heading_height + max_items * line_height + padding

    legend_x = (img_width - legend_width) // 2
    legend_y = start_y + 16

    draw.rounded_rectangle(
        [legend_x, legend_y, legend_x + legend_width, legend_y + legend_height],
        radius=12, fill=LEGEND_BG
    )

    for col_idx, section in enumerate(SECTION_ORDER):
        col_x = legend_x + padding + col_idx * col_width
        heading_y = legend_y + padding
        fill = SECTION_COLORS[section]

        # Subsection heading in the section hue
        draw.text((col_x, heading_y), section,
                  fill=fill, font=font_heading)

        # Items
        for row, (num, label, sec, *_rest) in enumerate(grouped[section]):
            row_y = heading_y + heading_height + row * line_height
            small_r = 11
            cx = col_x + small_r
            cy = row_y + small_r
            draw.ellipse(
                [cx - small_r, cy - small_r, cx + small_r, cy + small_r],
                fill=fill, outline=SECTION_OUTLINES[section], width=1,
            )
            text = str(num)
            tb = font_num.getbbox(text)
            tw = tb[2] - tb[0]
            th = tb[3] - tb[1]
            draw.text((cx - tw / 2 - tb[0], cy - th / 2 - tb[1]),
                      text, fill=TEXT_COLOR, font=font_num)
            draw.text((col_x + small_r * 2 + 10, row_y + 2), label,
                      fill=LEGEND_TEXT_COLOR, font=font_label)

    _ = block_gap  # reserved for future inter-section spacing
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

    overlay = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    font_number = ImageFont.truetype(FONT_PATH, 20)
    font_legend_num = ImageFont.truetype(FONT_PATH, 14)
    font_legend_label = ImageFont.truetype(FONT_PATH, 16)
    font_legend_heading = ImageFont.truetype(FONT_PATH, 14)

    adjusted_callouts = []
    for num, label, section, tx, ty, cx, cy in CALLOUTS:
        adj_tx = tx + margin_left
        adj_ty = ty + margin_top
        adj_cx = cx + margin_left
        adj_cy = cy + margin_top
        adjusted_callouts.append((num, label, section, adj_tx, adj_ty, adj_cx, adj_cy))

    for num, label, section, tx, ty, cx, cy in adjusted_callouts:
        draw_leader_line(draw, cx, cy, tx, ty, section)

    for num, label, section, tx, ty, cx, cy in adjusted_callouts:
        draw_circle_with_number(draw, cx, cy, num, font_number, section)

    legend_start_y = src_h + margin_top + 4
    draw_legend(
        draw,
        font_legend_label,
        font_legend_num,
        font_legend_heading,
        canvas_w,
        legend_start_y,
    )

    canvas = Image.alpha_composite(canvas, overlay)
    canvas.save(DST, "PNG")
    print(f"Saved annotated screenshot: {DST}")
    print(f"Output size: {canvas.size[0]}x{canvas.size[1]}")


if __name__ == "__main__":
    main()
