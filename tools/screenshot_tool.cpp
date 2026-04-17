//==============================================================================
// screenshot_tool — Standalone CLI for capturing plugin UI states as PNG
//
// Creates an OpenSpatialDelayProcessor instance, instantiates the editor,
// optionally loads a preset, drives it into a specific UI state, renders to an
// off-screen image, and saves to PNG. No DAW required.
//
// Usage:
//   screenshot_tool [output_path] [scale_factor] [options]
//
// Options:
//   --preset <name|path>   Load a factory preset by name or a preset file
//   --mode <name>          Capture mode (see below). Default: full.
//   --list-presets         List factory presets and exit
//
// Capture modes:
//   full              — Entire 820x580 editor, baseline state
//   drawer-open       — Editor with Global Tap Drawer open and populated
//   tone-section      — Crop of the TONE section (filter graph + readout)
//   osc-section       — Crop of the OSC section (receive + send rows)
//   save-overlay      — Editor with Save Preset overlay composited on top
//   preset-menu       — Editor with the Preset popup (largest category expanded)
//   output-dropdown   — Editor with the Output Format dropdown (all formats)
//   undo-active       — Editor with populated undo/redo history (buttons active)
//
// Examples:
//   screenshot_tool ui.png 2.0
//   screenshot_tool orbit.png 2.0 --preset "Orbit Dance"
//   screenshot_tool tone.png 2.0 --mode tone-section --preset "Warm Room"
//   screenshot_tool dropdown.png 2.0 --mode output-dropdown
//==============================================================================

#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/PresetData.h"
#include <iostream>
#include <algorithm>
#include <vector>

//==============================================================================
// Preset helpers
//==============================================================================
static int findPresetByName (const juce::String& searchName)
{
    const int numPresets = NUM_FACTORY_PRESETS;
    for (int i = 0; i < numPresets; ++i)
        if (factoryPresets[(size_t) i].name.equalsIgnoreCase (searchName))
            return i;
    for (int i = 0; i < numPresets; ++i)
        if (factoryPresets[(size_t) i].name.containsIgnoreCase (searchName))
            return i;
    return -1;
}

static void applyPresetToProcessor (OpenSpatialDelayProcessor& processor,
                                    const PresetData& preset)
{
    auto& apvts = processor.apvts;

    auto setFloat = [&] (const juce::String& id, float v) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (v));
    };
    auto setChoice = [&] (const juce::String& id, int choiceIndex) {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        {
            int n = p->choices.size();
            if (n > 1)
                p->setValueNotifyingHost ((float) choiceIndex / (float) (n - 1));
        }
    };
    auto setBool = [&] (const juce::String& id, bool v) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (v ? 1.0f : 0.0f);
    };

    setFloat  ("delayTime",     preset.delayTime);
    setBool   ("tempoSync",     preset.tempoSync);
    setFloat  ("noteDivision",  preset.noteDivision);
    setChoice ("syncMode",      preset.syncMode);
    setFloat  ("feedback",      preset.feedback);
    setFloat  ("filterLP",      preset.filterLP);
    setFloat  ("filterHP",      preset.filterHP);
    setFloat  ("filterLPQ",     preset.filterLPQ);
    setFloat  ("filterHPQ",     preset.filterHPQ);
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

    for (int i = 0; i < 12; ++i)
    {
        auto pre = "object" + juce::String (i + 1) + "_";
        const auto& t = preset.taps[i];
        setBool   (pre + "enabled",             t.enabled);
        setFloat  (pre + "azimuth",             t.azimuthDeg);
        setFloat  (pre + "elevation",           t.elevationDeg);
        setFloat  (pre + "distance",            t.distance);
        setFloat  (pre + "dopplerAmount",       t.dopplerAmount);
        setFloat  (pre + "pitchShift",          t.pitchShift);
        setChoice (pre + "trajectoryShape",     t.trajectoryShape);
        setFloat  (pre + "trajectorySpeed",     t.trajectorySpeed);
        setChoice (pre + "trajectoryDirection", t.trajectoryDirection);
        setChoice (pre + "inputChannel",        t.inputChannel);
    }
}

