# Trajectory Animation

Each of the 12 delay taps can have an independent trajectory animation that moves the object through 3D space over time. Trajectories are configured per-object in the bottom panel.

## Trajectory Shapes

### None (Index 0)
Static position. No animation -- the object stays at its manually set azimuth, elevation, and distance.

### Spiral (Index 1)
A full 3D spiral motion combining azimuth rotation, elevation oscillation, and distance pulsing.

- **Azimuth:** Sweeps a full 360 degrees per cycle (baseAz + 360 * phase)
- **Elevation:** Sine oscillation of +/-45 degrees around the base elevation
- **Distance:** Cosine pulse between 0.3 and 1.0 (ignores base distance)

### Orbit (Index 2)
A simple circular orbit at constant elevation and distance.

- **Azimuth:** Full 360-degree rotation per cycle (baseAz + 360 * phase)
- **Elevation:** Stays at base elevation
- **Distance:** Stays at base distance

### Bounce (Index 3)
A triangle-wave ping-pong oscillation in azimuth and elevation.

- **Azimuth:** Triangle wave oscillation of +/-90 degrees around the base
- **Elevation:** Triangle wave oscillation of +/-30 degrees around the base
- **Distance:** Stays at base distance

### Figure-8 (Index 4)
A Lissajous curve with a 2:1 frequency ratio between elevation and azimuth.

- **Azimuth:** sin(2 * pi * phase) * 90 degrees around the base
- **Elevation:** sin(4 * pi * phase) * 45 degrees around the base (double frequency)
- **Distance:** Stays at base distance

### Random (Index 5)
Deterministic pseudo-random motion using sums of sine oscillators at irrational frequency ratios.

- **Azimuth:** +/-60 degrees at frequency 1.0 + +/-30 degrees at frequency e (2.7183)
- **Elevation:** +/-25 degrees at frequency sqrt(2) (1.4142) + +/-15 degrees at frequency pi (3.1416)
- **Distance:** +/-0.3 at frequency sqrt(3) (1.7321) + +/-0.1 at frequency sqrt(5) (2.2361), clamped to 0-1

The "random" motion is fully deterministic and repeatable for a given phase value, producing smooth organic-feeling movement without true randomness.

## Speed Control

The **Trajectory Speed** parameter (0.0 to 10.0) controls the animation rate:

| Speed | Behavior |
|-------|----------|
| 0.0 | Frozen -- trajectory holds at its current phase position |
| 1.0 | Approximately 1 cycle per second |
| 10.0 | Approximately 10 cycles per second |

The trajectory engine runs at approximately 60 Hz (driven by the plugin's timer callback). Phase advances by `speed * dt` per timer tick, where dt = 1/60 second.

## Base Position

When a trajectory shape changes from **None** to any active shape, the object's current azimuth, elevation, and distance are captured as the **base position**. All trajectory calculations offset from this base, so the trajectory orbits around wherever the object was when animation started.

## Output Clamping

After trajectory computation, output values are clamped:

- **Azimuth:** Wrapped to -180 to +180 degrees
- **Elevation:** Clamped to -90 to +90 degrees
- **Distance:** Clamped to 0.0 to 1.0

## OSC Override Interaction

When ADM-OSC messages are actively received for an object, the trajectory animation for that object is **paused**. OSC position data takes priority. When OSC messages stop arriving (500 ms timeout), the trajectory resumes from where it left off.

## See Also

- [ADM-OSC Integration](adm-osc-integration.md) -- how OSC interacts with trajectories
- [Parameter Reference](parameters-reference.md) -- trajectory parameter IDs and ranges
