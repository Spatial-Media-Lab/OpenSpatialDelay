---
name: synthesis-techniques
description: >
  Expert in audio synthesis techniques for JUCE C++ audio plugin development.
  Covers 10 synthesis methods (FM, subtractive, wavetable, granular, additive,
  physical modeling, formant, vector, AM/ring mod, sample-based) with mathematical
  foundations, core synthesizer architecture (VCO/VCF/VCA/Envelope/LFO/mod matrix),
  band-limited oscillator generation (PolyBLEP, BLIT, MinBLEP, wavetable mipmap),
  filter topologies (Moog ladder, SVF, Sallen-Key, ZDF), and polyphonic voice management
  (allocation, stealing, unison detuning, Super Saw).
  Activate on 'synthesis', 'synthesizer', 'oscillator', 'VCO', 'VCF', 'VCA', 'FM synthesis',
  'subtractive', 'wavetable', 'granular', 'additive synthesis', 'physical modeling',
  'Karplus-Strong', 'waveguide', 'formant', 'LPC', 'vector synthesis', 'sample playback',
  'PolyBLEP', 'BLIT', 'MinBLEP', 'anti-aliasing', 'band-limited', 'filter', 'Moog ladder',
  'state variable filter', 'zero-delay feedback', 'ZDF', 'ADSR', 'envelope', 'LFO',
  'modulation matrix', 'voice allocation', 'unison', 'Super Saw', 'polyphony',
  'pulse width modulation', 'hard sync', 'ring modulation'.
  NOT for spatial audio (use spatial-audio-dsp), reverb algorithms (use reverb-algorithms),
  or time-based effects like chorus/flanger (use time-based-effects).
allowed-tools: Read,Write,Edit,Bash,Glob,Grep,WebFetch,WebSearch
metadata:
  category: Development & Engineering
  pairs-with:
    - skill: dsp-cookbook
      reason: Production-ready DSP code implementations
    - skill: juce-best-practices
      reason: JUCE-specific patterns and realtime safety
    - skill: time-based-effects
      reason: Modulation effects share oscillator/filter concepts
  tags:
    - synthesis
    - dsp
    - oscillator
    - filter
    - juce
    - audio-plugin
    - c++
---

# Audio Synthesis Techniques

## 1. FM Synthesis (Frequency Modulation)

**Core equation (Chowning, 1973, US Patent 4,018,121):**
```
y(t) = A · sin(2π·f_c·t + I · sin(2π·f_m·t))
```
- Sidebands at `f_c ± n·f_m`, amplitudes follow Bessel functions `J_n(I)`
- Significant sidebands ≈ `I + 1`
- Integer C:M ratios → harmonic spectra; non-integer → inharmonic/metallic
- DX7 uses phase modulation (PM), not true FM. Identical spectra for steady-state.

**Operator topology:** 6-op with 32 algorithms (DX7). Feedback FM: `y[n] = sin(2π·f·n/sr + fb·y[n-1])` → sawtooth-like at high fb.

**CPU:** Very cheap. ~6 sine lookups + 6 envelope mults per voice. Use 4096-point lookup table with linear interp (~-80dB THD). Anti-alias via 2-4x oversampling.

**Key implementations:** Yamaha DX7 (1983), OPL2/3, NI FM8, Dexed (open-source).

## 2. Subtractive Synthesis

**Signal flow:** Oscillator(s) → Mixer → Filter → VCA → Output, with Envelope/LFO modulation.

**Waveforms:**
- Saw: all harmonics, `1/n` amplitude
- Square: odd harmonics, `1/n` amplitude
- Pulse (PWM): `sin(nπd)/(nπ)`, d = duty cycle. LFO on d → classic PWM sound.
- Triangle: odd harmonics, `1/n²` amplitude

**Band-limited generation (critical for digital):**

*PolyBLEP (recommended for real-time):*
```cpp
float polyblep(float t, float dt) {
    if (t < dt) { t /= dt; return t+t - t*t - 1.0f; }
    else if (t > 1.0f - dt) { t = (t-1.0f)/dt; return t*t + t+t + 1.0f; }
    return 0.0f;
}
// Apply: naive_saw -= polyblep(phase, dt);
```

*BLIT:* `blit(t) = (M/P)·sin(πMt/P)/sin(πt/P)`. Integrate with leaky integrator for saw.

