# OpenSpatialFilter

**One-line:** Multiple filter instances each positioned in 3D space with independent modulation
**Type:** Effect
**Complexity:** Medium
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

Up to 12 independent filter instances, each positioned at a unique location in 3D space, processing the same input signal. Each filter has its own type, cutoff, resonance, and modulation source (LFO, envelope follower, or key tracking). Filters can be routed in serial (cascaded chain) or parallel (summed outputs), creating spatially distributed timbral textures. A sweeping bandpass at 2 o'clock, a resonant lowpass below, a notch filter orbiting overhead -- filter effects become spatial events.

## What SpatialCore Provides

- Per-filter spatialization (up to 12 filter objects)
- Trajectory system for animated filter positions (orbits create spatial sweeps)
- HRTF convolution with 6 profiles for binaural rendering
- ADM-OSC Receive/Send for external filter position control
- 22 output formats
- Distance attenuation per filter instance
- Output limiter

## Plugin-Specific DSP

- Multi-mode SVF filters (LP, HP, BP, notch, peak, low shelf, high shelf)
- Per-filter LFO with independent rate, depth, and waveform
- Envelope follower per filter (attack, release, sensitivity)
- Key tracking (MIDI note to cutoff mapping)
- Serial/parallel topology switching
- Filter feedback (resonance up to self-oscillation)
- Morph between filter types (smooth transition LP to HP)
- Filter FM (audio-rate modulation of cutoff)

## Parameters

**Global (effect-specific):** filterType, cutoff, resonance, lfoRate, lfoDepth, envAmount, topology (serial/parallel), dryWet
**Per-object (x12):** filter frequency, resonance, type (each filter instance independently configurable)

## Market Reference

No spatial filter effect exists. Existing multi-filter plugins (FabFilter Volcano 3, Xfer Cthulhu) are stereo and focus on serial/parallel filter routing without spatial positioning. The combination of per-filter 3D placement with trajectory animation creates entirely new sound design possibilities -- resonant peaks that orbit the listener, filter sweeps that move through physical space.

## Skills Needed

- `dsp-cookbook` — SVF filters, envelope followers, LFO design
- `synthesis-techniques` — filter topologies (Moog ladder, ZDF), filter FM
- `spatial-audio-dsp` — per-filter spatialization, position-to-timbre mapping
