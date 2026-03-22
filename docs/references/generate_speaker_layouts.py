#!/usr/bin/env python3
"""Generate speaker layout diagrams for OpenSpatialDelay surround formats.

Outputs a large PNG with 14 layouts in a 4-column grid, using the plugin's
dark color scheme.

Convention: positive azimuth = LEFT (ADM/IEM).
Plot orientation: 0 deg at top (front), +90 deg left, -90 deg right, 180 deg bottom.
"""

import math
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

# ── Colors (plugin theme) ───────────────────────────────────────────────
BG       = '#0A0A14'
CYAN     = '#00D4FF'   # ear-level speakers
PURPLE   = '#8B5CF6'   # height speakers
RED      = '#FF6B6B'   # LFE
TEXT     = '#E0E0E0'
DIM_TEXT = '#606080'
GRID_CLR = '#1E1E38'
FRONT_CLR = '#F5C542'  # amber accent for "F" indicator

# ── Layout data ─────────────────────────────────────────────────────────
layouts = [
    ("Quad", 4, None, [
        ("FL",  +45, 0, 1), ("FR",  -45, 0, 2),
        ("RL", +135, 0, 3), ("RR", -135, 0, 4),
    ]),
    ("5.0", 5, None, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls", +110, 0, 4), ("Rs", -110, 0, 5),
    ]),
    ("5.1", 6, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls", +110, 0, 5), ("Rs", -110, 0, 6),
    ]),
    ("7.0", 7, None, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 4), ("Rs",  -90, 0, 5),
        ("Lrs", +135, 0, 6), ("Rrs", -135, 0, 7),
    ]),
    ("7.1", 8, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 5), ("Rs",  -90, 0, 6),
        ("Lrs", +135, 0, 7), ("Rrs", -135, 0, 8),
    ]),
    ("5.1.2", 8, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls", +110, 0, 5), ("Rs", -110, 0, 6),
        ("Tsl", +90, +45, 7), ("Tsr", -90, +45, 8),
    ]),
    ("5.1.4", 10, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls", +110, 0, 5), ("Rs", -110, 0, 6),
        ("Tfl", +45, +45, 7), ("Tfr", -45, +45, 8),
        ("Trl", +135, +45, 9), ("Trr", -135, +45, 10),
    ]),
    ("7.0.2", 9, None, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 4), ("Rs",  -90, 0, 5),
        ("Lrs", +135, 0, 6), ("Rrs", -135, 0, 7),
        ("Tsl", +90, +45, 8), ("Tsr", -90, +45, 9),
    ]),
    ("7.1.2", 10, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 5), ("Rs",  -90, 0, 6),
        ("Lrs", +135, 0, 7), ("Rrs", -135, 0, 8),
        ("Tsl", +90, +45, 9), ("Tsr", -90, +45, 10),
    ]),
    ("7.1.4", 12, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 5), ("Rs",  -90, 0, 6),
        ("Lrs", +135, 0, 7), ("Rrs", -135, 0, 8),
        ("Tfl", +45, +45, 9), ("Tfr", -45, +45, 10),
        ("Trl", +135, +45, 11), ("Trr", -135, +45, 12),
    ]),
    ("7.1.6", 14, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 5), ("Rs",  -90, 0, 6),
        ("Lrs", +135, 0, 7), ("Rrs", -135, 0, 8),
        ("Tfl", +45, +45, 9), ("Tfr", -45, +45, 10),
        ("Trl", +135, +45, 11), ("Trr", -135, +45, 12),
        ("Tsl", +90, +45, 13), ("Tsr", -90, +45, 14),
    ]),
    ("9.1.6", 16, 4, [
        ("L",  +30, 0, 1), ("R",  -30, 0, 2), ("C", 0, 0, 3),
        ("Ls",  +90, 0, 5), ("Rs",  -90, 0, 6),
        ("Lrs", +135, 0, 7), ("Rrs", -135, 0, 8),
        ("Lw",  +60, 0, 9), ("Rw",  -60, 0, 10),
        ("Tfl", +45, +45, 11), ("Tfr", -45, +45, 12),
        ("Tsl", +90, +45, 13), ("Tsr", -90, +45, 14),
        ("Trl", +135, +45, 15), ("Trr", -135, +45, 16),
    ]),
    ("Octaphonic", 8, None, [
        ("C",  0, 0, 1), ("FR", -45, 0, 2), ("R", -90, 0, 3),
        ("RR", -135, 0, 4), ("RC", 180, 0, 5),
        ("RL", +135, 0, 6), ("L", +90, 0, 7), ("FL", +45, 0, 8),
    ]),
    ("SML13.1", 14, 14, [
        ("FC",  0, 0, 1), ("FR", -45, 0, 2), ("R", -90, 0, 3),
        ("RR", -135, 0, 4), ("RC", 180, 0, 5),
        ("RL", +135, 0, 6), ("L", +90, 0, 7), ("FL", +45, 0, 8),
        ("UFR", -45, +45, 9), ("URR", -135, +45, 10),
        ("URL", +135, +45, 11), ("UFL", +45, +45, 12),
        ("T", 0, +90, 13),
    ]),
]