static bool loadPresetFromFile (OpenSpatialDelayProcessor& processor,
                                const juce::File& file)
{
    if (! file.existsAsFile())
    {
        std::cerr << "Error: preset file not found: "
                  << file.getFullPathName().toStdString() << "\n";
        return false;
    }
    auto parsed = parsePresetJson (file.loadFileAsString());
    if (parsed.name.isEmpty())
    {
        std::cerr << "Error: could not parse preset file: "
                  << file.getFullPathName().toStdString() << "\n";
        return false;
    }
    applyPresetToProcessor (processor, parsed);
    std::cout << "Loaded preset from file: " << parsed.name.toStdString()
              << " (" << parsed.category.toStdString() << ")\n";
    return true;
}

//==============================================================================
// PopupMenuSnapshot — renders a list of items using OSDLookAndFeel's popup
// routines so the snapshot is pixel-identical to the real popup.
//==============================================================================
class PopupMenuSnapshot : public juce::Component
{
public:
    struct Item
    {
        juce::String text;
        bool isHeader     = false;
        bool isSeparator  = false;
        bool isTicked     = false;
        bool isHighlighted = false;
        bool hasSubMenu   = false;
    };

    PopupMenuSnapshot (OSDLookAndFeel& lnf, std::vector<Item> items_, int fixedWidth,
                       int rowHeight = 22)
        : look (lnf), items (std::move (items_)), itemHeight (rowHeight)
    {
        setLookAndFeel (&look);

        int h = kCardPadding;
        for (const auto& it : items)
            h += rowHeightFor (it);
        h += kCardPadding;
        setSize (fixedWidth, h);
    }

    ~PopupMenuSnapshot() override { setLookAndFeel (nullptr); }

    void paint (juce::Graphics& g) override
    {
        look.drawPopupMenuBackground (g, getWidth(), getHeight());

        int y = kCardPadding;
        for (const auto& it : items)
        {
            int ih = rowHeightFor (it);
            auto area = juce::Rectangle<int> (0, y, getWidth(), ih);

            if (it.isHeader)
            {
                g.setColour (juce::Colour (0xff7fc9ff));
                g.setFont (look.getPopupMenuFont().boldened());
                g.drawText (it.text, area.reduced (8, 0),
                            juce::Justification::centredLeft, true);
            }
            else
            {
                look.drawPopupMenuItem (g, area,
                                        it.isSeparator,
                                        /*isActive*/ true,
                                        it.isHighlighted,
                                        it.hasSubMenu,
                                        it.isTicked,
                                        it.text,
                                        /*shortcut*/ juce::String(),
                                        /*icon*/ nullptr,
                                        nullptr);
            }
            y += ih;
        }
    }

private:
    int rowHeightFor (const Item& it) const
    {
        if (it.isSeparator) return 8;
        return itemHeight;  // headers + regular rows use the same row height
    }

    OSDLookAndFeel& look;
    std::vector<Item> items;
    int itemHeight = 22;
    static constexpr int kCardPadding = 6;
};

//==============================================================================
// Composite a popup snapshot onto the editor snapshot at a given anchor point.
//==============================================================================
static juce::Image compositeOverlay (juce::Image base,
                                     juce::Image overlay,
                                     int anchorX,
                                     int anchorY,
                                     float scale)
{
    // anchorX/Y are in editor-local (unscaled) coordinates. Base image is
    // already scaled. We convert anchor → scaled pixel coords, then paint
    // overlay (which was rendered at the same scale) onto the base.
    juce::Graphics g (base);
    int px = juce::roundToInt (anchorX * scale);
    int py = juce::roundToInt (anchorY * scale);
    g.drawImageAt (overlay, px, py);
    return base;
}

//==============================================================================
// Render a Component to a PNG-ready Image at a given scale.
//==============================================================================
static juce::Image snapshotComponent (juce::Component& c, float scale)
{
    return c.createComponentSnapshot (c.getLocalBounds(), true, scale);
}

