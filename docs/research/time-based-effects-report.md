# Comprehensive Technical Report: Pitch Shifting, Flanging, Chorus, Phaser, and Related Time-Based Audio Effects

---

## Part 1: Effect Definitions and Mathematics

### 1.1 Chorus

**Principle:** Simulates multiple performers with slight pitch/timing deviations. Signal duplicated through a modulated delay line and summed with original.

**Core Mathematics:**

Single-voice:
```
y(t) = x(t) + g * x(t - d(t))
d(t) = d_0 + A * sin(2π*f_LFO*t + φ_0)
```

Parameters:
- Base delay `d_0`: 10-30ms
- Modulation depth `A`: 1-5ms
- LFO rate `f_LFO`: 0.1-5Hz (typical: 0.5-3Hz)

**Instantaneous Pitch Deviation:**
```
d'(t) = A * 2π*f_LFO * cos(2π*f_LFO*t)
cents_max = 1200 * log2(1 + 2π*f_LFO*A)
```

For A=2ms, f_LFO=1Hz: ~21.7 cents — sub-semitone, producing characteristic beating without obvious detuning.

**Multi-Voice Chorus (N voices):**
```
y(t) = x(t) + Σ_{k=0}^{N-1} g_k * x(t - d_k(t))
d_k(t) = d_0k + A_k * sin(2π*f_LFO_k*t + 2π*k/N)
```

Three-voice: phase offsets 0°, 120°, 240°.

**Stereo Chorus (inverted LFO phase):**
```
y_L(t) = x(t) + g * x(t - d_0 - A*sin(2π*f_LFO*t))
y_R(t) = x(t) + g * x(t - d_0 + A*sin(2π*f_LFO*t))
```

**BBD (Bucket Brigade) Modeling:**
1. Clock feedthrough (HF noise)
2. Charge transfer loss (cumulative lowpass)
3. Companding noise reduction (NE571/SA571 coloring)
4. Anti-aliasing/reconstruction filters

**Notable Circuits:**
- **Roland CE-1/CE-2 (1976):** MN3002 BBD, NE571 compander
- **Roland Dimension D (SDD-320):** Four BBDs with fixed parameter presets. Button 4 legendary.
- **Juno-60 built-in:** Two modes, single BBD

---

### 1.2 Flanger

**Principle:** Short modulated delay (0-10ms) creates sweeping comb filter with characteristic "jet engine" whoosh.

**Mathematics:**
```
Feedforward:  y[n] = x[n] + g_ff * x[n - M(n)]
Feedback:     y[n] = x[n] + g_ff * x[n - M(n)] + g_fb * y[n - M(n)]
M(n) = M_0 + A * sin(2π*f_LFO*n/f_s)
```

Parameters: Base delay 0-5ms, depth 0-5ms, LFO 0.05-2Hz, feedback -0.95 to +0.95.

**Comb Filter Analysis:**

Feedforward: `H(z) = 1 + g_ff * z^{-M}`
- Peaks at `f = k * f_s/M`, nulls at `f = (2k+1) * f_s/(2M)`

Feedback: `H(z) = 1 / (1 - g_fb * z^{-M})`
- Resonant peaks at `f = k * f_s/M`. Q increases as `|g_fb| → 1`.

**Sign of Feedback:**
- Positive: Resonant peaks at harmonics. Bright, ringing.
- Negative: Peaks shifted by half period. Hollower, more metallic.

**Through-Zero Flanging (TZF):**
```
d(t) = A * sin(2π*f_LFO*t)    (no d_0 offset — sweeps through zero)
```
At d=0, complete cancellation. Implementation: delay dry signal by A, sweep wet from 0 to 2A.

**History:** Term from tape reel flange. Discovered 1940s-50s. Abbey Road ADT (Ken Townsend). Electronic: Eventide FL201 (1975), MXR Flanger, Electric Mistress (1976).

---

### 1.3 Phaser

**Principle:** Allpass-filtered signal summed with dry creates frequency-dependent notches. Unlike flanger, notches are NOT harmonically spaced.

**First-Order Allpass:**
```
H(z) = (a + z^{-1}) / (1 + a * z^{-1})
a = (tan(π*f_c/f_s) - 1) / (tan(π*f_c/f_s) + 1)
```

`|H| = 1` for all frequencies. Phase shift = -π/2 at f_c.

**N-Stage Phaser — Notch Count:**
- 2 stages: 1 notch
- 4 stages: 2 notches
- 6 stages: 3 notches
- 2N stages: N notches

**LFO Modulation (logarithmic for perceptual uniformity):**
```
f_c(t) = f_min * (f_max/f_min)^((1 + sin(2π*f_LFO*t))/2)
```