*Wavetable mipmap:* Pre-compute band-limited tables per octave. ~10-12 tables.

*MinBLEP:* Better alias rejection than PolyBLEP, requires 16-64 sample residual buffer.

## 3. Wavetable Synthesis

**Playback:** Read single-cycle waveform at `phase_increment = f·N/sr`. Crossfade between frames:
```
output = (1-frac)·interpolate(W[frame_lo], phase) + frac·interpolate(W[frame_hi], phase)
```

**Anti-aliasing:** Mipmap approach — FFT each frame, zero harmonics above Nyquist per octave band, IFFT. ~10-12 versions per frame.

**Interpolation:** Cubic Hermite (Catmull-Rom) minimum for quality. 4-point, ~10 mults.

**Key implementations:** PPG Wave (1981), Serum (2014), Vital (2020, open-source).

## 4. Granular Synthesis

**Grain equation:** `grain(t) = w(t-t_0) · s(t-t_0+offset)`, output = sum of all active grains.

**Parameters:** Duration (1-100ms), density (1-1000/sec), pitch, position, envelope (Gaussian/Hann/trapezoid), spray/scatter.

**Time-stretch:** Advance read position by `1/S` per grain. **Pitch-shift:** `rate = 2^(P/12)`.

**Synchronous:** Regular grain rate → pitch-shifting/time-stretching. **Asynchronous:** Stochastic → cloud textures.

**CPU:** Scales with `density × duration × interpolation`. 100+ simultaneous grains = significant. Optimize: pre-compute envelope tables, SIMD.

## 5. Additive Synthesis

**Fourier:** `y(t) = Σ A_n(t)·sin(2π·n·f_0·t + φ_n(t))`. Each partial needs independent amplitude envelope.

**Optimization:** IFFT resynthesis when N > ~64 partials: O(N log N) vs O(N²).

**Partial tracking:** STFT → peak detection → track across frames → `{f_n(t), A_n(t)}` pairs.

**CPU:** ~10-15 ops per partial per sample. 200 partials = ~3000 ops/sample/voice. Expensive for polyphony.

## 6. Physical Modeling

**Karplus-Strong (plucked string):**
```
Init: delay line with noise. Loop: output = D[read]; D[write] = (D[read]+D[read+1])/2
```
Pitch = `sr/N`. Averaging filter = progressive HF damping.

**Digital waveguide (Smith, CCRMA):** Bidirectional delay lines with boundary reflections. Extensions: bowed strings (nonlinear junction), wind instruments (bore models), 2D meshes.

**Modal synthesis:** Each mode = biquad resonator: `ÿ + 2ζωẏ + ω²y = F(t)`. Efficient for bells (20-50 modes), bars (10-20).

**FDTD:** 2D membrane grid. CFL stability: `λ ≤ 1/√2`. Very expensive (10K+ updates/sample).

**Key implementations:** Yamaha VL1 (1994), Pianoteq (Modartt), AAS Chromaphone.

## 7. Formant Synthesis

**Source-filter model (Fant, 1960):** Glottal pulse → vocal tract formants (F1-F5).

**Vowel formants (adult male):**
| Vowel | F1 | F2 | F3 |
|-------|-----|------|------|
| /a/ | 730 | 1090 | 2440 |
| /i/ | 270 | 2290 | 3010 |
| /u/ | 300 | 870 | 2240 |

**LPC:** All-pole filter `H(z) = G/(1 - Σ a_k·z^{-k})`. Cross-synthesis: one signal's envelope + another's excitation.

## 8. AM / Ring Modulation

**AM:** `y = x·(1 + d·m)` — carrier preserved, sidebands at `f_c ± f_m`.
**Ring mod:** `y = x·m` — carrier suppressed, only sum/difference frequencies. Non-integer ratios → inharmonic.

## 9. Vector Synthesis

**Bilinear interpolation (4 sources):**
```
gain_A = (1-u)(1-v), gain_B = u(1-v), gain_C = (1-u)v, gain_D = uv
```
Where u,v ∈ [0,1] from joystick/envelope. Gains sum to 1.0.

**Key implementations:** Sequential Prophet VS (1986), Korg Wavestation (1990).

## 10. Sample-Based Synthesis

**Multisampling:** Key zones + velocity layers + round-robin. `playback_rate = 2^((note-root)/12)`.

