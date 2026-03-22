#!/usr/bin/env python3
"""Generate signal flow diagram for OpenSpatialDelay user manual."""

from PIL import Image, ImageDraw, ImageFont
import os

# Output
OUT_DIR = os.path.join(os.path.dirname(__file__), "assets")
OUT_PATH = os.path.join(OUT_DIR, "signal-flow.png")

# Canvas
W, H = 900, 340
BG = (10, 13, 18)          # #0A0D12
BOX_FILL = (0, 22, 51)     # #001633
BOX_STROKE = (128, 216, 255)  # #80D8FF
ARROW_COL = (159, 165, 174)   # #9FA5AE
TEXT_COL = (225, 229, 234)     # #E1E5EA
ANNOT_COL = (109, 114, 121)   # #6D7279
FB_COL = (240, 166, 70)       # #F0A646 amber for feedback

# Box layout
BOX_W, BOX_H = 130, 50
GAP = 22
START_X = 45
START_Y = 80

# Try to load a nice font, fall back to default
FONTS_DIR = os.path.join(os.path.dirname(__file__), "..", "fonts")
try:
    font_main = ImageFont.truetype(os.path.join(FONTS_DIR, "DM_Sans-SemiBold.ttf"), 14)
    font_small = ImageFont.truetype(os.path.join(FONTS_DIR, "DM_Sans-Regular.ttf"), 11)
    font_annot = ImageFont.truetype(os.path.join(FONTS_DIR, "JetBrains_Mono-Regular.ttf"), 10)
    font_title = ImageFont.truetype(os.path.join(FONTS_DIR, "DM_Sans-Bold.ttf"), 16)
except:
    font_main = ImageFont.load_default()
    font_small = font_main
    font_annot = font_main
    font_title = font_main


def rounded_rect(draw, xy, radius, fill, outline):
    x0, y0, x1, y1 = xy
    r = radius
    # Fill
    draw.rectangle([x0+r, y0, x1-r, y1], fill=fill)
    draw.rectangle([x0, y0+r, x1, y1-r], fill=fill)
    draw.pieslice([x0, y0, x0+2*r, y0+2*r], 180, 270, fill=fill)
    draw.pieslice([x1-2*r, y0, x1, y0+2*r], 270, 360, fill=fill)
    draw.pieslice([x0, y1-2*r, x0+2*r, y1], 90, 180, fill=fill)
    draw.pieslice([x1-2*r, y1-2*r, x1, y1], 0, 90, fill=fill)
    # Outline
    draw.arc([x0, y0, x0+2*r, y0+2*r], 180, 270, fill=outline, width=2)
    draw.arc([x1-2*r, y0, x1, y0+2*r], 270, 360, fill=outline, width=2)
    draw.arc([x0, y1-2*r, x0+2*r, y1], 90, 180, fill=outline, width=2)
    draw.arc([x1-2*r, y1-2*r, x1, y1], 0, 90, fill=outline, width=2)
    draw.line([x0+r, y0, x1-r, y0], fill=outline, width=2)
    draw.line([x0+r, y1, x1-r, y1], fill=outline, width=2)
    draw.line([x0, y0+r, x0, y1-r], fill=outline, width=2)
    draw.line([x1, y0+r, x1, y1-r], fill=outline, width=2)


def draw_arrow(draw, x1, y1, x2, y2, color, width=2):
    draw.line([x1, y1, x2, y2], fill=color, width=width)
    # Arrowhead
    import math
    angle = math.atan2(y2 - y1, x2 - x1)
    size = 8
    draw.polygon([
        (x2, y2),
        (x2 - size * math.cos(angle - 0.4), y2 - size * math.sin(angle - 0.4)),
        (x2 - size * math.cos(angle + 0.4), y2 - size * math.sin(angle + 0.4)),
    ], fill=color)