**Feedback** creates resonant peaks at notch frequencies, adding vocal "wah" quality.

**Barber-Pole Phaser:** Infinite ascending/descending via Hilbert transform + quadrature LFOs (Shepard tone illusion).

**Notable Circuits:**
- **MXR Phase 90:** 4 JFET allpass stages (2 notches). JFET nonlinearity = distinctive sound.
- **MXR Phase 100:** 6 and 12 stages switchable.
- **Uni-Vibe:** Lightbulb + 4 LDR photocells. Thermal inertia = asymmetric LFO.

---

### 1.4 Pitch Shifting

#### 1.4.1 Time-Domain Methods

**OLA (Overlap-Add):**
Segment input into frames, overlap-add with different hop size.
- Time-stretch ratio: `α = H_s / H_a`
- Pitch shift: time-stretch by α, resample by 1/α
- Artifacts: phase discontinuities at frame boundaries

**SOLA (Synchronous OLA):**
Cross-correlation for optimal overlap position:
```
R(k) = Σ y_prev[n+k] * x_new[n]
k* = argmax_k R(k)
```
Significantly reduces artifacts.

**PSOLA (Pitch-Synchronous OLA):**
Center windows on pitch marks. Pitch shift: compress/expand mark spacing.
- Requires reliable pitch detection → monophonic only
- High quality for vocals

**WSOLA (Verhelst & Roelands, 1993):**
Waveform-similarity-based positioning without pitch detection. Good on pitched and non-pitched signals.

#### 1.4.2 Frequency-Domain Methods

**Phase Vocoder:**
STFT analysis → phase propagation → modified resynthesis.

Instantaneous frequency:
```
ω_k = 2π*k/N + princarg(Δφ[k] - 2π*k*H_a/N) / H_a
```

For synthesis: `φ_synth_m[k] = φ_synth_{m-1}[k] + ω_k * H_s`

**Artifacts:** Transient smearing, phase coherence loss, metallic quality.

**Laroche & Dolson (1999) Improvements:** Phase locking preserves relationships between bins belonging to same partial.

**Transient-Preserving:** Detect via spectral flux, reset phases during transients.

#### 1.4.3 Granular Pitch Shifting

Extract grains → resample each by 1/β → overlap-add. High density (>100/sec) with 50-75% overlap = smooth results.

#### 1.4.4 Dual-Head Crossfade (as used in OpenSpatialDelay)

Two read pointers offset by half-buffer, Hann crossfade:
```
readPos[h] += speed    // speed = 1.0 - 2^(semitones/12)
fade[h] = sin²(π * phase[h])    // constant power
output = Σ read(readPos[h]) * fade[h]
```

Characteristics: Very low latency (10-50ms), low CPU, warm chorused artifacts at crossfade boundaries. Practical range: ±7 semitones.

**Eventide H910 (1975):** First commercial digital pitch shifter. Single pointer with splice. H949 (1977): dual-head with zero-crossing de-glitching. H3000 (1986): multiple pitch shifters with feedback paths.

#### 1.4.5 Formant-Preserving Pitch Shifting

**LPC method:** Extract spectral envelope via LPC → pitch-shift excitation → re-filter through original envelope.

**Cepstral method:** Low quefrency = formants, high quefrency = pitch. Modify high, keep low.

**True Envelope (Roebel, 2010):** Iterative spectral peak fitting. Used in IRCAM SuperVP.

---

### 1.5 Comb Filtering

**Feedforward (FIR):** `y[n] = x[n] + g * x[n-M]`
- Peaks at `f = k*f_s/M`, nulls at `f = (2k+1)*f_s/(2M)`

**Feedback (IIR):** `y[n] = x[n] + g * y[n-M]`
- Resonant peaks at `f = k*f_s/M`. RT60 = `-3M / (f_s * log10(|g|))`
- Must have `|g| < 1` for stability

**Allpass:** `y[n] = -g*x[n] + x[n-M] + g*y[n-M]`
- Flat magnitude, frequency-dependent phase. Building block for Schroeder reverb.

**Karplus-Strong:** Feedback comb + averaging LP = plucked string.
```
f_0 = f_s / M
```
Fractional delay via allpass interpolation for precise tuning.

---

### 1.6 Vibrato

Pure pitch modulation (no dry signal):
```
y(t) = x(t - d(t))
d(t) = d_0 + A * sin(2π*f_vib*t)
```

Musical parameters: Vocal vibrato 5-7Hz, 50-100 cents. String 5-8Hz, 20-50 cents.

**Key distinction from chorus:** Chorus = dry + wet (beating). Vibrato = wet only (pitch motion).

---

### 1.7 Tremolo

