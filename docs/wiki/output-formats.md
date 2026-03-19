# Output Formats

OpenSpatialDelay supports 21 output formats organized into four categories: Binaural, Stereo, Surround, and Ambisonics. The active format is selected from the dropdown in the header bar.

## Five Rendering Paths

Internally, the plugin uses five distinct rendering paths. The active path is determined by the selected output format:

| Rendering Path | Output Formats | Description |
|---|---|---|
| Direct Binaural HRTF | Binaural | Each tap convolved with measured HRTF at its exact 3D position. Full elevation and distance cues on headphones. |
| Simple Binaural (Woodworth) | Binaural (Simple profile) | Lightweight ITD+ILD model for low-latency monitoring. No convolution. |
| Stereo Variants | Stereo | Five mic simulation modes selected via the Algorithm dropdown. 2-channel output. |
| Discrete Surround | Quad through 9.1.6 | Speaker panning via the selected algorithm. Direct-to-speaker gain computation + LFE. |
| Ambisonics Output | 1st-6th Order Ambi | Spherical harmonic encoding (AmbiX, ACN/SN3D ordering). |

## Binaural (Headphone Monitoring)

Binaural mode renders each delay tap through HRTF (Head-Related Transfer Function) convolution, producing a convincing 3D sound field over headphones. Each tap is convolved at its exact 3D position -- azimuth, elevation, and distance are all fully represented.

### HRTF Profiles

When Binaural is selected, the rightmost dropdown in the header bar becomes the HRTF Profile selector. Six profiles are available:

| Profile | Source | Character |
|---|---|---|
| Simple (Low CPU) | Woodworth model | Lightweight ITD+ILD only -- no convolution. Good for low-latency monitoring or when CPU is limited. Less spatial realism. |
| Studio Reference | MIT KEMAR | Industry-standard dummy head measurement. Neutral, accurate localization. Good starting point. |
| Immersive | SADIE II D2 (KU100) | Neumann KU100 dummy head. Rich low end, wide spatial image. Excellent for music production. |
| Natural | CIPIC Subject003 | Human subject measurement. Organic, realistic externalization. Good for dialogue and field recordings. |
| Precise | HUTUBS PP2 | High-resolution measurement. Tight localization, analytical character. Useful for spatial design work. |
| Spatial | Bernschuetz KU100 | Full 2-degree resolution KU100. Smooth, even coverage. Great all-rounder for spatial mixing. |

> **Tip:** HRTF perception is highly individual. Try each profile and choose the one where you can most clearly locate sounds in space. The "right" profile depends on your head and ear shape.

All HRTF data comes from measured SOFA files, which are open-format standardized measurements of real heads and dummy heads.

## Stereo (Speaker Monitoring)

Stereo mode outputs a 2-channel signal using one of five mic simulation modes. These simulate different stereo microphone configurations, each with a distinct spatial character.

When Stereo is selected, the Algorithm dropdown shows the five stereo modes:

| Mode | Description | Best For |
|---|---|---|
| Equal Power | Standard equal-power panning. Smooth, familiar. | General mixing, widest compatibility |
| VBAP 2-Speaker | Vector-based panning between virtual speakers at +/- 30 degrees. | Focused stereo image matching standard speaker placement |
| XY Cardioid | Coincident XY pair at +/- 45 degrees. | Natural stereo image, good mono compatibility |
| MS Mid-Side | Mid-Side encoding (sum/difference). | Adjustable width in post, broadcast workflows |
| Blumlein | Crossed figure-8 microphones at +/- 45 degrees. | Natural ambience capture, classical recording aesthetic |

In Stereo mode, elevation contributes only to distance attenuation (perceived volume), not to left-right placement. This is a physical limitation of 2-channel playback.

> **Note:** The Algorithm and HRTF Profile dropdowns are both hidden in Stereo mode since neither applies.

## Surround (Speaker Arrays)

Surround formats output discrete speaker signals. The plugin computes per-speaker gains using the selected spatialization algorithm. All surround formats with an LFE channel include a 120 Hz low-pass filtered signal at -10 dB on the LFE channel.

### Supported Surround Formats

