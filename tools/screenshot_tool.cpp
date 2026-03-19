//==============================================================================
// screenshot_tool — Standalone CLI tool for capturing plugin UI as PNG
//
// Creates an OpenSpatialDelayProcessor instance, instantiates the editor,
// optionally loads a preset, renders to an off-screen image, and saves to PNG.
// No DAW required.
//
// Usage:
//   screenshot_tool [output_path] [scale_factor] [--preset name_or_path]
//
// Examples:
//   screenshot_tool                                       # default state, 2x
//   screenshot_tool ui.png 2.0                            # custom path, 2x
//   screenshot_tool orbit.png 2.0 --preset "Orbit Dance"  # load factory preset
//   screenshot_tool custom.png 2.0 --preset /path/to.osdpreset  # load file
//   screenshot_tool --list-presets                         # list all factory presets
//==============================================================================

#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/PresetData.h"
#include <iostream>

//==============================================================================
// Find a factory preset by name (case-insensitive partial match)
//==============================================================================
static int findPresetByName (const juce::String& searchName)
{
    const int numPresets = NUM_FACTORY_PRESETS;

    // Exact match first (case-insensitive)
    for (int i = 0; i < numPresets; ++i)
    {
        if (factoryPresets[static_cast<size_t> (i)].name.equalsIgnoreCase (searchName))
            return i;
    }

    // Partial match (case-insensitive, name contains search string)
    for (int i = 0; i < numPresets; ++i)
    {
        if (factoryPresets[static_cast<size_t> (i)].name.containsIgnoreCase (searchName))
            return i;
    }

    return -1; // not found
}

//==============================================================================
// Apply a PresetData to the processor via APVTS parameter updates
//==============================================================================
static void applyPresetToProcessor (OpenSpatialDelayProcessor& processor,
                                    const PresetData& preset)
{
    auto& apvts = processor.apvts;

    auto setFloat = [&] (const juce::String& paramId, float value) {
        if (auto* p = apvts.getParameter (paramId))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };
    auto setChoice = [&] (const juce::String& paramId, int choiceIndex) {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
        {
            int numItems = p->choices.size();
            if (numItems > 1)
                p->setValueNotifyingHost (static_cast<float> (choiceIndex)
                                          / static_cast<float> (numItems - 1));
        }
    };
    auto setBool = [&] (const juce::String& paramId, bool value) {
        if (auto* p = apvts.getParameter (paramId))
            p->setValueNotifyingHost (value ? 1.0f : 0.0f);
    };

    // Global params
    setFloat  ("delayTime",     preset.delayTime);
    setBool   ("tempoSync",     preset.tempoSync);
    setFloat  ("noteDivision",  preset.noteDivision);
    setChoice ("syncMode",      preset.syncMode);
    setFloat  ("feedback",      preset.feedback);
    setFloat  ("filterLP",      preset.filterLP);
    setFloat  ("filterHP",      preset.filterHP);
    setFloat  ("filterLPQ",     preset.filterLPQ);
    setFloat  ("filterHPQ",     preset.filterHPQ);
    setFloat  ("pitchShift",    preset.pitchShift);
    setFloat  ("dryWet",        preset.dryWet);
    setFloat  ("inputGain",     preset.inputGain);
    setFloat  ("outputGain",    preset.outputGain);
    setBool   ("airAbsorption", preset.airAbsorption);
    setBool   ("filterEnabled", preset.filterEnabled);
    setBool   ("wobbleEnabled", preset.wobbleEnabled);
    setFloat  ("wobbleAmount",  preset.wobbleAmount);
    setFloat  ("wobbleMorph",   preset.wobbleMorph);
    setChoice ("algorithm",     preset.algorithm);
    setChoice ("hrtfProfile",   preset.hrtfProfile);

    // Per-tap params
    for (int i = 0; i < 12; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        const auto& tap = preset.taps[i];

        setBool   (prefix + "enabled",             tap.enabled);
        setFloat  (prefix + "azimuth",             tap.azimuthDeg);
        setFloat  (prefix + "elevation",           tap.elevationDeg);
        setFloat  (prefix + "distance",            tap.distance);
        setFloat  (prefix + "dopplerAmount",       tap.dopplerAmount);
        setFloat  (prefix + "pitchShift",          tap.pitchShift);
        setChoice (prefix + "trajectoryShape",     tap.trajectoryShape);
        setFloat  (prefix + "trajectorySpeed",     tap.trajectorySpeed);
        setChoice (prefix + "trajectoryDirection", tap.trajectoryDirection);
        setChoice (prefix + "inputChannel",        tap.inputChannel);
    }
}

//==============================================================================
// Load a preset from a .osdpreset / .json file on disk
//==============================================================================
static bool loadPresetFromFile (OpenSpatialDelayProcessor& processor,
                                const juce::File& file)
{
    if (! file.existsAsFile())
    {
        std::cerr << "Error: preset file not found: "
                  << file.getFullPathName().toStdString() << std::endl;
        return false;
    }

    auto json = file.loadFileAsString();
    auto parsed = parsePresetJson (json);
    if (parsed.name.isEmpty())
    {
        std::cerr << "Error: could not parse preset file: "
                  << file.getFullPathName().toStdString() << std::endl;
        return false;
    }

    applyPresetToProcessor (processor, parsed);
    std::cout << "Loaded preset from file: " << parsed.name.toStdString()
              << " (" << parsed.category.toStdString() << ")" << std::endl;
    return true;
}