Amplitude modulation at sub-audio rates:
```
y(t) = x(t) * (1 - d/2 + d/2 * sin(2π*f_trem*t))
```

**Stereo Tremolo (Autopan):** Opposite-phase L/R modulation. Equal-power panning:
```
y_L = x * cos(θ(t)),  y_R = x * sin(θ(t))
```

**History:** Built into Fender amps from 1950s (mislabeled as "vibrato"). Optocoupler circuit (neon bulb + LDR).

---

### 1.8 Ring Modulation

```
y(t) = x(t) * cos(2π*f_c*t) = 0.5 * [cos(2π*(f_x+f_c)*t) + cos(2π*(f_x-f_c)*t)]
```

Only sum and difference frequencies — original frequencies absent. Non-integer carrier ratios = inharmonic, metallic timbres.

**vs AM:** Ring mod: modulator swings -1 to +1 (carrier suppressed). AM: modulator biased positive (carrier preserved).

**Frequency Shifter (Bode):** Single-sideband via Hilbert transform. Additive shift: `f_out = f_in + f_shift`.

---

### 1.9 Rotary Speaker (Leslie)

Doppler + amplitude modulation from rotating horn (treble) and drum (bass). Two speed modes: Slow (~0.8Hz), Fast (~6.5Hz) with characteristic acceleration/deceleration ramp.

---

## Part 2: Historical Development

### Tape Era (1940s-1960s)
- Les Paul: multitrack recording, tape delay, speed manipulation
- Echoplex (1959), Roland Space Echo RE-201 (1974)
- Tape flanging at Abbey Road. ADT by Ken Townsend for The Beatles.

### Analog BBD Era (1970s)
- BBD invented 1969 by Sangster & Teer at Philips
- Key ICs: Reticon SAD-1024, Panasonic MN3004/MN3007/MN3205
- MXR Phase 90 (1974), Boss CE-1 (1976), Electric Mistress (1976), Roland Dimension D (1979)

### Early Digital (1975-1990)
- **Eventide H910 (1975):** First digital pitch shifter. 8-bit/12kHz.
- **Eventide H949 (1977):** Dual-head de-glitching.
- **Lexicon PCM42 (1981):** 16-bit digital delay with modulation.
- **Yamaha SPX90 (1985):** Affordable digital multi-effects.
- **Eventide H3000 (1986):** Algorithm architecture. Micropitch = ±5-10 cents with short delays.

### Software Era (1990s-2000s)
- Steinberg VST (1996), Apple Audio Units (2002)
- Waves, Soundtoys, Eventide plugins, iZotope

### Modern Era (2010s-present)
- Circuit modeling / Virtual Analog (Arturia, Universal Audio)
- Machine learning neural models (WaveNet, LSTM)
- Real-time granular (Output Portal, Arturia Efx Fragments)

---

## Part 3: Patents and Published Research

### Key Patents

| Patent | Inventor | Year | Subject |
|--------|----------|------|---------|
| US 4,018,121 | John Chowning | 1977 | FM Synthesis |
| US 3,992,584 | Anthony Agnello (Eventide) | 1976 | Digital pitch shifting (H910) |
| US 4,267,581 | Anthony Agnello (Eventide) | 1981 | Improved pitch shifting (H949) |
| FR 2,611,065 | Moulines & Charpentier | 1988 | PSOLA |
| US 4,984,276 | Julius O. Smith III | 1991 | Digital waveguide synthesis |
| EP0965085B1 | Roland | 1996 | Super Saw oscillator (JP-8000) |
| US 3,546,490 | Sangster (Philips) | 1970 | BBD (Bucket Brigade Device) |

### Key Academic Papers

1. **Schroeder, M.R. (1962).** "Natural Sounding Artificial Reverberation." JAES 10(3), 219-223.
2. **Chowning, J.M. (1973).** "Synthesis of Complex Audio Spectra by FM." JAES 21(7), 526-534.
3. **Moorer, J.A. (1979).** "About This Reverberation Business." CMJ 3(2), 13-28.
4. **Karplus, K. & Strong, A. (1983).** "Digital Synthesis of Plucked-String and Drum Timbres." CMJ 7(2), 43-55.
5. **Smith, J.O. (1992).** "Physical Modeling Using Digital Waveguides." CMJ 16(4), 74-91.
6. **Flanagan, J.L. & Golden, R.M. (1966).** "Phase Vocoder." Bell System Technical Journal, 45, 1493-1509.
7. **Dolson, M. (1986).** "The Phase Vocoder: A Tutorial." CMJ 10(4), 14-27.
8. **Laroche, J. & Dolson, M. (1999).** "Improved Phase Vocoder Time-Scale Modification." IEEE Trans. Speech & Audio 7(3).
9. **Moulines, E. & Charpentier, F. (1990).** "Pitch-Synchronous Waveform Processing (PSOLA)." Speech Communication 9(5-6).
10. **Verhelst, W. & Roelands, M. (1993).** "WSOLA." IEEE ICASSP vol.2, 554-557.
11. **Dattorro, J. (1997).** "Effect Design Part 1." JAES 45(9), 660-684.
12. **Zolzer, U. (2011).** "DAFX: Digital Audio Effects." 2nd Ed., Wiley.
13. **Roads, C. (2001).** "Microsound." MIT Press.
14. **Puckette, M.** "Theory and Technique of Electronic Music."
15. **Valimaki, V. & Laakso, T.I. (2000).** "Principles of Fractional Delay Filters." IEEE ICASSP.
16. **Roebel, A. (2010).** "Shape-Invariant Speech Transformation." Interspeech.
17. **Bristow-Johnson, R.** Audio EQ Cookbook.

