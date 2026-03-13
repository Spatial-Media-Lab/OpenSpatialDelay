# How to use the PluginTemplate

## Copy and rename files

- make a copy of the folder "_PluginTemplate"
- rename it to any desired plug-in name.
  - the plug-in's name should be composed of two words with capitals (eg. NicestPlugin, SuperDecoder, HolyGrail)

## Change project settings

In order to be able to integrate your Plug-In into the Suite, [CMake](https://cmake.org) should be used for new projects instead of the (deprecated) Projucer. A brief documentation of the CMake API in JUCE can be found in its [repository](https://github.com/juce-framework/JUCE/blob/ba8d5e3e1cc58b6666565e068854991f15ab231b/docs/CMake%20API.md). The usage with the IEM Audioplugins is described in the [readme](https://git.iem.at/audioplugins/IEMPluginSuite#compilation-guide)

A few general settings in `CMakeLists.txt`:

- Replace all occurrences of `PluginTemplate` with the name of your plug-In
- The `PLUGIN_CODE` should resemble the first two letters of the two words of the plug-in's name (eg. NiPl, SuDe, HoGr)
- Set an appropriate `VERSION`
- If needed: Add additional source files in `target_sources`, eg. `../resources/efficientSHvanilla.cpp` will most likely be necessary for Ambisonics stuff
- If needed: You can easily [link libraries](https://cmake.org/cmake/help/v3.15/command/target_link_libraries.html) with CMake!

For development and debugging of a single plug-in, you may want to add your plug-in to the `CMakeLists.txt` in the root directory and comment all other plug-ins out.

## Change file content

There are four main files in the `Source` directory:

- `PluginEditor.cpp`
- `PluginEditor.h`
- `PluginProcessor.cpp`
- `PluginProcessor.h`

Each of them has to be changed:

- replace the author name (`Daniel Rudrich`) with your name (or names if several persons are working together)
- replace every appearance of `PluginTemplate` with your plug-in's name (eg. `PluginTemplateAudioProcessor` -> `NicestPluginAudioProcessor`)

Change the TitleText in `PluginEditor.cpp` to your plug-in's name  (eg. `title.setTitle(String("Nicest"),String("Plugin"));`)