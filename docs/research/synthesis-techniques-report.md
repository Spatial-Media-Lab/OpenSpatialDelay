# Comprehensive Technical Report: Synthesis Techniques and Architecture

## Part 1: Synthesis Techniques

---

### 1.1 FM Synthesis (Frequency Modulation)

#### Mathematical Foundation

John Chowning discovered FM synthesis at Stanford in 1967 (US Patent 4,018,121, filed 1975, granted 1977). The fundamental equation for a simple two-operator FM patch is:

```
y(t) = A · sin(2π·f_c·t + I · sin(2π·f_m·t))
```

Where:
- `f_c` = carrier frequency (Hz)
- `f_m` = modulator frequency (Hz)
- `I` = modulation index (peak phase deviation in radians)
- `A` = amplitude

The spectrum of a simple FM tone is described by Bessel functions of the first kind. The amplitude of the nth sideband is proportional to `J_n(I)`, where `J_n` is the nth-order Bessel function. The sidebands appear at frequencies:

```
f_sideband = f_c ± n·f_m,  for n = 0, 1, 2, 3, ...
```

The number of significant sidebands is approximately `I + 1`. As modulation index increases, energy spreads from the carrier into sidebands, creating progressively brighter and more complex spectra. This is the key insight: a single parameter (modulation index) controls spectral complexity continuously, from a pure sine (I=0) to a rich, harmonically dense tone.

**Carrier-to-modulator ratio (C:M ratio)** determines the harmonic structure:
- Integer ratios (1:1, 1:2, 2:1, 3:1) produce harmonic spectra
- Non-integer ratios produce inharmonic, metallic, bell-like timbres
- Ratio 1:1 produces all harmonics (similar to a sawtooth at high I)
- Ratio 1:2 produces only odd harmonics (similar to a square wave)

**Phase Modulation vs. True FM:** The Yamaha DX7 actually implements phase modulation (PM), not true frequency modulation. The difference:

```
True FM:   y(t) = sin(2π · ∫[f_c + I·f_m·sin(2π·f_m·t)] dt)
Phase Mod: y(t) = sin(2π·f_c·t + I·sin(2π·f_m·t))
```

For steady-state signals these produce identical spectra, but PM has advantages: (1) the carrier frequency remains constant regardless of modulation depth, making tuning stable; (2) it is simpler to implement digitally since it requires no integration; (3) feedback paths are more stable.

#### Operator Topology (Algorithms)

An "operator" is a sine oscillator with its own frequency, amplitude envelope, and output level. Operators are wired in configurations called "algorithms":

**2-op:** Simplest. One modulator, one carrier. Limited timbral range. Used in Yamaha OPL chips (SoundBlaster cards).

**4-op:** Intermediate complexity.

**6-op:** DX7 standard. 32 algorithms. Each algorithm is a fixed routing of modulators and carriers. Examples:
- Algorithm 1: `[6→5→4→3→2→1]` — full serial chain, extremely bright
- Algorithm 5: `[6→5→4→3→2]→1` — two parallel chains feeding one carrier
- Algorithm 32: `[1] [2] [3] [4] [5] [6]` — all carriers, additive synthesis

**8-op:** FM8 (Native Instruments) offers 8 operators with a fully flexible routing matrix rather than fixed algorithms.

#### Feedback FM

An operator can modulate itself via a one-sample delay feedback path:

```
y[n] = sin(2π·f·n/sr + fb·y[n-1])
```

Where `fb` is the feedback amount. This creates a progressively saw-like waveform as feedback increases. At maximum feedback, the waveform approaches noise. The DX7 allows one operator per algorithm to have self-feedback.

#### Multi-Carrier FM and Complex Spectra

When multiple carriers receive modulation simultaneously, their spectra combine additively. For a cascade of N modulators:

```
y(t) = sin(2π·f_c·t + I_1·sin(2π·f_m1·t + I_2·sin(2π·f_m2·t + ...)))
```

The spectral complexity grows combinatorially. Even 3 modulators in series can produce spectra with hundreds of significant partials.

#### CPU Considerations

