# Version Management & v0.2 Rebuild Plan

**Date:** 2026-03-09
**Status:** Pending approval

---

## Root Cause: Plugin Identity Collision

Both v0.2 and v0.3 have identical AU/VST3 identifiers, so the DAW sees them as the same plugin and only loads one:

| Identifier | v0.1 | v0.2 | v0.3 |
|---|---|---|---|
| AU subtype (PLUGIN_CODE) | `Osd1` | **`Osd3`** | **`Osd3`** |
| CFBundleIdentifier | `...OpenSpatialDelay_v01` | **`...OpenSpatialDelay`** | **`...OpenSpatialDelay`** |
| CFBundleVersion | `0.1.0` | **`0.2.0`** | **`0.2.0`** |
| manufacturer | `Smli` | `Smli` | `Smli` |

The AU system identifies plugins by the triplet `(type, subtype, manufacturer)`. Both v0.2 and v0.3 register as `(aufx, Osd3, Smli)` — macOS only loads one. VST3 has the same issue: class ID is derived from PLUGIN_CODE, so both generate identical IDs.

v0.1 works because it has unique codes (`Osd1`).

---

## Phase 1: Fix and Rebuild v0.2

1. **Back up** current v0.3 Source/ files and CMakeLists.txt
2. **Copy** v0.2 archive sources into Source/ (rename from `*_v0.2.*` to standard names)
3. **Modify CMakeLists.txt** for v0.2 build:
   - `PLUGIN_CODE Osd2` (unique to v0.2)
   - `PRODUCT_NAME "OpenSpatialDelay v0.2"`
   - `project(... VERSION 0.2.0 ...)`
4. **Update install script** to reference v0.2 name
5. **Build** → verify .component and .vst3 are produced
6. **Restore** v0.3 sources and CMakeLists.txt

## Phase 2: Fix v0.3 CMakeLists.txt

- Update `project(... VERSION 0.3.0 ...)` (currently says 0.2.0)
- Keep `PLUGIN_CODE Osd3`
- Rebuild v0.3
- Both versions now have unique identifiers and can coexist

## Phase 3: Store Pre-Built Binaries

Create archive structure:
```
Archive/v0.1/builds/macOS/
  OpenSpatialDelay v0.1.component/
  OpenSpatialDelay v0.1.vst3/
Archive/v0.2/builds/macOS/
  OpenSpatialDelay v0.2.component/
  OpenSpatialDelay v0.2.vst3/
```
Create a `scripts/install_version.sh <version>` script that copies from archive to `~/Library/Audio/Plug-Ins/`.

## Phase 4: Long-Term Version Management

### Semantic Versioning (MAJOR.MINOR.PATCH)

- **MINOR** bump (v0.1 → v0.2 → v0.3): New features, gets unique PLUGIN_CODE (`Osd1`, `Osd2`, `Osd3`), can coexist side-by-side in DAW
- **PATCH** bump (v0.2.0 → v0.2.1): Bug fix, shares PLUGIN_CODE with its minor version (replaces in-place, same DAW slot)
- **MAJOR** bump (v0.x → v1.0): Breaking changes (preset format, parameter layout)

### Git Branches

- `main` — latest stable development (currently v0.3)
- `release/v0.1`, `release/v0.2`, `release/v0.3` — long-lived release branches
- Bug fix workflow: branch from `release/v0.2`, fix, tag `v0.2.1`, cherry-pick to `main`
- Git tags for every release: `v0.1.0`, `v0.2.0`, `v0.3.0`

### Plugin Identity Convention

| Version | PLUGIN_CODE | CFBundleIdentifier suffix | CMake VERSION |
|---|---|---|---|
| v0.1.x | `Osd1` | `_v01` | `0.1.x` |
| v0.2.x | `Osd2` | `_v02` | `0.2.x` |
| v0.3.x | `Osd3` | `_v03` | `0.3.x` |
| v1.0.x | `OsD1` | `_v10` | `1.0.x` |

### GitHub Releases for CI Artifacts

- Tag-triggered workflow builds both macOS + Windows binaries
- Uploads to GitHub Releases (downloadable without rebuilding)
- `scripts/install_version.sh` can pull from local archive or GitHub Release
