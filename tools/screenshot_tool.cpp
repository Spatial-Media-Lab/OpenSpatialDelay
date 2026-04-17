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
//   preset-menu       — Editor with nested preset menu (folders + expanded submenu)
//   output-dropdown   — Editor with the Output Format dropdown (flat list)
//   undo-active       — Editor with populated undo/redo history (buttons active)
//   spatial-map       — Just the SpatialMap component (no drawer, no bottom panel)
//   elevation-map     — SpatialMap with all 12 taps in a +90°→-90° spiral, labelled
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
// Build a preset-menu mock that mirrors the real plugin's nested popup:
// a top-level list of category folders (each with hasSubMenu=true), with one
// shown in the hovered state, and a second popup rendered alongside it listing
// that category's presets. The second popup is returned separately so the
// caller can composite it next to the first.
//==============================================================================
struct PresetMenuMockPair
{
    std::unique_ptr<PopupMenuSnapshot> parent;
    std::unique_ptr<PopupMenuSnapshot> submenu;
    juce::String hoveredCategory;
};

static PresetMenuMockPair
buildPresetMenuMock (OSDLookAndFeel& lnf,
                     OpenSpatialDelayProcessor& processor,
                     int parentWidth,
                     int submenuWidth)
{
    auto presets = processor.getCategorizedPresets();

    // Preserve the category order as they appear in the flat list.
    std::vector<juce::String> categories;
    for (const auto& p : presets)
    {
        if (std::find (categories.begin(), categories.end(), p.category) == categories.end())
            categories.push_back (p.category);
    }

    // Pick the category that contains the currently-selected preset to render
    // as "hovered". If none are current, fall back to the first category.
    int currentIdx = processor.getCurrentPresetIndex();
    juce::String hovered = categories.empty() ? juce::String() : categories.front();
    for (const auto& p : presets)
    {
        if (p.originalIndex == currentIdx)
        {
            hovered = p.category;
            break;
        }
    }

    // Build the parent (category folders) menu.
    std::vector<PopupMenuSnapshot::Item> parentItems;
    for (const auto& cat : categories)
    {
        PopupMenuSnapshot::Item it;
        it.text          = cat;
        it.hasSubMenu    = true;
        it.isHighlighted = (cat == hovered);
        parentItems.push_back (it);
    }

    PresetMenuMockPair result;
    result.parent = std::make_unique<PopupMenuSnapshot> (lnf, std::move (parentItems), parentWidth);
    result.hoveredCategory = hovered;

    // Build the submenu (presets inside the hovered category).
    std::vector<PopupMenuSnapshot::Item> subItems;
    bool sawFactory = false, insertedSep = false;
    for (const auto& p : presets)
    {
        if (p.category != hovered) continue;
        if (! p.isFactory && sawFactory && ! insertedSep)
        {
            PopupMenuSnapshot::Item sep;
            sep.isSeparator = true;
            subItems.push_back (sep);
            insertedSep = true;
        }
        PopupMenuSnapshot::Item it;
        it.text     = p.name;
        it.isTicked = (p.originalIndex == currentIdx);
        subItems.push_back (it);
        if (p.isFactory) sawFactory = true;
    }

    result.submenu = std::make_unique<PopupMenuSnapshot> (lnf, std::move (subItems), submenuWidth);
    return result;
}

