#!/usr/bin/env bash
# Encode a plugin-capture .mov into web-ready WebM (AV1) + MP4 (H.264).
#
# Usage:
#   bash scripts/encode_plugin_video.sh <source.mov> <out-basename>
#
# Example:
#   bash scripts/encode_plugin_video.sh \
#     ~/Desktop/merry-go-round.mov docs/assets/hero-plugin
#   # → docs/assets/hero-plugin.webm
#   # → docs/assets/hero-plugin.mp4
#
# Pipeline provenance: docs/SCREENSHOT_REVIEW_ROUND_3.md items 18–21
# (2026-04-20). Hero is 1640×1160 to match andrewrahman-com reserved
# hero box; scale only runs when source dimensions differ.
#
# Invariants (each learned the hard way):
#   - -fps_mode passthrough preserves source VFR. CFR resampling
#     duplicates frames and creates visible stutter at the loop seam.
#   - No audio track (-an). Audio is unused and adds weight.
#   - yuv420p for universal browser compatibility.
#   - +faststart writes the moov atom at the head for streaming.
#   - H.264 level is auto-selected (typically 5.1). Source
#     r_frame_rate tag can trip level-4.0's MB-rate cap even on
#     ~30 fps content; 5.1 is universally supported.
#
# Budgets (enforced by user spec in items 18–21):
#   - WebM (AV1): ≤2 MB
#   - MP4  (H.264): ≤4 MB
# Current 12–15 s captures at 1640×1160 land ~300–600 KB with these
# settings. If you exceed budget, bump CRF by 2–4 and re-encode.

set -euo pipefail

SRC="${1:-}"
OUT="${2:-}"

if [[ -z "$SRC" || -z "$OUT" ]]; then
  echo "usage: $0 <source.mov> <out-basename>" >&2
  echo "example: $0 ~/Desktop/orbit-dance.mov docs/assets/orbit-dance" >&2
  exit 2
fi

if [[ ! -f "$SRC" ]]; then
  echo "error: source not found: $SRC" >&2
  exit 1
fi

for bin in ffmpeg ffprobe; do
  if ! command -v "$bin" >/dev/null 2>&1; then
    echo "error: $bin not on PATH (try: brew install ffmpeg)" >&2
    exit 1
  fi
done

TARGET_W=1640
TARGET_H=1160

IFS=x read -r SRC_W SRC_H < <(ffprobe -v error -select_streams v:0 \
  -show_entries stream=width,height -of csv=p=0:s=x "$SRC")

VF=()
if [[ "$SRC_W" == "$TARGET_W" && "$SRC_H" == "$TARGET_H" ]]; then
  echo "Source already ${TARGET_W}×${TARGET_H}; encoding directly (no scale)."
else
  VF=(-vf "scale=${TARGET_W}:${TARGET_H}:flags=lanczos")
  echo "Source ${SRC_W}×${SRC_H} → scaling to ${TARGET_W}×${TARGET_H} (lanczos)."
fi

mkdir -p "$(dirname "$OUT")"

echo "Encoding AV1 → ${OUT}.webm"
ffmpeg -y -i "$SRC" "${VF[@]}" \
  -c:v libsvtav1 -preset 6 -crf 32 \
  -pix_fmt yuv420p -an -fps_mode passthrough \
  -movflags +faststart \
  "${OUT}.webm"

echo "Encoding H.264 → ${OUT}.mp4"
ffmpeg -y -i "$SRC" "${VF[@]}" \
  -c:v libx264 -preset slow -crf 23 -profile:v high \
  -pix_fmt yuv420p -an -fps_mode passthrough \
  -movflags +faststart \
  "${OUT}.mp4"

echo ""
echo "Artefacts (budget: webm ≤2 MB, mp4 ≤4 MB):"
for f in "${OUT}.webm" "${OUT}.mp4"; do
  size_bytes=$(stat -f%z "$f" 2>/dev/null || stat -c%s "$f")
  size_kb=$(( size_bytes / 1024 ))
  printf "  %6d KB  %s\n" "$size_kb" "$f"
done