FM synthesis is computationally cheap per voice: each operator is a single sine lookup plus an addition and multiplication. A 6-op voice requires approximately 6 sine evaluations, 6 envelope multiplications, and the routing additions per sample.

In software, FM is one of the cheapest synthesis methods. A 6-op, 16-voice FM engine processes in well under 1% CPU on modern hardware. The main cost drivers are: (1) sine computation (use lookup tables with linear or cubic interpolation; a 4096-point table with linear interpolation gives approximately -80dB THD); (2) envelope processing per operator per voice; (3) anti-aliasing (FM naturally generates aliasing when sidebands exceed Nyquist; oversampling at 2x or 4x is common).

#### Mono vs Unison vs Poly

**Mono:** Single voice FM. Portamento can be applied to all operator frequencies simultaneously. Legato mode keeps envelopes running on new notes.

**Unison:** Multiple detuned FM voices summed. Detuning the carrier frequencies while keeping modulator ratios locked produces thick chorus-like tones. 4-8 voices with 5-20 cent spread.

**Poly:** The DX7 had 16-voice polyphony. Each voice is independent with its own operator states, envelopes, and feedback history.

#### Notable Implementations

- **Yamaha DX7 (1983):** 6-op, 32 algorithms, 16-voice poly. Sold over 200,000 units.
- **Yamaha OPL2/OPL3 (1985-1992):** 2-op and 4-op FM for PC sound cards.
- **Native Instruments FM8:** 8-op with flexible matrix routing.
- **Dexed:** Open-source DX7 emulator.
- **Image-Line Sytrus:** 6-op FM with ring modulation and waveshaping.

---

### 1.2 Subtractive Synthesis

#### Signal Flow

The classic subtractive signal path:

```
Oscillator(s) → Mixer → Filter → Amplifier → Output
     ↑                    ↑          ↑
   Pitch Env          Filter Env   Amp Env
   LFO (vibrato)     LFO (wah)    LFO (tremolo)
```

#### Oscillator Waveforms

**Sawtooth:** Contains all harmonics with amplitudes `1/n`:
```
saw(t) = (2A/π) · Σ_{n=1}^{∞} (-1)^{n+1} · sin(2π·n·f·t) / n
```

**Square/Pulse:** Contains only odd harmonics with amplitudes `1/n`:
```
square(t) = (4A/π) · Σ_{n=1,3,5,...}^{∞} sin(2π·n·f·t) / n
```

**Pulse (variable width):** Pulse Width Modulation (PWM) varies the duty cycle `d` (0 to 1):
```
pulse(t, d) = (2A/π) · Σ_{n=1}^{∞} sin(π·n·d) · sin(2π·n·f·t) / n
```

**Triangle:** Contains only odd harmonics with amplitudes `1/n²`:
```
tri(t) = (8A/π²) · Σ_{n=1,3,5,...}^{∞} (-1)^{(n-1)/2} · sin(2π·n·f·t) / n²
```

#### Band-Limited Oscillator Generation

**PolyBLEP (Polynomial Band-Limited Step):**
The most popular real-time method. At each discontinuity in the waveform, apply a polynomial correction:

```cpp
float polyblep(float t, float dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t*t - 1.0f;
    } else if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t*t + t + t + 1.0f;
    }
    return 0.0f;
}
```

**BLIT (Band-Limited Impulse Train):**
```
blit(t) = (M/P) · sin(π·M·t/P) / sin(π·t/P)
```
Where P = period in samples, M = number of harmonics. Integration of BLIT gives a band-limited saw.

**Wavetable approach:** Pre-compute band-limited versions of each waveform at different frequency ranges (octave bands). Very efficient at runtime.

**MinBLEP (Minimum-phase Band-Limited Step):** Better alias rejection than PolyBLEP, requires a residual buffer (16-64 samples).

#### Filters

The filter is the soul of subtractive synthesis. Key parameters:
- **Cutoff frequency (f_c)**
- **Resonance (Q):** At maximum, the filter self-oscillates
- **Type:** Low-pass, high-pass, band-pass, notch
- **Slope:** 6dB/oct (1-pole), 12dB/oct (2-pole), 18dB/oct (3-pole), 24dB/oct (4-pole)

