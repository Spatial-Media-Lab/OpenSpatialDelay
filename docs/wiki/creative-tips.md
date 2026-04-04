# Creative Tips

This page collects sound design recipes and production techniques for getting the most out of OpenSpatialDelay. Each recipe includes the key parameter settings and a description of the effect.

## Self-Oscillation

When feedback exceeds approximately 95%, the delay begins to self-oscillate -- the echoes sustain and build rather than decaying. This creates drones, washes, and evolving textures from even a brief input signal.

**Setup:**
- FEEDBACK: 97-100%
- FLT: On, LP around 3000-6000 Hz
- TIME: 200-800 ms (longer = more tonal, shorter = more chaotic)

**How it works:** OpenSpatialDelay has a built-in output limiter (+2 dB ceiling) across all rendering paths, so self-oscillation never clips your DAW output. The soft clipper and limiter together keep the signal controlled while allowing the musical buildup of self-oscillation.

> **Tip:** Feed a single note or chord into a self-oscillating delay, then mute the input. The echoes continue indefinitely. Gradually reduce feedback below 95% to let the texture decay naturally.

## Pitch Spiraling and Shimmer

The cumulative pitch shift creates effects where each repeat is pitched higher (or lower) than the last. With feedback, these pitch changes accumulate through multiple cycles.

### Octave Shimmer
- PITCH (global): +1200 ct (= +12 semitones = one octave up)
- FEEDBACK: 40-60%
- FLT: On, LP around 8000 Hz (prevents harsh buildup)
- TIME: 300-600 ms

Each repeat jumps up one octave. With feedback, you get a cascading shimmer effect. The low-pass filter is essential -- without it, the high-frequency content builds up harshly.

### Harmonic Spirals
- PITCH (global): +700 ct (+7 semitones = a perfect fifth)
- FEEDBACK: 50-70%
- FLT: On, LP around 6000 Hz

Creates a spiral of fifths -- each echo sounds a fifth higher. This produces a rich, evolving harmonic texture that slowly climbs through the overtone series.

### Descending Gloom
- PITCH (global): -500 ct (-5 semitones = a perfect fourth down)
- FEEDBACK: 60-80%
- FLT: On, HP around 200 Hz (prevents rumble buildup)

Each repeat drops a fourth, creating a descending spiral into low frequencies. The high-pass filter prevents sub-bass buildup.

### Per-Tap Pitch Chords
- PITCH (global): 0 ct
- Per-tap PITCH: Set different values per tap (e.g., tap 1 = 0 st, tap 2 = +7 st, tap 3 = +12 st)

Since per-tap pitch uses WSOLA (time-domain stretching), the delay timing stays perfectly locked while each tap produces a different pitch. This creates chord-like harmonizations in the echo pattern.

## Wobble and Tape Effects

The MOD section adds delay-time modulation, simulating tape machine imperfections.

### Vintage Tape Echo
- MOD: On
- AMOUNT: 30-50%
- MORPH: 0-15% (mostly sine wave)
- FLT: On, LP around 4000-6000 Hz, HP around 100-200 Hz

The gentle sine modulation creates the slow pitch drift (wow) characteristic of tape echo machines. Combined with filtering, this produces a warm, vintage character.

### Tape Flutter
- MOD: On
- AMOUNT: 15-25%
- MORPH: 40-60% (triangle territory)

Faster, more irregular modulation simulating tape flutter -- the rapid speed variations caused by mechanical imperfections in tape transport mechanisms.

### Warped Vinyl
- MOD: On
- AMOUNT: 70-100%
- MORPH: 80-100% (square wave)
- FEEDBACK: 60%+

Extreme modulation with a near-square waveform creates dramatic pitch jumps, simulating a warped vinyl record or a malfunctioning tape machine.

## Stereo Width with Input Routing

When your DAW provides a stereo input to the plugin, per-tap input channel selection creates wide stereo effects.

### Alternating L/R
- Tap 1: INPUT = L, azimuth = -45 degrees
- Tap 2: INPUT = R, azimuth = +45 degrees
- Tap 3: INPUT = L, azimuth = -90 degrees
- Tap 4: INPUT = R, azimuth = +90 degrees

This creates a true spatial ping-pong where the left channel's echoes appear on the left side and the right channel's echoes on the right, with successive echoes spreading wider.

### Channel Separation
- Odd taps (1, 3, 5...): INPUT = L
- Even taps (2, 4, 6...): INPUT = R

Combined with different azimuth positions, this creates a wide, immersive stereo field where left and right channel echoes occupy different spatial locations.

## Feedback Filter Design

The feedback filters shape the tonal evolution of repeats over time. Since filters are in the feedback path, their effect is cumulative -- each repeat passes through the filters again.

### Natural Decay (Radio Effect)
- FLT: On
- HP: 200 Hz
- LP: 4000 Hz
- Resonance: 0.71 (flat)

