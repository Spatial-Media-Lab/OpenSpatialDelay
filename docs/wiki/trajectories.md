# Trajectories

Trajectories animate a delay tap's position in 3D space over time. Each of the 12 taps can run its own trajectory independently, with its own shape, speed, and direction.

## How Trajectories Work

### Origin-Point Architecture

Trajectories use an origin-point system: the tap's knob values (azimuth, elevation, distance) define the **origin point**, and the trajectory shape computes a path relative to that origin.

This means:

- You can **reposition a running trajectory** by adjusting the knobs -- the entire path moves with the origin
- The **crosshair marker** on the spatial map shows the origin point
- The **animated dot** shows the tap's current position as it follows the trajectory path

### Enabling a Trajectory

1. Select a tap by clicking its number in the bottom panel
2. Set the **TRAJ** dropdown to any shape other than "None"
3. Adjust **SPEED** to control how fast the tap moves (in Hz -- cycles per second)
4. Optionally toggle **DIR** between Forward and Reverse

The trajectory starts immediately and runs continuously until you set it back to "None".

## Trajectory Shapes

### None

No trajectory -- the tap stays at its knob position. This is the default.

### Bounce

The tap moves back and forth along a straight line through the origin point, like a ball bouncing between two walls.

- **Motion:** Linear oscillation along the azimuth axis
- **On the map:** A straight line through the origin, bouncing at each end
- **Use cases:** Ping-pong effects, rhythmic side-to-side movement
- **Creative tip:** Use with Doppler enabled for pitch-bending bounce effects

### Circle

The tap traces a circular path around the listener at the origin's distance.

- **Motion:** Continuous rotation at a fixed distance and elevation
- **On the map:** A smooth circle centered on the listener (not the origin)
- **Use cases:** Classic rotary speaker simulation, spatial panning automation
- **Creative tip:** Set different speeds on multiple taps for polyrhythmic circular motion

### Cross

The tap traces a cross (plus sign) pattern, moving through the origin in both horizontal and vertical directions.

- **Motion:** Alternating horizontal and vertical sweeps through the origin
- **On the map:** A plus-shaped path centered on the origin
- **Use cases:** Complex spatial movement, sound design
- **Creative tip:** Pairs well with Doppler for dramatic directional shifts

### Figure-8

The tap traces a figure-eight pattern using two tangent circles.

- **Motion:** Two connected loops forming an 8 shape
- **On the map:** A horizontal figure-8 centered on the origin, rotating by the origin azimuth
- **Use cases:** Flowing spatial movement, natural-sounding panning
- **Creative tip:** At slow speeds, creates a gentle swaying effect perfect for ambient textures

### Heart

The tap traces a heart-shaped (cardioid) curve around the origin.

- **Motion:** A cardioid path with the cusp at the origin, scaled by origin distance
- **On the map:** A heart shape with the point at the origin
- **Use cases:** Expressive, organic movement patterns
- **Creative tip:** Try at very slow speeds (0.05 Hz) for subtle, emotional spatial drift

### Helix

The tap spirals in azimuth while oscillating in elevation, creating a 3D helical path.

- **Motion:** Continuous azimuth rotation combined with up-and-down elevation movement
- **On the map:** Appears as a circle (the elevation change is not visible on the 2D map, but the tap dot changes opacity/size to indicate height)
- **Use cases:** 3D spatial movement, height-enabled formats (Atmos, binaural)
- **Creative tip:** Most effective in binaural or Atmos modes where elevation is audible. Combine with air absorption for height-dependent tonal changes.

### Infinity

The tap traces an infinity symbol (lemniscate) path -- a smooth, continuous figure-eight.

- **Motion:** Lemniscate of Bernoulli curve, horizontally oriented
- **On the map:** A smooth infinity/figure-eight shape centered on the origin
- **Use cases:** Flowing bilateral movement, stereo-friendly panning
- **Creative tip:** Works well on stereo output since the motion is primarily left-right

### Line

The tap moves back and forth along a straight line at the origin's azimuth angle.

- **Motion:** Linear oscillation along a radial line from the listener through the origin
- **On the map:** A straight line from near the center toward the edge, through the origin
- **Use cases:** Distance-based effects, approach/recede movement
- **Creative tip:** Combine with Doppler and air absorption for realistic approach-and-pass effects

### Orbit

The tap orbits around the origin point at a fixed offset distance.