#### Mono vs Unison vs Poly

**Mono:** The Minimoog, TB-303, SH-101. Portamento:
```
f[n] = f[n-1] + (f_target - f[n-1]) · (1 - e^{-1/(sr·glideTime)})
```

**Unison:** The Roland JP-8000's "Super Saw" (Patent EP0965085B1, 1996) uses 7 detuned sawtooth oscillators:
```
frequencies = [f·(1-3d), f·(1-2d), f·(1-d), f, f·(1+d), f·(1+2d), f·(1+3d)]
```

**Poly:** Independent voice instances. Voice allocation strategies: oldest, quietest, same-note priority, low/high priority.

#### Notable Implementations
- **Minimoog (1970):** 3 VCO, Moog ladder 24dB/oct LPF
- **Roland TB-303 (1981):** 18dB/oct diode ladder LPF. Defined acid house.
- **Sequential Prophet-5 (1978):** First programmable poly synth.
- **Roland Jupiter-8 (1981):** 2 VCO per voice, multimode filter, 8-voice poly.
- **Software:** Diva (u-he), Monark (NI), Repro (u-he).

---

### 1.3 Wavetable Synthesis

#### Concept

A wavetable is an ordered collection of single-cycle waveforms (typically 256-2048 samples each). Playback reads through each waveform cyclically at the desired pitch, and the position within the table can be modulated to scan through different timbres.

#### Mathematical Description

For a wavetable with M frames, the table position `p` (0.0 to M-1) selects and crossfades between frames:

```
frame_lo = floor(p)
frame_hi = frame_lo + 1
frac = p - frame_lo
output[n] = (1-frac) · interpolate(W_{frame_lo}, phase[n])
          + frac · interpolate(W_{frame_hi}, phase[n])
```

#### Anti-Aliasing: Band-Limited Wavetables

The "mipmap" approach:
1. Analyze each wavetable frame via FFT
2. For each octave band, create a version with harmonics above Nyquist zeroed out
3. At runtime, select the appropriate band-limited version for the current playback frequency

#### Interpolation Methods

- **Linear:** Simple weighted average. Can produce audible stepping artifacts.
- **Cubic Hermite (Catmull-Rom):**
```cpp
float hermite(float frac, float y0, float y1, float y2, float y3) {
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}
```

#### Notable Implementations
- **PPG Wave (1981):** First commercial wavetable synth (Wolfgang Palm).
- **Xfer Serum (2014):** Modern wavetable powerhouse.
- **Matt Tytel's Vital (2020):** Open-source wavetable synth.
- **Ableton Wavetable (2018):** Built into Ableton Live.

---

### 1.4 Granular Synthesis

#### Theoretical Foundation

Granular synthesis, theorized by Dennis Gabor (1947) and developed by Iannis Xenakis (1960s) and Curtis Roads ("Microsound," MIT Press, 2001), decomposes sound into tiny fragments called "grains." Each grain is a short windowed segment (1-100ms). The ensemble of many overlapping grains creates complex textures.

```
grain(t) = w(t - t_0) · s(t - t_0 + offset)
output(t) = Σ_{i} grain_i(t)
```

#### Grain Parameters

- **Duration (1-100ms):** Short = buzzy/tonal; longer = preserves source character
- **Density (grains/second):** Low (1-10/s) = sparse textures; High (100-1000/s) = continuous
- **Pitch:** Each grain's playback rate independently controlled
- **Position:** Where in source material each grain reads from
- **Envelope shape:** Gaussian, Hann, trapezoid, Tukey
- **Spray/Scatter:** Random deviation on position, pitch, timing, panning

#### Synchronous vs Asynchronous

- **Synchronous:** Regular grain rate. Used for pitch-shifting and time-stretching.
- **Asynchronous:** Stochastic timing (Poisson). Cloud-like textures.

#### Granular Time-Stretching and Pitch-Shifting

Time and pitch are decoupled:
- **Time-stretch by S:** advance read position by `1/S` per grain period
- **Pitch-shift by P semitones:** `playback_rate = 2^(P/12)`

