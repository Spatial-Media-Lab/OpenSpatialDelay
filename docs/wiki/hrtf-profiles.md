# HRTF Profiles

OpenSpatialDelay includes 6 HRTF profiles for binaural output. When the output format is set to **Binaural**, the algorithm dropdown in the header bar is replaced by the HRTF Profile selector.

## Profile Summary

| # | Name | SOFA File | Head Radius | ILD Scale | Shadow Freq | Description |
|---|------|-----------|-------------|-----------|-------------|-------------|
| 0 | Simple (Low CPU) | None | -- | -- | -- | Woodworth ITD+ILD model only. No SOFA convolution. Lowest CPU usage. |
| 1 | Studio Reference | MIT KEMAR Large Pinna | 0.0875 m | 1.0 | 1500 Hz | Flat, accurate response based on the MIT KEMAR mannequin. |
| 2 | Immersive | SADIE II D2 KU100 | 0.0920 m | 1.3 | 1200 Hz | Neumann KU100 dummy head. Lush, spacious sound with wider ILD. |
| 3 | Natural | CIPIC Subject 003 | 0.0850 m | 0.8 | 1800 Hz | Human subject measurement. Subtler, more intimate spatial cues. |
| 4 | Precise | HUTUBS PP2 | 0.0900 m | 1.1 | 1400 Hz | Cross-validated, balanced profile for detailed positioning. |
| 5 | Spatial | Bernschuetz KU100 | 0.0875 m | 1.5 | 1100 Hz | High-resolution KU100 capture. Exaggerated spatial cues for wide image. |

## Technical Details

### Convolution Engine

- **Method:** Partitioned FFT overlap-save convolution
- **Per-source processing:** 12 independent convolver pairs (left + right ear), one per delay tap
- **HRIR updates:** Triggered when a source position changes by more than approximately 1 degree; uses KD-tree nearest-neighbor lookup via libmysofa
- **Double-buffered renderer:** HRTF profile switching uses two `BinauralRenderer` instances. The new profile is loaded into the inactive renderer on the message thread, then atomically swapped -- no audio dropout during switches.
- **Sample rate matching:** SOFA HRIR data is resampled to match the current DAW session sample rate during loading.

### Simple (Low CPU) Profile

Profile 0 bypasses SOFA convolution entirely. It uses the **Woodworth spherical head model** to compute:

- **ITD (Interaural Time Difference):** Based on head radius and source angle
- **ILD (Interaural Level Difference):** Frequency-dependent head shadow model

This profile is useful for quick previews or CPU-constrained sessions.

### SOFA Profiles (1-5)

Profiles 1-5 load SOFA files embedded as binary data in the plugin. The head radius, ILD scale, and shadow frequency values listed in the table above are used by the Woodworth fallback model when computing speaker-domain binaural gains; the primary SOFA convolution uses the measured HRIR data directly.

## Choosing a Profile

| Use Case | Recommended Profile |
|----------|-------------------|
| Low CPU / quick preview | Simple (Low CPU) |
| Flat, accurate monitoring | Studio Reference |
| Lush, spacious sound design | Immersive |
| Intimate vocal/instrument recordings | Natural |
| Detailed spatial positioning work | Precise |
| Wide panoramic image | Spatial |

## See Also

- [Spatialization Algorithms](algorithms-guide.md) -- how binaural rendering relates to algorithm selection
- [Output Formats](output-formats.md) -- selecting Binaural as the output format
