---
name: reverb-algorithms
description: >
  Expert in reverberation algorithms and implementation for JUCE C++ audio plugins.
  Covers algorithmic reverb (Schroeder, Moorer, FDN with Hadamard/Householder matrices,
  Dattorro plate, Freeverb, Velvet Noise, Scattering Delay Networks), convolution reverb
  (uniform/non-uniform partitioned FFT, zero-latency, true stereo), physical plate/spring
  modeling (FDTD, modal synthesis, dispersive waveguide), shimmer reverb, impulse response
  libraries and licensing, and famous hardware unit characteristics (EMT, Lexicon, AMS, Bricasti).
  Activate on 'reverb', 'reverberation', 'FDN', 'feedback delay network', 'Schroeder',
  'Moorer', 'Dattorro', 'plate reverb', 'convolution reverb', 'impulse response', 'IR',
  'spring reverb', 'shimmer', 'allpass', 'comb filter', 'early reflections', 'late reverb',
  'RT60', 'decay time', 'diffusion', 'Freeverb', 'Hadamard', 'Householder', 'room simulation',
  'Lexicon', 'EMT 140', 'Bricasti', 'algorithmic reverb', 'velvet noise'.
  NOT for spatial audio panning (use spatial-audio-dsp), synthesis techniques (use synthesis-techniques),
  or chorus/flanger/phaser (use time-based-effects).
allowed-tools: Read,Write,Edit,Bash,Glob,Grep,WebFetch,WebSearch
metadata:
  category: Development & Engineering
  pairs-with:
    - skill: dsp-cookbook
      reason: Shared filter and delay building blocks
    - skill: spatial-audio-dsp
      reason: Reverb often combined with spatialization
    - skill: time-based-effects
      reason: Shared comb/allpass filter foundations
  tags:
    - reverb
    - dsp
    - convolution
    - fdn
    - juce
    - audio-plugin
    - c++
---

# Reverberation Algorithms

## 1. Schroeder Reverb (1962)

**Feedback comb filter:** `y[n] = x[n-M] + g·y[n-M]`
```
RT60 = -3·M / (fs · log10(|g|))
g = 10^(-3·M / (fs · RT60))
```

**Allpass filter:** `y[n] = -g·x[n] + x[n-M] + g·y[n-M]`. |H|=1 for all f.

**Topology:** 4 parallel comb filters → 2 series allpass. Delay lengths must be mutually prime.

**Limitations:** Metallic coloration, no early reflections, linear density growth.

**CPU:** ~15-25 ops/sample. Extremely cheap.

## 2. Moorer Reverb (1979)

Extends Schroeder with:
- **Tapped delay line** for early reflections (~18 taps from image sources)
- **LP filter in comb feedback:** `y_f[n] = (1-d)·x_f[n] + d·y_f[n-1]`

Creates frequency-dependent RT60 (shorter at HF), matching real rooms.

## 3. Feedback Delay Network (FDN)

**Architecture (Jot & Chaigne, 1991):** N delay lines + N×N unitary feedback matrix + N absorption filters.

**Feedback matrices (must be unitary: A^T·A = I):**

*Hadamard (N=2^k):*
```
H_{2k} = (1/√2) · [H_k  H_k; H_k -H_k]
```
Fast Walsh-Hadamard: O(N log N) vs O(N²).

*Householder:* `A = I - (2/N)·ones(N,N)`. Maximum energy distribution.

**Delay selection:** `M_i = nextPrime(M_min · (M_max/M_min)^(i/(N-1)))`. Must be coprime.

**Absorption filters per line:**
```
|g_i(f)| = 10^(-3·M_i / (fs · RT60(f)))
```
Implement as first-order shelf (biquad for finer control).

**Modulation:** Slow LFO (0.5-2Hz, 1-8 sample depth) on delay reads breaks metallic coloration. Different rates per line.

**CPU:**
| Size | Ops/Sample | Quality |
|------|-----------|---------|
| 4ch | 40-50 | Medium-High |
| 8ch | 120-180 | High |
| 16ch | 400-500 | Very High |
| 32ch | 1400-1800 | Excellent |

## 4. Dattorro Plate Reverb (1997)

From "Effect Design Part 1" (JAES 45(9)). Excellent quality-to-CPU ratio.

**Input diffusion (4 series allpass):**
```
Input → BW_LPF → AP(142,0.75) → AP(107,0.75) → AP(379,0.625) → AP(277,0.625) → Tank
```

**Tank (figure-8 cross-coupled):**
```
Loop 1: AP_mod(672,dd1) → Delay(4453) → Damp_LPF → ×decay → AP(1800,dd2) → Delay(3720) → Loop 2
Loop 2: AP_mod(908,dd1) → Delay(4217) → Damp_LPF → ×decay → AP(2656,dd2) → Delay(3163) → Loop 1
```
All delay values at 29761 Hz — scale by `fs/29761`.

**Key parameters:** `decay` (0-0.999), `damping` (0-1), `decay_diffusion_1` (~0.7), `decay_diffusion_2` (~0.5).

**Modulated allpass:** ~1Hz LFO, ±8 sample excursion. Use allpass interpolation.

**Stereo output:** 14 taps from both loops, some subtracted for decorrelation.

**CPU:** ~60-80 ops/sample. One of the best quality/cost ratios.

## 5. Convolution Reverb

**IR capture:** Exponential sine sweep + inverse deconvolution.

**Uniform partitioned (overlap-save):** Divide IR into B blocks of size N. Per block: FFT(2N) + B complex mults + IFFT(2N). Latency = N samples.

**Non-uniform (Gardner 1995):** Small partitions for head, larger for tail. 30-50% CPU savings.

