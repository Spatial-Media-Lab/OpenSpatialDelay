# Algorithm Comparison Report: DirAC, MDAP, NFC vs. Existing Algorithms

*Generated 2026-03-10 — OpenSpatialDelay v0.5*

---

## 1. DirAC (Directional Audio Coding)

**What it is:** A perceptual spatial audio *analysis-synthesis* method developed by Ville Pulkki at Aalto University. Unlike a panning algorithm, DirAC is a **representation framework** — it analyzes a B-format (Ambisonics) input into time-frequency bins, extracts a Direction of Arrival (DOA) and a diffuseness parameter per bin, then resynthesizes using any panning method for the direct part and decorrelated signals for the diffuse part.

**How it works:** Takes first-order Ambisonics (W,X,Y,Z) → STFT → per-bin intensity vector → DOA + diffuseness. Synthesis: direct stream (1-psi) panned via VBAP or similar; diffuse stream (psi) distributed decorrelated to all speakers.

**Coordinate system:** Internally Cartesian — the X,Y,Z channels are literally Cartesian particle velocity components. DOA is extracted from that Cartesian intensity vector, then converted to azimuth/elevation for panning synthesis.

**Relevance to OpenSpatialDelay:** **Low.** DirAC is designed for *decoding/upmixing* an existing spatial recording or Ambisonics stream, not for *positioning* individual point sources. Our plugin generates discrete delay taps at known positions — we already know each source's exact direction. DirAC would add no value here since there's nothing to analyze.

---

## 2. MDAP (Multiple-Direction Amplitude Panning)

**What it is:** An extension of VBAP, also by Pulkki, that creates **source spread** by rendering multiple "phantom" VBAP sources around the intended direction, then summing and normalizing their gains. Where VBAP activates the minimum number of speakers (2 in 2D, 3 in 3D), MDAP deliberately activates more.

**How it works:** For a desired direction, place N auxiliary sources on a ring around it on the unit sphere (typically 8 points). Each auxiliary source gets standard VBAP gains. Sum all gains, normalize for constant energy. The spread angle controls the ring radius — wider ring = more speakers activated = broader image.

**Coordinate system:** Same as VBAP — polar input, **Cartesian internal**. The spread ring geometry is computed via rotations on the unit sphere in Cartesian space.

**Relevance to OpenSpatialDelay:** **Moderate.** Could be useful as a "spread" parameter on top of existing VBAP — each tap could have adjustable width rather than being a hard point source. However, MDAP inherits all of VBAP's zenith problems since it's built on top of VBAP. It mitigates them slightly because the spread ring activates more triangles, but doesn't solve the fundamental issue.

---

## 3. NFC-HOA (Near-Field Compensation for Higher Order Ambisonics)

**What it is:** An extension of Ambisonics that adds **distance encoding** — standard HOA assumes plane waves (infinite distance sources), NFC adds radial filters that model the wavefront curvature of near-field sources. Developed by Daniel and Moreau (~2003).

**How it works:** Applies per-order shelf filters that compensate for the bass-boost that occurs when encoding spherical (near-field) waves into spherical harmonics. The filters depend on source distance and the reference loudspeaker radius. Without NFC, encoding a close source at HOA order 3 produces extreme low-frequency energy in higher-order channels.

**Coordinate system:** **Fully spherical** — azimuth, elevation, distance. The spherical harmonics are inherently spherical. NFC adds only radial (distance) filters, completely decoupled from direction.

**Relevance to OpenSpatialDelay:** **Moderate-to-low for current design.** Our distance parameter currently controls amplitude attenuation and air absorption filtering. NFC-HOA would only matter for the Ambisonics output path, and only if we wanted distance-encoded Ambisonics output where the renderer handles distance. Could be a niche feature for Ambisonics-specific users.

---

## Comparison Against Existing Algorithms

| | **Coordinate System** | **Zenith Behavior** | **Layout Requirement** | **Purpose** |
|---|---|---|---|---|
| **Constant Power** (existing) | Cartesian dot product | Graceful — cosine rolloff distributes naturally | Arbitrary (any) | Smooth diffuse panning (default) |
| **VBAP** (existing) | Polar input, Cartesian internal | Poor without zenith speaker — degenerate triangles, energy drop | Arbitrary surrounding (convex hull) | Point-source panning |
| **VBIP** (existing) | Same as VBAP | Same as VBAP | Same as VBAP | Intensity-optimized panning |
| **KNN** (existing) | Cartesian distance | Graceful — distributes to nearest speakers | Arbitrary (any) | Diffuse distance-weighted |
| **DBAP** (existing) | Cartesian (Euclidean) | No singularity — becomes diffuse | Arbitrary (any, non-surrounding OK) | Distance-weighted panning |
| **Ambisonics** (existing) | Spherical (SH basis) | Mathematically isotropic | Regular/spherical preferred | Full-sphere encoding |
| **DirAC** (new) | Cartesian intensity, polar DOA | No analysis singularity | None (it is a codec, not a panner) | Analysis-synthesis framework |
| **MDAP** (new) | Same as VBAP | Slightly better than VBAP (spread helps) | Same as VBAP | Spread/width panning |
| **NFC-HOA** (new) | Spherical + radial | Same as HOA | Same as HOA | Distance encoding for Ambisonics |

---

## The Zenith Problem — Root Cause

The centering behavior past ~60 degrees elevation is the documented VBAP zenith problem. It affects VBAP, VBIP, and to a lesser extent KNN in the surround rendering path.

**Virtual speaker layout (binaural path):** The 16-speaker virtual array includes a zenith speaker at index 15 (+90 degrees elevation), with proper Tier 2 triangulation connecting the top ring to it. So the binaural HRTF path handles zenith correctly.

**Real surround layouts (the problem):** None of the surround layouts have a zenith speaker:
- 7.1.4: highest speakers at +45 degrees elevation
- 9.1.6: highest speakers at +45 degrees elevation
- All surround layouts top out at +45 degrees

When VBAP/VBIP is used on these layouts with a source above ~60 degrees, the algorithm tries to pan into triangles where all three vertices are at +45 degrees or below. The Cartesian unit vector for the source points steeply upward, but the triangulation has no vertex above +45 degrees. The gain computation either falls back to a degenerate triangle or distributes gains across multiple top-ring speakers roughly equally — creating the centering effect.

**The fix:** Adding an imaginary zenith speaker to surround layouts that lack one, computing VBAP gains including that speaker, then downmixing the imaginary speaker's gain to the real top-ring speakers using a 1/sqrt(M) factor. This is Pulkki's own documented solution.

---

## Bottom Line

- **DirAC:** Not applicable to our use case (it is a codec for Ambisonics streams, not a panner for known point sources)
- **MDAP:** Could add a spread/width feature on top of VBAP for surround rendering
- **NFC-HOA:** Relevant for Ambisonics output path distance encoding, especially at higher orders

None of these three algorithms solve the zenith centering issue. The real fix is the imaginary zenith speaker technique applied to the surround layout triangulations in the existing VBAP/VBIP code path.
