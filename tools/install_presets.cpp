//==============================================================================
// install_presets — Build-time CLI tool for generating factory preset files
//
// Serializes compiled-in factory presets to .osdpreset JSON files in the
// standard preset directory.  Linked against juce_core only (no audio/GUI).
//
// Usage:
//   install_presets [target_directory]
//
// Default target: ~/Library/Audio/Presets/OpenSpatialDelay/
//==============================================================================

#include "../Source/PresetData.h"
#include <iostream>

int main (int argc, char* argv[])
{
    // juce_core functions (File, JSON, String) work without GUI initialisation

    juce::File targetDir;

    if (argc > 1)
    {
        targetDir = juce::File (juce::String (argv[1]));
    }
    else
    {
        // Default: ~/Library/Audio/Presets/OpenSpatialDelay/
        targetDir = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
                        .getChildFile ("Library")
                        .getChildFile ("Audio")
                        .getChildFile ("Presets")
                        .getChildFile ("OpenSpatialDelay");
    }

    if (! targetDir.exists())
        targetDir.createDirectory();

    installFactoryPresets (targetDir);

    std::cout << "Installed " << NUM_FACTORY_PRESETS
              << " factory presets to " << targetDir.getFullPathName().toStdString()
              << std::endl;

    return 0;
}
