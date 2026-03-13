This file only contains the major changes of the IEM Plug-in Suite.
For a list of all changes and bugfixes see the git log.

Please note, that breaking changes are marked with `!!BREAKING CHANGE!!`. They might lead to an unexpected behavior and might not be compatible with your previous projects without making some adaptions. See the [Breaking changes article](https://plugins.iem.at/docs/breakingchanges/) for more information.

## v1.14.0
- general changes
    - moved to JUCE 7.0.4
    - LV2 plug-ins for macOS, Linux and Windows
    - remove deprecated *.jucer files
    - `clang-format`ting the whole codebase
    - smaller bug fixes and improvements
- new plug-ins
    - **Granular**Encoder
        - Granular synthesizer and encoder
-  plug-in specific changes
    -  **Distance**Compensator
        - Change plug-in code from DiCo to DisC, so it is different to the **Directional**Compressor
    -  **Energy**Visualizer
        - Added variable RMS time constant and max hold 

## v1.13.0
- general changes
  - moved from Projucer to CMake build setup
  - added VST3 support (which may have some limitations)

## v1.12.0
- general changes
    - moved to JUCE 6
- plug-in specific changes
    - **Fdn**Reverb
        - fixed shared memory bug
    - **Multi**Encoder
        - fixed bug where coordinates weren't recalled correctly when master rotation was active
        - adds visualization of input levels

## v1.11.1
- general changes
    - standalones: prevents crashes when there's no audio input device
    - smaller bug fixes
- plug-in specific changes
    - **AllRA**Decoder
        - removed the 64 channel number limit for loudspeakers layouts with more than 64 loudspeakers
    - **Simple**Decoder
        - decoder configuration is now stored within the plug-in state when saving a session, so no need to provide the decoder file when sharing projects
        - added decoder presets for standard layouts and IEM studios
    -**Energy**Visualizer
        - sending RMS values with OSCAddressPattern "/RMS"

## v1.11.0
- general changes
    - new standalone versions with JACK support on linux and macOS
    - adds OSC Send capabilities to each plug-in

- plug-in specific changes
    -**AllRA**Decoder
        - adds Ambisonic weight selector: switch between basic, maxrE, and inPhase weights
        - selected weights are exported to JSON
        - changes decibel range of energy visualization from +-/3dB to +/- 1.5dB
    - **Binaural**Decoder
        - fixes bug which lead to +6dB per sampleRate doubling
    - **Energy**Visualizer
        - performance improvements
        - sends RMS values for 426 sample points via OSC
    - **Multi**Encoder
        - source directions can be imported via JSON config files
        - adds master controls to adjust directions and gains of all sources simultaneously
    - **Room**Encoder
        - shelving-filters now operate on all reflection orders (not only the first two)
    - **Simple**Decoder
        - adds master gain
        - Ambisonic weight selector: switch between basic, maxrE, and inPhase weights
        - modified level compensation for different input orders

## v1.10.2
- general changes
    - fixed bug in SIMD IIR processing (**MultiBand**Compressor, **Multi**EQ, **Room**Encoder), which in some constellations led to very loud output
    - fixed filter visualization being different for different sampling frequencies
    - limits internal filter frequencies to Nyquist-frequency for IIR stability
    - minor fixes and improvements

- plug-in specific changes
    - **Room**Encoder
        - fixed faulty room sync on channel #4


## v1.10.1
- plug-in specific changes
    - **Room**Encoder
        - hides direct path visualization when disabled
        - adds extra parameters to adjust an additional attenuation per wall
        - fixes crackles bug
    - **Simple**Decoder
        - fixes bug: reset of Ambisonic order when re-opening GUI
    - **Tool**Box
        - adds overall gain control
    - **Directivity**Shaper
        - fixing directions of elevation sliders

## v1.10.0
- new plug-ins
    - **MultiBand**Compressor

- general changes
    - Linux Builds: JACK clients name will be the plug-in's name
    - all parameters can be controlled via vendorSpecific calls
    - OSCPort can be opened via vendorSpecific calls

- plug-in specific changes
    - **Fdn**Reverb
        - reverberation time visualization bug fixed
    - **Room**Encoder
        - restraining source and listener positions to room boundaries
        - increases minimum distance between source and listener to 0.1m
        - deactivates minimum distance control, if direct path isn't renderer
        - added direct path unity-gain option
        - added direct path zero-delay option
        - added button to deactivate direct path rendering (feature was hidden before)
    - **Scene**Rotator
        - MIDI Devices can now be opened directly to receive rotation data (for now support for MrHeadTracker only, more will follow)


## v1.9.0
- general changes
    - plug-in host will be notified when paramater values change automatically (e.g. conversions)
    - osc port can be changed on the fly when opened, plug-ins will close and re-open port immediately

- plug-in specific changes
    - **Binaural**Decoder
        - output level reduced by 10dB
    - **Distance**Compensator
        - fixes broken import of loudspeaker layouts
    - **Energy**Visualizer
        - levels will only be calculated when the GUI is open
        - adds control to change visualized dynamic range
    - **Scene**Rotator
        - adds quaternion inversion
        - adds selectable sequence of rotations

## v1.8.1
- plug-in specific changes
    - **AllRA**Decoder
        - fixes: AllRADecoder forgetting layout when loading previous state

## v1.8.0
- general changes
    - rafactored widgets (internal change)

- new plug-ins:
    - **Multi**EQ
        - equalizer plug-in with several filter types, filtering up to 64 audio channels
    - **Scene**Rotator
        - simple rotator of Ambisonic scenes

- plug-in specific changes
    - **Fdn**Reverb
        - mouse-wheel controls Q factors of filters within the FilterVisualizer



## v1.7.1
- general changes
    - smaller GUI re-draw improvements
    - support for GenericLayout in configuration files

 - plug-in specific changes
    - **Room**Encoder
        - fixes possible crashs when host's buffersize is smaller than reported (can happen with loops)
    - **Stereo**Encoder
        - performance improvements
    - **Matrix**Multiplier
        - fixes wrong displayed number of in/out channels

## v1.7.0
- general changes
    - OSC 'connect/disconnect' buttons replaced by  'open/close' buttons
    - support for OSC bundles

- plug-in specific changes
    - **Fdn**Reverb
        - fade-in functionality for more diffuseness at the beginning
        - internal fdn size now switchable (for different sounds and lower cpu usage)
    - **Stereo**Encoder
        -   adds shortcuts for panning positions (e.g. shift+f for front, ...)

## v1.6.0
- general changes
    - OSC support: every plug-in is now controllable via OSC
    - SpherePanner will change only azimuth when right-clicked and dragged

- plug-in specific changes
    - **AllRA**Decoder
        - alt+clicking the Noise button, will encode the test-signal to the loudspeaker's direction
        - tooltips added to some buttons (noise, import, export, rotate, add loudspeakers)
    - **Coordinate**Converter
        - added flip toggles for each in- and output parameter



## v1.5.0
- general changes
    - decoders (e.g. SimpleDecoder) can handle inPhase weights now

- new plug-ins:
    - **Coordinate**Converter
        - a plug-in which converts spherical to cartesian coordinates and vice versa, e.g. for converting automations

- plug-in specific changes
    - **Distance**Compensator
        - fixes wrong initialization of gains and delays
        - adds error message when no loudspeaker layout has been loaded


## v1.4.0
- general changes
    - all plug-ins now make use of methods which save/recall the plug-in states (necessary for some DAWs)

- new plug-ins:
    - **Distance**Compensator
        - a plug-in which compensates gain and delay for loudspeaker arrays with different distances to the listener

- plug-in specific changes
    - **AllRA**Decoder
        - rotate feature added: rotates the whole layout by any arbitrary angle around the z-axis
    - **Energy**Visualizer
        - colormaps now switchable by clicking on the colormap
    - **Room**Encoder
        - added parameter to disable direct path
    - **Omni**Compressor
        - added visualization of the compressor's characteristic
        - added look-ahead feature to avoid distortion artifacts when brickwall-limiting

## v1.3.1
- plug-in specific changes
    - **Energy**Visualizer
        - using a perceptually motivated colormap
    - **AllRA**Decoder
        - clicking 'add loudspeaker' while holding the alt-key adds an imaginary loudspeaker at the nadir
        - added noise generator for testing correct signal routing
    - **Room**Encoder
        - fixing wrong slider values (gui issue)
    - **Fdn**Reverb
        - fixing wrong slider values (gui issue)

## v1.3.0
- general changes
    - unity gain normalization has moved fully to the decoding stage:  (`!!BREAKING CHANGE!!`)
        - encoding a source results in unity gain in the W-channel (omni)
        - decoding is normalized resulting in unity gain for the sampled mono signal
    - renamed **Matrix**Multiplicator to **Matrix**Multiplier (`!!BREAKING CHANGE!!`)
    - refactored SpherePanner, which fixes automation writing issues
    - added linear elevation plot style to SpherePanner (triggered by double-click)

- plug-in specific changes
    - **AllRA**Decoder
        - fixed distorted visualization
        - added visualization of the acos-rE source width
    - **Binaural**Decoder
        - performance improvements
        - new binaural filters
    - **Dual**Delay
        - fixed high CPU load which occurred occasionally
    - **Matrix**Multiplier
        - smaller GUI changes
    - **Multi**Encoder
        - colour-chooser now has the same look as the elements in the sphere panner, again
        - Solo/Mute changes will repaint sphere
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **Stereo**Encoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **Probe**Decoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **DirectionalCompressor**Decoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
        - adjusted parameter limits (threshold and makeup gain)  (`!!BREAKING CHANGE!!`)
    - **Directivity**Shaper
        - added SN3D support (`!!BREAKING CHANGE!!`)
        - renamed parameters (probe instead of master)  (`!!BREAKING CHANGE!!`)
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
        - adjusted parameter limits (threshold and makeup gain)  (`!!BREAKING CHANGE!!`)
    - **Room**Shaper
        - added SN3D support (`!!BREAKING CHANGE!!`)
    - **Simple**Decoder
        - new bass-management (`!!BREAKING CHANGE!!`)
        - added warning, showing if subwoofer channel is already occupied


## v1.2.0
- new plug-ins:
    - **AllRA**Decoder

- plug-in specific changes
    - **Binaural**Decoder
        - refactored convolution -> way better performance on all platforms
    - **Matrix**Multiplicator
        - fixed bug that only input channels up to a square-number count are processed (e.g. 16 instead of 20)

## v1.1.1
- general changes
    - added binaural IO widget
    - directivity IO widget now shows the normalization (which is N3D)
    - added warning sign to titlebar widgets when bus is too small

- plug-in specific changes
    - **Multi**Encoder
        - changed element colours are now displayed at once
    - **Binaural**Decoder
        - making use of new binaural IO widget
        - performance improvements on windows and linux
    - **Directivity**Shaper
        - better GUI performance
        - 5dB subgrid for directivity shaper
        - more linear default values for filters
    - **Room**Encoder
        - fixed bug which lead to filter output values way beyond of 1
    - **Tool**Box
        - added adaptor tool for lower order Ambisonic (LOA) signals to be compatible with HOA decoder weights


## v1.1.0
- new plug-ins:
    - **Matrix**Multiplicator
    - **Simple**Decoder
    - **Tool**Box
    - **Binaural**Decoder

- general changes
    - switched to JUCE develop branch !!
    - added configurations (decoders,...)
    - some GUIs now resizable
    - smaller GUI layout changes (e.g. ComboBox PopupMenu)

- plug-in specific changes
    - **Multi**Encoder
        - maximum input channel size increased to 64
        - parameters and colours are stored correctly now
        - MasterPanner is now controllable with mouse wheel (and modifiers)
        - GUI resizable
    - **Omni**Compressor and **Directional**Compressor
        -  new compressor engine
        -  added 'knee' parameter
        -  can be used as limiters now
    - **Fdn**Reverb
        -  added a visual T60 recommendation
        -  maximum reverberation time reduced
        -  filters can now have positive gain
        -  layout restructured, resizable
    -  **Room**Encoder
        -  added floor reflections (now up to 236 reflections possible)
        -  default number of reflections set to 19 (chosen arbitrarily)
    -  **Stereo**Encoder
        -  added hidden high-quality mode