#### Notable Implementations
- **Ableton Granulator II (Robert Henke):** Max for Live granular instrument.
- **Output Portal:** Granular effects processor.
- **Tasty Chips GR-1:** Hardware granular synthesizer.

---

### 1.5 Additive Synthesis

#### Mathematical Foundation

```
y(t) = Σ_{n=1}^{N} A_n(t) · sin(2π · n · f_0 · t + φ_n(t))
```

For inharmonic sounds, partial frequencies are not integer multiples:
```
y(t) = Σ_{n=1}^{N} A_n(t) · sin(2π · f_n · t + φ_n(t))
```

#### Partial Tracking and Resynthesis

1. **Analysis:** Windowed FFT, track spectral peaks across frames
2. **Partial tracks:** `{f_n(t), A_n(t)}` time-varying pairs
3. **Resynthesis:** Oscillator bank with interpolated parameters
4. **Residual:** Difference = noise + transients

#### CPU Considerations

For N partials: ~10-15 operations per partial per sample. A realistic violin: 40-80 partials; complex bell: 200+.

**Optimization: IFFT resynthesis** — O(N log N) instead of O(N²). Efficient when N > ~64.

#### Notable Implementations
- **Kawai K5 (1987):** 128 harmonics per source.
- **Native Instruments Razor:** 320 partials, spectral domain filters.
- **Camel Audio Alchemy (now in Logic Pro).**

---

### 1.6 Physical Modeling

#### Karplus-Strong (Plucked String)

```
Initialize: fill delay line D[0..N-1] with random noise
Loop: output = D[read_pos]
      D[write_pos] = (D[read_pos] + D[read_pos + 1]) / 2
      advance read_pos, write_pos
```

Pitch: `f = sr / N`. The averaging filter progressively damps high frequencies on each pass.

**Refinements:**
- Fractional delay for precise tuning (allpass interpolation)
- Variable lowpass cutoff for brightness decay control
- Pluck position simulation via comb-filtered excitation
- Allpass filters for string stiffness/inharmonicity

#### Digital Waveguide Synthesis (Julius O. Smith, CCRMA)

Two delay lines (one per direction) with reflections at boundaries:

```
Right-traveling: --→ [delay line, N samples] --→ [reflection/filter] --→
Left-traveling:  ←-- [delay line, N samples] ←-- [reflection/filter] ←--
```

Extensions: bowed strings (nonlinear bow-string junction), wind instruments (bore models), 2D meshes for drums.

#### Modal Synthesis

Each mode is a damped harmonic oscillator:
```
mode_k(t): ÿ_k + (2ζ_k·ω_k)·ẏ_k + ω_k²·y_k = F(t)
```

Efficient for objects with finite important modes (bells: 20-50, bars: 10-20).

#### Finite Difference (FDTD)

For 2D membrane:
```
u[i,j,n+1] = 2·u[i,j,n] - u[i,j,n-1] + λ²·(u[i+1,j,n] + u[i-1,j,n] + u[i,j+1,n] + u[i,j-1,n] - 4·u[i,j,n])
```

CFL stability: `λ = c·dt/dx ≤ 1/√2` for 2D. Very expensive: 100×100 grid = 10,000 updates per sample.

#### Notable Implementations
- **Yamaha VL1 (1994):** First commercial PM synth. 2-voice polyphony.
- **Pianoteq (Modartt):** Piano physical model, no samples.
- **Applied Acoustics Chromaphone / Collision.**
- **Madrona Labs Kaivo:** Granular excitation through resonator models.

---

### 1.7 Formant Synthesis

#### Vocal Tract Modeling

Source-filter model (Gunnar Fant, 1960): glottal pulse train → vocal tract resonances (formants).

**Formant frequencies (Peterson & Barney, 1952, adult male):**

| Vowel | F1 (Hz) | F2 (Hz) | F3 (Hz) |
|-------|---------|---------|---------|
| /a/ (father) | 730 | 1090 | 2440 |
| /i/ (beet) | 270 | 2290 | 3010 |
| /u/ (boot) | 300 | 870 | 2240 |
| /e/ (bet) | 530 | 1840 | 2480 |
| /o/ (bought) | 570 | 840 | 2410 |

