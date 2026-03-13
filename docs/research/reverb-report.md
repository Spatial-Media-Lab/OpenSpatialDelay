# Comprehensive Technical Report on Reverb

## Algorithms, Impulse Response Resources, Historical Units, and CPU Analysis

---

## PART 1: REVERB TYPES AND ALGORITHMS

---

### 1.1 Schroeder Reverb (1961-1962)

Manfred Schroeder and Ben Logan at Bell Labs published the foundational work on digital artificial reverberation.

**Feedback Comb Filter:**
```
y[n] = x[n - M] + g * y[n - M]
```

RT60 relationship:
```
RT60 = -3 * M / (fs * log10(|g|))
g = 10^(-3 * M / (fs * RT60))
```

**Allpass Filter:**
```
y[n] = -g * x[n] + x[n - M] + g * y[n - M]
H(z) = (-g + z^{-M}) / (1 - g * z^{-M})
```

`|H(e^{jw})| = 1` for all frequencies when `|g| < 1`.

**Classic Topology:** 4 parallel feedback comb filters summed → 2 series allpass filters.

**Delay Length Selection:** Lengths must be mutually prime. Formula:
```
M_i = nextPrime(Mmin + i * (Mmax - Mmin) / (N - 1))
```

**Limitations:** Metallic coloration from comb peaks; no early reflections; linear echo density growth; allpass smearing.

**CPU Cost:** ~15-25 ops/sample. Extremely low.

---

### 1.2 Moorer Reverb (1979)

James Moorer's "About This Reverberation Business" (CMJ, 1979).

**Early Reflection Tapped Delay Line:** ~18 taps for first-order + second-order reflections from image source distances.

**Low-Pass Filters in Comb Feedback:**
```
y_f[n] = (1 - d) * x_f[n] + d * y_f[n-1]    (one-pole LPF)
y[n] = x[n - M] + g * y_f[n - M]
```

Creates frequency-dependent RT60: longer at low frequencies, shorter at high, matching real rooms.

---

### 1.3 Feedback Delay Network (FDN)

Formalized by Jean-Marc Jot and Antoine Chaigne (1991). The backbone of most modern algorithmic reverbs.

**Architecture:** N delay lines + N×N feedback matrix A + N absorption filters + input/output gains.

**Feedback Matrix: Stability**
Matrix `A` must be unitary: `A^T * A = I`.

*Hadamard (N = 2^k):*
```
H_{2k} = (1/sqrt(2)) * [ H_k   H_k  ]
                        [ H_k  -H_k  ]
```
Computable via fast Walsh-Hadamard in O(N log N).

*Householder:* `A = I - (2/N) * ones(N,N)`. Maximally distributes energy.

**Delay Length Selection:**
```
M_i = nextPrime(M_min * (M_max / M_min)^(i / (N-1)))
```

**Absorption Filters:**
```
|g_i(f)| = 10^(-3 * M_i / (fs * RT60(f)))
```

**Modulation:** Slow LFO (0.5-2 Hz, 1-8 samples depth) breaks metallic coloration.

**CPU Cost:**

| FDN Size | Ops/Sample |
|----------|-----------|
| 4-channel | 40-50 |
| 8-channel | 120-180 |
| 16-channel | 400-500 |
| 32-channel | 1400-1800 |

---

### 1.4 Dattorro Plate Reverb (1997)

Jon Dattorro's "Effect Design Part 1" (JAES, 1997). One of the most widely implemented reverb designs.

**Input Diffusion (4 series allpass):**
```
Input → bandwidth LPF → AP(142, 0.75) → AP(107, 0.75)
→ AP(379, 0.625) → AP(277, 0.625) → Tank Input
```

**Tank (Figure-8 Cross-Coupled Loops):**

Loop 1 (Left):
```
→ AP_mod(672, decay_diffusion_1) [modulated, LFO ~1Hz, ±8 samples]
→ Delay(4453) → Damping LPF → × decay_gain
→ AP(1800, decay_diffusion_2) → Delay(3720) → cross-feed to Loop 2
```

Loop 2 (Right): Similar with different delay lengths (908, 4217, 2656, 3163).

**Key Parameters:**
- `decay_gain` (0 to ~0.999): overall decay time
- `decay_diffusion_1` (~0.7): modulated allpass diffusion
- `decay_diffusion_2` (~0.5): non-modulated allpass diffusion
- `damping` (0-1): HF decay rate

**Stereo Output:** Multiple taps from both loops, some subtracted for decorrelation (14 taps total).

**CPU Cost:** ~60-80 ops/sample. Excellent quality-to-CPU ratio.

---

### 1.5 Convolution Reverb

**Impulse Response Capture via Exponential Sine Sweep:**
```
x(t) = sin( (2π·f1·T / ln(f2/f1)) · (exp(t · ln(f2/f1) / T) - 1) )
```

**Uniform Partitioned Convolution (Overlap-Save):**
Divide IR into B blocks of size N. Per block: 1 forward FFT(2N) + B complex vector multiplies + 1 inverse FFT(2N).

**Non-Uniform Partitioned Convolution (Gardner 1995):**
Small partitions for IR head (low latency), larger for tail (efficiency). 30-50% CPU reduction.

