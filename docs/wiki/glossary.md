# Glossary

A reference of spatial audio and plugin terminology used throughout the OpenSpatialDelay documentation.

---

**ACN (Ambisonic Channel Number)**
The standardized channel ordering convention for Ambisonics signals. ACN numbers channels sequentially: 0 = W (omnidirectional), 1 = Y, 2 = Z, 3 = X for first order, and so on. OpenSpatialDelay uses ACN ordering in its AmbiX output.

**ADM-OSC (Audio Definition Model over Open Sound Control)**
An open protocol for transmitting object-based audio positions over a network. Uses OSC messages in the `/adm/obj/N/` namespace to communicate azimuth, elevation, distance, and Cartesian coordinates. Enables integration with tools like SPAT Revolution and L-ISA Controller.

**Air Absorption**
The natural phenomenon where high frequencies are attenuated more than low frequencies as sound travels through air. In OpenSpatialDelay, the AIR toggle simulates this effect based on each tap's distance from the listener, using a quadratic distance curve.

**Ambisonics**
A full-sphere spatial audio format that represents a sound field using spherical harmonics rather than discrete speaker signals. Can be decoded to any speaker layout. OpenSpatialDelay outputs AmbiX format (ACN/SN3D) from 1st through 6th order.

**AmbiX**
The standard file and signal format for Ambisonics, using ACN channel ordering and SN3D normalization. This is the most widely supported Ambisonics convention and the format OpenSpatialDelay uses for its Ambisonics output.

**Attenuation**
A reduction in signal level. In spatial audio, distance attenuation refers to how sounds become quieter as their source moves further from the listener, following an inverse-square law.

**AU (Audio Unit)**
Apple's native plugin format for macOS. Supported by Logic Pro, GarageBand, and most macOS DAWs. OpenSpatialDelay provides AU on macOS alongside VST3.

**Azimuth**
The horizontal angle of a sound source relative to the listener, measured in degrees. In OpenSpatialDelay: 0 degrees = front, positive = right (+90 = hard right), negative = left (-90 = hard left), +/-180 = behind.

**Binaural**
Audio specifically designed for headphone playback, using HRTF processing to create the perception of sounds coming from specific 3D locations around the listener. Distinguished from standard stereo, which only provides left-right positioning.

**Cardioid**
A directional microphone polar pattern shaped like a heart, most sensitive to sound from the front. Used in OpenSpatialDelay's XY stereo mode, which simulates a coincident pair of cardioid microphones.

**Convolution**
A mathematical operation that combines two signals -- in spatial audio, typically an audio signal with an impulse response (such as an HRTF). OpenSpatialDelay uses partitioned convolution for efficient real-time HRTF processing.

**Cutoff Frequency**
The frequency at which a filter begins to attenuate the signal, typically defined as the -3 dB point. In OpenSpatialDelay, the LP (low-pass) and HP (high-pass) knobs set cutoff frequencies for the feedback filters.

**DAW (Digital Audio Workstation)**
Software for recording, editing, and mixing audio. Examples include Reaper, Logic Pro, Ableton Live, Cubase, and Pro Tools. OpenSpatialDelay runs as a plugin within a DAW.

**DBAP (Distance-Based Amplitude Panning)**
A spatialization algorithm that computes speaker gains based purely on Euclidean distance from source to speaker. Unlike VBAP, it does not require triangulation and works with any speaker layout, including irregular arrangements.

**Dolby Atmos**
An object-based and channel-based audio format supporting height speakers. Speaker configurations include 7.1.4 (12 channels), 5.1.4 (10 channels), and others. OpenSpatialDelay supports Atmos bed formats from 5.1.2 through 9.1.6.

**Doppler Effect**
The change in perceived pitch when a sound source moves toward or away from the listener. Approaching sources sound higher in pitch; receding sources sound lower. OpenSpatialDelay simulates this per-tap based on trajectory velocity.

**Elevation**
The vertical angle of a sound source relative to the listener, measured in degrees. 0 degrees = ear level, +90 = directly above, -90 = directly below. Only audible in binaural, height-enabled surround, or Ambisonics output.

**Figure-8**
A bidirectional microphone polar pattern equally sensitive to sound from front and back, with nulls at the sides. Used in OpenSpatialDelay's Blumlein stereo mode (crossed figure-8 pair).

**FOA (First Order Ambisonics)**
The lowest order of Ambisonics, using 4 channels (W, Y, Z, X). Provides basic 3D spatial encoding but with limited spatial resolution. Higher orders improve imaging sharpness.

**Gain**
The amount of amplification or attenuation applied to a signal, typically measured in decibels (dB). Positive gain increases volume; negative gain decreases it. 0 dB = unity (no change).

**High-Pass Filter**
A filter that passes frequencies above its cutoff frequency and attenuates frequencies below it. In OpenSpatialDelay's feedback path, it removes low-frequency content from successive repeats.