#### Implementation: Cascade vs Parallel

**Cascade:** Formant filters in series. Natural for vowels.
**Parallel:** Formant filters with independent amplitudes. Better for nasals/fricatives.

#### LPC (Linear Predictive Coding)

All-pole filter: `H(z) = G / (1 - Σ_{k=1}^{p} a_k · z^{-k})`

**Cross-synthesis:** Apply one signal's LPC filter to another signal's excitation → "talking instrument."

#### Morphing Between Vowels

Interpolate formant parameters:
```
F_k(t) = (1-α(t))·F_k_source + α(t)·F_k_target
```

#### Notable Implementations
- **CHANT (IRCAM, 1979):** FOF synthesis.
- **Vocaloid (Yamaha, 2003):** Hybrid concatenative/parametric singing synthesis.

---

### 1.8 AM Synthesis (Amplitude Modulation)

**AM:** `output = carrier × (1 + depth × modulator)`
Preserves carrier frequency. Produces sidebands at `f_c ± f_m` plus carrier.

**Ring Modulation:** `output = carrier × modulator`
No carrier in output — only sidebands at `f_c ± f_m`. Metallic, inharmonic tones.

The difference: AM preserves the carrier frequency, ring mod does not.

---

### 1.9 Vector Synthesis

#### Bilinear Interpolation (4 sources)

```
u = (x + 1) / 2,  v = (y + 1) / 2

gain_A = (1 - u) · (1 - v)
gain_B = u · (1 - v)
gain_C = (1 - u) · v
gain_D = u · v

output = gain_A·sourceA + gain_B·sourceB + gain_C·sourceC + gain_D·sourceD
```

Gains always sum to 1.0. A vector envelope defines (x, y) as a function of time.

#### Notable Implementations
- **Sequential Prophet VS (1986):** First vector synth.
- **Korg Wavestation (1990):** Wave sequencing + vector synthesis.

---

### 1.10 Sample-Based / ROMpler

#### Multi-Sample Architecture

**Key zones:** Keyboard divided into regions, each with a different sample:
```
playback_rate = 2^((midi_note - root_note) / 12)
```
Practical zone width: ~3-4 semitones.

**Velocity layers:** Multiple samples per zone triggered by different velocity ranges.

**Round-robin:** Cycle through N variations to avoid "machine gun" effect.

**Crossfade looping:**
```
output = (1-α) · buffer[pos] + α · buffer[loop_start + offset]
```

**Streaming vs RAM:** RAM for instant access; disk streaming for massive libraries (Kontakt: preload buffer + background streaming).

#### Notable Implementations
- **Akai S-series (1986-2000):** Defined hip-hop production.
- **Native Instruments Kontakt (2002):** Industry standard software sampler.
- **Decent Sampler (David Hilowitz):** Free, open-source sampler plugin.

---

## Part 2: Core Synthesis Architecture

---

### 2.1 Oscillators (VCO/DCO)

#### Analog vs Digital

**Analog (VCO):** Component drift (±5-20 cents), DC offset, soft transitions, component nonlinearity.
**Digital (DCO):** Crystal-accurate pitch. Perfect stability.
**Virtual Analog (VA):** Digital oscillators designed to sound analog: drift modeling, oversampled band-limited oscillators, component nonlinearity modeling.

#### Hard Sync

Slave oscillator reset to zero phase when master completes a cycle:
```cpp
if (masterPhase >= 1.0f) {
    masterPhase -= 1.0f;
    slavePhase = masterPhase * (slaveFreq / masterFreq);
}
```

Creates harmonics at multiples of master frequency with spectral envelope peak at slave frequency.

#### Pulse Width Modulation

Modulating `w` with an LFO creates the classic PWM sound. Spectrum: `sin(n·π·w)/(n·π)`.

---

### 2.2 Filters (VCF)

#### Moog Ladder Filter (4-pole, 24dB/oct)

```
Stage 1: y1 = y1 + g·(tanh(input - k·tanh(y4)) - tanh(y1))
Stage 2: y2 = y2 + g·(tanh(y1) - tanh(y2))
Stage 3: y3 = y3 + g·(tanh(y2) - tanh(y3))
Stage 4: y4 = y4 + g·(tanh(y3) - tanh(y4))
```

