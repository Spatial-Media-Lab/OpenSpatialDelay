#!/usr/bin/env python3
"""
Annotate OpenSpatialDelay plugin screenshot with numbered callouts.
Generates screenshot_annotated.png with leader lines, callout circles, and a legend.
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
# Amber/orange callouts: high contrast against the plugin's cyan/blue accent
# so the numbers never blend into the UI colour. Dark text on top keeps the
# numerals legible without relying on hue against the bright circle.
CIRCLE_COLOR = (255, 176, 59)      # #FFB03B warm amber
CIRCLE_OUTLINE = (70, 40, 0)       # dark amber outline for pop
LINE_COLOR = (255, 176, 59, 210)   # amber at ~82% opacity
LINE_WIDTH = 2
TEXT_COLOR = (20, 14, 2)           # near-black for legibility on amber
LEGEND_BG = (20, 24, 32, 220)      # dark semi-transparent
LEGEND_TEXT_COLOR = (225, 228, 235)
LEGEND_NUM_COLOR = CIRCLE_COLOR

# Callout definitions: (number, label, target_x, target_y, callout_x, callout_y)
# target = point on the UI element; callout = where the circle goes (outside the element)
CALLOUTS = [
    (1,  "SML Badge",          80,   52,   80,    -40),
    (2,  "Preset Browser",     540,  52,   540,   -40),
    (3,  "Undo / Redo",        900,  52,   900,   -40),
    (4,  "Input Format",       1100, 52,   1100,  -40),
    (5,  "Output Format",      1280, 52,   1280,  -40),
    (6,  "HRTF Profile",       1500, 52,   1500,  -40),
    (7,  "Global Drawer",      30,   420,  -50,   420),
    (8,  "Spatial Map",        500,  450,  -50,   560),
    (9,  "DELAY Section",      1350, 200,  1590,  160),
    (10, "MOD Section",        1350, 360,  1590,  340),
    (11, "TONE Section",       1350, 520,  1590,  520),
    (12, "MIX Section",        1350, 680,  1590,  680),
    (13, "OSC Section",        1350, 890,  1590,  890),
    (14, "Tap Selector",       200,  878,  -50,   878),
    (15, "Per-Tap Controls",   620,  1010, 420,   1130),
    (16, "Trajectory Controls",1100, 1010, 1100,  1130),
]


def draw_circle_with_number(draw, cx, cy, number, font):
    """Draw a filled amber circle with a dark number centered inside."""
    bbox = [cx - CIRCLE_RADIUS, cy - CIRCLE_RADIUS,
            cx + CIRCLE_RADIUS, cy + CIRCLE_RADIUS]
    # Outer glow / shadow for visibility
    for offset in range(4, 0, -1):
        glow_bbox = [bbox[0] - offset, bbox[1] - offset,
                     bbox[2] + offset, bbox[3] + offset]
        alpha = int(40 * (5 - offset) / 4)
        draw.ellipse(glow_bbox, fill=(0, 0, 0, alpha))
    # Main circle with a thin dark outline for separation against light UI
    draw.ellipse(bbox, fill=CIRCLE_COLOR, outline=CIRCLE_OUTLINE, width=2)

    # Center the number text
    text = str(number)
    text_bbox = font.getbbox(text)
    tw = text_bbox[2] - text_bbox[0]
    th = text_bbox[3] - text_bbox[1]
    # Adjust for font metrics offset
    tx = cx - tw / 2 - text_bbox[0]
    ty = cy - th / 2 - text_bbox[1]
    draw.text((tx, ty), text, fill=TEXT_COLOR, font=font)


def draw_leader_line(draw, x1, y1, x2, y2):
    """Draw a thin leader line from callout circle edge to target point."""
    # Calculate direction from callout center to target
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx * dx + dy * dy)
    if dist < CIRCLE_RADIUS + 4:
        return  # Too close, skip line

    # Start line at the edge of the circle
    start_x = x1 + (dx / dist) * (CIRCLE_RADIUS + 2)
    start_y = y1 + (dy / dist) * (CIRCLE_RADIUS + 2)

    # Small dot at the target end
    draw.line([(start_x, start_y), (x2, y2)], fill=LINE_COLOR, width=LINE_WIDTH)
    # Target dot
    dot_r = 3
    draw.ellipse([x2 - dot_r, y2 - dot_r, x2 + dot_r, y2 + dot_r], fill=LINE_COLOR)


def draw_legend(canvas, draw, font_label, font_num, img_width, start_y):
    """Draw the legend area at the bottom of the canvas."""
    padding = 24
    line_height = 30
    col_width = 360
    num_cols = 3
    items_per_col = (len(CALLOUTS) + num_cols - 1) // num_cols  # ceil

    legend_width = col_width * num_cols + padding * 2
    legend_height = items_per_col * line_height + padding * 2
    legend_x = (img_width - legend_width) // 2
    legend_y = start_y + 16

    # Background rounded rectangle
    draw.rounded_rectangle(
        [legend_x, legend_y, legend_x + legend_width, legend_y + legend_height],
        radius=12, fill=LEGEND_BG
    )

    for i, (num, label, *_) in enumerate(CALLOUTS):
        col = i // items_per_col
        row = i % items_per_col
        x = legend_x + padding + col * col_width
        y = legend_y + padding + row * line_height

        # Small circle
        small_r = 11
        cx = x + small_r
        cy = y + small_r
        draw.ellipse([cx - small_r, cy - small_r, cx + small_r, cy + small_r],
                      fill=CIRCLE_COLOR, outline=CIRCLE_OUTLINE, width=1)
        # Number in small circle
        text = str(num)
        tb = font_num.getbbox(text)
        tw = tb[2] - tb[0]
        th = tb[3] - tb[1]
        draw.text((cx - tw / 2 - tb[0], cy - th / 2 - tb[1]),
                  text, fill=TEXT_COLOR, font=font_num)

        # Label text
        draw.text((x + small_r * 2 + 10, y + 2), label,
                  fill=LEGEND_TEXT_COLOR, font=font_label)

    return legend_y + legend_height


def main():
    # Load source image
    src = Image.open(SRC).convert("RGBA")
    src_w, src_h = src.size
    print(f"Source image: {src_w}x{src_h}")

    # Expand canvas: add margin at top, right, bottom, left for callout placement.
    # Legend height depends on the number of callouts — size margin_bottom
    # dynamically so the last row is never clipped.
    margin_top = 60
    margin_right = 80
    margin_left = 80

    legend_padding = 24
    legend_line_height = 30
    legend_num_cols = 3
    legend_items_per_col = (len(CALLOUTS) + legend_num_cols - 1) // legend_num_cols
    legend_block_height = legend_items_per_col * legend_line_height + legend_padding * 2
    # 4px lead-in (above legend) + 16px vertical offset + legend block + 24px tail buffer
    margin_bottom = 4 + 16 + legend_block_height + 24

    canvas_w = src_w + margin_left + margin_right
    canvas_h = src_h + margin_top + margin_bottom

    # Create canvas with dark background
    canvas = Image.new("RGBA", (canvas_w, canvas_h), (14, 17, 23, 255))

    # Paste the original screenshot with offset
    canvas.paste(src, (margin_left, margin_top), src)

    # Create overlay for drawing (allows alpha blending for lines)
    overlay = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    # Load fonts
    font_number = ImageFont.truetype(FONT_PATH, 20)
    font_legend_num = ImageFont.truetype(FONT_PATH, 14)
    font_legend_label = ImageFont.truetype(FONT_PATH, 16)

    # Adjust all coordinates by the margin offset
    adjusted_callouts = []
    for num, label, tx, ty, cx, cy in CALLOUTS:
        adj_tx = tx + margin_left
        adj_ty = ty + margin_top
        adj_cx = cx + margin_left
        adj_cy = cy + margin_top
        adjusted_callouts.append((num, label, adj_tx, adj_ty, adj_cx, adj_cy))

    # Draw leader lines first (behind circles)
    for num, label, tx, ty, cx, cy in adjusted_callouts:
        draw_leader_line(draw, cx, cy, tx, ty)

    # Draw callout circles on top
    for num, label, tx, ty, cx, cy in adjusted_callouts:
        draw_circle_with_number(draw, cx, cy, num, font_number)

    # Draw legend
    legend_start_y = src_h + margin_top + 4
    draw_legend(canvas, draw, font_legend_label, font_legend_num, canvas_w, legend_start_y)

    # Composite overlay onto canvas
    canvas = Image.alpha_composite(canvas, overlay)

    # Save
    canvas.save(DST, "PNG")
    print(f"Saved annotated screenshot: {DST}")
    print(f"Output size: {canvas.size[0]}x{canvas.size[1]}")


if __name__ == "__main__":
    main()
