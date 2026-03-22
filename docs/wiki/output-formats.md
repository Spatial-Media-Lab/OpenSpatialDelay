# Output Formats

OpenSpatialDelay supports 21 output formats across 4 categories. The output format is selected via the **Output Format** dropdown in the header bar.

## Format Registry

### Binaural

| Format | Short Name | Channels | LFE | Height | Notes |
|--------|-----------|----------|-----|--------|-------|
| Binaural | Bin | 2 | No | No | Default. HRTF-based headphone monitoring. |

### Stereo

| Format | Short Name | Channels | LFE | Height | Notes |
|--------|-----------|----------|-----|--------|-------|
| Stereo | St | 2 | No | No | 5 sub-modes selected via Algorithm parameter (Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein). |

### Surround

| Format | Short Name | Channels | LFE | Height |
|--------|-----------|----------|-----|--------|
| Quadraphonic | Quad | 4 | No | No |
| 5.0 Surround | 5.0 | 5 | No | No |
| 5.1 Surround | 5.1 | 6 | Yes | No |
| 7.0 Surround | 7.0 | 7 | No | No |
| 5.1.2 Atmos | 5.1.2 | 8 | Yes | Yes |
| 7.1 Surround | 7.1 | 8 | Yes | No |
| Octaphonic | Oct | 8 | No | No |
| 7.0.2 | 7.0.2 | 9 | No | Yes |
| 5.1.4 Atmos | 5.1.4 | 10 | Yes | Yes |
| 7.1.2 Atmos | 7.1.2 | 10 | Yes | Yes |
| 7.1.4 Atmos | 7.1.4 | 12 | Yes | Yes |
| 7.1.6 Atmos | 7.1.6 | 14 | Yes | Yes |
| 9.1.6 Atmos | 9.1.6 | 16 | Yes | Yes |

### Ambisonics (AmbiX ACN/SN3D)

| Format | Short Name | Channels | Order |
|--------|-----------|----------|-------|
| 1st Order Ambi | FOA | 4 | 1 |
| 2nd Order Ambi | SOA | 9 | 2 |
| 3rd Order Ambi | HOA | 16 | 3 |
| 4th Order Ambi | 4OA | 25 | 4 |
| 5th Order Ambi | 5OA | 36 | 5 |
| 6th Order Ambi | 6OA | 49 | 6 |

## LFE Generation

For surround formats that include an LFE channel:

- **Filter:** 2nd-order Butterworth low-pass at 120 Hz
- **Level:** -10 dB (gain factor 0.316)
- **Source:** Derived from the mono sum of all wet delay taps
- **LFE channel index:** Typically channel 3 (JUCE/SMPTE convention: L, R, C, LFE, ...)

## Bus Layout Behavior

The plugin accepts any output channel count provided by the DAW. Internally, it maps the available channels to the closest supported format. If the DAW provides more channels than the selected format requires, extra channels are zeroed. If the DAW provides fewer channels than requested, the plugin falls back to the closest matching format.

## Speaker Positions

For detailed virtual speaker positions used by each surround layout, see the speaker layout definitions in `../speaker-layouts.md`.

## See Also

- [Spatialization Algorithms](algorithms-guide.md) -- which algorithms are available per output format
- [Getting Started](getting-started.md) -- initial setup and output format selection