The `tanh()` saturations provide warm, musical distortion and stabilize self-oscillation. At `k = 4`, the filter self-oscillates as a pure sine.

#### Korg MS-20 Filter (Sallen-Key)

Aggressive clipping in feedback path. High resonance creates screaming overtones (unlike smooth Moog).

#### Roland TB-303 Filter (18dB/oct Diode Ladder)

The "squelchy" acid character from: diode saturation, accent circuit (increases drive AND resonance simultaneously), slide circuit interaction.

#### Oberheim SEM State Variable Filter (12dB/oct)

Simultaneous LP, BP, HP, notch outputs:
```
HPF = input - (2·Q·BPF) - LPF
BPF = BPF + f·HPF
LPF = LPF + f·BPF
```

#### Zero-Delay Feedback (ZDF) Filters

Uses trapezoidal integrator to eliminate one-sample feedback delay:
```cpp
float v = g / (1.0f + g);
float y_new = y_state + v * (input - y_state);
y_state = 2.0f * y_new - y_state;
```

Reference: Vadim Zavalishin, "The Art of VA Filter Design" (2018, Native Instruments).

---

### 2.3 Envelopes

#### ADSR — Mathematical Description

**Linear segments:**
```
Attack:  level += 1.0 / (attack_time · sr)
Decay:   level -= (1.0 - sustain) / (decay_time · sr)
Release: level -= current_level / (release_time · sr)
```

**Exponential segments (more natural):**
```
level = level + (target - level) * coefficient
coefficient = 1 - exp(-1 / (time_constant · sr))
```

**Analog envelope behavior (RC circuits):**
```
Charging (attack):    V(t) = V_target · (1 - e^{-t/RC})
Discharging (decay):  V(t) = V_current · e^{-t/RC}
```

#### Envelope Curves

- **Linear:** Constant rate. Sounds mechanical.
- **Exponential (concave):** Fast initial, slow approach. Natural for amplitude decay.
- **Logarithmic (convex):** Slow initial, fast approach. Good for filter sweeps.
- **S-curve:** `y = 3t² - 2t³` (Hermite smoothstep)
- **Customizable:** `y = t^c` where c < 1 = log, c = 1 = linear, c > 1 = exponential.

#### Retriggering Modes

- **Retrigger:** Reset envelope on new note.
- **Legato:** Continue envelopes, only change pitch.
- **Multi-trigger:** Retrigger from current level (no reset to zero).

---

### 2.4 Amplifier (VCA)

#### Linear vs Exponential

Human loudness perception is logarithmic:
```
amplitude = envelope_level^2     // simple power curve
// or
amplitude = (exp(cv * ln(range)) - 1) / (range - 1)  // true exponential
```

#### Velocity Sensitivity

```
Linear:       amp = velocity / 127
Exponential:  amp = (velocity / 127)^curve_power
```

---

### 2.5 Modulation Architecture

#### LFO

**Waveforms:** Sine, triangle, sawtooth, square, sample & hold, smooth random.

**Sync modes:** Free-running, key sync (reset on note-on), tempo sync.

**Fade-in:**
```cpp
float lfoFadeIn(float time_since_noteon, float fade_time) {
    if (time_since_noteon >= fade_time) return 1.0f;
    return time_since_noteon / fade_time;
}
```

#### Modulation Matrix

```
Source          → Amount → Destination
Envelope 1      →  +50% → Filter Cutoff
LFO 1           →  +10% → Oscillator Pitch (vibrato)
Velocity        →  +80% → Filter Cutoff
Mod Wheel       → +100% → LFO 1 Amount
```

Implementation per sample/block:
```cpp
float mod_value = 0.0f;
for (auto& slot : mod_matrix) {
    if (slot.destination == FILTER_CUTOFF) {
        float source_val = getModSource(slot.source_id);
        mod_value += source_val * slot.amount;
    }
}
final_cutoff = base_cutoff + mod_value * cutoff_range;
```

#### Key Tracking

