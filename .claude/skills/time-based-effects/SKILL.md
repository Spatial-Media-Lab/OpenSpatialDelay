---
name: time-based-effects
description: >
  Expert in time-based audio effects for JUCE C++ audio plugin development.
  Covers chorus (multi-voice, stereo, BBD modeling), flanger (comb filter analysis,
  through-zero flanging), phaser (allpass chains, barber-pole), pitch shifting
  (10 methods: OLA, SOLA, PSOLA, WSOLA, phase vocoder, granular, dual-head crossfade,
  formant-preserving), comb filtering (feedforward/feedback/allpass, Karplus-Strong),
  vibrato, tremolo, ring modulation, and rotary speaker simulation.
  Includes mathematical formulations, historical context (tape era through modern DSP),
  interpolation methods for modulated delays, and key patents/papers.
  Activate on 'chorus', 'flanger', 'phaser', 'pitch shift', 'pitch shifting', 'vibrato',
  'tremolo', 'ring modulation', 'comb filter', 'allpass filter', 'BBD', 'bucket brigade',
  'through-zero', 'phase vocoder', 'PSOLA', 'WSOLA', 'OLA', 'overlap-add', 'time stretch',
  'time-based effect', 'modulated delay', 'Leslie', 'rotary speaker', 'flanging',
  'Dimension D', 'CE-1', 'Phase 90', 'Uni-Vibe', 'Eventide', 'H910', 'H3000',
  'dual-head', 'crossfade pitch'.
  NOT for spatial audio (use spatial-audio-dsp), reverb (use reverb-algorithms),
  or synthesis techniques (use synthesis-techniques).
allowed-tools: Read,Write,Edit,Bash,Glob,Grep,WebFetch,WebSearch
metadata:
  category: Development & Engineering
  pairs-with:
    - skill: dsp-cookbook
      reason: Production-ready effect implementations
    - skill: reverb-algorithms
      reason: Shared comb/allpass foundations
    - skill: synthesis-techniques
      reason: Shared oscillator/modulation concepts
  tags:
    - chorus
    - flanger
    - phaser
    - pitch-shifting
    - dsp
    - juce
    - audio-plugin
    - c++
---

# Time-Based Audio Effects

## 1. Chorus

**Single-voice:** `y(t) = x(t) + g·x(t - d(t))`, where `d(t) = d_0 + A·sin(2πf_LFO·t)`

**Parameters:** Base delay 10-30ms, depth 1-5ms, LFO 0.5-3Hz.

**Pitch deviation:** `cents_max = 1200·log2(1 + 2πf_LFO·A)`. At A=2ms, f=1Hz: ~22 cents.

**Multi-voice (N voices, equidistant phases):**
```
d_k(t) = d_0k + A_k · sin(2πf_k·t + 2πk/N)
```

**Stereo (inverted LFO):**
```
y_L = x + g·x(t - d_0 - A·sin(ωt))
y_R = x + g·x(t - d_0 + A·sin(ωt))
```

**BBD modeling characteristics:**
- Clock feedthrough → HF noise (filter with reconstruction LP)
- Charge transfer loss → cumulative lowpass
- Companding (NE571) → dynamic range coloring
- Clock rate modulation (not interpolation) for delay changes

**Key circuits:** Roland CE-1/CE-2 (MN3002 BBD), Dimension D (SDD-320, 4 fixed presets), Juno-60 built-in.

## 2. Flanger

**With feedback:** `y[n] = x[n] + g_ff·x[n-M(n)] + g_fb·y[n-M(n)]`

**Parameters:** Base delay 0-5ms, depth 0-5ms, LFO 0.05-2Hz, feedback ±0.95 max.

**Comb filter analysis:**
- Feedforward: `H(z) = 1 + g·z^{-M}`. Peaks at `f = k·fs/M`, nulls at `(2k+1)·fs/(2M)`.
- Feedback: `H(z) = 1/(1 - g·z^{-M})`. Resonant peaks. Q increases as |g|→1.

**Feedback sign:**
- Positive: Peaks at harmonics. Bright, ringing.
- Negative: Peaks shifted by half period. Hollow, metallic.

**Through-Zero Flanging:** `d(t) = A·sin(ωt)` (no offset). At d=0, complete cancellation. Implementation: delay dry by A, sweep wet 0→2A.

**Key distinction from chorus:** Shorter delay (0-10ms vs 10-30ms), feedback is critical, deeper sweep.

**History:** Tape flanging (Abbey Road, ADT by Ken Townsend). Electronic: Eventide FL201 (1975), MXR Flanger, Electric Mistress (1976).