**HOA (Higher Order Ambisonics)**
Ambisonics of 2nd order or above, providing sharper spatial resolution than FOA. OpenSpatialDelay supports up to 6th order (49 channels). In common usage, HOA often refers specifically to 3rd order.

**HRTF (Head-Related Transfer Function)**
A pair of filters (one per ear) that describe how sound is modified by the head, ears (pinnae), and torso as it travels from a specific direction to the eardrums. HRTFs encode the cues our brains use for 3D localization, including spectral shaping, interaural time differences, and interaural level differences.

**ILD (Interaural Level Difference)**
The difference in sound pressure level between the left and right ears for a given source direction. Sound from the right arrives louder at the right ear due to head shadowing. ILD is a primary cue for localizing mid and high-frequency sounds.

**ITD (Interaural Time Difference)**
The difference in arrival time of sound between the left and right ears. Sound from the right arrives at the right ear before the left ear. ITD is the dominant cue for localizing low-frequency sounds.

**KNN (K-Nearest Neighbor)**
A spatialization algorithm that computes speaker gains using inverse-distance-squared weighting to the nearest K speakers. Produces smooth, diffuse panning suitable for ambient sources.

**Latency**
The delay between when audio enters a plugin and when it exits. OpenSpatialDelay's latency depends on the HRTF convolution block size and the DAW's buffer settings. The "Simple (Low CPU)" profile has the lowest latency as it avoids convolution.

**LFE (Low-Frequency Effects)**
A dedicated low-frequency channel in surround formats (the ".1" in 5.1). OpenSpatialDelay derives its LFE signal by low-pass filtering the mix at 120 Hz and attenuating by -10 dB, following standard practice.

**LFO (Low-Frequency Oscillator)**
An oscillator running below audible frequencies, used for modulation. In OpenSpatialDelay, the MOD section uses an LFO to modulate the delay time, creating wobble/tape effects.

**Lissajous**
A family of curves traced by combining two sinusoidal motions at right angles. Several of OpenSpatialDelay's trajectory shapes (Figure-8, Infinity) are based on Lissajous figures with different frequency ratios.

**Low-Pass Filter**
A filter that passes frequencies below its cutoff frequency and attenuates frequencies above it. In OpenSpatialDelay's feedback path, it removes high-frequency content from successive repeats, creating a natural darkening effect.

**MDAP (Multiple Direction Amplitude Panning)**
An extension of VBAP that distributes a source across multiple VBAP directions simultaneously, creating a wider, more spread spatial image. Useful for sources that should occupy a region rather than a point.

**Object-Based Audio**
An audio paradigm where individual sound sources (objects) carry position metadata rather than being pre-mixed to specific channels. The renderer maps objects to the playback speaker layout in real time. Dolby Atmos and ADM-OSC are object-based systems.

**OSC (Open Sound Control)**
A network protocol for real-time communication of multimedia parameters. Uses UDP for low-latency message delivery. OpenSpatialDelay uses OSC to implement the ADM-OSC position control protocol.

**Resonance (Q)**
A parameter of a filter that controls the sharpness of the response at the cutoff frequency. Low Q (0.5-0.71) = smooth rolloff. High Q (2.0-8.0) = a peak at the cutoff, creating a ringing or whistling quality. Q of 0.707 is the Butterworth (maximally flat) response.

**SN3D (Schmidt Semi-Normalized)**
A normalization scheme for spherical harmonics used in the AmbiX Ambisonics convention. SN3D ensures that all harmonic components have similar energy levels regardless of order. OpenSpatialDelay uses SN3D in its Ambisonics output.

**SOFA (Spatially Oriented Format for Acoustics)**
An open file format for storing measured spatial acoustic data, including HRTFs. SOFA files contain HRIR (Head-Related Impulse Response) measurements at many directions. OpenSpatialDelay includes 5 SOFA files from different measurement sources.

**VBAP (Vector Base Amplitude Panning)**
A spatialization algorithm developed by Ville Pulkki (1997) that selects the two nearest speakers (2D) or three nearest speakers (3D) and distributes gain across them using vector mathematics. Produces focused, precise spatial images.

**VBIP (Vector Base Intensity Panning)**
A variant of VBAP that squares the gains, producing an even tighter spatial image. The energy is more concentrated on the nearest speakers, resulting in a sharper perceived source position.

**VST3 (Virtual Studio Technology 3)**
Steinberg's cross-platform plugin format, supported by virtually all modern DAWs. VST3 supports multi-channel audio, sample-accurate automation, and dynamic I/O. OpenSpatialDelay provides VST3 on both macOS and Windows.

**WSOLA (Waveform Similarity Overlap-Add)**
A time-domain pitch shifting algorithm that preserves timing by finding similar waveform segments and overlapping them. OpenSpatialDelay uses WSOLA-lite for per-tap pitch shifting, ensuring delay rhythm stays locked regardless of pitch offset.