**Crossfade looping:** `output = (1-α)·buffer[pos] + α·buffer[loop_start + offset]`.

**Streaming:** Preload attack (64-256KB), stream sustain/release from disk (SSD required).

---

## Core Architecture

### VCO/DCO
- **Analog modeling:** Add drift `f·(1 + drift·noise_lp(t))`, soft transitions, DC offset
- **Hard sync:** Reset slave phase on master cycle completion
- **Sub-oscillator:** Frequency division (toggle per master cycle)

### VCF — Filter Topologies

**Moog Ladder (24dB/oct):**
```
y1 += g·(tanh(in - k·tanh(y4)) - tanh(y1))  // k=4 → self-oscillation
y2 += g·(tanh(y1) - tanh(y2))
y3 += g·(tanh(y2) - tanh(y3))
y4 += g·(tanh(y3) - tanh(y4))
```

**State Variable Filter (12dB/oct, Oberheim SEM):**
```
HP = in - 2Q·BP - LP;  BP += f·HP;  LP += f·BP
```
Simultaneous LP/BP/HP/Notch outputs. Continuous morph.

**Zero-Delay Feedback (Zavalishin, "Art of VA Filter Design"):**
```cpp
float v = g / (1.0f + g);
float y_new = y_state + v * (input - y_state);
y_state = 2.0f * y_new - y_state;
```
Eliminates one-sample feedback delay. Modern standard.

**TB-303 (18dB/oct diode ladder):** Sharper diode nonlinearity, accent = simultaneous drive + resonance boost.

**MS-20 (Sallen-Key):** Aggressive diode clipping in feedback. Screaming resonance character.

### Envelopes

**Exponential ADSR (natural-sounding):**
```
level += (target - level) · (1 - exp(-1/(time · sr)))
```

**Analog RC curves:** `V(t) = V_target·(1 - e^{-t/RC})` for charge, `V·e^{-t/RC}` for discharge.

**Curve parameter:** `y = t^c` — c<1 logarithmic, c=1 linear, c>1 exponential.

**Retrigger modes:** Retrigger (reset), Legato (continue), Multi-trigger (from current level).

### LFO
Waveforms: sine, triangle, saw, square, S&H, smooth random.
Sync: free-running, key sync, tempo sync.
Fade-in: `min(1, time_since_noteon / fade_time)`.

### Modulation Matrix
Per slot: `{source_id, destination_id, amount, bipolar}`.
```cpp
for (auto& slot : matrix)
    if (slot.dest == target) mod += getSource(slot.src) * slot.amount;
```

---

## Voice Management

### Allocation Strategies
- **Last note priority:** Most common mono mode
- **Oldest/Quietest steal:** For polyphonic voice stealing
- **Same-note priority + release-phase first:** Recommended combined strategy

### Unison Detuning
**Linear:** `detune[i] = -D + 2D·i/(N-1)`
**Super Saw (Roland JP-8000):** 7 voices, center + 3 pairs, nonlinear amplitude/detune curve per Adam Szabo analysis.
**Stereo spread:** `pan[i] = -spread + 2·spread·i/(N-1)`

### Soft Voice Stealing
Apply 1-5ms fade-out before reassigning stolen voice. Prevents clicks.

### CPU Scaling (relative per voice)
| Type | Cost | 16 voices |
|------|------|-----------|
| FM 6-op | 1x | 16x |
| Subtractive | 1.5x | 24x |
| Wavetable | 1x | 16x |
| Physical model | 3-5x | 48-80x |
| Additive (100) | 10x | 160x |
| Granular (50) | 15x | 240x |

---

## Key References

1. Chowning (1973) — FM Synthesis, JAES 21(7)
2. Karplus & Strong (1983) — Plucked string, CMJ 7(2)
3. Smith (1992) — Digital waveguides, CMJ 16(4)
4. Roads (2001) — Microsound (granular), MIT Press
5. Zavalishin (2018) — Art of VA Filter Design, NI
6. Stilson & Smith (1996) — Alias-free synthesis, ICMC
7. Szabo (2010) — Super Saw analysis
8. Zolzer (2011) — DAFX, 2nd Ed., Wiley
9. Fant (1960) — Acoustic Theory of Speech Production
10. Gabor (1947) — Acoustical quanta, Nature