## 3. Phaser

**First-order allpass:** `H(z) = (a + z^{-1})/(1 + a·z^{-1})`, where `a = (tan(πf_c/fs) - 1)/(tan(πf_c/fs) + 1)`

Flat magnitude, phase = -π/2 at f_c. |H|=1 always.

**Notch count:** 2 stages = 1 notch, 4 = 2, 6 = 3, 2N = N notches.

**Key distinction from flanger:** Notches are NOT harmonically spaced (non-integer relationship). More "organic" and "vocal" character.

**LFO modulation (logarithmic for perceptual uniformity):**
```
f_c(t) = f_min · (f_max/f_min)^((1 + sin(2πf_LFO·t))/2)
```

**Feedback** → resonant peaks at notch frequencies, vocal/wah quality.

**Barber-pole phaser:** Infinite ascending/descending via Hilbert transform + quadrature LFOs (Shepard tone illusion).

**Key circuits:**
- MXR Phase 90: 4 JFET allpass stages (2 notches). JFET nonlinearity = signature.
- MXR Phase 100: 6/12 stages switchable.
- Uni-Vibe: Lightbulb + 4 LDR cells. Thermal inertia → asymmetric LFO.

## 4. Pitch Shifting

### 4.1 OLA (Overlap-Add)
Segment → overlap-add with modified hop. Time-stretch α = H_s/H_a. Pitch: stretch + resample. **Artifacts:** Phase discontinuities.

### 4.2 SOLA (Synchronous OLA)
Cross-correlation for optimal overlap: `k* = argmax Σ y_prev[n+k]·x_new[n]`. Significant improvement.

### 4.3 PSOLA (Moulines & Charpentier, 1990, FR 2,611,065)
Center windows on pitch marks. Requires pitch detection → monophonic only. High quality for vocals.

### 4.4 WSOLA (Verhelst & Roelands, 1993)
Waveform-similarity positioning without pitch detection. Good on pitched + non-pitched.

### 4.5 Phase Vocoder (Flanagan & Golden, 1966)
STFT → phase propagation → resynthesis. Instantaneous frequency: `ω_k = 2πk/N + princarg(Δφ - 2πkH_a/N)/H_a`.

**Laroche & Dolson (1999):** Phase locking preserves inter-bin coherence.
**Transient-preserving:** Detect via spectral flux, reset phases during transients.

### 4.6 Granular Pitch Shifting
Extract grains → resample by 1/β → overlap-add. High density (>100/sec) + 50-75% overlap = smooth.

### 4.7 Dual-Head Crossfade (as used in OpenSpatialDelay)
Two read pointers offset by half-buffer:
```
readPos[h] += speed  // speed = 1.0 - 2^(semitones/12)
fade[h] = sin²(π · phase[h])  // constant power
output = Σ read(readPos[h]) · fade[h]
```
- Very low latency (10-50ms), very low CPU
- Warm chorused character at crossfade boundaries
- Practical range: ±7 semitones
- 250ms window in OpenSpatialDelay

**Eventide lineage:** H910 (1975, single pointer + splice), H949 (1977, dual-head + zero-crossing), H3000 (1986, multiple shifters + feedback).

### 4.8 Formant-Preserving
**LPC:** Extract envelope → pitch-shift excitation → re-filter.
**Cepstral:** Low quefrency = formants, high = pitch. Modify high, keep low.
**True Envelope (Roebel, 2010):** Iterative spectral peak fitting (IRCAM SuperVP).

### Comparison Table

| Method | Quality | Latency | CPU | Polyphonic | Range |
|--------|---------|---------|-----|-----------|-------|
| OLA | Low-Med | Low | Very Low | Yes (poor) | ±12st |
| SOLA | Medium | Low-Med | Low | Marginal | ±12st |
| PSOLA | High (mono) | Med | Low | No | ±12st |
| WSOLA | Med-High | Med | Medium | Yes | ±12st |
| Phase Vocoder | Med-High | High | High | Yes | ±24st |
| PV + phase lock | High | High | High | Yes | ±24st |
| Granular | Variable | Variable | Medium | Yes | Unlimited |
| Dual-head | Medium | Very Low | Very Low | Yes | ±7st |

## 5. Comb Filtering

**Feedforward (FIR):** `y[n] = x[n] + g·x[n-M]`
- Peaks at `k·fs/M`, nulls at `(2k+1)·fs/(2M)`. Always stable.

**Feedback (IIR):** `y[n] = x[n] + g·y[n-M]`
- Resonant peaks at `k·fs/M`. RT60 = `-3M/(fs·log10|g|)`. Must have |g|<1.

