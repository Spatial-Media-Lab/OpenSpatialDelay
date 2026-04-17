#!/usr/bin/env bash
# phase5-smoke.sh — one-command URL smoke for OpenSpatialDelay v1.0.0 launch day.
# Runs curl -fsIL (silent, follow redirects, fail on non-2xx) against every
# launch-day URL. Used at T-1 (2026-04-27) and T-0 (2026-04-28).
# Usage: bash scripts/phase5-smoke.sh [--help]
# Env:   SML_BLOG_POST_URL, KVR_URL (optional; appended to probe list)
# Exit:  0 = all hard URLs pass; 1 = any hard URL fails.

set -euo pipefail
[[ "${1:-}" == "--help" || "${1:-}" == "-h" ]] && { sed -n '2,7p' "$0" | sed 's/^# \{0,1\}//'; exit 0; }
if [[ -t 1 ]]; then G=$'\033[0;32m'; R=$'\033[0;31m'; Y=$'\033[0;33m'; N=$'\033[0m'; else G=""; R=""; Y=""; N=""; fi

HARD_URLS=(
  "https://spatialmedialab.org/"
  "https://andrewrahman.com/get-osd"
  "https://andrewrahman.com/privacy"
  "https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0"
  "https://patreon.com/AndrewRahman"
)
OPT=()
[[ -n "${SML_BLOG_POST_URL:-}" ]] && OPT+=("$SML_BLOG_POST_URL") || echo "${Y}skipped: SML_BLOG_POST_URL not set — fill in T-1${N}"
[[ -n "${KVR_URL:-}" ]] && OPT+=("$KVR_URL") || echo "${Y}skipped: KVR_URL not set — fill in T-1${N}"

check() {
  local url="$1" hard="$2" code
  # HEAD first (curl -fsIL); GET-range fallback for hosts that reject HEAD.
  if code=$(curl -fsIL --max-time 10 -o /dev/null -w "%{http_code}" "$url" 2>/dev/null); then
    echo "${G}PASS  ${url}  (HTTP ${code})${N}"; return 0
  elif code=$(curl -fsL --max-time 10 -r 0-0 -o /dev/null -w "%{http_code}" "$url" 2>/dev/null); then
    echo "${G}PASS  ${url}  (HTTP ${code}; GET fallback)${N}"; return 0
  fi
  [[ "$hard" == "hard" ]] && echo "${R}FAIL  ${url}  (curl failed or non-2xx)${N}" || echo "${Y}WARN  ${url}  (optional; non-2xx)${N}"
  return 1
}

pass=0; fail=0; total=${#HARD_URLS[@]}
for u in "${HARD_URLS[@]}"; do check "$u" hard && pass=$((pass+1)) || fail=$((fail+1)); done
for u in "${OPT[@]+"${OPT[@]}"}"; do check "$u" optional || true; done

echo
echo "${pass} of ${total} hard URLs passed"
[[ $fail -gt 0 ]] && exit 1 || exit 0
