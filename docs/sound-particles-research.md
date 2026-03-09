# Sound Particles Architecture Research Report

## Purpose

Research the binaural rendering architecture of Sound Particles plugins (inDelay, SkyDust 3D, Energy Panner, etc.) to understand how they handle binaural output without requiring a "monitoring format" dropdown. Evaluate whether OpenSpatialDelay should adopt a similar approach.

---

## 1. Company & Team Background

**Sound Particles S.A.** — Founded 2016 (technology in development since 2012), Leiria, Portugal.

**Nuno Fonseca (CEO/Founder):**
- PhD in Computer Engineering (Computer Audio), University of Porto (FEUP), 2011-2012
- PhD dissertation: *"Singing Voice Resynthesis using Concatenative-based Techniques"* — NOT about spatial audio
- The particle system concept for audio came AFTER his PhD (~2013), when he noticed CGI particle systems (fire, explosions) had no audio equivalent
- Professor at Polytechnic Institute of Leiria; AES, SMPTE, CAS member
- Author of 20+ papers on audio research

**Other team:** ~30 employees. Marco Conceicao (PhD in Spatial Audio, Trinity College Dublin) is an academic collaborator. Luis Marcelino (Senior Software Engineer).

---

## 2. Patents

**CB Insights reports 3 patents filed**, but none are publicly indexed in USPTO, EPO, or WIPO databases. Likely filed under Portuguese national system (INPI) or as unpublished PCT applications.

**Known patent-pending technology:**
1. **Space Controller** — Motion-sensor spatial panning via smartphone (patent pending, explicitly stated in marketing)
2. **Personalized HRTF** — AI/ML-powered individualized binaural using 3D ear/head scans (in development since ~2018, partnered with Google Cloud)
3. **Core particle system technology** — Likely the third filing

**No patents found describing their binaural rendering approach.**

---

## 3. Key Academic Papers

| Year | Title | Venue | Key Content |
|------|-------|-------|-------------|
| 2013 | "Sound Spatialization with Particle Systems" | Academia.edu | Foundational paper — applies CGI particle concept to audio |
| 2014 | "Particle Systems for Creating Highly Complex Sound Design Content" | AES 137 (Paper 9132) | Virtual microphones capture particle scenes; supports surround, Ambisonics, binaural, partial Atmos |
| 2015 | "Immersive Sound Design Using Particle Systems" | AES 138 (EB 193) | "Immersive virtual microphones" render thousands of sources for cinema formats (Auro-3D, Atmos) |
| 2015 | "The Future of Audio Post-Production using Virtual 3D Scenes" | SMPTE | Argues for 3D virtual scenes in audio post (like CGI + virtual cameras for visual) |
| 2020 | *"All You Need to Know About 3D Audio"* | Free eBook | Covers channel-based, object-based, Ambisonics, binaural; VBAP, HRTF, HRIR, AmbiX, ACN, SN3D |

---

## 4. Core Architecture: The Virtual Microphone Paradigm

Sound Particles uses a fundamentally different architecture from OpenSpatialDelay:

### OpenSpatialDelay (current): Virtual Speaker Paradigm
```
3D Source Positions → Algorithm (VBAP/KNN/etc.) → Speaker Gains[16]
→ HRTF Convolution per Virtual Speaker → Sum to L/R
```

### Sound Particles: Virtual Microphone Paradigm
```
3D Source Positions (particles) → Virtual Microphone captures scene
→ Output depends on microphone type:
   - Virtual XY mic → Stereo (XY)
   - Virtual MS mic → Stereo (MS)
   - Virtual Blumlein mic → Stereo (Blumlein)
   - Virtual binaural mic (HRTF) → Binaural L/R
   - Virtual 5.1 array → 5.1 surround
   - Virtual Ambisonics mic → HOA
```

The rendering paradigm determines the output format — the same 3D scene is "captured" by different virtual microphone types. Binaural is rendered by a **virtual binaural microphone** that applies per-source HRTF convolution, NOT by decoding through an intermediate virtual speaker layout.

---

## 5. Binaural Rendering: Direct Object-Based (Most Likely)

Sound Particles does NOT publicly document their binaural pipeline. Based on all available evidence, their approach is **direct object-based binaural rendering**:

**Each 3D source → look up HRTF for that direction → convolve with HRIR → sum all sources → L/R output**

Evidence supporting this:
1. **SOFA file support** in SkyDust 3D — SOFA stores per-direction HRIRs, directly suitable for object-based rendering
2. **"Virtual binaural microphone" concept** — aligns with per-source HRTF convolution (the mic "hears" each source from its direction)
3. **Binaural is a peer output format** alongside VBAP/XY/MS stereo modes — suggests its own dedicated rendering path, not an intermediate decode
4. **5 built-in HRTF datasets with 100+ profiles** in SkyDust 3D — profile selection is tied to binaural output, not speaker layout selection
5. **No "monitoring format" concept** in any plugin — binaural rendering doesn't need a speaker layout reference

### Why this avoids the monitoring format problem:
With direct object-based binaural, there are no virtual speakers to select. The HRTF is applied directly per-source at the source's 3D position. The concept of "what speaker layout am I monitoring" doesn't exist — the binaural output inherently represents the full 3D sound field.

---

## 6. Output Format Handling

### Plugin-Side Format Selector (NOT DAW-Driven)

From the inDelay screenshots (`docs/references/inDelay Output list.png`):
- **IN dropdown** (top-right): Mono, Stereo
- **OUT dropdown** (top-right): Mono, Stereo, Stereo (VBAP), Stereo (XY), Stereo (MS), Stereo (Blumlein), Binaural, + ~30 more (surround up to 11.1.8, Ambisonics 1st-6th order)

