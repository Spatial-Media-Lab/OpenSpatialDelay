# New plugin just dropped!

One-line pitch: each echo has a 3D position in space.

OpenSpatialDelay v1.0.0 is out today — a free, open-source spatial delay
plugin for VST3 and AU on macOS and Windows. GPL-3.0, binaural HRTF
rendering, object-based positioning, and the first tool in the Spatial
Media Library pipeline.

## OpenSpatialDelay v1.0.0 — a free spatial delay for VST3 and AU

![OpenSpatialDelay plugin UI](https://spatialmedialab.org/wp-content/uploads/2026/04/screenshot_full.png)

I've been wanting a delay like this for years.

Most delays give you a stereo field — left, right, and a few tricks in the
middle. When I started working in immersive formats — Ambisonics sessions,
Atmos mixes, binaural headphone pieces — I kept reaching for a delay and
finding that the thing I actually wanted was missing. Not another ping-pong,
not a wider stereo image. I wanted each repeat of a sound to live somewhere
real. Above, behind, four metres to the left, drifting out and upward at
the end of the tail. A delay that thinks in space rather than in a pair of
speakers.

So I built one. I started on March 6, 2026, and seven weeks later — today
— it ships. Free, GPL-3.0, VST3 and AU, macOS and Windows. That's
OpenSpatialDelay v1.0.0.

The demo below is 30 seconds, binaural, headphones on. If you don't have
headphones handy right now that's fine — come back to it. The whole point
of a binaural delay is that it only reveals itself through ears, not
speakers.

[AUDIO EMBED — CONT-01 — SoundCloud iframe / Bandcamp iframe / WP Media
Library `<audio controls>` tag. Caption: "Headphones required. The effect
is binaural — it lives in stereo-with-HRTFs, not in the speaker field."]

![Spatial map view](https://spatialmedialab.org/wp-content/uploads/2026/04/screenshot_spatial_map.png)

## What's inside

I wanted the feature surface to match how spatial mixers actually think,
not how delay plugins usually present themselves. A spatial mixer cares
about where a sound is, how it moves, and whether you can sync those
positions to the outside world. A delay plugin cares about taps, feedback,
filtering, and time. OpenSpatialDelay does both — every parameter you'd
expect from a serious delay, plus every parameter you'd expect from a
scene-based spatial tool. So here's what's inside:

- **12 delay taps**, each with an independent 3D position. Put them on
  a circle, a spiral, a staircase, a cloud, a line overhead — wherever the
  piece wants them. Each tap has its own feedback, its own filter, its own
  level, its own trajectory.
- **7 spatialization algorithms** (Ambisonics, ConstantPower, DBAP, KNN,
  MDAP, VBAP, VBIP) so you can pick the right one for a stereo mix, a
  5.1 room, a dome, or a full immersive rig. Swap between them per preset.
  Swap between them mid-session to hear how the same arrangement translates
  across formats.
- **5 measured HRTF profiles** (KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE)
  plus a Simple mode for basic binaural spatialization without the
  coloration of HRTFs. The profiles are real measured datasets, not
  modelled approximations — each one places sound differently, and finding
  the one that works best for your ears is part of the fun.
- **70 factory presets** to start from — shaped by what real mixers do
  with real spatial delays, not just programmer demos. Ambient beds,
  cinematic whooshes, rhythmic counter-lines, binaural-headphone specials,
  8-channel dome setups, practical utility patches for Atmos sessions.
- **A trajectory engine** that moves each delay tap through space. Linear
  sweeps, orbits, pendulums, random paths. Every delay tap can have its
  own path.
- **ADM-OSC in and out** so OSD talks to Spat Revolution, Panoramix, Iannix,
  TouchDesigner, and the rest of the object-based ecosystem. Automate
  positions from outside. Send the plugin's positions outward to drive a
  visualiser. Drop it into an existing ADM workflow effortlessly.
- **VST3 + AU, macOS + Windows, GPL-3.0.** Free to download, free to use
  in commercial work, free to fork and modify. No trial timer, no licence
  server, no unlock dance. Simply, if you like it and want to see more
  where that came from, sign up for my
  [Patreon](https://patreon.com/AndrewRahman) and support my work for the
  price of a coffee a month.

![Signal flow](https://spatialmedialab.org/wp-content/uploads/2026/04/signal-flow.png)

OpenSpatialDelay is the first tool in the Spatial Media Library pipeline.
The plan is a small family of spatial-audio plugins that share the same
framework underneath — panners, choruses, reverbs, and synthesizers.
Build the engine once, ship several tools on top of it.

Keeping the whole pipeline open means two things.

One: the tools are genuinely free, right now, for everyone —

- students learning Ambisonics
- composers scoring their first VR piece
- sound designers on a deadline with no budget for a dearVR seat
- mixers who want to experiment with immersive without committing to a
  subscription

Two: the work is transparent. The source is on GitHub. If you want to see
exactly how the HRTF path is wired, or why a particular edge case is
handled a particular way, you can read it. You can patch it. You can fork
the whole thing and take it somewhere I never would have thought of.

If you want to support the work and keep the pipeline moving,
[Patreon](https://patreon.com/AndrewRahman) is where that happens. Nothing
goes behind a paywall — backers fund the time that makes the next tool
possible, and help steer the direction of the project. That's the deal.

## Download

**[Download OpenSpatialDelay v1.0.0](https://andrewrahman.com/get-osd)**

Drop your email at the link above and we'll send you the VST3 and AU
binaries for macOS and Windows. macOS Sequoia users will need the `xattr`
command documented there — Apple's Gatekeeper flags unsigned free plugins
more aggressively on recent OS versions, and the fix is one terminal line.

The source is GPL-3.0, so if you'd rather build from source, everything's
on GitHub — the CMake config, the JUCE submodule reference, the test
suite, the SOFA files for every HRTF profile, the preset bank, the lot.
**[github.com/Spatial-Media-Lab/OpenSpatialDelay](https://github.com/Spatial-Media-Lab/OpenSpatialDelay)**

If you want to support the work, Patreon keeps the tools free:
**[patreon.com/AndrewRahman](https://patreon.com/AndrewRahman)**

**Links:**

- **Source code:** [github.com/Spatial-Media-Lab/OpenSpatialDelay](https://github.com/Spatial-Media-Lab/OpenSpatialDelay)
- **Get OSD (mailing list + download):** [andrewrahman.com/get-osd](https://andrewrahman.com/get-osd)
- **Support on Patreon:** [patreon.com/AndrewRahman](https://patreon.com/AndrewRahman)
- **Spatial Media Lab (org):** [spatialmedialab.org](https://spatialmedialab.org)
- **Creator home:** [andrewrahman.com](https://andrewrahman.com)

If you make something with it — a track, a field recording treatment, a
game-audio experiment, a piece for headphones, anything — I'd love to
hear it. Tag @spatialmedialab or drop me a line through the site. It's
been a very long time since I've wanted this thing and I can't wait to
see what everyone else does with it.

See you at Superbooth!

---

Tags: Spatial Audio, Binaural, Dolby Atmos, VST3, AU, Free, GPL, OpenSpatialDelay
Categories: Announcements, Software

Published April 28, 2026 By Andrew