**Zero-Latency:** Time-domain convolution for first partition (N multiplies/sample), frequency-domain for remainder.

**True Stereo:** 4 IRs (LL, LR, RL, RR). Quadruples CPU.

**CPU Cost:**

| IR Length (48kHz) | Uniform | Non-Uniform |
|-------------------|---------|-------------|
| 0.5s (24K samples) | ~200 ops/sample | ~130 |
| 1.0s (48K samples) | ~390 | ~250 |
| 2.0s (96K samples) | ~770 | ~500 |
| 5.0s (240K samples) | ~960 | ~600 |

---

### 1.6 Physical Plate Modeling

**Kirchhoff Thin Plate Equation:**
```
ρh · d²u/dt² = -D · ∇⁴(u) + f(x,y,t)
```

**FDTD:** 13-point biharmonic stencil on 2D grid. CFL stability: `dt ≤ dx² / (2·κ)`. For 1m×2m plate at dx=1cm: ~260,000 ops/sample.

**Modal Decomposition:** 500-1000 modes as biquad filters. ~2,000-4,000 ops/sample. Much more practical.

---

### 1.7 Spring Reverb Modeling

Waves in helical springs exhibit dispersion: `v_phase(f) = v_0 · sqrt(1 + (f/f_c)²)`.

**Digital Model:** 50-200 allpass filters approximating measured group delay curve + feedback comb + ripple filter.

**CPU Cost:** ~200-500 ops/sample.

---

### 1.8 Shimmer Reverb

```
Input → [Reverb (FDN or Plate)] → wet output
              ↓
        [Pitch Shift (+12 st)] → [LPF] → [× feedback_gain] → back to input
```

Intervals: +12 (octave, classic), +7 (fifth, organ-like), +5 (fourth, mystical).

**CPU Cost:** ~400-1500 ops/sample.

---

### 1.9 Velvet Noise Reverb

Developed by Vesa Valimaki et al. at Aalto University. Sparse random impulses (2000/sec) perceptually equivalent to Gaussian noise for late reverb.

**CPU Cost:** ~50-200 ops/sample with high quality.

---

### 1.10 Freeverb (Jezar, 2000)

Open-source (public domain). 8 parallel lowpass-feedback comb filters + 4 series allpass, stereo via offset delays.

Comb delays (44.1kHz): 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617. Right channel +23 each.
Allpass delays: 556, 441, 341, 225.

**CPU Cost:** ~30-40 ops/sample/channel.

---

### 1.11 Scattering Delay Network (SDN)

De Sena et al. (2015). Encodes room geometry into FDN topology. 6 nodes (one per wall) for shoebox room. Physically meaningful parameters.

---

## PART 2: FREE IMPULSE RESPONSE LIBRARIES

| Library | License | Embeddable in Plugin? | Content |
|---------|---------|----------------------|---------|
| **OpenAIR** (Univ. York) | CC-BY 4.0 (per-file) | Yes (with attribution, per-file check) | ~60-80 spaces, churches, halls, tunnels |
| **EchoThief** | Custom (derivatives OK) | Requires permission | 115+ spaces, caves, tunnels, castles |
| **Adventure Kid** | CC-BY 4.0 | Yes (with attribution) | Small rooms, spring reverbs |
| **Freesound.org** | CC0 or CC-BY (per-file) | CC0: yes; CC-BY: with attribution | Massive collection, must curate |
| **Google Resonance Audio** | Apache 2.0 | Yes (with attribution) | HRTF + room modeling assets |
| **Voxengo** | Custom | No (redistribution prohibited) | Rooms, halls, plates |
| **Fokke van Saane** | Informal | Requires permission | Speakers, tube radios, small rooms |
| **MIT IR Survey** | Academic | No (research only) | 271 real-world IRs |
| **Soundwoofer** | Public domain (stated) | Likely yes (verify) | Cabinets + room IRs |

**Best for Plugin Embedding:** Adventure Kid (CC-BY 4.0), Freesound CC0 files, Google Resonance Audio (Apache 2.0), OpenAIR CC-BY 4.0 files.

---

## PART 3: FAMOUS HISTORICAL REVERB UNITS

### Hardware Units

**EMT 140 Plate Reverb (1957):** First commercial artificial reverb. ~1m×2m×0.5mm steel plate, suspended from frame. Electromechanical transducer excites vibrations; contact pickups capture result. Motorized damping pad controls RT (1-5s). Weighed ~600 lbs. Used on virtually every major recording 1950s-1980s.

**EMT 250 (1976):** First digital reverb unit.

**Lexicon 224 (1978):** First widely adopted digital reverb. 8080 processor, 12-bit converters. Concert Hall, Rich Plate, Vocal Plate programs. LARC remote controller.

**Lexicon 480L (1986):** Four processing cores. Random algorithm = famously smooth long tails. Industry standard for two decades. LARC II controller.

**AMS RMX-16 (1982):** Mark Crabtree design. Famous for NonLin (gated reverb) and Reverse programs. NonLin2 = programmable Phil Collins gated drum sound.