Simulates the natural bandwidth narrowing of sound reflecting off surfaces. Each repeat loses both high and low frequencies, mimicking how echoes sound in real spaces.

### Resonant Feedback
- FLT: On
- HP: 300 Hz, HP RES: 3.0
- LP: 2000 Hz, LP RES: 4.0

High resonance values create resonant peaks at the filter cutoff frequencies. As feedback cycles, these peaks become increasingly pronounced, creating a tuned, ringing quality.

### Progressive Darkening
- FLT: On
- HP: 20 Hz (minimal)
- LP: 3000-5000 Hz
- FEEDBACK: 70%

Only the low-pass filter is doing significant work. Each repeat gets progressively darker as high frequencies are gradually removed. This is the most natural and commonly used feedback filter configuration.

## Spatial Ping-Pong

The classic ping-pong effect, elevated to 3D.

### Classic L/R Ping-Pong
- 2 taps: azimuth -90 and +90 degrees, distance 0.5
- FEEDBACK: 50-70%

The most basic spatial delay -- echoes bounce between hard left and hard right. Simple but effective.

### Quad Ping-Pong
- 4 taps: azimuth 0, 90, 180, -90 degrees (front, right, back, left)
- FEEDBACK: 40-60%

Echoes travel around the listener in a circle. Works best in surround or binaural modes where all four positions are audible.

### 3D Ping-Pong
- 4 taps with elevation: (0 deg, +45 el), (90 deg, -20 el), (180 deg, +45 el), (-90 deg, -20 el)
- Output: Binaural or Atmos 7.1.4

Echoes bounce between upper and lower positions as well as around the horizontal plane. Most effective in binaural or height-enabled surround formats.

## Immersive Ambiences

Create lush spatial environments using many taps with subtle settings.

### Spatial Wash
- 8-12 taps enabled
- All taps at different azimuth, elevation, and distance values
- TIME: 100-300 ms
- FEEDBACK: 20-40%
- PITCH: +3 to +5 ct (subtle upward drift)
- FLT: On, LP 6000 Hz
- DRY/WET: 30-40%

Short delay times with many scattered taps create a dense spatial reverb-like effect. The subtle pitch shift adds shimmer without obvious pitch stepping.

### Random Cloud
- 6-8 taps enabled
- TRAJ: Random on all taps
- SPEED: 0.05-0.15 Hz (very slow)
- FEEDBACK: 30-50%
- AIR: On

Slowly drifting random trajectories on all taps create an ever-changing spatial texture. Air absorption adds natural distance-based tonal variation.

### Height Mist
- 6 taps at varying elevations (-40 to +60 degrees)
- TRAJ: Helix on all taps
- SPEED: 0.08 Hz
- FEEDBACK: 25-35%
- Output: Binaural or Atmos

Helix trajectories make each tap slowly orbit while moving up and down through the height field, creating a misty, three-dimensional ambience.

## Using with Dolby Atmos

### Object-Based Workflow

1. Set output format to **7.1.4 Atmos** (or your Atmos bed format)
2. Configure the track in your DAW for the matching channel count (12 for 7.1.4)
3. Set algorithm to **Constant Power** (default) for smooth panning, or **VBAP** for precise placement
4. Position taps at bed speaker positions or anywhere in the 3D field

### Atmos Renderer Integration

For Dolby Atmos production with an external renderer:

1. Use Binaural mode in OpenSpatialDelay for monitoring
2. Enable OSC SEND to broadcast tap positions
3. Connect the OSC output to your Atmos renderer's object input
4. The renderer receives animated positions in real time via ADM-OSC

### Height-Aware Presets

Several factory presets are designed for height-enabled formats:

- **Falling Cascade** -- taps descend from above with pitch drops
- **Rain** -- scattered taps at various heights
- **Hemisphere Spread** -- taps distributed across the upper hemisphere
- **Ascending Staircase** -- echoes climb upward
- **Dome Ring** -- taps circling at elevated positions

These presets use elevation values that are only audible in binaural or height-speaker formats. In standard surround (e.g., 5.1), the height component is folded down to the horizontal plane.

## Doppler Effects

### Rotating Speaker
- 1 tap with Orbit trajectory
- SPEED: 1.0-3.0 Hz
- DOPPLER: 50-100%
- DIST: 0.3

Fast orbit with Doppler creates a Leslie/rotary speaker-like pitch modulation. The closer the tap (smaller distance), the more pronounced the effect.

### Passing Train
- 1 tap with Line trajectory
- SPEED: 0.2 Hz
- DOPPLER: 100%
- AIR: On
- DIST: 0.8

The tap moves toward and away from the listener along a radial line. Doppler shifts the pitch up on approach and down on departure. Air absorption adds realistic high-frequency attenuation at distance.

### Doppler Swarm
- 6+ taps with various trajectories (Orbit, Figure-8, Random)
- DOPPLER: 30-60% (varied per tap)
- SPEED: varied per tap

Multiple taps with different Doppler amounts and speeds create a complex, living texture of shifting pitches and positions.