# ── Helpers ─────────────────────────────────────────────────────────────

EAR_RADIUS = 1.0
HEIGHT_RADIUS = 0.55

def azim_to_plot_angle_rad(az_deg):
    """Convert ADM azimuth (positive=left) to matplotlib polar angle."""
    return math.pi / 2 + math.radians(az_deg)


def get_speaker_radius(el_deg):
    if el_deg >= 90:
        return 0.0
    elif el_deg > 0:
        return HEIGHT_RADIUS
    else:
        return EAR_RADIUS


def normalize_az(az):
    while az > 180: az -= 360
    while az < -180: az += 360
    return az


def get_outward_alignment(norm_az):
    """Get ha, va for a label placed outward from a speaker at this azimuth."""
    if abs(norm_az) < 12:
        return 'center', 'bottom'
    elif abs(abs(norm_az) - 180) < 12:
        return 'center', 'top'
    elif norm_az > 0:  # left side
        if norm_az < 55:
            return 'right', 'bottom'
        elif norm_az < 105:
            return 'right', 'center'
        else:
            return 'right', 'top'
    else:  # right side
        if norm_az > -55:
            return 'left', 'bottom'
        elif norm_az > -105:
            return 'left', 'center'
        else:
            return 'left', 'top'


def get_inward_alignment(norm_az):
    """Get ha, va for a label placed inward from a speaker at this azimuth."""
    if abs(norm_az) < 12:
        return 'center', 'top'
    elif abs(abs(norm_az) - 180) < 12:
        return 'center', 'bottom'
    elif norm_az > 0:  # left side
        return 'left', 'center'
    else:  # right side
        return 'right', 'center'


def compute_label_positions(speakers):
    """Compute label positions for all speakers, handling collisions.

    Strategy:
    - Ear-level speakers: always label outward
    - Height speakers with no ear-level neighbor at same azimuth: label outward
    - Height speakers WITH ear-level neighbor: label inward (toward center)
    - Top speaker (zenith): label offset to side
    """
    results = []

    for i, (name, az, el, ch) in enumerate(speakers):
        spk_angle = azim_to_plot_angle_rad(az)
        norm_az = normalize_az(az)
        is_top = (el >= 90)
        is_height = (el > 0 and el < 90)
        is_ear = (el == 0)

        # Check for radial neighbor (different elevation, similar azimuth)
        has_radial_neighbor = False
        for j, (on, oa, oe, oc) in enumerate(speakers):
            if i == j:
                continue
            if abs(az - oa) < 20:
                if (is_height and oe == 0) or (is_ear and 0 < oe < 90):
                    has_radial_neighbor = True
                    break

        if is_top:
            # Top/zenith speaker - label to the upper-left
            results.append((0.30, spk_angle + math.pi/4, 'left', 'center'))

        elif is_height and has_radial_neighbor:
            # Height speaker with ear-level neighbor: label INWARD
            label_r = HEIGHT_RADIUS - 0.24
            if label_r < 0.15:
                label_r = 0.15
            ha, va = get_inward_alignment(norm_az)
            results.append((label_r, spk_angle, ha, va))

        elif is_height and not has_radial_neighbor:
            # Height speaker, no collision: label outward
            label_r = HEIGHT_RADIUS + 0.30
            ha, va = get_outward_alignment(norm_az)
            results.append((label_r, spk_angle, ha, va))

        elif is_ear and has_radial_neighbor:
            # Ear-level with height neighbor: label outward (extra distance)
            label_r = EAR_RADIUS + 0.38
            ha, va = get_outward_alignment(norm_az)
            results.append((label_r, spk_angle, ha, va))

        else:
            # Ear-level, no collision: standard outward
            label_r = EAR_RADIUS + 0.32
            ha, va = get_outward_alignment(norm_az)
            results.append((label_r, spk_angle, ha, va))

    return results


