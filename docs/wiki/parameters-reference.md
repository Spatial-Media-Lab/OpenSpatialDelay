# Parameter Reference

All parameters are exposed through JUCE's `AudioProcessorValueTreeState` (APVTS) and are fully automatable in any compatible DAW.

## Parameter ID Naming Convention

- **Global parameters:** `paramName` (e.g., `delayTime`, `feedback`, `dryWet`)
- **Per-object parameters:** `object{N}_{paramName}` where N = 1-12 (e.g., `object1_azimuth`, `object12_enabled`)

## Global Parameters

| Parameter ID | Name | Type | Range | Default | Unit | Notes |
|-------------|------|------|-------|---------|------|-------|
| `delayTime` | Delay Time | Float | 1.0 -- 2000.0 | 500.0 | ms | Skew 0.3 (log-like). Displays as seconds when >= 1000 ms. |
| `tempoSync` | Tempo Sync | Bool | off/on | off | -- | When on, delay time is derived from DAW tempo. |
| `noteDivision` | Note Division | Float | 1 -- 16 | 4 | 16th notes | 1=1/16, 2=1/8, 4=1/4, 8=1/2, 16=1/1. Integer steps. |
| `syncMode` | Sync Mode | Choice | 0-3 | 0 | -- | 0=Notes, 1=Triplet, 2=Dotted, 3=16th |
| `feedback` | Feedback | Float | 0.0 -- 1.0 | 0.3 | % | Displayed as 0-100%. |
| `filterLP` | Low-Pass Filter | Float | 200 -- 20000 | 20000 | Hz | Skew 0.3. Applied in feedback loop. |
| `filterHP` | High-Pass Filter | Float | 20 -- 5000 | 20 | Hz | Skew 0.3. Applied in feedback loop. |
| `pitchShift` | Pitch Shift | Float | -24.0 -- 24.0 | 0.0 | st | Applied per feedback cycle (grain-based). |
| `dryWet` | Dry/Wet | Float | 0.0 -- 1.0 | 0.5 | % | Displayed as 0-100%. |
| `inputGain` | Input Gain | Float | -60.0 -- 12.0 | 0.0 | dB | Skew 2.0. |
| `outputGain` | Output Gain | Float | -60.0 -- 12.0 | 0.0 | dB | Skew 2.0. |
| `algorithm` | Algorithm | Choice | 0-10 | 0 | -- | 0=Ambisonics HOA, 1=DBAP, 2=KNN, 3=MDAP, 4=VBAP, 5=VBIP, 6=Equal Power, 7=Stereo VBAP, 8=XY Pair, 9=MS Encode, 10=Blumlein. Indices 0-5 for surround, 6-10 for stereo. |
| `hrtfProfile` | HRTF Profile | Choice | 0-5 | 0 | -- | 0=Simple (Low CPU), 1=Studio Reference, 2=Immersive, 3=Natural, 4=Precise, 5=Spatial |
| `outputFormat` | Output Format | Choice | 0-20 | 0 | -- | See [Output Formats](output-formats.md) for the full registry. Default is Binaural (index 0). |
| `airAbsorption` | Air Absorption | Bool | off/on | off | -- | Distance-driven HF rolloff per tap. |
| `admOscEnabled` | ADM-OSC Enabled | Bool | off/on | off | -- | Enable/disable ADM-OSC receive. |

## Per-Object Parameters (x12)

Each of the 12 objects (N = 1-12) has the following parameters:

| Parameter ID Pattern | Name | Type | Range | Default | Notes |
|---------------------|------|------|-------|---------|-------|
| `object{N}_enabled` | Enabled | Bool | off/on | on (1-4), off (5-12) | First 4 objects enabled by default. |
| `object{N}_time` | Time | Float | 1.0 -- 2000.0 ms | N * 100 ms | Skew 0.3. Object 1 = 100 ms, Object 2 = 200 ms, ..., Object 12 = 1200 ms. |
| `object{N}_azimuth` | Azimuth | Float | -180.0 -- 180.0 | (see table) | Degrees. 0 = front, positive = right. |
| `object{N}_elevation` | Elevation | Float | -90.0 -- 90.0 | 0.0 | Degrees. Positive = above ear level. |
| `object{N}_distance` | Distance | Float | 0.0 -- 1.0 | 0.5 | Normalized. 0 = center, 1 = outer ring. |
| `object{N}_dopplerAmount` | Doppler Amount | Float | 0.0 -- 1.0 | 0.0 | Displayed as 0-100%. 0 = off. |
| `object{N}_trajectoryShape` | Trajectory Shape | Choice | 0-5 | 0 | 0=None, 1=Spiral, 2=Orbit, 3=Bounce, 4=Figure-8, 5=Random |
| `object{N}_trajectorySpeed` | Trajectory Speed | Float | 0.0 -- 10.0 | 1.0 | Cycles per second (approximately). |

## Default Azimuth Values

| Object | Default Azimuth |
|--------|----------------|
| 1 | -45.0 |
| 2 | +45.0 |
| 3 | -135.0 |
| 4 | +135.0 |
| 5 | 0.0 |
| 6 | +90.0 |
| 7 | -90.0 |
| 8 | +180.0 |
| 9 | -30.0 |
| 10 | +30.0 |
| 11 | -60.0 |
| 12 | +60.0 |

Objects 1-4 form a diagonal cross pattern and are **enabled by default**. Objects 5-12 are disabled by default.

## See Also

- [Spatialization Algorithms](algorithms-guide.md) -- how the algorithm parameter maps to rendering behavior
- [Trajectory Animation](trajectory-system.md) -- details on trajectory shapes and speed