//==============================================================================
int main (int argc, char* argv[])
{
    // ScopedJuceInitialiser_GUI initialises the MessageManager, graphics
    // subsystem, and everything else JUCE GUI components need to function.
    juce::ScopedJuceInitialiser_GUI juceInit;

    // ── Check for --list-presets ──────────────────────────────────────────
    for (int i = 1; i < argc; ++i)
    {
        if (juce::String (argv[i]) == "--list-presets")
        {
            std::cout << "Factory presets (" << NUM_FACTORY_PRESETS << "):" << std::endl;
            for (int j = 0; j < NUM_FACTORY_PRESETS; ++j)
            {
                std::cout << "  " << j << ": ["
                          << factoryPresets[j].category.toStdString() << "] "
                          << factoryPresets[j].name.toStdString() << std::endl;
            }
            return 0;
        }
    }

    // ── Parse CLI arguments ──────────────────────────────────────────────
    juce::String outputPath = "plugin_screenshot.png";
    float scaleFactor = 2.0f;
    juce::String presetArg;

    // Collect positional args and --preset flag
    std::vector<juce::String> positionalArgs;
    for (int i = 1; i < argc; ++i)
    {
        juce::String arg (argv[i]);
        if (arg == "--preset" && i + 1 < argc)
        {
            presetArg = juce::String (argv[++i]);
        }
        else if (! arg.startsWith ("--"))
        {
            positionalArgs.push_back (arg);
        }
    }

    if (positionalArgs.size() > 0)
        outputPath = positionalArgs[0];

    if (positionalArgs.size() > 1)
    {
        float parsed = positionalArgs[1].getFloatValue();
        if (parsed > 0.0f && parsed <= 8.0f)
            scaleFactor = parsed;
        else
            std::cerr << "Warning: scale factor must be between 0 and 8. "
                      << "Using default 2x." << std::endl;
    }

    // ── Create processor and editor ──────────────────────────────────────
    OpenSpatialDelayProcessor processor;

    // prepareToPlay is required before the processor is in a valid state.
    processor.prepareToPlay (48000.0, 512);

    // ── Load preset if requested ─────────────────────────────────────────
    if (presetArg.isNotEmpty())
    {
        // Check if it's a file path
        juce::File presetFile (presetArg);
        if (presetFile.existsAsFile())
        {
            if (! loadPresetFromFile (processor, presetFile))
                return 1;
        }
        else
        {
            // Try as a factory preset name
            int idx = findPresetByName (presetArg);
            if (idx >= 0)
            {
                applyPresetToProcessor (processor, factoryPresets[static_cast<size_t> (idx)]);
                std::cout << "Loaded factory preset: "
                          << factoryPresets[static_cast<size_t> (idx)].name.toStdString()
                          << std::endl;
            }
            else
            {
                std::cerr << "Error: preset not found: "
                          << presetArg.toStdString() << std::endl;
                std::cerr << "Use --list-presets to see available factory presets."
                          << std::endl;
                return 1;
            }
        }
    }

    // ── Create editor ────────────────────────────────────────────────────
    auto* editorRaw = processor.createEditor();
    if (editorRaw == nullptr)
    {
        std::cerr << "Error: createEditor() returned nullptr." << std::endl;
        return 1;
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (editorRaw);

    // Set bounds to the plugin's designed window size
    editor->setBounds (0, 0, 820, 580);

    // ── Render to image ──────────────────────────────────────────────────
    auto image = editor->createComponentSnapshot (
        editor->getLocalBounds(),
        true,   // paintEntireComponent
        scaleFactor
    );

    if (! image.isValid())
    {
        std::cerr << "Error: createComponentSnapshot returned an invalid image."
                  << std::endl;
        return 1;
    }

    // ── Write PNG ────────────────────────────────────────────────────────
    juce::File outFile = juce::File::getCurrentWorkingDirectory()
                             .getChildFile (outputPath);

    // Create parent directories if needed
    outFile.getParentDirectory().createDirectory();

    juce::FileOutputStream stream (outFile);
    if (stream.failedToOpen())
    {
        std::cerr << "Error: could not open output file: "
                  << outFile.getFullPathName().toStdString() << std::endl;
        return 1;
    }

    juce::PNGImageFormat pngFormat;
    if (! pngFormat.writeImageToStream (image, stream))
    {
        std::cerr << "Error: failed to write PNG data." << std::endl;
        return 1;
    }

    std::cout << "Screenshot saved to "
              << outFile.getFullPathName().toStdString()
              << " (" << image.getWidth() << "x" << image.getHeight()
              << " @ " << scaleFactor << "x)"
              << std::endl;

    // ── Cleanup ──────────────────────────────────────────────────────────
    // Release editor before processor goes out of scope
    editor.reset();
    processor.releaseResources();

    return 0;
}