//==============================================================================
// Build an output-format dropdown mock that matches the real plugin's popup:
// a flat list of every format, with the current one ticked. No category headers.
//==============================================================================
static std::unique_ptr<PopupMenuSnapshot>
buildOutputDropdownMock (OSDLookAndFeel& lnf,
                         OpenSpatialDelayProcessor& processor,
                         int minWidth)
{
    std::vector<PopupMenuSnapshot::Item> items;
    int currentFmt = processor.configOutputFormat.load();

    for (int i = 0; i < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS; ++i)
    {
        const auto& info = OpenSpatialDelayProcessor::outputFormatRegistry[(size_t) i];
        PopupMenuSnapshot::Item it;
        it.text    = info.name;
        it.isTicked = (i == currentFmt);
        items.push_back (it);
    }

    // Compact row height so all 23 formats fit within the editor.
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
        auto pair = buildPresetMenuMock (osd->getOSDLookAndFeel(), processor,
                                         /*parentWidth*/ 170,
                                         /*submenuWidth*/ 190);
        auto parentImg  = snapshotComponent (*pair.parent, scaleFactor);
        auto submenuImg = snapshotComponent (*pair.submenu, scaleFactor);

        // Parent popup anchored below the preset-name button.
        auto btn = osd->getPresetNameButtonBounds();
        int parentX = btn.getX();
        int parentY = btn.getBottom() + 2;

        // Submenu anchored to the right of the parent at the hovered row.
        // Each category row in the parent uses the default 22px row height,
        // plus the 6px card padding at the top.
        int rowH  = 22;
        int pad   = 6;
        int hoverIdx = 0;
        auto presets = processor.getCategorizedPresets();
        std::vector<juce::String> seen;
        for (const auto& p : presets)
        {
            if (std::find (seen.begin(), seen.end(), p.category) == seen.end())
            {
                if (p.category == pair.hoveredCategory) break;
                seen.push_back (p.category);
                ++hoverIdx;
            }
        }
        int submenuX = parentX + pair.parent->getWidth() - 2;
        int submenuY = parentY + pad + hoverIdx * rowH - pad;

        // Clamp so both popups stay inside the editor.
        int maxRight = osd->getWidth() - 4;
        if (submenuX + pair.submenu->getWidth() > maxRight)
            submenuX = maxRight - pair.submenu->getWidth();
        if (submenuY + pair.submenu->getHeight() > osd->getHeight() - 4)
            submenuY = osd->getHeight() - 4 - pair.submenu->getHeight();
        if (submenuY < 4) submenuY = 4;

        result = compositeOverlay (base, parentImg,  parentX,  parentY,  scaleFactor);
        result = compositeOverlay (result, submenuImg, submenuX, submenuY, scaleFactor);
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
    else if (mode == "spatial-map")
    {
        // Snapshot the SpatialMap component directly — this excludes the
        // Global Drawer tab (sibling component, not a child) and yields a
        // clean square map with no bottom-panel bleed.
        auto& map = osd->getSpatialMapForScreenshot();
        result = snapshotComponent (map, scaleFactor);
    }
    else if (mode == "elevation-map")
    {
        // Configure all 12 taps as a spiral: azimuths evenly spread around the
        // circle, distances progressing outward, elevations running from +90°
        // down to -90°. Then snapshot the spatial map and overlay each tap's
        // elevation value as a label next to its dot.
        auto& apvts = processor.apvts;
        for (int i = 0; i < 12; ++i)
        {
            auto pre = "object" + juce::String (i + 1) + "_";
            auto setFloat = [&] (const juce::String& id, float v) {
                if (auto* p = apvts.getParameter (pre + id))
                    p->setValueNotifyingHost (p->convertTo0to1 (v));
            };
            auto setBool  = [&] (const juce::String& id, bool v) {
                if (auto* p = apvts.getParameter (pre + id))
                    p->setValueNotifyingHost (v ? 1.0f : 0.0f);
            };

            float t   = (float) i / 11.0f;              // 0..1
            float az  = -180.0f + t * 360.0f;           // full sweep
            float dist = 0.38f + t * 0.55f;             // 0.38 → 0.93
            float elev = 90.0f - t * 180.0f;            // +90 → -90

            setBool  ("enabled",    true);
            setFloat ("azimuth",    az);
            setFloat ("elevation",  elev);
            setFloat ("distance",   dist);
        }
        osd->syncForScreenshot();

        auto& map = osd->getSpatialMapForScreenshot();
        juce::Image base = snapshotComponent (map, scaleFactor);

        // Overlay elevation labels on the rendered image.
        juce::Graphics g (base);
        g.addTransform (juce::AffineTransform::scale (scaleFactor));
        juce::Font labelFont (osd->getOSDLookAndFeel().jetbrainsMedium);
        labelFont.setHeight (11.0f);
        g.setFont (labelFont);

        for (int i = 0; i < 12; ++i)
        {
            if (! map.isObjectEnabled (i)) continue;
            auto pos = map.getObjectScreenPos (i);
            float z = std::sin (juce::degreesToRadians (map.getObjectElevation (i)));
            float dotR = 7.0f + (z >= 0.0f ? 2.5f * z : 1.5f * z);

            juce::String text = juce::String (juce::roundToInt (map.getObjectElevation (i)))
                              + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0"));

            // Place label outside the dot — right side if the dot is on the
            // left half of the map, left side otherwise. Small vertical nudge
            // so labels for neighbouring taps don't overlap too badly.
            float mapCx = map.getWidth() * 0.5f;
            bool placeRight = pos.x < mapCx;
            int labelW = 30;
            int labelH = 14;
            float lx = placeRight ? (pos.x + dotR + 4.0f)
                                  : (pos.x - dotR - 4.0f - labelW);
            float ly = pos.y - labelH * 0.5f;

            // Soft drop shadow for legibility against the starfield.
            g.setColour (juce::Colour (0x99000000));
            g.drawText (text,
                        juce::Rectangle<float> (lx + 1.0f, ly + 1.0f,
                                                (float) labelW, (float) labelH),
                        placeRight ? juce::Justification::centredLeft
                                   : juce::Justification::centredRight,
                        false);
            g.setColour (juce::Colour (0xfff0f4f8));
            g.drawText (text,
                        juce::Rectangle<float> (lx, ly,
                                                (float) labelW, (float) labelH),
                        placeRight ? juce::Justification::centredLeft
                                   : juce::Justification::centredRight,
                        false);
        }
        result = base;
    }
    else
    {
        std::cerr << "Error: unknown --mode: " << mode.toStdString() << "\n";
        std::cerr << "Valid modes: full, drawer-open, tone-section, osc-section,\n"
                     "             save-overlay, preset-menu, output-dropdown,\n"
                     "             undo-active, spatial-map, elevation-map\n";
        return 1;
    }

    if (! savePng (result, outFile, scaleFactor)) return 1;

    editor.reset();
    processor.releaseResources();
    return 0;
}