- **Motion:** Circular path centered on the origin (not the listener)
- **On the map:** A circle centered on the tap's origin position
- **Use cases:** Satellite-like movement, complex compound motion
- **Creative tip:** Place the origin off-center, then orbit creates asymmetric spatial movement relative to the listener

### Random

The tap moves unpredictably using multi-sine noise -- a sum of sine waves with randomized frequencies, phases, and signs unique to each tap instance.

- **Motion:** Smooth but unpredictable wandering in azimuth and distance
- **On the map:** A continuously shifting position with no repeating pattern. The trail shows a 2-second look-ahead of the predicted path
- **Use cases:** Ambient textures, organic spatial variation, generative music
- **Creative tip:** Use on multiple taps at once for a swarm-like effect. Runs at half the base speed for smoother motion. The path never repeats due to non-wrapping time accumulation.

### Spiral

The tap spirals outward from the origin and then wraps back, creating a continuous expanding-and-contracting circular path.

- **Motion:** Rotation with distance that increases then resets
- **On the map:** A spiral that expands outward from the origin. The wrap-back segment is not drawn to keep the trail clean.
- **Use cases:** Building tension, spatial expansion effects
- **Creative tip:** Runs at half the base speed. Combine with increasing feedback for spiraling echo buildups.

### Square

The tap traces a square path centered on the origin.

- **Motion:** Four straight segments forming a square, with corners aligned to the cardinal directions relative to the origin azimuth
- **On the map:** A square rotated by the origin azimuth, offset by origin distance
- **Use cases:** Geometric, mechanical spatial effects
- **Creative tip:** At moderate speeds, creates a distinctive four-corner panning effect ideal for rhythmic material

### Triangle

The tap traces an equilateral triangle path centered on the origin.

- **Motion:** Three straight segments forming a triangle, rotated by the origin azimuth
- **On the map:** A triangle rotated by the origin azimuth, offset by origin distance
- **Use cases:** Triangular panning, odd-meter spatial rhythms
- **Creative tip:** Three-sided symmetry pairs naturally with triplet note divisions for rhythmic spatial patterns

## Speed Control

The SPEED parameter controls how fast the trajectory runs, measured in Hertz (cycles per second):

| Speed | Cycle Time | Character |
|---|---|---|
| 0.05 Hz | 20 seconds | Very slow drift -- barely perceptible movement |
| 0.10 Hz | 10 seconds | Slow, gentle panning |
| 0.30 Hz | ~3.3 seconds | Default -- moderate, musical movement |
| 1.00 Hz | 1 second | Fast, noticeable rotation |
| 3.00 Hz | ~0.33 seconds | Very fast -- creates tremolo-like effects |
| 5.00 Hz | 0.2 seconds | Maximum -- extreme modulation territory |

> **Note:** Random and Spiral shapes run at half the displayed speed for smoother, more natural results.

## Direction Control

Each trajectory can run in **Forward** or **Reverse** direction:

- **Forward:** The default motion path (counterclockwise for circular shapes)
- **Reverse:** The path plays backward (clockwise for circular shapes)

Use the arrow buttons next to the trajectory selector in the bottom panel to toggle direction.

## Glow Trail Visualization

When a tap has an active trajectory, the spatial map draws a glow trail showing the trajectory path:

- **Brightness:** The trail is brightest near the tap's current animated position and dims further away along the path
- **Elevation encoding:** Trail opacity (0.3 to 1.0) and line thickness (1.0 to 5.5 pixels) vary with elevation, matching the tap dot's elevation visualization
- **Crosshair marker:** A small crosshair appears at the origin (knob) position, distinct from the animated dot

> **Note:** Trails are not drawn for shapes where a continuous path would be misleading (e.g., Random uses a 2-second look-ahead prediction trail instead).

## Interaction with ADM-OSC

When ADM-OSC receive is active and incoming position data is received for a tap:

- The trajectory is **temporarily paused** while external control is active
- After 500ms of silence (no incoming OSC messages), the trajectory **resumes** from the externally-set position
- OSC receive sets the origin position when a trajectory is active -- it does not override the animated position directly

When ADM-OSC send is active:

- The **animated position** (not the origin) is broadcast via `/adm/obj/N/aed` at 30 Hz
- This allows external renderers to track the trajectory animation in real time

See [ADM-OSC Integration](adm-osc.md) for protocol details.
