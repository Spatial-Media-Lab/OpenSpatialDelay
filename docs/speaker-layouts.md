# Virtual Speaker Layouts — Spatial Media Library Reference

Single source of truth for all virtual speaker layouts used by the Monitoring Format dropdown
in binaural rendering mode. These layouts define the spatial positions of virtualized speakers
that are rendered to headphones via HRTF convolution.

Convention: 0° azimuth = front, positive azimuth = left, negative = right.
All positions in degrees. Elevation: 0° = ear level, +90° = zenith.

---

## Full (16 speakers) — Default Binaural Mode

**Standard:** 9.1.6 bed (ITU-R BS.2051 System J) + Zenith
**Max Ambisonics Order:** 3rd (exactly (N+1)² = 16)
**Use case:** Highest quality binaural rendering with full 3D coverage

### Ear Level (9 speakers, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | C | 0° | 0° | Centre |
| 1 | L | +30° | 0° | Left |
| 2 | R | -30° | 0° | Right |
| 3 | Lw | +60° | 0° | Left Wide |
| 4 | Rw | -60° | 0° | Right Wide |
| 5 | Ls | +90° | 0° | Left Surround |
| 6 | Rs | -90° | 0° | Right Surround |
| 7 | Lrs | +135° | 0° | Left Rear Surround |
| 8 | Rrs | -135° | 0° | Right Rear Surround |

### Top Ring (6 speakers, elevation +45°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 9 | Tfl | +45° | +45° | Top Front Left |
| 10 | Tfr | -45° | +45° | Top Front Right |
| 11 | Tsl | +90° | +45° | Top Side Left |
| 12 | Tsr | -90° | +45° | Top Side Right |
| 13 | Trl | +135° | +45° | Top Rear Left |
| 14 | Trr | -135° | +45° | Top Rear Right |

### Zenith (1 speaker)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 15 | T | 0° | +90° | Top Centre (Zenith) |

---

## 9.1.6 (15 speakers)

**Standard:** ITU-R BS.2051 System J / Dolby Atmos 9.1.6
**Max Ambisonics Order:** 2nd–3rd (15 speakers, just under (3+1)²=16)
**Use case:** Simulates full Atmos speaker system without zenith

Same as Full (16) minus the Zenith speaker (index 15).
Uses indices 0–14 from the Full layout above.

---

## 7.1.4 (11 speakers)

**Standard:** Dolby Atmos 7.1.4 / ITU-R BS.2051 System D
**Max Ambisonics Order:** 2nd (11 > (2+1)²=9)
**Use case:** Most common Atmos home/studio layout

### Ear Level (7 speakers, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | L | +30° | 0° | Left |
| 1 | R | -30° | 0° | Right |
| 2 | C | 0° | 0° | Centre |
| 3 | Lss | +90° | 0° | Left Side Surround |
| 4 | Rss | -90° | 0° | Right Side Surround |
| 5 | Lsr | +135° | 0° | Left Rear Surround |
| 6 | Rsr | -135° | 0° | Right Rear Surround |

### Top (4 speakers, elevation +45°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 7 | Tfl | +45° | +45° | Top Front Left |
| 8 | Tfr | -45° | +45° | Top Front Right |
| 9 | Trl | +135° | +45° | Top Rear Left |
| 10 | Trr | -135° | +45° | Top Rear Right |

---

## 7.1 (7 speakers)

**Standard:** ITU-R BS.2051 System C
**Max Ambisonics Order:** 2nd (7 < (2+1)²=9, marginal)
**Use case:** Standard surround without height

### Ear Level (7 speakers, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | L | +30° | 0° | Left |
| 1 | R | -30° | 0° | Right |
| 2 | C | 0° | 0° | Centre |
| 3 | Lss | +90° | 0° | Left Side Surround |
| 4 | Rss | -90° | 0° | Right Side Surround |
| 5 | Lsr | +135° | 0° | Left Rear Surround |
| 6 | Rsr | -135° | 0° | Right Rear Surround |

---

## 5.1 (5 speakers)

**Standard:** ITU-R BS.775-3
**Max Ambisonics Order:** 1st (5 > (1+1)²=4)
**Use case:** Classic surround format

### Ear Level (5 speakers, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | L | +30° | 0° | Left |
| 1 | R | -30° | 0° | Right |
| 2 | C | 0° | 0° | Centre |
| 3 | Ls | +110° | 0° | Left Surround |
| 4 | Rs | -110° | 0° | Right Surround |

---

## Quad (4 speakers)

**Standard:** ITU-R BS.775
**Max Ambisonics Order:** 1st (4 = (1+1)²)
**Use case:** Minimal surround format

### Ear Level (4 speakers, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | L | +30° | 0° | Left |
| 1 | R | -30° | 0° | Right |
| 2 | Ls | +110° | 0° | Left Surround |
| 3 | Rs | -110° | 0° | Right Surround |

---

## Octaphonic (8 speakers)

**Standard:** Equal-angle octagonal (Centre configuration)
**Max Ambisonics Order:** 2nd (8 < (2+1)²=9, marginal)
**Use case:** Art installations, experimental music, ring speaker setups

### Ear Level (8 speakers at 45° intervals, elevation 0°)

| Index | Label | Azimuth | Elevation | Description |
|-------|-------|---------|-----------|-------------|
| 0 | C | 0° | 0° | Centre |
| 1 | RF | -45° | 0° | Right Front |
| 2 | R | -90° | 0° | Right |
| 3 | RR | -135° | 0° | Rear Right |
| 4 | Rear | +180° | 0° | Rear |
| 5 | RL | +135° | 0° | Rear Left |
| 6 | L | +90° | 0° | Left |
| 7 | FL | +45° | 0° | Front Left |

---

## Summary Table

| Format | Speakers | Ear-Level | Height | Zenith | Max Ambi | 3D VBAP |
|--------|----------|-----------|--------|--------|----------|---------|
| Full (16) | 16 | 9 | 6 (+45°) | 1 (+90°) | 3rd | Yes (30 triplets) |
| 9.1.6 (15) | 15 | 9 | 6 (+45°) | — | 2nd–3rd | Yes |
| 7.1.4 (11) | 11 | 7 | 4 (+45°) | — | 2nd | Yes |
| 7.1 (7) | 7 | 7 | — | — | 2nd* | No (2D only) |
| 5.1 (5) | 5 | 5 | — | — | 1st | No (2D only) |
| Quad (4) | 4 | 4 | — | — | 1st | No (2D only) |
| Octaphonic (8) | 8 | 8 | — | — | 2nd* | No (2D only) |

*Marginal: speaker count < (N+1)², decode quality reduced at that order.

---

## Sound Particles inDelay Reference

For comparison, Sound Particles inDelay supports 30+ output formats including:
- Stereo variants: Mono, Stereo, Stereo (VBAP), Stereo (XY), Stereo (MS), Stereo (Blumlein), Binaural
- Surround: Quad, 5.0, 5.1, 7.0, 7.1, and variations
- Immersive: 5.0.2, 5.1.2, 5.0.4, 5.1.4, 7.0.4, 7.1.4, 7.0.6, 7.1.6, 9.0.8, 9.1.8, 11.1.8
- Ambisonics: 1st through 6th order
- Sony 360 Reality Audio, Dolby Atmos

Our 7 monitoring format options cover the standard layouts most relevant to
professional spatial audio production.