---

## Part 4: Comparison Tables

### Pitch Shifting Methods

| Method | Quality | Latency | CPU | Transients | Formant | Polyphonic | Max Range | Best Use |
|--------|---------|---------|-----|-----------|---------|-----------|-----------|----------|
| OLA | Low-Med | Low (10-50ms) | Very Low | Poor | No | Yes (poor) | ±12st | Simple apps |
| SOLA | Medium | Low-Med | Low | Improved | No | Marginal | ±12st | Speech |
| PSOLA | High (mono) | Med (20-50ms) | Low | Good | Preserved | No | ±12st | Vocals |
| WSOLA | Med-High | Med | Medium | Good | Partial | Yes | ±12st | General purpose |
| Phase Vocoder | Med-High | High (50-100ms) | High | Poor | No | Yes | ±24st | Large shifts |
| PV + phase lock | High | High | High | Medium | No | Yes | ±24st | Music production |
| PV + transient | Very High | High | Very High | Good | Optional | Yes | ±24st | Professional |
| Granular | Variable | Variable | Medium | Variable | No | Yes | Unlimited | Creative |
| Dual-head | Medium | Very Low | Very Low | Crossfade artifacts | No | Yes | ±7st | Real-time delay FX |

### Time-Based Effects Parameter Comparison

| Effect | Delay Range | Feedback | LFO | Notch Type | Perceptual Result |
|--------|------------|----------|-----|-----------|-------------------|
| Vibrato | 0-10ms (wet only) | No | 4-8Hz | None | Pitch wobble |
| Chorus | 10-30ms | Optional | 0.5-3Hz | Moving comb | Thickening, shimmer |
| Flanger | 0-10ms | Yes (critical) | 0.05-2Hz | Harmonic comb sweep | Jet engine, metallic |
| Phaser | N/A (allpass) | Optional | 0.1-5Hz | Non-harmonic | Warm sweep, vocal |
| Comb filter | 1-50ms (fixed) | Yes | Optional | Harmonic (static) | Resonant, metallic |

### Interpolation Quality for Modulated Delays

| Method | Points | CPU | Frequency Response | Suitable For |
|--------|--------|-----|-------------------|-------------|
| Linear | 2 | 2 mul + 1 add | -3dB at f_s/4 | Quick prototyping |
| Cubic Hermite | 4 | 10 mul + 10 add | Flat to ~0.8*Nyquist | Good general purpose |
| Lagrange 5th | 6 | ~20 mul | Flat to ~0.9*Nyquist | High quality |
| Sinc (8-point) | 8 | ~24 mul | Excellent | Highest quality |
| Thiran allpass | Variable | Variable (IIR) | Flat (allpass) | Waveguides |

---

## Implementation Recommendations

1. **Interpolation:** Use cubic Hermite (Catmull-Rom) minimum for modulated delay lines. Negligible extra cost vs linear, substantial quality gain.

2. **Delay line:** Power-of-2 buffers with bitmask indexing (`& mask`). Pre-compute LFO per block when possible.

3. **Feedback stability:** Always include soft clipper. Makeup gain for self-oscillation: `1.0 + fb² * 0.2` (as in OpenSpatialDelay).

4. **Pitch shifting for delay effects:** Dual-head crossfade is ideal — minimal latency, low CPU, warm character blends naturally with echo.

5. **Parameter smoothing:** `LinearSmoothedValue` or one-pole for delay time changes. Smoothed changes = musical "repitch" effect.

6. **Lock-free processing:** No malloc, no mutex, no system calls in processBlock. Pre-allocate, cache parameter pointers, atomics only where needed.

7. **Sample rate independence:** Express delays in seconds, convert to samples at current rate. Recalculate filter coefficients on rate change.