Both formats are set **within the plugin UI**, not inferred from the DAW's track bus configuration. The plugin negotiates the channel count with the DAW based on the user's selection.

### Contrast with OpenSpatialDelay:
OpenSpatialDelay detects the DAW's bus width and derives the output format automatically. Binaural is triggered when the bus is stereo (2ch). The user cannot select "binaural" on a 7.1 bus or "7.1" on a stereo bus.

---

## 7. No "Monitoring Format" in Sound Particles Plugins

**This is the key finding.** Sound Particles plugins have NO monitoring format concept because:

1. **Binaural IS an output format**, not a monitoring mode
2. There is no "preview what my 7.1 mix sounds like on headphones" workflow within the plugins
3. If you want 7.1, select 7.1. If you want binaural, select Binaural. They are peer choices.
4. The binaural output works acceptably over loudspeakers too ("just a slightly different tonality" — Sound on Sound review)

**The standalone Sound Particles application (v2/v3) DOES have binaural monitoring** — you can work in any format and switch to binaural monitoring for headphone preview. But this is a feature of the full workstation, not the DAW plugins.

---

## 8. Architecture Comparison

| Aspect | Sound Particles Plugins | OpenSpatialDelay v0.3 |
|--------|------------------------|----------------------|
| **Format selection** | Plugin-side dropdown (~30 formats) | DAW-driven bus detection |
| **Binaural rendering** | Direct object-based HRTF (per-source) | Virtual speakers → HRTF convolution |
| **Monitoring format** | None — binaural = output format | Dropdown selects virtual speaker layout |
| **HRTF data** | 5 datasets, 100+ profiles, SOFA import | 5 SOFA files |
| **Core paradigm** | Virtual microphones capture 3D scene | Algorithms → speaker gains → renderer |
| **Format-agnostic** | Yes — same scene, different renderers | Partially — algorithms are pluggable |

---

## 9. Implications for OpenSpatialDelay

### The monitoring format dropdown exists because OpenSpatialDelay uses virtual speakers for binaural rendering

The virtual speaker approach requires choosing WHICH speakers to virtualize. Sound Particles avoids this entirely by rendering binaural directly — each source gets its own HRTF, no intermediate speaker layout.

### What would it take to eliminate the monitoring format?

To remove the monitoring format dropdown, OpenSpatialDelay would need to add a **direct binaural rendering path** where:
1. Each tap's 3D position → look up nearest HRTF measurement in the SOFA file → convolve with HRIR
2. Sum all convolved taps → L/R output
3. No virtual speakers involved

This is conceptually similar to the existing `DirectBinauralAlgorithm` (Woodworth ITD+ILD) but with full HRTF convolution instead of the simple Woodworth model.

### Trade-offs:

**Direct binaural (Sound Particles approach):**
- No monitoring format needed
- Highest spatial accuracy (HRTF at exact source position)
- More computationally expensive (HRTF convolution per source, not per speaker)
- With 12 taps: 12 HRTF convolutions vs. current 4-16 speaker convolutions

**Virtual speaker binaural (current approach):**
- Requires monitoring format selection
- Speaker count limits spatial resolution
- Fewer convolutions (speaker count, not source count)
- Allows monitoring different speaker layouts (useful for production workflow)

### Possible hybrid approach:
Keep BOTH paths — direct binaural as default (no monitoring format needed), with an optional "speaker preview" mode for users who specifically want to hear what their mix sounds like on a particular speaker system. This would make the monitoring format dropdown optional rather than required.

---

## 10. Sources

### Patents
- CB Insights company profile (3 patents reported, not publicly indexed)
- Space Controller: patent pending (audioXpress, KVR Audio)

### Academic Papers
- Fonseca, N. (2012). PhD: "Singing Voice Resynthesis using Concatenative-based Techniques." FEUP, University of Porto. [IC-Online](https://iconline.ipleiria.pt/handle/10400.8/540)
- Fonseca, N. (2014). "Particle Systems for Creating Highly Complex Sound Design Content." AES 137, Paper 9132. [AES E-Library](https://aes2.org/publications/elibrary-page/?id=17455)
- Fonseca, N. (2015). "Immersive Sound Design Using Particle Systems." AES 138, EB 193. [AES E-Library](https://aes2.org/publications/elibrary-page/?id=17628)
- Fonseca, N. (2015). "The Future of Audio Post-Production using Virtual 3D Scenes." SMPTE. [IEEE Xplore](https://ieeexplore.ieee.org/document/7399640)
- Fonseca, N. (2013). "Sound Spatialization with Particle Systems." [Academia.edu](https://www.academia.edu/6680058/SOUND_SPATIALIZATION_WITH_PARTICLE_SYSTEMS)

### Reviews & Product Documentation
- Sound on Sound: inDelay review, SkyDust 3D review, Energy Panner review
- Sound Particles blog: "What is Binaural?", "Exploring inDelay's Modes"
- inDelay Output list screenshot: `docs/references/inDelay Output list.png`
- Nuno Fonseca (2020). "All You Need to Know About 3D Audio." Free eBook via Sound Particles.

### Company
- [Composium interview](https://composium.substack.com/p/how-nuno-fonseca-built-an-audio-startup)
- [Google Cloud case study](https://cloud.google.com/customers/sound-particles)
- [EU H2020 grant 873306](https://cordis.europa.eu/project/id/873306) (EUR 1.78M, 2019-2021)
