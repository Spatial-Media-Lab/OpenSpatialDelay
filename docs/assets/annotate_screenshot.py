#!/usr/bin/env python3
"""
Annotate OpenSpatialDelay plugin screenshot with numbered callouts.
Generates screenshot_annotated.png with leader lines, callout circles, and a legend.
"""

from PIL import Image, ImageDraw, ImageFont
import math

# --- Configuration ---
SRC = "/Users/andrewrahman/conductor/workspaces/openspatialdelay/moscow-v1/docs/assets/screenshot.png"
DST = "/Users/andrewrahman/conductor/workspaces/openspatialdelay/moscow-v1/docs/assets/screenshot_annotated.png"
FONT_PATH = "/Users/andrewrahman/conductor/workspaces/openspatialdelay/moscow-v1/fonts/DM_Sans-Bold.ttf"

CIRCLE_RADIUS = 18  # 36px diameter
CIRCLE_COLOR = (128, 216, 255)  # #80D8FF cyan
LINE_COLOR = (128, 216, 255, 153)  # #80D8FF at 60% opacity
LINE_WIDTH = 2
TEXT_COLOR = (255, 255, 255)  # White
LEGEND_BG = (20, 24, 32, 220)  # Dark semi-transparent
LEGEND_TEXT_COLOR = (200, 210, 220)
LEGEND_NUM_COLOR = CIRCLE_COLOR

# Callout definitions: (number, label, target_x, target_y, callout_x, callout_y)
# target = point on the UI element; callout = where the circle goes (outside the element)
CALLOUTS = [
    (1,  "SML Badge",          80,   52,   80,    -40),
    (2,  "Preset Browser",     340,  52,   340,   -40),
    (3,  "Input Format",       640,  52,   640,   -40),
    (4,  "Output Format",      900,  52,   900,   -40),
    (5,  "HRTF Profile",       1150, 52,   1150,  -40),
    (6,  "Spatial Map",        500,  450,  -50,   450),
    (7,  "DELAY Section",      1350, 180,  1590,  140),
    (8,  "MOD Section",        1350, 320,  1590,  320),
    (9,  "TONE Section",       1350, 460,  1590,  460),
    (10, "MIX Section",        1350, 600,  1590,  600),
    (11, "OSC Section",        1350, 740,  1590,  740),
    (12, "Tap Selector",       200,  878,  -50,   878),
    (13, "Per-Tap Controls",   620,  1010, 420,   1130),
    (14, "Trajectory Controls",1100, 1010, 1100,  1130),
]


def draw_circle_with_number(draw, cx, cy, number, font):
    """Draw a filled cyan circle with a white number centered inside."""
    bbox = [cx - CIRCLE_RADIUS, cy - CIRCLE_RADIUS,
            cx + CIRCLE_RADIUS, cy + CIRCLE_RADIUS]
    # Outer glow / shadow for visibility
    for offset in range(4, 0, -1):
        glow_bbox = [bbox[0] - offset, bbox[1] - offset,
                     bbox[2] + offset, bbox[3] + offset]
        alpha = int(40 * (5 - offset) / 4)
        draw.ellipse(glow_bbox, fill=(0, 0, 0, alpha))
    # Main circle
    draw.ellipse(bbox, fill=CIRCLE_COLOR)

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
    items_per_col = 5  # ceil(14/3)

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
                      fill=CIRCLE_COLOR)
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

    # Expand canvas: add margin at top, right, bottom, left for callout placement
    margin_top = 60
    margin_right = 80
    margin_bottom = 210  # Space for legend
    margin_left = 80

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