```
cutoff_actual = cutoff_base + key_track_amount × (midi_note - 60) × (cutoff_range / 60)
```

---

## Part 3: Mono vs. Unison vs. Poly

### 3.1 Mono Voice Architecture

#### Portamento

**Exponential asymptotic (RC-style):**
```cpp
current_pitch += (target_pitch - current_pitch) * (1.0f - exp(-1.0f / (glide_time * sr)));
```

**Glide modes:** Always, legato only, rate-based, time-based.

#### Legato Triggering

Track the note stack:
```cpp
std::vector<int> held_notes;

void noteOn(int note) {
    bool was_empty = held_notes.empty();
    held_notes.push_back(note);
    if (was_empty) {
        triggerEnvelopes();
        setTargetPitch(note);
    } else {
        setTargetPitch(note);  // will glide if portamento enabled
    }
}

void noteOff(int note) {
    held_notes.erase(std::remove(...), held_notes.end());
    if (held_notes.empty()) {
        releaseEnvelopes();
    } else {
        setTargetPitch(held_notes.back());
    }
}
```

#### Note Priority Modes

- **Last note priority:** Most common. Return to previous held note on release.
- **Low note priority:** Classic for bass lines (Minimoog default).
- **High note priority.**

### 3.2 Unison Voice Architecture

#### Detune Spread Algorithms

**Linear:** `detune[i] = -D + 2·D·i/(N-1)` for i = 0..N-1

**Center-weighted (Super Saw style):** Central oscillator at nominal, outer pairs progressively more detuned with specific amplitude distribution.

**Exponential:** `detune[i] = sign(i-center) · D · (|i-center|/(N/2))^2`

#### Stereo Spread

```
pan[i] = -spread + 2·spread·i/(N-1)
```

#### Phase Management

On note trigger: random, zero (synchronized), or evenly spread phases.

### 3.3 Polyphonic Voice Architecture

#### Voice Stealing Strategies

**Release-phase first (recommended):**
```cpp
Voice* findBestToSteal(int newNote) {
    if (auto* v = findSameNote(newNote)) return v;
    if (auto* v = findQuietestReleasing()) return v;
    return findOldest();
}
```

#### Soft Stealing

Apply 1-5ms fade-out before reassigning stolen voice to prevent clicks.

#### CPU Scaling

| Synthesis Type | Per-Voice Cost (relative) | 16 voices | 64 voices |
|----------------|---------------------------|-----------|-----------|
| FM (6-op) | 1x | 16x | 64x |
| Subtractive (2 osc + filter) | 1.5x | 24x | 96x |
| Wavetable | 1x | 16x | 64x |
| Physical Model (string) | 3-5x | 48-80x | expensive |
| Additive (100 partials) | 10x | 160x | impractical |
| Granular (50 grains) | 15x | 240x | impractical |

---

## Key References

1. Chowning, J.M. (1973). "The Synthesis of Complex Audio Spectra by Means of FM." JAES 21(7).
2. Karplus, K. & Strong, A. (1983). "Digital Synthesis of Plucked-String and Drum Timbres." CMJ 7(2).
3. Smith, J.O. (1992). "Physical Modeling Using Digital Waveguides." CMJ 16(4).
4. Roads, C. (2001). "Microsound." MIT Press.
5. Zavalishin, V. (2018). "The Art of VA Filter Design." Native Instruments.
6. Gabor, D. (1947). "Acoustical Quanta and the Theory of Hearing." Nature 159(4044).
7. Fant, G. (1960). "Acoustic Theory of Speech Production." Mouton.
8. Stilson, T. & Smith, J.O. (1996). "Alias-Free Digital Synthesis of Classic Analog Waveforms." ICMC.
9. Valimaki, V. et al. "Oscillator and Filter Algorithms for Virtual Analog Synthesis." CMJ.
10. Brandt, E. (2001). "Hard Sync Without Aliasing." ICMC.
11. Szabo, A. (2010). "How to Emulate the Super Saw." (Analysis of Roland JP-8000)
12. Zolzer, U. (2011). "DAFX: Digital Audio Effects." 2nd Ed., Wiley.