//==============================================================================
// Build a preset menu mock — picks the category with the most presets and
// renders it fully expanded with all items.
//==============================================================================
static std::unique_ptr<PopupMenuSnapshot>
buildPresetMenuMock (OSDLookAndFeel& lnf,
                     OpenSpatialDelayProcessor& processor,
                     int minWidth)
{
    auto presets = processor.getCategorizedPresets();

    // Count presets per category, pick the largest
    juce::HashMap<juce::String, int> counts;
    for (const auto& p : presets)
        counts.set (p.category, counts[p.category] + 1);

    juce::String largest;
    int best = 0;
    for (juce::HashMap<juce::String, int>::Iterator it (counts); it.next();)
    {
        if (it.getValue() > best)
        {
            best = it.getValue();
            largest = it.getKey();
        }
    }

    std::vector<PopupMenuSnapshot::Item> items;

    // Show the largest category expanded
    PopupMenuSnapshot::Item header;
    header.text = largest.toUpperCase();
    header.isHeader = true;
    items.push_back (header);

    int currentIdx = processor.getCurrentPresetIndex();
    bool sawFactory = false, insertedSep = false;
    for (const auto& p : presets)
    {
        if (p.category != largest) continue;
        if (! p.isFactory && sawFactory && ! insertedSep)
        {
            PopupMenuSnapshot::Item sep;
            sep.isSeparator = true;
            items.push_back (sep);
            insertedSep = true;
        }
        PopupMenuSnapshot::Item it;
        it.text    = p.name;
        it.isTicked = (p.originalIndex == currentIdx);
        items.push_back (it);
        if (p.isFactory) sawFactory = true;
    }

    return std::make_unique<PopupMenuSnapshot> (lnf, std::move (items), minWidth);
}

//==============================================================================
// Build an output-format dropdown mock — all 23 formats, current one ticked.
//==============================================================================
static std::unique_ptr<PopupMenuSnapshot>
buildOutputDropdownMock (OSDLookAndFeel& lnf,
                         OpenSpatialDelayProcessor& processor,
                         int minWidth)
{
    std::vector<PopupMenuSnapshot::Item> items;
    int currentFmt = processor.configOutputFormat.load();

    auto categoryFor = [] (const OpenSpatialDelayProcessor::OutputFormatInfo& info) -> juce::String
    {
        if (info.isStereoVariant)     return "Stereo";
        if (info.isAmbisonicsOutput)  return "Ambisonics";
        if (juce::String (info.name) == "Binaural") return "Binaural";
        if (info.hasHeight)           return "Immersive";
        return "Surround";
    };

    juce::String currentCategory;
    for (int i = 0; i < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS; ++i)
    {
        const auto& info = OpenSpatialDelayProcessor::outputFormatRegistry[(size_t) i];
        juce::String cat = categoryFor (info);
        if (cat != currentCategory)
        {
            PopupMenuSnapshot::Item header;
            header.text = cat.toUpperCase();
            header.isHeader = true;
            items.push_back (header);
            currentCategory = cat;
        }

        PopupMenuSnapshot::Item it;
        it.text    = info.name;
        it.isTicked = (i == currentFmt);
        items.push_back (it);
    }

    // Compact row height so all 23 formats + 5 headers fit within the editor.
    return std::make_unique<PopupMenuSnapshot> (lnf, std::move (items), minWidth, /*row h*/ 18);
}

//==============================================================================
// Populate the internal undo history with a few state changes so the undo
// and redo arrows are both active in the screenshot.
//==============================================================================
static void populateUndoHistory (OpenSpatialDelayProcessor& processor)
{
    // 1. Initial baseline capture
    processor.captureUndoState ("baseline");

    // 2. Change feedback — capture
    if (auto* p = processor.apvts.getParameter ("feedback"))
    {
        p->setValueNotifyingHost (p->convertTo0to1 (0.55f));
        processor.captureUndoState ("feedback → 55%");
    }

    // 3. Change dryWet — capture
    if (auto* p = processor.apvts.getParameter ("dryWet"))
    {
        p->setValueNotifyingHost (p->convertTo0to1 (0.65f));
        processor.captureUndoState ("dryWet → 65%");
    }

    // 4. Change delay time — capture
    if (auto* p = processor.apvts.getParameter ("delayTime"))
    {
        p->setValueNotifyingHost (p->convertTo0to1 (0.42f));
        processor.captureUndoState ("delayTime → 420ms");
    }

    // 5. Undo once → both undo AND redo arrows become active
    processor.performInternalUndo();
}

