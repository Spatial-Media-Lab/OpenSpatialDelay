//==============================================================================
// screenshot_tool — Standalone CLI tool for capturing plugin UI as PNG
//
// Creates an OpenSpatialDelayProcessor instance, instantiates the editor,
// renders it to an off-screen image, and saves to PNG — no DAW required.
//
// Usage:
//   screenshot_tool [output_path] [scale_factor]
//
// Default: plugin_screenshot.png at 2x scale
//==============================================================================

#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <iostream>

int main (int argc, char* argv[])
{
    // ScopedJuceInitialiser_GUI initialises the MessageManager, graphics
    // subsystem, and everything else JUCE GUI components need to function.
    juce::ScopedJuceInitialiser_GUI juceInit;

    // ── Parse CLI arguments ──────────────────────────────────────────────
    juce::String outputPath = "plugin_screenshot.png";
    float scaleFactor = 2.0f;

    if (argc > 1)
        outputPath = juce::String (argv[1]);

    if (argc > 2)
    {
        float parsed = juce::String (argv[2]).getFloatValue();
        if (parsed > 0.0f && parsed <= 8.0f)
            scaleFactor = parsed;
        else
            std::cerr << "Warning: scale factor must be between 0 and 8. "
                      << "Using default 2x." << std::endl;
    }

    // ── Create processor and editor ──────────────────────────────────────
    OpenSpatialDelayProcessor processor;

    // prepareToPlay is required before the processor is in a valid state.
    // Use a standard sample rate and block size — the UI doesn't depend on
    // exact values, but JUCE's internal plumbing expects preparation.
    processor.prepareToPlay (48000.0, 512);

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