**Quantec QRS (1982):** Modeled room resonances based on air volume rather than reflections. Remarkably realistic ambiences. First commercial Freeze function.

**Eventide SP2016 (1982):** Grittier character than Lexicon. Stereo Room algorithm popular for rock/pop.

**Yamaha REV-7 (1984):** First affordable ($1000) professional digital reverb.

**Yamaha SPX90 (1985):** Even more affordable multi-effects ($800). Ubiquitous.

**Bricasti M7 (2007):** By former Lexicon engineers. Models how air carries sound (room resonances, not just reflections). Three engines per program: early reverberation, late decay, sub-80Hz. Modern reference standard.

**TC Electronic System 6000 (2000):** VSS algorithms. Multichannel for film/broadcast.

### Notable Software

- **Valhalla DSP (Sean Costello):** VintageVerb, Room, Plate, Shimmer, Supermassive (free)
- **FabFilter Pro-R:** Visual feedback, per-band decay control
- **Exponential Audio (Michael Carnes, ex-Lexicon):** PhoenixVerb, R2, Nimbus
- **Altiverb (Audio Ease):** Industry-standard convolution reverb
- **LiquidSonics:** Fusion-IR hybrid (convolution + algorithmic modulation), Seventh Heaven (M7)
- **Dragonfly Reverb (GPL 3.0):** Open-source, based on Freeverb3/Zita
- **u-he Protoverb:** Experimental mode-based approach

---

## PART 4: CPU USAGE COMPARISON

| Algorithm | Ops/Sample | % of Core (48kHz) | Latency | Quality |
|-----------|-----------|-------------------|---------|---------|
| Schroeder (4C+2AP) | 15-25 | 0.03% | 0 | Low-Medium |
| Moorer (6C+2AP+ER) | 40-60 | 0.06% | 0 | Medium |
| Freeverb (8C+4AP) | 30-40/ch | 0.05% | 0 | Medium |
| FDN 4-channel | 40-50 | 0.05% | 0 | Medium-High |
| FDN 8-channel | 120-180 | 0.18% | 0 | High |
| FDN 16-channel | 400-500 | 0.55% | 0 | Very High |
| Dattorro Plate | 60-80 | 0.09% | 0 | High |
| Velvet Noise (2s) | 50-200 | 0.18% | 0 | High |
| Spring Model | 200-500 | 0.45% | 0 | High |
| Shimmer | 400-1500 | 1.3% | varies | High |
| Convolution 1.0s | ~390 | 0.47% | 1 block | Very High |
| Convolution 2.0s (non-uniform) | ~500 | 0.60% | 0* | Very High |
| Convolution 5.0s (non-uniform) | ~600 | 0.72% | 0* | Very High |
| Convolution true stereo 2s | ~3000 | 3.6% | 1 block | Very High |
| Modal Plate (500 modes) | ~2,000 | 2.4% | 0 | High |
| Modal Plate (1000 modes) | ~4,000 | 4.8% | 0 | Very High |
| FDTD Plate (dx=1cm) | ~260,000 | 313% | 0 | Very High |

*"0*" = zero-latency via time-domain head + frequency-domain tail

### Feasibility Tiers

**Easily real-time (< 1% core):** Schroeder, Moorer, Freeverb, FDN up to 16ch, Dattorro, Velvet Noise, SDN.

**Moderate CPU (1-5%):** FDN 32+ch, Shimmer, Convolution up to 2s, Modal plate.

**Significant CPU (5-20%):** Long true-stereo convolution, Modal plate 2000+ modes.

**Borderline:** FDTD plate at coarse grid.

---

## Implementation Recommendations

1. **General-purpose algorithmic:** 8-16 channel FDN with Hadamard + Dattorro-style input diffusion + modulated delays.
2. **Plate emulation:** Implement Dattorro directly from 1997 paper. Or 500-1000 mode modal synthesis.
3. **Convolution:** Non-uniform partitioned with time-domain head for zero latency. JUCE `dsp::Convolution` built-in.
4. **Spring:** 100-200 allpass cascade with feedback.
5. **Hybrid (best of both):** Algorithmic early reflections + FDN/velvet noise late reverb.
6. **Bricasti-style:** Very dense early reflections (high-order FDN or dense tapped delay) + separate late reverb.

---

## Key References

1. Schroeder, M.R. (1962). "Natural Sounding Artificial Reverberation." JAES 10(3).
2. Moorer, J.A. (1979). "About This Reverberation Business." CMJ 3(2).
3. Jot, J-M. & Chaigne, A. (1991). "Digital Delay Networks for Designing Artificial Reverberators." AES Convention.
4. Dattorro, J. (1997). "Effect Design Part 1: Reverberator and Other Filters." JAES 45(9).
5. Gardner, W.G. (1995). "Efficient Convolution without Input-Output Delay." JAES 43(3).
6. De Sena, E. et al. (2015). "Efficient Synthesis of Room Acoustics via Scattering Delay Networks."
7. Valimaki, V. et al. "Late Reverberation Synthesis Using Filtered Velvet Noise." Applied Sciences.
8. Costello, S. Valhalla DSP Blog — reverb algorithm design resources.