//==============================================================================
// Simple, clean PNG writer.
//==============================================================================
static bool savePng (const juce::Image& image, const juce::File& out, float scale)
{
    if (! image.isValid())
    {
        std::cerr << "Error: image is not valid.\n";
        return false;
    }
    out.getParentDirectory().createDirectory();
    // JUCE's FileOutputStream does not truncate by default — delete any existing
    // file first so we don't append the new PNG onto the old one (which yields
    // a valid-looking file whose first IDAT chunk is stale).
    if (out.existsAsFile()) out.deleteFile();
    juce::FileOutputStream stream (out);
    if (stream.failedToOpen())
    {
        std::cerr << "Error: could not open " << out.getFullPathName().toStdString() << "\n";
        return false;
    }
    juce::PNGImageFormat png;
    if (! png.writeImageToStream (image, stream))
    {
        std::cerr << "Error: failed to write PNG data.\n";
        return false;
    }
    std::cout << "Saved " << out.getFullPathName().toStdString()
              << " (" << image.getWidth() << "x" << image.getHeight()
              << " @ " << scale << "x)\n";
    return true;
}

//==============================================================================
int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    // ── Parse CLI ────────────────────────────────────────────────────────
    juce::String outputPath = "plugin_screenshot.png";
    float scaleFactor = 2.0f;
    juce::String presetArg;
    juce::String mode = "full";

    std::vector<juce::String> positional;
    for (int i = 1; i < argc; ++i)
    {
        juce::String a (argv[i]);
        if (a == "--list-presets")
        {
            std::cout << "Factory presets (" << NUM_FACTORY_PRESETS << "):\n";
            for (int j = 0; j < NUM_FACTORY_PRESETS; ++j)
                std::cout << "  " << j << ": ["
                          << factoryPresets[j].category.toStdString() << "] "
                          << factoryPresets[j].name.toStdString() << "\n";
            return 0;
        }
        else if (a == "--preset" && i + 1 < argc) { presetArg = juce::String (argv[++i]); }
        else if (a == "--mode"   && i + 1 < argc) { mode      = juce::String (argv[++i]); }
        else if (! a.startsWith ("--"))           { positional.push_back (a); }
    }

    if (positional.size() > 0) outputPath = positional[0];
    if (positional.size() > 1)
    {
        float s = positional[1].getFloatValue();
        if (s > 0.0f && s <= 8.0f) scaleFactor = s;
    }

    // ── Set up processor + editor ─────────────────────────────────────────
    OpenSpatialDelayProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    if (presetArg.isNotEmpty())
    {
        juce::File f (presetArg);
        if (f.existsAsFile())
        {
            if (! loadPresetFromFile (processor, f)) return 1;
        }
        else
        {
            // Look up the preset name in the processor's loaded preset list
            // (factoryPresets[] is a compile-time array; processor.getPresetNames()
            // is the runtime-merged list in the same order loadPreset() expects).
            auto names = processor.getPresetNames();
            int runtimeIdx = -1;
            for (int n = 0; n < names.size(); ++n)
            {
                if (names[n].equalsIgnoreCase (presetArg)
                    || names[n].containsIgnoreCase (presetArg))
                {
                    runtimeIdx = n;
                    break;
                }
            }
            if (runtimeIdx < 0)
            {
                std::cerr << "Error: preset not found: " << presetArg.toStdString() << "\n";
                return 1;
            }
            processor.loadPreset (runtimeIdx);
            std::cout << "Loaded preset: " << names[runtimeIdx].toStdString() << "\n";
        }
    }

    // OSC Receive: leave enabled for visual completeness
    processor.setOscReceiveEnabled (true);

    auto* editorRaw = processor.createEditor();
    if (editorRaw == nullptr)
    {
        std::cerr << "Error: createEditor() returned nullptr.\n";
        return 1;
    }
    std::unique_ptr<juce::AudioProcessorEditor> editor (editorRaw);
    editor->setBounds (0, 0, 820, 580);

    auto* osd = dynamic_cast<OpenSpatialDelayEditor*> (editor.get());
    if (osd == nullptr)
    {
        std::cerr << "Error: editor is not an OpenSpatialDelayEditor.\n";
        return 1;
    }
    osd->syncForScreenshot();

    // ── Apply per-mode state and render ───────────────────────────────────
    juce::Image result;
    juce::File outFile = juce::File::getCurrentWorkingDirectory().getChildFile (outputPath);

    if (mode == "full")
    {
        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "drawer-open")
    {
        const float demoKnobs[6] = { 0.0f, 0.0f, 0.15f, -0.1f, 0.0f, 0.0f };
        osd->configureGlobalDrawer (true, demoKnobs, 6);
        osd->resized();
        osd->syncForScreenshot();
        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "tone-section")
    {
        auto b = osd->getToneSectionBoundsForScreenshot();
        result = osd->createComponentSnapshot (b, true, scaleFactor);
    }
    else if (mode == "osc-section")
    {
        auto b = osd->getOscSectionBoundsForScreenshot();
        result = osd->createComponentSnapshot (b, true, scaleFactor);
    }
    else if (mode == "save-overlay")
    {
        auto& overlay = osd->getPresetSaveOverlay();
        overlay.showForSnapshot ("My Preset", osd);
        osd->resized();
        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "preset-menu")
    {
        auto base = snapshotComponent (*osd, scaleFactor);
        auto mock = buildPresetMenuMock (osd->getOSDLookAndFeel(), processor, 200);
        auto mockImg = snapshotComponent (*mock, scaleFactor);

        // Anchor popup below the preset-name button
        auto btn = osd->getPresetNameButtonBounds();
        int anchorX = btn.getX();
        int anchorY = btn.getBottom() + 2;
        result = compositeOverlay (base, mockImg, anchorX, anchorY, scaleFactor);
    }
    else if (mode == "output-dropdown")
    {
        auto base = snapshotComponent (*osd, scaleFactor);
        auto mock = buildOutputDropdownMock (osd->getOSDLookAndFeel(), processor, 200);
        auto mockImg = snapshotComponent (*mock, scaleFactor);

        // Anchor horizontally near the Output Format button, but shift left so the
        // popup stays inside the editor. Anchor vertically just below the button,
        // then clamp so the bottom of the mock stays within the editor.
        auto btn = osd->getOutputFormatBoxBounds();
        const int mockW = mock->getWidth();
        const int mockH = mock->getHeight();
        int anchorX = btn.getRight() - mockW;
        if (anchorX < 4) anchorX = 4;
        int anchorY = btn.getBottom() + 2;
        if (anchorY + mockH > osd->getHeight() - 4)
            anchorY = osd->getHeight() - 4 - mockH;
        if (anchorY < 4) anchorY = 4;
        result = compositeOverlay (base, mockImg, anchorX, anchorY, scaleFactor);
    }
    else if (mode == "undo-active")
    {
        // Populate AFTER editor creation. The editor's constructor calls
        // captureUndoState("Initial State") which would trim any prior redo
        // history. So we build the history here, then undo once to leave
        // both arrows active.
        populateUndoHistory (processor);
        osd->syncForScreenshot();  // triggers timerCallback → updateUndoButtons
        result = snapshotComponent (*osd, scaleFactor);
    }
    else
    {
        std::cerr << "Error: unknown --mode: " << mode.toStdString() << "\n";
        std::cerr << "Valid modes: full, drawer-open, tone-section, osc-section,\n"
                     "             save-overlay, preset-menu, output-dropdown,\n"
                     "             undo-active\n";
        return 1;
    }

    if (! savePng (result, outFile, scaleFactor)) return 1;

    editor.reset();
    processor.releaseResources();
    return 0;
}
