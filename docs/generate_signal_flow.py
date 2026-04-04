#!/usr/bin/env python3
"""
Generate professional signal flow diagram for OpenSpatialDelay.
Orbital Cartography design: dark background, cyan/violet/amber accents.
"""

from PIL import Image, ImageDraw, ImageFont
import math
import os

# --- Configuration ---
W, H = 2400, 1400
BG = (10, 10, 20)          # #0A0A14
CYAN = (128, 216, 255)     # #80D8FF
VIOLET = (116, 87, 209)    # #7457D1
AMBER = (240, 166, 70)     # #F0A646
WHITE = (225, 229, 234)    # #E1E5EA
DIM = (100, 105, 120)      # Muted for secondary text
DARK_PANEL = (18, 18, 32)  # Panel fill
CYAN_DIM = (50, 85, 105)   # Subtle cyan border
VIOLET_DIM = (55, 40, 100) # Subtle violet border
AMBER_DIM = (90, 65, 35)   # Subtle amber border

FONT_DIR = os.path.join(os.path.dirname(__file__), "..", "fonts")
OUT_DIR = os.path.join(os.path.dirname(__file__), "assets")
OUT_PATH = os.path.join(OUT_DIR, "signal-flow.png")


def load_font(name, size):
    return ImageFont.truetype(os.path.join(FONT_DIR, name), size)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # Load fonts
    dm_bold_32 = load_font("DM_Sans-Bold.ttf", 32)
    dm_bold_24 = load_font("DM_Sans-Bold.ttf", 24)
    dm_bold_22 = load_font("DM_Sans-Bold.ttf", 22)
    dm_semi_20 = load_font("DM_Sans-SemiBold.ttf", 20)
    dm_med_18 = load_font("DM_Sans-Medium.ttf", 18)
    dm_med_16 = load_font("DM_Sans-Medium.ttf", 16)
    dm_reg_14 = load_font("DM_Sans-Regular.ttf", 14)
    jb_reg_13 = load_font("JetBrains_Mono-Regular.ttf", 13)
    jb_reg_12 = load_font("JetBrains_Mono-Regular.ttf", 12)

    # Create image
    img = Image.new("RGBA", (W, H), BG + (255,))
    draw = ImageDraw.Draw(img, "RGBA")

    # --- Drawing helpers ---
    def rounded_rect(x, y, w, h, r, fill, border_color, border_width=2):
        draw.rounded_rectangle(
            [x, y, x + w, y + h], radius=r,
            fill=fill, outline=border_color, width=border_width
        )

    def draw_block(x, y, w, h, label, fill=DARK_PANEL, border=CYAN_DIM,
                   text_color=WHITE, font=dm_semi_20, sublabel=None,
                   sublabel_font=None, sublabel_color=DIM, border_width=2,
                   radius=12):
        rounded_rect(x, y, w, h, radius, fill, border, border_width)
        bbox = font.getbbox(label)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        if sublabel:
            ty = y + h // 2 - th - 2
        else:
            ty = y + (h - th) // 2
        tx = x + (w - tw) // 2
        draw.text((tx, ty), label, fill=text_color, font=font)
        if sublabel:
            sf = sublabel_font or jb_reg_13
            sb = sf.getbbox(sublabel)
            sw = sb[2] - sb[0]
            draw.text((x + (w - sw) // 2, ty + th + 6), sublabel,
                      fill=sublabel_color, font=sf)

    def draw_arrow_h(x1, y1, x2, y2, color=CYAN, width=2, head_size=10):
        draw.line([(x1, y1), (x2, y2)], fill=color, width=width)
        dx = x2 - x1
        dy = y2 - y1
        length = math.sqrt(dx * dx + dy * dy)
        if length == 0:
            return
        ux, uy = dx / length, dy / length
        px, py = -uy, ux
        hx, hy = x2 - ux * head_size, y2 - uy * head_size
        points = [
            (x2, y2),
            (hx + px * head_size * 0.5, hy + py * head_size * 0.5),
            (hx - px * head_size * 0.5, hy - py * head_size * 0.5),
        ]
        draw.polygon(points, fill=color)

    def draw_arrow_down(x, y1, y2, color=CYAN, width=2, head_size=10):
        draw_arrow_h(x, y1, x, y2, color, width, head_size)

    # --- Background grid pattern (subtle) ---
    grid_color = (20, 20, 35, 80)
    for gx in range(0, W, 40):
        draw.line([(gx, 0), (gx, H)], fill=grid_color, width=1)
    for gy in range(0, H, 40):
        draw.line([(0, gy), (W, gy)], fill=grid_color, width=1)

    # --- Title ---
    draw.text((60, 30), "OpenSpatialDelay", fill=CYAN, font=dm_bold_32)
    draw.text((60, 68), "Signal Flow Diagram", fill=DIM, font=dm_med_18)
    draw.text((60, 92), "v1.0  |  1-49 channel spatial delay",
              fill=(70, 75, 90), font=jb_reg_13)

    # --- Legend ---
    legend_x = 1900
    legend_y = 30
    draw.text((legend_x, legend_y), "LEGEND", fill=DIM, font=dm_bold_22)
    draw.line([(legend_x, legend_y + 35), (legend_x + 30, legend_y + 35)],
              fill=CYAN, width=3)
    draw.text((legend_x + 40, legend_y + 27), "Main Signal Path",
              fill=WHITE, font=dm_reg_14)
    draw.line([(legend_x, legend_y + 60), (legend_x + 30, legend_y + 60)],
              fill=AMBER, width=3)
    draw.text((legend_x + 40, legend_y + 52), "Dry Path (Latency Comp)",
              fill=WHITE, font=dm_reg_14)
    draw.line([(legend_x, legend_y + 85), (legend_x + 30, legend_y + 85)],
              fill=VIOLET, width=3)
    draw.text((legend_x + 40, legend_y + 77), "Feedback Path",
              fill=WHITE, font=dm_reg_14)

    # =====================================================================
    #  LAYOUT -- top-to-bottom flow with horizontal sub-chains
    # =====================================================================

    # ---- Row 0: Stereo Input ----
    input_x, input_y = 1050, 150
    input_w, input_h = 300, 56
    draw_block(input_x, input_y, input_w, input_h, "STEREO INPUT",
               border=CYAN, text_color=CYAN, font=dm_bold_22,
               sublabel="L / R", sublabel_color=DIM, sublabel_font=jb_reg_13)

    # ---- Split point ----
    split_y = input_y + input_h + 30
    split_x = input_x + input_w // 2  # center = 1200
    draw_arrow_down(split_x, input_y + input_h, split_y, color=CYAN, width=3)

    # Split dot
    dot_r = 6
    draw.ellipse([split_x - dot_r, split_y - dot_r,
                  split_x + dot_r, split_y + dot_r], fill=CYAN)

    # =====================================================================
    #  DRY PATH (top-right branch) -- amber
    # =====================================================================
    dry_block_x = 1520
    dry_block_y = split_y - 25
    dry_block_w = 320
    dry_block_h = 50

    draw.line([(split_x, split_y), (dry_block_x, split_y)],
              fill=AMBER, width=3)

    draw_block(dry_block_x, dry_block_y, dry_block_w, dry_block_h,
               "Latency Compensation",
               border=AMBER_DIM, text_color=AMBER, font=dm_semi_20,
               sublabel="2048 samples", sublabel_font=jb_reg_13,
               sublabel_color=(200, 140, 60), border_width=2)

    draw.text((split_x + 100, split_y - 22), "DRY", fill=AMBER, font=dm_bold_22)

    dry_exit_x = dry_block_x + dry_block_w // 2
    dry_exit_y = dry_block_y + dry_block_h

    # =====================================================================
    #  WET PATH (main downward flow) -- cyan
    # =====================================================================
    draw.text((split_x - 110, split_y - 22), "WET", fill=CYAN, font=dm_bold_22)

    wet_arrow_start = split_y + dot_r
    ig_y = split_y + 55
    ig_x = split_x - 100
    ig_w = 200
    ig_h = 48
    draw_arrow_down(split_x, wet_arrow_start, ig_y, color=CYAN, width=3)
    draw_block(ig_x, ig_y, ig_w, ig_h, "Input Gain",
               border=CYAN_DIM, font=dm_semi_20)

    # ---- Dual Delay Lines ----
    dd_y = ig_y + ig_h + 40
    dd_x = split_x - 140
    dd_w = 280
    dd_h = 50
    draw_arrow_down(split_x, ig_y + ig_h, dd_y, color=CYAN, width=3)
    draw_block(dd_x, dd_y, dd_w, dd_h, "Dual Delay Lines",
               border=CYAN_DIM, font=dm_semi_20,
               sublabel="L / R circular buffers", sublabel_font=jb_reg_13)

    # =====================================================================
    #  Three branches: Per-Tap, Spatialization, Feedback
    # =====================================================================
    branch_y = dd_y + dd_h + 45
    branch_dot_y = dd_y + dd_h + 25
    draw_arrow_down(split_x, dd_y + dd_h, branch_dot_y, color=CYAN, width=3)

    draw.ellipse([split_x - dot_r, branch_dot_y - dot_r,
                  split_x + dot_r, branch_dot_y + dot_r], fill=CYAN)

    tap_cx = 480
    spat_cx = split_x
    fb_cx = 1920

    draw.line([(tap_cx, branch_dot_y), (fb_cx, branch_dot_y)],
              fill=CYAN, width=3)

    draw_arrow_down(tap_cx, branch_dot_y, branch_y, color=CYAN, width=3)
    draw_arrow_down(spat_cx, branch_dot_y, branch_y, color=CYAN, width=3)
    draw_arrow_down(fb_cx, branch_dot_y, branch_y, color=VIOLET, width=3)

    # =====================================================================
    #  Per-Tap Processing (left column)
    # =====================================================================
    tap_panel_x = 120
    tap_panel_y = branch_y
    tap_panel_w = 720
    tap_panel_h = 430

    rounded_rect(tap_panel_x, tap_panel_y, tap_panel_w, tap_panel_h, 16,
                 (14, 14, 28, 200), CYAN_DIM, 2)

    draw.text((tap_panel_x + 24, tap_panel_y + 14),
              "Per-Tap Processing", fill=CYAN, font=dm_bold_22)
    draw.text((tap_panel_x + 24, tap_panel_y + 42),
              "x12 taps per delay line", fill=DIM, font=jb_reg_13)

    tap_blocks = [
        ("Read from Delay Line", "tap position + interpolation"),
        ("Phase Vocoder Pitch Shift", "STFT 2048pt, 4x overlap, +/-12 st"),
        ("Doppler Effect", "velocity-based pitch modulation"),
        ("Air Absorption", "distance-dependent HF rolloff"),
        ("HP / LP Filters", "per-tap frequency shaping"),
    ]

    tb_x = tap_panel_x + 40
    tb_w = tap_panel_w - 80
    tb_h = 56
    tb_start_y = tap_panel_y + 68
    tb_gap = 12

    for i, (label, sub) in enumerate(tap_blocks):
        by = tb_start_y + i * (tb_h + tb_gap)
        draw_block(tb_x, by, tb_w, tb_h, label,
                   fill=(20, 20, 38), border=CYAN_DIM,
                   font=dm_med_18, sublabel=sub, sublabel_font=jb_reg_12,
                   sublabel_color=DIM, border_width=1, radius=8)
        if i < len(tap_blocks) - 1:
            draw_arrow_down(tb_x + tb_w // 2, by + tb_h,
                            by + tb_h + tb_gap, color=CYAN, width=2,
                            head_size=7)

    # =====================================================================
    #  Spatialization Engine (center column)
    # =====================================================================
    spat_panel_x = 880
    spat_panel_y = branch_y
    spat_panel_w = 640
    spat_panel_h = 430

    rounded_rect(spat_panel_x, spat_panel_y, spat_panel_w, spat_panel_h, 16,
                 (14, 14, 28, 200), CYAN_DIM, 2)

    draw.text((spat_panel_x + 24, spat_panel_y + 14),
              "Spatialization Engine", fill=CYAN, font=dm_bold_22)
    draw.text((spat_panel_x + 24, spat_panel_y + 42),
              "output format routing", fill=DIM, font=jb_reg_13)

    spat_methods_left = ["VBAP", "VBIP", "MDAP", "KNN"]
    spat_methods_right = ["DBAP", "Ambisonics", "HRTF Convolution", "Simple Stereo"]

    sm_x = spat_panel_x + 30
    sm_w = 270
    sm_h = 44
    sm_start_y = spat_panel_y + 72
    sm_gap = 10

    for i, label in enumerate(spat_methods_left):
        by = sm_start_y + i * (sm_h + sm_gap)
        draw_block(sm_x, by, sm_w, sm_h, label,
                   fill=(20, 20, 38), border=CYAN_DIM,
                   font=dm_med_18, border_width=1, radius=8)

    sm_x2 = spat_panel_x + 340
    for i, label in enumerate(spat_methods_right):
        by = sm_start_y + i * (sm_h + sm_gap)
        bc = CYAN_DIM
        if label == "HRTF Convolution":
            bc = (60, 90, 110)
        draw_block(sm_x2, by, sm_w, sm_h, label,
                   fill=(20, 20, 38), border=bc,
                   font=dm_med_18, border_width=1, radius=8)

    hrtf_note_y = sm_start_y + 2 * (sm_h + sm_gap) + sm_h + 6
    draw.text((sm_x2 + 10, hrtf_note_y),
              "PartitionedConvolver + ITD", fill=DIM, font=jb_reg_12)

    profiles_y = hrtf_note_y + 18
    draw.text((sm_x2 + 10, profiles_y),
              "6 HRTF profiles available", fill=(70, 75, 90), font=jb_reg_12)

    sel_y = spat_panel_y + spat_panel_h - 48
    draw_block(spat_panel_x + 30, sel_y, spat_panel_w - 60, 36,
               "Output: 1-49 channels  |  Stereo  |  Binaural  |  Ambisonics",
               fill=(22, 22, 40), border=(40, 45, 60), font=jb_reg_13,
               text_color=DIM, border_width=1, radius=6)

    # =====================================================================
    #  Feedback Path (right column) -- violet
    # =====================================================================
    fb_panel_x = 1560
    fb_panel_y = branch_y
    fb_panel_w = 440
    fb_panel_h = 430

    rounded_rect(fb_panel_x, fb_panel_y, fb_panel_w, fb_panel_h, 16,
                 (18, 14, 32, 200), VIOLET_DIM, 2)

    draw.text((fb_panel_x + 24, fb_panel_y + 14),
              "Feedback Path", fill=VIOLET, font=dm_bold_22)
    draw.text((fb_panel_x + 24, fb_panel_y + 42),
              "recirculation loop", fill=DIM, font=jb_reg_13)

    fb_blocks = [
        ("Soft Clipper", "tanh saturation"),
        ("HP / LP Filters", "feedback frequency shaping"),
        ("Feedback Gain", "0-100% recirculation"),
    ]

    fb_bx = fb_panel_x + 40
    fb_bw = fb_panel_w - 80
    fb_bh = 56
    fb_start_y = fb_panel_y + 72
    fb_gap = 16

    for i, (label, sub) in enumerate(fb_blocks):
        by = fb_start_y + i * (fb_bh + fb_gap)
        draw_block(fb_bx, by, fb_bw, fb_bh, label,
                   fill=(22, 18, 38), border=VIOLET_DIM,
                   font=dm_med_18, sublabel=sub, sublabel_font=jb_reg_12,
                   sublabel_color=(100, 80, 140), border_width=1, radius=8)
        if i < len(fb_blocks) - 1:
            draw_arrow_down(fb_bx + fb_bw // 2, by + fb_bh,
                            by + fb_bh + fb_gap, color=VIOLET, width=2,
                            head_size=7)

    # Feedback return arrow back up to delay lines
    fb_last_y = fb_start_y + 2 * (fb_bh + fb_gap) + fb_bh
    fb_return_x = fb_panel_x + fb_panel_w + 20
    fb_bottom_y = fb_panel_y + fb_panel_h + 15

    draw.line([(fb_bx + fb_bw // 2, fb_last_y),
               (fb_bx + fb_bw // 2, fb_bottom_y)], fill=VIOLET, width=2)
    draw.line([(fb_bx + fb_bw // 2, fb_bottom_y),
               (fb_return_x, fb_bottom_y)], fill=VIOLET, width=2)

    fb_return_top = dd_y + dd_h // 2
    draw.line([(fb_return_x, fb_bottom_y),
               (fb_return_x, fb_return_top)], fill=VIOLET, width=2)

    dd_right = dd_x + dd_w
    draw.line([(fb_return_x, fb_return_top),
               (dd_right + 12, fb_return_top)], fill=VIOLET, width=2)

    # Arrowhead pointing left
    draw.polygon([
        (dd_right, fb_return_top),
        (dd_right + 12, fb_return_top - 6),
        (dd_right + 12, fb_return_top + 6),
    ], fill=VIOLET)

    draw.text((fb_return_x + 8, fb_return_top - 22),
              "back to delay input", fill=(100, 80, 140), font=jb_reg_12)

    # =====================================================================
    #  Arrow: Per-Tap output -> Spatialization input
    # =====================================================================
    tap_out_x = tap_panel_x + tap_panel_w
    merge_y = spat_panel_y + spat_panel_h // 2

    draw.line([(tap_out_x, merge_y), (spat_panel_x, merge_y)],
              fill=CYAN, width=3)
    draw.polygon([
        (spat_panel_x, merge_y),
        (spat_panel_x - 10, merge_y - 6),
        (spat_panel_x - 10, merge_y + 6),
    ], fill=CYAN)
    draw.text((tap_out_x + 10, merge_y - 20), "mono sources",
              fill=DIM, font=jb_reg_13)

    # =====================================================================
    #  Below panels: convergence to crossfade
    # =====================================================================
    panels_bottom = max(tap_panel_y + tap_panel_h,
                        spat_panel_y + spat_panel_h,
                        fb_panel_y + fb_panel_h) + 50

    spat_out_y = spat_panel_y + spat_panel_h
    draw_arrow_down(spat_cx, spat_out_y, panels_bottom, color=CYAN, width=3)

    # ---- Equal-Power Crossfade ----
    xf_w = 440
    xf_h = 58
    xf_x = spat_cx - xf_w // 2
    xf_y = panels_bottom

    draw_block(xf_x, xf_y, xf_w, xf_h, "Equal-Power Crossfade",
               border=CYAN, text_color=WHITE, font=dm_bold_22,
               sublabel="cos/sin  |  dry / wet mix",
               sublabel_font=jb_reg_13, sublabel_color=DIM, border_width=2)

    # Dry path arrow down to crossfade
    dry_down_target = xf_y + xf_h // 2
    dry_join_x = xf_x + xf_w

    draw.line([(dry_exit_x, dry_exit_y), (dry_exit_x, dry_down_target)],
              fill=AMBER, width=3)
    draw.line([(dry_exit_x, dry_down_target),
               (dry_join_x + 12, dry_down_target)], fill=AMBER, width=3)
    draw.polygon([
        (dry_join_x, dry_down_target),
        (dry_join_x + 12, dry_down_target - 6),
        (dry_join_x + 12, dry_down_target + 6),
    ], fill=AMBER)

    # ---- Output Gain ----
    og_y = xf_y + xf_h + 40
    og_w = 200
    og_h = 48
    og_x = spat_cx - og_w // 2
    draw_arrow_down(spat_cx, xf_y + xf_h, og_y, color=CYAN, width=3)
    draw_block(og_x, og_y, og_w, og_h, "Output Gain",
               border=CYAN_DIM, font=dm_semi_20)

    # ---- OUTPUT ----
    out_y = og_y + og_h + 40
    out_w = 300
    out_h = 56
    out_x = spat_cx - out_w // 2
    draw_arrow_down(spat_cx, og_y + og_h, out_y, color=CYAN, width=3)
    draw_block(out_x, out_y, out_w, out_h, "OUTPUT",
               border=CYAN, text_color=CYAN, font=dm_bold_22,
               sublabel="1-49 channels", sublabel_font=jb_reg_13,
               sublabel_color=DIM)

    # =====================================================================
    #  Phase Vocoder annotation
    # =====================================================================
    pv_ann_x = 120
    pv_ann_y = branch_y + tap_panel_h + 50
    ann_w = 500
    ann_h = 85

    rounded_rect(pv_ann_x, pv_ann_y, ann_w, ann_h, 8,
                 (14, 14, 28, 180), (40, 45, 60), 1)
    draw.text((pv_ann_x + 14, pv_ann_y + 10),
              "Phase Vocoder Details", fill=CYAN, font=dm_med_16)
    draw.text((pv_ann_x + 14, pv_ann_y + 32),
              "Laroche-Dolson phase locking", fill=DIM, font=jb_reg_12)
    draw.text((pv_ann_x + 14, pv_ann_y + 48),
              "Robel spectral flux transient detection",
              fill=DIM, font=jb_reg_12)
    draw.text((pv_ann_x + 14, pv_ann_y + 64),
              "Latency: 2048 samples (reported to DAW)",
              fill=DIM, font=jb_reg_12)

    # =====================================================================
    #  Tape Wobble annotation
    # =====================================================================
    tw_ann_x = 660
    tw_ann_y = pv_ann_y
    tw_w = 380
    tw_h = 85

    rounded_rect(tw_ann_x, tw_ann_y, tw_w, tw_h, 8,
                 (14, 14, 28, 180), (40, 45, 60), 1)
    draw.text((tw_ann_x + 14, tw_ann_y + 10),
              "Tape Wobble Emulation", fill=AMBER, font=dm_med_16)
    draw.text((tw_ann_x + 14, tw_ann_y + 32),
              "4-layer modulation (wow + flutter)",
              fill=DIM, font=jb_reg_12)
    draw.text((tw_ann_x + 14, tw_ann_y + 48),
              "Spectral envelope EMA smoothing",
              fill=DIM, font=jb_reg_12)
    draw.text((tw_ann_x + 14, tw_ann_y + 64),
              "PartitionedConvolver crossfade",
              fill=DIM, font=jb_reg_12)

    # =====================================================================
    #  Footer
    # =====================================================================
    draw.text((40, H - 40), "OpenSpatialDelay  |  Spatial Media Lab",
              fill=(40, 42, 55), font=jb_reg_13)
    draw.text((W - 340, H - 40), "github.com/Spatial-Media-Lab",
              fill=(40, 42, 55), font=jb_reg_13)

    # =====================================================================
    #  Save
    # =====================================================================
    img_rgb = Image.new("RGB", (W, H), BG)
    img_rgb.paste(img, (0, 0), img)
    img_rgb.save(OUT_PATH, "PNG", quality=95)

    print(f"Signal flow diagram saved: {OUT_PATH}")
    print(f"Size: {os.path.getsize(OUT_PATH) / 1024:.1f} KB")


if __name__ == "__main__":
    main()