def center_text(draw, text, x, y, w, font, color):
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((x + (w - tw) / 2, y), text, fill=color, font=font)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    img = Image.new("RGBA", (W, H), BG)
    draw = ImageDraw.Draw(img)

    # Title
    center_text(draw, "Signal Flow", 0, 12, W, font_title, BOX_STROKE)

    # Stage definitions: (label, annotation_above, annotation_below)
    stages = [
        ("INPUT", "", "mono / stereo"),
        ("WRITE", "soft clip", "+ feedback"),
        ("READ &\nPITCH", "per tap k", "k x delay\nk x pitch"),
        ("SPATIALIZE", "per tap", "HRTF / speakers"),
        ("OUTPUT", "dry/wet", "limiter"),
    ]

    boxes = []
    x = START_X
    for i, (label, annot_top, annot_bot) in enumerate(stages):
        bx = x
        by = START_Y
        rounded_rect(draw, (bx, by, bx + BOX_W, by + BOX_H), 8, BOX_FILL, BOX_STROKE)

        # Label
        lines = label.split("\n")
        if len(lines) == 1:
            center_text(draw, label, bx, by + 15, BOX_W, font_main, TEXT_COL)
        else:
            center_text(draw, lines[0], bx, by + 7, BOX_W, font_main, TEXT_COL)
            center_text(draw, lines[1], bx, by + 24, BOX_W, font_main, TEXT_COL)

        # Annotations
        if annot_top:
            center_text(draw, annot_top, bx, by - 18, BOX_W, font_annot, ANNOT_COL)
        if annot_bot:
            bot_lines = annot_bot.split("\n")
            for j, bl in enumerate(bot_lines):
                center_text(draw, bl, bx, by + BOX_H + 6 + j * 16, BOX_W, font_annot, ANNOT_COL)

        boxes.append((bx, by))
        x += BOX_W + GAP

    # Arrows between stages
    for i in range(len(boxes) - 1):
        x1 = boxes[i][0] + BOX_W
        y1 = boxes[i][1] + BOX_H // 2
        x2 = boxes[i + 1][0]
        y2 = boxes[i + 1][1] + BOX_H // 2
        draw_arrow(draw, x1 + 2, y1, x2 - 2, y2, ARROW_COL)

    # Feedback loop — curved path from after SPATIALIZE back to WRITE
    fb_start_x = boxes[3][0] + BOX_W // 2  # center of SPATIALIZE
    fb_start_y = boxes[3][1] + BOX_H  # bottom of SPATIALIZE
    fb_end_x = boxes[1][0] + BOX_W // 2  # center of WRITE
    fb_end_y = boxes[1][1] + BOX_H  # bottom of WRITE

    fb_y_bottom = START_Y + BOX_H + 80  # how far down the loop goes

    # Draw feedback box
    fb_box_x = (fb_start_x + fb_end_x) // 2 - BOX_W // 2
    fb_box_y = fb_y_bottom + 20
    rounded_rect(draw, (fb_box_x, fb_box_y, fb_box_x + BOX_W, fb_box_y + BOX_H), 8, BOX_FILL, FB_COL)
    center_text(draw, "FEEDBACK", fb_box_x, fb_box_y + 7, BOX_W, font_main, TEXT_COL)
    center_text(draw, "LP + HP", fb_box_x, fb_box_y + 24, BOX_W, font_small, FB_COL)

    # Annotations for feedback
    center_text(draw, "mono from tap N", fb_box_x + BOX_W + 8, fb_box_y + 7, 130, font_annot, ANNOT_COL)
    center_text(draw, "N x pitch", fb_box_x + BOX_W + 8, fb_box_y + 24, 130, font_annot, ANNOT_COL)

    # Arrow: SPATIALIZE bottom → down → FEEDBACK top
    draw.line([fb_start_x, fb_start_y + 2, fb_start_x, fb_box_y - 8], fill=FB_COL, width=2)
    draw.line([fb_start_x, fb_box_y - 8, fb_box_x + BOX_W, fb_box_y + BOX_H // 2], fill=FB_COL, width=2)
    draw_arrow(draw, fb_start_x, fb_box_y - 12, fb_box_x + BOX_W, fb_box_y + BOX_H // 2, FB_COL)

    # Arrow: FEEDBACK left → up → WRITE bottom
    draw.line([fb_box_x, fb_box_y + BOX_H // 2, fb_end_x, fb_box_y + BOX_H // 2], fill=FB_COL, width=2)
    draw_arrow(draw, fb_end_x, fb_box_y + BOX_H // 2, fb_end_x, fb_end_y + 4, FB_COL)

    # Stage numbers
    for i, (bx, by) in enumerate(boxes):
        center_text(draw, f"Stage {i+1}", bx, by + BOX_H + 42, BOX_W, font_small, (90, 95, 102))

    img.save(OUT_PATH, "PNG")
    print(f"Signal flow diagram saved: {OUT_PATH}")


if __name__ == "__main__":
    main()
