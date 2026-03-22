# Spatialization Algorithms

OpenSpatialDelay provides 7 core spatialization algorithms and 5 stereo panning modes. The active algorithm depends on the selected output format.

## Core Algorithms (Surround Output)

These 6 user-facing algorithms are available when the output format is a surround layout (Quad through 9.1.6):

### Ambisonics (HOA)
- **Index:** 0
- **Technique:** Spherical harmonic encoding using ACN channel ordering and SN3D normalization
- **Weighting:** max-rE for improved off-axis localization
- **Best for:** Ambisonics bus output, flexible decoding, rotation-friendly workflows
- **Notes:** Supports up to 6th order (49 channels) in Ambisonics output mode. For surround output, encodes to 3rd order internally and decodes to the speaker layout.

### DBAP
- **Index:** 1
- **Technique:** Distance-Based Amplitude Panning (Lossius et al., ICMC 2009)
- **Weighting:** Inverse-distance-squared from source to each speaker in Cartesian space
- **Best for:** Irregular or non-standard speaker arrangements where VBAP triangulation may fail
- **Notes:** Does not require speaker triangulation. Gain distribution is purely distance-based.

### KNN
- **Index:** 2
- **Technique:** K-Nearest Neighbor panning with inverse-distance weighting
- **Best for:** General-purpose use with smooth spatial transitions
- **Notes:** Provides smooth gain interpolation between nearby speakers. Good default choice for most layouts.

### MDAP
- **Index:** 3
- **Technique:** Multiple-Direction Amplitude Panning (Pulkki 2000)
- **Weighting:** VBAP applied to 8 spread sub-sources distributed on a ring around the main direction
- **Best for:** Wider, more stable spatial images; diffuse source rendering
- **Notes:** Extension of VBAP that trades localization precision for source width and stability.

### VBAP
- **Index:** 4
- **Technique:** Vector Base Amplitude Panning (Pulkki 1997)
- **Weighting:** 2D speaker pairs or 3D speaker triplets with energy-preserving gain calculation
- **Best for:** Precise localization with standard speaker layouts
- **Notes:** Default algorithm. Uses pre-computed triangulation for 3D layouts. Activates at most 2 (2D) or 3 (3D) speakers per source.

### VBIP
- **Index:** 5
- **Technique:** Vector Base Intensity Panning
- **Weighting:** Squared VBAP gains (intensity domain rather than amplitude domain)
- **Best for:** Tighter source focus, reduced phantom image width
- **Notes:** Variant of VBAP that produces narrower spatial images.

### Direct Binaural (Internal)
- **Index:** Not user-selectable
- **Technique:** Woodworth ITD + ILD model
- **Usage:** Used internally by the "Simple (Low CPU)" HRTF profile for binaural output
- **Notes:** Computes interaural time and level differences using a spherical head model. No SOFA file required.

## Stereo Modes

When the output format is **Stereo**, the algorithm parameter switches to stereo panning modes (indices 6-10):

| Index | Mode | Description |
|-------|------|-------------|
| 6 | Equal Power | Standard equal-power panning law |
| 7 | Stereo VBAP | VBAP applied to a virtual L/R speaker pair |
| 8 | XY Pair | Coincident XY microphone pair simulation |
| 9 | MS Encode | Mid-Side encoding (Mid = center, Side = azimuth offset) |
| 10 | Blumlein | Blumlein figure-8 pair simulation |

## Algorithm Availability by Output Format

| Output Format | Available Algorithms | Notes |
|---------------|---------------------|-------|
| **Binaural** | Locked internally | Uses Direct Binaural (Woodworth) for Simple profile, or per-source HRTF convolution for SOFA profiles. Algorithm dropdown is replaced by HRTF Profile selector. |
| **Stereo** | Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein (indices 6-10) | Algorithm dropdown shows stereo modes only. |
| **Surround** (Quad through 9.1.6) | Ambisonics HOA, DBAP, KNN, MDAP, VBAP, VBIP (indices 0-5) | All 6 surround algorithms available. |
| **Ambisonics** (FOA through 6OA) | Locked to Ambisonics Encode | Spherical harmonic encoding at the selected order. Algorithm dropdown is not applicable. |

## See Also

- [Output Formats](output-formats.md) -- output format details and channel counts
- [HRTF Profiles](hrtf-profiles.md) -- binaural rendering profiles