**Zero-latency:** Time-domain convolution for first N samples (N mults/sample), FFT for rest.

**True stereo:** 4 IRs (LL, LR, RL, RR). 4× CPU.

**CPU:**
| IR Length | Non-Uniform Ops/Sample |
|-----------|----------------------|
| 0.5s | ~130 |
| 1.0s | ~250 |
| 2.0s | ~500 |
| 5.0s | ~600 |

**JUCE:** `dsp::Convolution` provides built-in implementation.

## 6. Physical Plate Modeling

**Kirchhoff thin plate:** `ρh·ü = -D·∇⁴u + f(x,y,t)`, D = Eh³/(12(1-ν²))

**FDTD:** 13-point biharmonic stencil. CFL: `dt ≤ dx²/(2κ)`. At dx=1cm: ~260K ops/sample (borderline real-time).

**Modal decomposition (preferred):** 500-1000 modes as biquad resonators. ~2K-4K ops/sample. Easily real-time.

## 7. Spring Reverb

Dispersive propagation: `v_phase(f) = v_0·√(1 + (f/f_c)²)`. Characteristic chirp.

**Digital model:** 50-200 cascaded allpass filters + feedback comb + EQ. ~200-500 ops/sample.

## 8. Shimmer Reverb

```
Input → Reverb (FDN/Plate) → output
              ↓
        Pitch Shift (+12st) → LPF → ×feedback → back to input
```
Intervals: +12 (octave), +7 (fifth), +5 (fourth). ~400-1500 ops/sample.

## 9. Velvet Noise Reverb (Valimaki, Aalto)

Sparse random impulses (2000/sec) perceptually equivalent to Gaussian noise above ~1500 impulses/sec.

Only non-zero positions contribute → very efficient. ~50-200 ops/sample with high quality.

## 10. Freeverb (Jezar, 2000)

Public domain. 8 LP-feedback combs parallel + 4 series allpass, stereo via offset delays (+23 samples R).

Comb delays (44.1kHz): 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617.
Allpass delays: 556, 441, 341, 225. Feedback coeff: 0.5 (non-standard).

~30-40 ops/sample/channel.

## 11. Scattering Delay Network (De Sena, 2015)

6 nodes (walls of shoebox room). Delays = wall-to-wall distances. Physically meaningful parameters. ~100-200 ops/sample.

---

## Free IR Libraries for Plugin Embedding

| Library | License | Embeddable? |
|---------|---------|------------|
| Adventure Kid | CC-BY 4.0 | Yes (attribution) |
| Freesound CC0 | CC0 | Yes (unrestricted) |
| Google Resonance Audio | Apache 2.0 | Yes (attribution) |
| OpenAIR (select) | CC-BY 4.0 | Yes (per-file check) |
| EchoThief | Custom | Contact author |
| Voxengo | Custom | No (redistribution prohibited) |

---

## Famous Hardware Reference

| Unit | Year | Type | Character |
|------|------|------|-----------|
| EMT 140 | 1957 | Plate (steel) | Dense, bright, classic studio |
| EMT 250 | 1976 | First digital | Early digital character |
| Lexicon 224 | 1978 | Digital (12-bit) | Warm, characteristic distortion |
| AMS RMX-16 | 1982 | Digital | NonLin gated reverb, Reverse |
| Quantec QRS | 1982 | Room resonance | Uniquely natural, Freeze function |
| Lexicon 480L | 1986 | Digital | Random Hall, industry standard |
| Eventide SP2016 | 1982 | Digital | Gritty, characterful |
| Bricasti M7 | 2007 | Digital | Dense early reflections, modern reference |
| TC System 6000 | 2000 | Digital | VSS algorithms, surround |

**Software:** Valhalla DSP (Costello), FabFilter Pro-R, Exponential Audio (ex-Lexicon), Altiverb (convolution standard), LiquidSonics (Fusion-IR hybrid), Dragonfly (GPL, open-source).

---

## CPU Comparison Summary

| Algorithm | Ops/Sample | % Core (48kHz) |
|-----------|-----------|----------------|
| Schroeder | 15-25 | 0.03% |
| Dattorro Plate | 60-80 | 0.09% |
| FDN 8ch | 120-180 | 0.18% |
| FDN 16ch | 400-500 | 0.55% |
| Velvet Noise | 50-200 | 0.18% |
| Convolution 2s (non-uniform) | ~500 | 0.60% |
| Spring model | 200-500 | 0.45% |
| Shimmer | 400-1500 | 1.3% |
| Modal plate (500) | ~2000 | 2.4% |
| True stereo conv 2s | ~3000 | 3.6% |

---

## Implementation Recommendations

1. **General algorithmic:** 8-16ch FDN + Hadamard + input diffusion + modulation
2. **Plate:** Dattorro (1997 paper directly) or 500-1000 mode modal
3. **Convolution:** Non-uniform partitioned + time-domain head for zero latency
4. **Spring:** 100-200 allpass cascade + feedback
5. **Hybrid (best results):** Algorithmic early reflections + FDN/velvet noise late tail
6. **Bricasti-style:** Very dense early reflections (high-order FDN) + separate late engine

---

## Key References

1. Schroeder (1962) — JAES 10(3), 219-223
2. Moorer (1979) — CMJ 3(2), 13-28
3. Jot & Chaigne (1991) — FDN, AES Convention
4. Dattorro (1997) — JAES 45(9), 660-684
5. Gardner (1995) — Efficient convolution, JAES 43(3)
6. De Sena et al. (2015) — Scattering Delay Networks
7. Valimaki et al. — Velvet noise reverb, Applied Sciences
8. Costello — Valhalla DSP blog (reverb design resources)