| Format | Channels | LFE | Height | Typical Use |
|---|---|---|---|---|
| Quadraphonic | 4 | No | No | Basic surround, art installations |
| 5.0 Surround | 5 | No | No | Music surround without LFE |
| 5.1 Surround | 6 | Yes | No | Standard film/broadcast surround |
| 7.0 Surround | 7 | No | No | Extended surround without LFE |
| 5.1.2 Atmos | 8 | Yes | Yes | Entry-level Atmos with 2 height speakers |
| 7.1 Surround | 8 | Yes | No | Standard high-channel surround |
| Octaphonic | 8 | No | No | 8 equidistant speakers (ring), art/research |
| 7.0.2 | 9 | No | Yes | Extended surround with 2 height speakers |
| 5.1.4 Atmos | 10 | Yes | Yes | Atmos with 4 height speakers |
| 7.1.2 Atmos | 10 | Yes | Yes | Atmos with 2 height speakers on 7.1 bed |
| 7.1.4 Atmos | 12 | Yes | Yes | Full Dolby Atmos home cinema |
| 7.1.6 Atmos | 14 | Yes | Yes | Extended Atmos with 6 height speakers |
| 9.1.6 Atmos | 16 | Yes | Yes | Maximum Atmos configuration |

### Spatialization Algorithms

When a surround format is selected, the Algorithm dropdown becomes active. Choose the algorithm that best fits your speaker layout and creative intent:

| Algorithm | Full Name | Description | Best For |
|---|---|---|---|
| VBAP | Vector Base Amplitude Panning | Selects the nearest speaker triangle and distributes gain across up to 3 speakers. Sharp, focused image. | Standard surround, precise placement |
| VBIP | Vector Base Intensity Panning | Like VBAP but with squared gains for even tighter localization. | Precision work, forensic audio |
| MDAP | Multiple Direction Amplitude Panning | Spreads the source across multiple VBAP directions for a wider image. | Ambient sources, wide pads |
| KNN | K-Nearest Neighbor | Inverse-distance weighting to the nearest speakers. Smooth, diffuse. | Gentle panning, ambient textures |
| DBAP | Distance-Based Amplitude Panning | Pure distance-based gain (no direction). Works with any speaker layout including irregular ones. | Non-standard layouts, installations |
| Ambisonics | Ambisonics Decode | Encodes to spherical harmonics then decodes to speakers. Even coverage, layout-independent. | When format-agnostic rendering is needed |

> **Tip:** For most surround work, start with VBAP. If sources sound too pinpointed, try MDAP or KNN for a wider spread. DBAP is the go-to choice for non-standard or irregular speaker arrays.

## Ambisonics (Spherical Harmonic Output)

Ambisonics formats output spherical harmonic channels in AmbiX format (ACN channel ordering, SN3D normalization). These are designed to feed into an Ambisonics decoder or renderer downstream in your signal chain.

| Format | Order | Channels | Typical Use |
|---|---|---|---|
| 1st Order Ambi (FOA) | 1 | 4 | VR, 360 video, basic spatial audio |
| 2nd Order Ambi (SOA) | 2 | 9 | Improved spatial resolution |
| 3rd Order Ambi (HOA) | 3 | 16 | Standard high-quality ambisonics |
| 4th Order Ambi | 4 | 25 | High-resolution ambisonics |
| 5th Order Ambi | 5 | 36 | Very high resolution |
| 6th Order Ambi | 6 | 49 | Maximum resolution |

Higher orders provide sharper spatial imaging but require more channels. 3rd order (16 channels) is the most common for production work.

> **Note:** When an Ambisonics format is selected, the Algorithm dropdown shows "Ambisonics Encode" and is disabled -- there is no algorithm choice for Ambisonics output since encoding is a fixed mathematical operation.

## Format Quick Pick Guide

| I want to... | Choose | Algorithm | Notes |
|---|---|---|---|
| Monitor on headphones with 3D | Binaural | (hidden) | Select an HRTF profile |
| Quick headphone check (low CPU) | Binaural | (hidden) | Select "Simple (Low CPU)" profile |
| Mix on stereo speakers | Stereo | Equal Power | Or try XY/Blumlein for character |
| Mix in 5.1 | 5.1 Surround | VBAP | Industry standard |
| Mix in Dolby Atmos | 7.1.4 Atmos | VBAP | Standard Atmos bed |
| Feed an Atmos renderer | 7.1.4 Atmos | VBAP | Or use Ambisonics for object-based |
| Feed an Ambisonics decoder | 3rd Order Ambi | (disabled) | 16 channels, good balance of resolution and efficiency |
| Work in VR / 360 video | 1st Order Ambi | (disabled) | Widely supported FOA format |
| Non-standard speaker layout | Any surround | DBAP | Works with any geometry |
| Research / art installation | Octaphonic | KNN or DBAP | 8 equidistant speakers |

## Changing Formats

Select the output format from the dropdown in the header bar. The format change takes effect immediately -- you do not need to reload the plugin.

> **Important:** Your DAW track must be configured with enough output channels for the selected format. If the track has fewer channels than the format requires, the extra channels will be silent. Check the "Channels" column in the surround formats table above.

Output format is intentionally **not saved in presets**. This allows you to load any preset regardless of your monitoring setup -- the spatial positions and delay settings translate naturally across all formats.
