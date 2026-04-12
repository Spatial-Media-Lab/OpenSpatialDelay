# OpenSpatialDelay v1.0.0 — Release Plan

**Date:** 2026-04-07
**Target ship:** Week of 2026-04-10
**Repository:** `Spatial-Media-Lab/OpenSpatialDelay` (currently private)

---

## Current State

- **macOS build:** Working (AU + VST3, arm64, ad-hoc signed)
- **Windows CI:** Blocked (GitHub Actions billing — free minutes exhausted on private repo)
- **Tests:** 291 Catch2 tests, 110,205 assertions — all passing (1 pre-existing bus layout test excluded)
- **Open issues:** 1 (#59 — refactor, explicitly post-v1.0 scope)
- **Closed bugs:** 30+ since v0.9
- **Manual:** PDF + DOCX generated, update plan marked COMPLETED
- **Legal notices:** Generated (DOCX)
- **License:** Dual GPL-3.0 / Commercial, LICENSE file at repo root
- **Presets:** 70 factory presets compiled into binary
- **Notion test DB:** Populated with manual test items (sections A-H)

### Deferred Features (not shipping in v1.0.0)

| Feature | Disposition |
|---------|-------------|
| Custom SOFA import | Deferred indefinitely |
| AAX format | Deferred post-v1.0 |
| Code signing / notarization | Deferred post-v1.0 |
| Refactor (#59) | Post-v1.0 |

---

## Release Sequence

### Phase 1: Pre-Release Testing (in studio)

**Owner:** Andrew (manual listening tests require studio monitoring)

1. Run full release test checklist (`docs/RELEASE_TEST_CHECKLIST.md`)
   - All 23 output formats (A1–A23), especially Ambisonics verification
   - All 6 HRTF profiles (B1–B10)
   - All 14 trajectory shapes (C1–C18)
   - 70 factory preset cycle (D1–D14)
   - DAW integration in REAPER (E1–E19)
   - Regression checks for closed bugs (F1–F12)
   - Edge cases and stress tests (G1–G13)
   - Stereo input routing (H1)
2. Log results in Notion DB (`OSD v1.0 Release Tests`)
3. File any new bugs as GitHub issues; fix before proceeding

### Phase 2: Final Build + Documentation Verification

1. Run automated tests one final time:
   ```bash
   cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)
   ./build/OpenSpatialDelayTests
   ```
2. Build final macOS Release:
   ```bash
   cmake --build build --config Release
   ```
3. Verify AU + VST3 install to `~/Library/Audio/Plug-Ins/`
4. Verify in REAPER: load plugin, cycle 3 presets, confirm audio
5. Verify user manual PDF opens and TOC links work
6. Verify LICENSE file at repo root has dual-license header
7. Verify legal notices doc generated (`docs/OpenSpatialDelay_Legal_Notices.docx`)
8. Ensure `docs/RELEASE_TEST_CHECKLIST.md` counts match source code (23 formats, 8 algos, 70 presets, 14 trajectories)

### Phase 3: Make Repo Public + Trigger Windows CI

Making the repo public unblocks the Windows CI build (public repos have unlimited GitHub Actions minutes on the Free plan).

1. Verify no secrets, credentials, or private keys in the repo:
   ```bash
   git log --all --diff-filter=A --name-only | grep -iE '\.env|secret|key|credential|token|password' || echo "Clean"
   ```
2. Verify `.gitignore` excludes sensitive files
3. Make repo public:
   ```bash
   gh repo edit Spatial-Media-Lab/OpenSpatialDelay --visibility public
   ```
4. Trigger Windows CI build:
   ```bash
   gh workflow run build-windows.yml --repo Spatial-Media-Lab/OpenSpatialDelay
   ```
5. Wait for Windows CI to complete:
   ```bash
   gh run list --repo Spatial-Media-Lab/OpenSpatialDelay --workflow=build-windows.yml --limit=1 --watch
   ```
6. Download Windows build:
   ```bash
   bash scripts/download_windows_build.sh
   ```

### Phase 4: Create GitHub Release

1. Tag the release:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
2. Prepare release artifacts:
   - `OpenSpatialDelay-v1.0.0-macOS-arm64.zip` — containing AU (.component) + VST3 (.vst3)
   - `OpenSpatialDelay-v1.0.0-Windows-x64.zip` — containing VST3
   - `OpenSpatialDelay_Manual_v1.0.pdf` — user manual
   - `SHA256SUMS.txt` — checksums for all artifacts
3. Create the release:
   ```bash
   gh release create v1.0.0 \
     --repo Spatial-Media-Lab/OpenSpatialDelay \
     --title "OpenSpatialDelay v1.0.0" \
     --notes-file docs/RELEASE_NOTES_v1.0.0.md \
     OpenSpatialDelay-v1.0.0-macOS-arm64.zip \
     OpenSpatialDelay-v1.0.0-Windows-x64.zip \
     docs/OpenSpatialDelay_Manual_v1.0.pdf \
     SHA256SUMS.txt
   ```
4. Verify: download each artifact from an incognito browser, confirm checksums match

### Phase 5: Windows Smoke Test

Since Windows CI can only run after the repo goes public, and there is no local Windows test machine:

1. Download Windows VST3 from the GitHub Release
2. If possible, test in a Windows VM or ask a collaborator to test:
   - Load in REAPER Windows → plugin opens, audio passes, presets cycle
   - Basic functionality smoke test (Notion test E19)
3. If no Windows access, note "Windows untested by maintainer" in release notes and monitor for community reports

---

## Documentation Consistency Checklist

| Document | Status | Notes |
|----------|--------|-------|
| `CLAUDE.md` | Current | v1.0.0, version registry, deferred features noted |
| `README.md` | Current | 23 formats, 8 algos, 70 presets, 14 trajectories — matches source |
| `SPECIFICATION.md` | Updated | Phase 5 features table corrected, deferred items marked, v1.0.0 tag |
| `docs/RELEASE_TEST_CHECKLIST.md` | Updated | 23 formats (9.1 added), HRTF user-facing names, test count corrected |
| `docs/VERSION_HISTORY.md` | Current | Up to date through pre-release audit |
| `docs/OpenSpatialDelay_Manual_v1.0.pdf` | Current | Manual update plan marked COMPLETED |
| `docs/OpenSpatialDelay_Legal_Notices.docx` | Current | All third-party licenses covered |
| `LICENSE` | Current | Dual GPL-3.0 / Commercial header |
| `docs/RELEASE_NOTES_v1.0.0.md` | Current | Created 2026-04-08, referenced by `gh release create` |

### Documentation Discrepancy — RESOLVED (2026-04-08)

The manual generator (`docs/generate_manual.js`) now correctly shows 23 formats, 8 spatialization approaches (7 user-selectable algorithms in the table + Direct Binaural HRTF in the intro text), and 70 factory presets. All counts match the source code. No action needed before regenerating the manual.

---

## New Automated Tests (Recommended)

Based on test coverage analysis, these gaps should be addressed before or shortly after release:

**High priority — COMPLETED (2026-04-08, `Tests/PreReleaseTests.cpp`):**
- ~~Stereo input routing tests (L+R, L, R per tap)~~ — 4 tests added
- ~~SML 13.1 and 9.1 surround format tests~~ — 7 tests added
- ~~Higher-order Ambisonics (HOA, 4OA–6OA) initialization tests~~ — 5 tests added

**Medium priority (post-release):**
- HRTF profile pair-wise switching stress test
- Feedback saturation at extremes with pitch shift
- Tempo sync transition tests
- LFE gain verification for all Atmos formats

---

## New Manual Tests Added to Notion

9 new entries added to `OSD v1.0 Release Tests` database:

| Test ID | Section | Test |
|---------|---------|------|
| A8 | Output Formats | 9.1 Surround — 10ch (ear-level, no height) |
| A17 | Output Formats | SML 13.1 — 14ch (custom multi-use room) |
| A20 | Output Formats | HOA 3rd order Ambisonics — 16ch ACN/SN3D |
| A24 | Output Formats | Ambisonics output with non-Ambisonics algorithm selected |
| B10 | HRTF Profiles | HRTF profile switching — all 6 profiles, no artifacts |
| E18 | DAW Integration | Tempo sync mode transitions |
| E19 | DAW Integration | Windows VST3 smoke test (post-build) |
| G13 | Edge Cases | Feedback + pitch shift stability at extremes |
| H1 | General | Per-tap stereo input routing (L+R / L / R) |

---

## Release Announcement Strategy

### Where to Post (ordered by priority)

1. **GitHub Release page** — Primary download location. Well-formatted release notes with features, system requirements, installation instructions, and Gatekeeper bypass.

2. **KVR Audio** — Submit product listing via [kvraudio.com/submissions](https://www.kvraudio.com/submissions) (free developer account). KVR is the single most important directory for audio plugins. Keep the description factual — they edit out marketing language.

3. **Gearspace "New Product Alert"** — Post on [gearspace.com/board/new-product-alert/](https://gearspace.com/board/new-product-alert/) on release day with a brief description, screenshot, and download link.

4. **Reddit** — Post to:
   - `r/AudioProductionDeals` — for free/open-source plugin announcements
   - `r/WeAreTheMusicMakers` — broader music production community
   - `r/SpatialAudio` — niche but highly relevant audience

5. **Audio blogs** — Reach out to Bedroom Producers Blog, Rekkerd.org, and Plugin Boutique blog. They regularly cover free/open-source releases.

6. **Social media** — X/Twitter announcement with screenshot + download link.

### Announcement Content to Prepare

- **Screenshot:** Already generated (`docs/assets/screenshot.png`)
- **Short description (1–2 sentences):** "OpenSpatialDelay is a free, open-source spatial delay plugin where each echo lives in 3D space. 12 taps, 23 output formats (binaural through 9.1.6 Atmos), 8 panning algorithms, 70 presets — VST3 + AU for macOS and Windows."
- **Demo video:** Record a 60–90 second screen capture showing the spatial map with active trajectories, preset cycling, and format switching. Post to YouTube.
- **Release notes:** See `docs/RELEASE_NOTES_v1.0.0.md` (to be created from this plan)

---

## Installation Instructions (for README / Release Notes)

### macOS

1. Download `OpenSpatialDelay-v1.0.0-macOS-arm64.zip` from [Releases](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases)
2. Unzip and copy:
   - `OpenSpatialDelay v1.0.component` → `~/Library/Audio/Plug-Ins/Components/`
   - `OpenSpatialDelay v1.0.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
3. Remove quarantine (required — not yet notarized):
   ```bash
   xattr -cr ~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay*.vst3
   xattr -cr ~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay*.component
   ```
4. Restart your DAW

### Windows

1. Download `OpenSpatialDelay-v1.0.0-Windows-x64.zip` from [Releases](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases)
2. Unzip and copy `OpenSpatialDelay v1.0.vst3` → `C:\Program Files\Common Files\VST3\`
3. Restart your DAW
4. If Windows SmartScreen blocks the file, click "More info" → "Run anyway"

### Uninstallation

**macOS:** Delete the following files:
```
~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay*.component
~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay*.vst3
~/Library/Audio/Presets/OpenSpatialDelay/    (user presets — optional)
```

**Windows:** Delete the following:
```
C:\Program Files\Common Files\VST3\OpenSpatialDelay*.vst3
%APPDATA%\OpenSpatialDelay\    (user presets — optional, if created)
```

---

## Post-Release Checklist

- [ ] Monitor GitHub Issues for Windows-specific bug reports (first 48 hours)
- [ ] Verify KVR listing is live and correct
- [ ] Archive the v1.0.0 release in local backup
- [ ] Update CLAUDE.md to reflect "v1.0.0 shipped" status
- [ ] Plan v1.1 milestone: code signing, installer packages, AAX

---

## Industry Reference: Standard Plugin Release SOP

Based on research of FabFilter, Valhalla DSP, Soundtoys, Surge XT, and Dexed:

| Practice | Industry Standard | OSD v1.0.0 | Post-v1.0 |
|----------|-------------------|------------|-----------|
| macOS packaging | `.pkg` in `.dmg` (signed, notarized) | `.zip` with loose bundles (ad-hoc signed) | Create `.pkg` installer |
| Windows packaging | `.exe` via InnoSetup | `.zip` with VST3 | Create InnoSetup installer |
| Code signing (macOS) | Apple Developer ID ($99/yr) | Ad-hoc | Enroll in Apple Developer Program |
| Code signing (Windows) | EV cert or Azure Trusted Signing | Unsigned | Apply for SignPath (free for OSS) |
| Plugin validation | `pluginval` at strictness 5+ | Catch2 tests only | Add `pluginval` to CI |
| AU validation | `auval -v aufx` | Not run | Add to CI |
| Uninstaller (macOS) | Not standard — manual delete | Manual delete instructions | n/a |
| Uninstaller (Windows) | InnoSetup auto-generates | Manual delete instructions | Include with installer |
| SHA-256 checksums | Standard for open-source | Planned for release | Include |
| Demo video | 60–90s screen recording | Not yet created | Create before announce |