def draw_layout(ax, title, total_ch, lfe_ch, speakers):
    """Draw one speaker layout on the given polar axes."""
    ax.set_facecolor(BG)
    ax.set_ylim(0, 1.85)
    ax.set_yticks([])
    ax.set_xticks([])
    ax.grid(False)
    ax.spines['polar'].set_visible(False)

    theta_circle = np.linspace(0, 2 * math.pi, 300)

    # Outer ring (ear level)
    ax.plot(theta_circle, np.ones_like(theta_circle) * EAR_RADIUS,
            color=GRID_CLR, linewidth=2.0, zorder=1)

    # Inner ring (height layer)
    has_height = any(s[2] > 0 for s in speakers)
    if has_height:
        ax.plot(theta_circle, np.ones_like(theta_circle) * HEIGHT_RADIUS,
                color=GRID_CLR, linewidth=1.0, linestyle=(0, (5, 5)), zorder=1)

    # Front indicator
    front_angle = math.pi / 2
    ax.annotate('F', xy=(front_angle, 1.38), fontsize=15, fontweight='bold',
                color=FRONT_CLR, ha='center', va='center', zorder=10)
    tri_a = [front_angle - 0.055, front_angle, front_angle + 0.055]
    tri_r = [1.47, 1.40, 1.47]
    ax.fill(tri_a, tri_r, color=FRONT_CLR, alpha=0.5, zorder=9)

    # Listener cross at center
    for da in [0, math.pi/2, math.pi, 3*math.pi/2]:
        ax.plot([da, da], [0, 0.045], color=DIM_TEXT, linewidth=0.7, zorder=3)

    # LFE indicator
    if lfe_ch is not None:
        ax.annotate(f'LFE  Ch {lfe_ch}', xy=(math.pi * 1.32, 1.70),
                    fontsize=10, fontweight='bold', color=RED,
                    ha='center', va='center', zorder=10,
                    bbox=dict(boxstyle='round,pad=0.3', facecolor=RED,
                              alpha=0.12, edgecolor=RED, linewidth=1.0))

    # Subtle radial lines
    for (name, az, el, ch) in speakers:
        if el >= 90:
            continue
        angle = azim_to_plot_angle_rad(az)
        radius = get_speaker_radius(el)
        ax.plot([angle, angle], [0.06, radius - 0.06], color=GRID_CLR,
                linewidth=0.35, alpha=0.35, zorder=1)

    # Compute labels
    label_positions = compute_label_positions(speakers)

    # Draw speakers and labels
    for idx, (name, az, el, ch) in enumerate(speakers):
        angle = azim_to_plot_angle_rad(az)
        radius = get_speaker_radius(el)
        is_height = (el > 0 and el < 90)
        is_top = (el >= 90)

        if is_top:
            color = PURPLE
            ms = 14
        elif is_height:
            color = PURPLE
            ms = 11
        else:
            color = CYAN
            ms = 13

        # Glow
        ax.plot(angle, radius, 'o', color=color, markersize=ms + 5,
                alpha=0.12, zorder=7)
        # Dot
        ax.plot(angle, radius, 'o', color=color, markersize=ms,
                markeredgecolor='white', markeredgewidth=0.8, zorder=8)

        # Label
        lr, la, ha, va = label_positions[idx]
        label_text = f"{name}\nCh {ch}"

        ax.annotate(label_text, xy=(la, lr),
                    fontsize=10.5 if len(speakers) <= 10 else 9.5,
                    color=color, fontweight='bold',
                    ha=ha, va=va, zorder=10,
                    linespacing=1.15)

    # Title
    ch_str = f"{total_ch}ch"
    if lfe_ch is not None:
        ch_str += f"  (LFE = Ch {lfe_ch})"
    ax.set_title(f"{title}\n{ch_str}", fontsize=16, fontweight='bold',
                 color=TEXT, pad=30)


# ── Main figure ─────────────────────────────────────────────────────────

n_layouts = len(layouts)
n_cols = 4
n_rows = math.ceil(n_layouts / n_cols)

fig = plt.figure(figsize=(30, 38), facecolor=BG, dpi=100)

# Super title
fig.suptitle('OpenSpatialDelay \u2014 Speaker Layouts',
             fontsize=30, fontweight='bold', color=TEXT, y=0.982)

# Legend
legend_y = 0.968
fig.text(0.30, legend_y, '\u25cf  Ear Level (elev 0\u00b0)', fontsize=15,
         color=CYAN, fontweight='bold', ha='center', va='center')
fig.text(0.50, legend_y, '\u25cf  Height (elev +45\u00b0 / +90\u00b0)', fontsize=15,
         color=PURPLE, fontweight='bold', ha='center', va='center')
fig.text(0.70, legend_y, '\u25cf  LFE (low-frequency)', fontsize=15,
         color=RED, fontweight='bold', ha='center', va='center')

# Convention note
fig.text(0.50, 0.956,
         '+azimuth = LEFT (ADM/IEM)  \u2502  0\u00b0 = Front  \u2502  Channels 1-indexed  \u2502  SMPTE channel ordering',
         fontsize=12.5, color=DIM_TEXT, ha='center', va='center')

for idx, (title, total_ch, lfe_ch, speakers) in enumerate(layouts):
    ax = fig.add_subplot(n_rows, n_cols, idx + 1, polar=True)
    draw_layout(ax, title, total_ch, lfe_ch, speakers)

# Hide unused subplots
for idx in range(n_layouts, n_rows * n_cols):
    ax = fig.add_subplot(n_rows, n_cols, idx + 1)
    ax.set_visible(False)

plt.subplots_adjust(left=0.04, right=0.96, top=0.94, bottom=0.01,
                    hspace=0.30, wspace=0.30)

output_path = '/Users/andrewrahman/Desktop/CLAUDE/Project/OpenSpatialDelay/docs/references/speaker-layouts.png'
plt.savefig(output_path, dpi=100, facecolor=BG, edgecolor='none',
            bbox_inches='tight', pad_inches=0.5)
plt.close()

print(f"Saved to: {output_path}")

import os
size = os.path.getsize(output_path)
print(f"File size: {size:,} bytes ({size/1024/1024:.1f} MB)")

from PIL import Image
img = Image.open(output_path)
print(f"Dimensions: {img.width} x {img.height} px")