**Allpass:** `y[n] = -g·x[n] + x[n-M] + g·y[n-M]`
- Flat magnitude, frequency-dependent phase. Reverb building block.

**Karplus-Strong:** Feedback comb + averaging LP = plucked string. `f_0 = fs/M`. Fractional delay via allpass interpolation.

## 6. Additional Effects

### Vibrato
Pure wet signal (no dry): `y(t) = x(t - d(t))`. Vocal: 5-7Hz, 50-100 cents. String: 5-8Hz, 20-50 cents.

### Tremolo
AM at sub-audio rate: `y(t) = x(t)·(1 - d/2 + d/2·sin(2πf·t))`. Stereo = opposite-phase (autopan).

### Ring Modulation
`y = x·cos(2πf_c·t)` → sum/difference frequencies only, carrier suppressed. Non-integer ratios → inharmonic.
**vs AM:** AM preserves carrier (`y = x·(1+d·m)`), ring mod does not.

### Rotary Speaker (Leslie)
Doppler + AM from rotating horn (treble) + drum (bass). Slow ~0.8Hz, Fast ~6.5Hz with acceleration ramp.

---

## Interpolation for Modulated Delays

| Method | Points | Quality | Use |
|--------|--------|---------|-----|
| Linear | 2 | -3dB at fs/4 | Prototyping only |
| Cubic Hermite | 4 | Flat to ~0.8×Nyquist | **Recommended minimum** |
| Lagrange 5th | 6 | Flat to ~0.9×Nyquist | High quality |
| Sinc (8-pt) | 8 | Excellent | Highest quality |
| Thiran allpass | Variable | Flat (allpass) | Waveguides |

**Implementation (Catmull-Rom):**
```cpp
float hermite(float f, float y0, float y1, float y2, float y3) {
    float c0=y1, c1=0.5f*(y2-y0);
    float c2=y0-2.5f*y1+2.0f*y2-0.5f*y3;
    float c3=0.5f*(y3-y0)+1.5f*(y1-y2);
    return ((c3*f+c2)*f+c1)*f+c0;
}
```

---

## Historical Timeline

| Era | Period | Key Developments |
|-----|--------|-----------------|
| Tape | 1940s-60s | Les Paul experiments, ADT (Abbey Road), tape flanging |
| Analog BBD | 1970s-80s | SAD-1024, MN3xxx, CE-1, Phase 90, Dimension D |
| Early Digital | 1975-90 | H910 (1975), H949, PCM42, SPX90, H3000 (1986) |
| Software | 1990s-2000s | VST (1996), AU (2002), Waves, Soundtoys |
| Modern | 2010s+ | Circuit modeling, neural networks, real-time granular |

---

## Key Patents & Papers

| Reference | Subject |
|-----------|---------|
| US 4,018,121 (Chowning, 1977) | FM synthesis |
| US 3,992,584 (Agnello/Eventide, 1976) | H910 pitch shifting |
| FR 2,611,065 (Moulines, 1988) | PSOLA |
| US 4,984,276 (Smith, 1991) | Digital waveguide |
| Flanagan & Golden (1966) | Phase vocoder, Bell System TJ |
| Dolson (1986) | Phase vocoder tutorial, CMJ 10(4) |
| Laroche & Dolson (1999) | Improved PV, IEEE Trans. |
| Verhelst & Roelands (1993) | WSOLA, IEEE ICASSP |
| Dattorro (1997) | Effect Design Pt 1, JAES 45(9) |
| Schroeder (1962) | Artificial reverb, JAES 10(3) |
| Zolzer (2011) | DAFX 2nd Ed., Wiley |
| Roads (2001) | Microsound, MIT Press |
| Valimaki & Laakso (2000) | Fractional delay filters |

---

## Implementation Recommendations

1. **Interpolation:** Cubic Hermite minimum for all modulated delays. Negligible extra cost vs linear.
2. **Delay line:** Power-of-2 buffer + bitmask (`& mask`). Pre-compute LFO per block.
3. **Feedback safety:** Always soft-clip in feedback path. Makeup gain for self-oscillation.
4. **Pitch shifting in delay plugins:** Dual-head crossfade — minimal latency, low CPU, warm character.
5. **Parameter smoothing:** LinearSmoothedValue or one-pole for delay time → musical "repitch" effect.
6. **Lock-free:** No malloc/mutex/syscalls in processBlock. Pre-allocate all buffers.
7. **Sample rate independence:** Delays in seconds → samples at current rate. Recalculate coefficients on rate change.
