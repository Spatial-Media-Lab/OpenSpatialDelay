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
//   --showcase             Override processor state with a "features-on" demo
//                          (issue #168 round 2): 9.1.6 Atmos + VBAP, 9 taps
//                          with an Orbit trajectory on Tap 1, MOD/FLT/AIR
//                          engaged, Delay Sync + Triplet, OSC Send enabled.
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
//   undo-active       — Editor with populated undo history (undo active, redo inactive)
//   preset-showcase   — Full editor for a loaded preset with trajectory trails
//                       rendered (--output-format / --surround-algo / --stereo-mode
//                       / --hrtf-profile select the header state)
//   annotated-source  — Showcase state + Global Drawer open + undo history populated.
//                       Source image for the annotated callout overlay (issue #168 r3).
//   hero              — Hero screenshot for docs/README: showcase + drawer open +
//                       undo populated + per-tap activity glow + custom preset name
//                       (issue #168 r3). --showcase is implicit.
//   spatial-map       — Just the SpatialMap component (no drawer, no bottom panel)
//   elevation-map     — SpatialMap with all 12 taps in a −90°→+90° spiral, labelled
//   trajectory        — SpatialMap only, one tap (--tap N, default 1) at
//                       (az=0, el=0, dist=0.5) with --trajectory <name> drawn
//                       at full brightness. Every other tap disabled.
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
                // Subtle "currently-selected" wash behind the ticked row — matches
                // the real JUCE popup's pre-selected-item treatment without using
                // the full cyan highlightedBackgroundColourId (too aggressive for
                // a static screenshot).
                if (it.isTicked && ! it.isHighlighted && ! it.isSeparator)
                {
                    g.setColour (juce::Colour::fromFloatRGBA (1.0f, 1.0f, 1.0f, 0.08f));
                    g.fillRect (area);
                }

                look.drawPopupMenuItem (g, area,
                                        it.isSeparator,
                                        /*isActive*/ true,
                                        it.isHighlighted,
                                        it.isTicked,
                                        it.hasSubMenu,
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
    // Issue #168 round 2: tick the category that contains the currently-active
    // preset, so the parent list mirrors JUCE's real popup (which ticks the
    // category of the active preset *and* highlights the row where the submenu
    // is open).
    std::vector<PopupMenuSnapshot::Item> parentItems;
    for (const auto& cat : categories)
    {
        PopupMenuSnapshot::Item it;
        it.text          = cat;
        it.hasSubMenu    = true;
        it.isHighlighted = (cat == hovered);
        it.isTicked      = (cat == hovered);
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
// Build an output-format dropdown mock that matches the real JUCE ComboBox
// popup (issue #168 round 3 reference
// .context/attachments/Screenshot 2026-04-17 at 15.50.02-v1.png):
//   flat list of all output formats in natural order, no separator, no
//   duplicate-at-top, no highlight background. The currently-selected
//   item is marked only by the tick glyph at its left.
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
        it.text     = info.name;
        it.isTicked = (i == currentFmt);
        items.push_back (it);
    }

    // Row height 14 so all 23 formats fit within the editor height without
    // clipping. The OSDLookAndFeel popup font is DM Sans Regular 13pt — a
    // 14px row gives 1px vertical padding above/below.
    return std::make_unique<PopupMenuSnapshot> (lnf, std::move (items), minWidth, /*row h*/ 14);
}

//==============================================================================
// Apply the "feature showcase" state — every major section of the plugin
// engaged, 9 taps with an Orbit trajectory on Tap 1, 9.1.6 Atmos + VBAP.
// Called before snapshot when --showcase is passed.
//==============================================================================
static void applyShowcaseState (OpenSpatialDelayProcessor& processor,
                                OpenSpatialDelayEditor& editor)
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

    // --- Header: output format + algorithm -----------------------------------
    // 9.1.6 Atmos sits at registry index 15; VBAP is algorithm index 5.
    constexpr int kOutputFormat_9_1_6 = 15;
    constexpr int kAlgorithm_VBAP     = 5;
    processor.configOutputFormat.store (kOutputFormat_9_1_6, std::memory_order_relaxed);
    processor.configAlgorithm.store    (kAlgorithm_VBAP,     std::memory_order_relaxed);
    processor.markConfigStateDirty();
    processor.requestOutputFormatChange (kOutputFormat_9_1_6);

    // --- DELAY section: Sync on, Triplet --------------------------------------
    setBool   ("tempoSync",     true);
    setChoice ("syncMode",      2);          // 0=Straight, 1=Dotted, 2=Triplet
    setFloat  ("noteDivision",  4.0f);        // 1/4
    setFloat  ("feedback",      0.45f);

    // --- MOD section (wobble): on with visible, non-zero knobs ---------------
    // wobbleAmount / wobbleMorph are 0..100% ranges (not 0..1), so pass
    // percentages directly.
    setBool  ("wobbleEnabled", true);
    setFloat ("wobbleAmount",  55.0f);
    setFloat ("wobbleMorph",   40.0f);

    // --- TONE section: FLT on, min-Q one side / max-Q the other --------------
    // Qs span the 0.1 → 8.0 param range, so HP at 0.1 is maximally broad and
    // LP at 8.0 is sharply resonant — the filter graph shows a clear asymmetry.
    setBool  ("filterEnabled", true);
    setFloat ("filterHP",      120.0f);
    setFloat ("filterHPQ",     0.10f);
    setFloat ("filterLP",      4000.0f);
    setFloat ("filterLPQ",     8.00f);

    // --- MIX section: AIR on, Dry/Wet + gains at pleasing values -------------
    setBool  ("airAbsorption", true);
    setFloat ("dryWet",        0.55f);
    setFloat ("inputGain",     0.0f);
    setFloat ("outputGain",    0.0f);

    // --- OSC: Send on, Receive off -------------------------------------------
    processor.setOscReceiveEnabled (false);
    processor.setOscSendEnabled    (true);

    // --- Taps: 9 enabled, varied az / el / dist ------------------------------
    // Round-3 update (issue #168): Tap 1 now carries an Infinity trajectory
    // (figure-∞) so the spatial map shows a figure-8 trail. Tap 1 also
    // splits out to R-only input with +7 st pitch shift and 75% Doppler —
    // a soloable "lead" tap. Taps 3, 6 and 9 are disabled; taps 10, 11 and
    // 12 take their place so the scene still carries 9 active echoes.
    //
    // trajectoryShape index 7 = "Infinity" (alphabetical list in
    // PluginProcessor.cpp: None=0, Bounce=1, Circle=2, Cross=3, Figure-8=4,
    // Heart=5, Helix=6, Infinity=7, ...).
    // inputChannel index 1 = "L" (choices: L+R=0, L=1, R=2).
    //
    // Round-3 update 2: the Infinity lemniscate centred on Tap 1
    // (az=15°, el=20°, dist=0.35) extends roughly ±0.65 in map-space on
    // each side, which would collide with the inner taps from the first
    // revision. Non-Tap-1 positions have been pushed toward the edges of
    // the map so the full figure-∞ reads clean, and Tap 1 is now routed
    // from the L channel rather than R.
    struct TapDef { bool enabled; float az, el, dist; int trajShape;
                    float trajSpeed; int inputCh; float dopplerAmt;
                    float pitchSt; };
    // Tap 1 is the Infinity-trajectory lead (az=0°, el=+20°, dist=0.25), so
    // the lemniscate spans mapX ∈ [-0.75, 0.75] and mapY ∈ [-0.015, 0.515]
    // in (sin(az)·dist, cos(az)·dist) space. The remaining 8 enabled taps
    // are hand-placed outside that bounding region — a pseudo-random but
    // deliberately non-uniform distribution: varied azimuths covering
    // front-high, sides, rear-low and behind; distances span 0.42–0.95 so
    // they don't form a ring; elevations span −55° to +72° so the scene
    // reads as a real 3D spread rather than a flat plane.
    const TapDef defs[12] = {
        { true,     0.0f,  20.0f, 0.25f, 7, 0.40f, 1, 0.75f, 7.0f },  // Tap 1: Infinity, L, +7st, 75% Doppler
        { true,   -65.0f, -25.0f, 0.92f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 2:  past left lobe, low
        { false,   0.0f,   0.0f, 0.50f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 3 DISABLED
        { true,  -155.0f,  -5.0f, 0.58f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 4:  rear-left mid
        { true,   115.0f,  45.0f, 0.42f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 5:  side-right upper
        { false,   0.0f,   0.0f, 0.50f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 6 DISABLED
        { true,  -110.0f,  28.0f, 0.75f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 7:  rear-left upper
        { true,   148.0f, -38.0f, 0.55f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 8:  rear-right low
        { false,   0.0f,   0.0f, 0.50f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 9 DISABLED
        { true,     0.0f,  72.0f, 0.88f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 10: directly front, very high + far
        { true,    85.0f, -55.0f, 0.95f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 11: past right lobe, very low
        { true,   175.0f,  15.0f, 0.50f, 0, 0.0f,  0, 0.0f,  0.0f },  // Tap 12: directly behind mid
    };
    for (int i = 0; i < 12; ++i)
    {
        auto pre = "object" + juce::String (i + 1) + "_";
        auto setObjFloat = [&] (const juce::String& id, float v) {
            if (auto* p = apvts.getParameter (pre + id))
                p->setValueNotifyingHost (p->convertTo0to1 (v));
        };
        auto setObjChoice = [&] (const juce::String& id, int idx) {
            if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (pre + id)))
            {
                int n = p->choices.size();
                if (n > 1)
                    p->setValueNotifyingHost ((float) idx / (float) (n - 1));
            }
        };
        auto setObjBool = [&] (const juce::String& id, bool v) {
            if (auto* p = apvts.getParameter (pre + id))
                p->setValueNotifyingHost (v ? 1.0f : 0.0f);
        };

        const auto& d = defs[i];
        setObjBool ("enabled", d.enabled);
        if (d.enabled)
        {
            setObjFloat  ("azimuth",             d.az);
            setObjFloat  ("elevation",           d.el);
            setObjFloat  ("distance",            d.dist);
            setObjChoice ("trajectoryShape",     d.trajShape);
            setObjFloat  ("trajectorySpeed",     d.trajSpeed);
            setObjChoice ("inputChannel",        d.inputCh);
            setObjFloat  ("dopplerAmount",       d.dopplerAmt);
            setObjFloat  ("pitchShift",          d.pitchSt);
        }
    }

    editor.applyShowcaseHeaderForScreenshot();
}

//==============================================================================
// Populate the internal undo history with a few state changes so the undo arrow
// is active in the screenshot. Redo stays inactive — we capture with a fresh
// forward history so only the back-arrow is live.
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

    // Leave redo empty: undo arrow active, redo arrow inactive.
}

//==============================================================================
// Preset-showcase helpers (round-3 item 18+): resolve user-facing output /
// algorithm / HRTF / stereo-mode names to their integer indices, and pump the
// processor so trajectory.tick() advances each object's TrajectoryState. Used
// by the --mode preset-showcase capture path.
//==============================================================================
static int resolveOutputFormatIndex (const juce::String& name)
{
    for (int i = 0; i < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS; ++i)
    {
        const auto& info = OpenSpatialDelayProcessor::outputFormatRegistry[(size_t) i];
        if (juce::String (info.name).equalsIgnoreCase (name)) return i;
    }
    return -1;
}

// Surround algorithm names in the order stored in OpenSpatialDelayProcessor::algorithms[0..6].
static const char* const kSurroundAlgoNames[] = {
    "Ambisonics", "Constant Power", "DBAP", "KNN", "MDAP", "VBAP", "VBIP"
};

static int resolveSurroundAlgoIndex (const juce::String& name)
{
    for (int i = 0; i < 7; ++i)
        if (name.equalsIgnoreCase (kSurroundAlgoNames[i])) return i;
    return -1;
}

// Stereo modes (configAlgorithm = 7 + stereoMode). Stored in the switch at
// PluginProcessor.cpp ~L5102: 0=Equal Power, 1=VBAP, 2=XY, 3=MS, 4=Blumlein.
static const char* const kStereoModeNames[] = {
    "Equal Power", "VBAP", "XY Pair", "MS Encode", "Blumlein"
};

static int resolveStereoModeIndex (const juce::String& name)
{
    for (int i = 0; i < 5; ++i)
        if (name.equalsIgnoreCase (kStereoModeNames[i])) return i;
    return -1;
}

static int resolveHrtfProfileIndex (const juce::String& name)
{
    for (int i = 0; i < OpenSpatialDelayProcessor::NUM_HRTF_PROFILES; ++i)
        if (name.equalsIgnoreCase (OpenSpatialDelayProcessor::hrtfProfileNames[i])) return i;
    return -1;
}

// Trajectory shape names, in the order declared in PluginProcessor.cpp
// (trajectoryShape parameter). Index 0 = None; 1..13 = drawn shapes.
static const char* const kTrajectoryNames[] = {
    "None", "Bounce", "Circle", "Cross", "Figure-8", "Heart", "Helix",
    "Infinity", "Line", "Orbit", "Random", "Spiral", "Square", "Triangle"
};

static int resolveTrajectoryIndex (const juce::String& name)
{
    for (int i = 0; i < 14; ++i)
        if (name.equalsIgnoreCase (kTrajectoryNames[i])) return i;
    // Accept "Figure8" as an alias for "Figure-8".
    if (name.equalsIgnoreCase ("Figure8")) return 4;
    return -1;
}

// Run the processor for N blocks of silent audio so trajectory.tick() advances.
// Required because getTrajectoryState() reports all zeros in a freshly-loaded
// processor — the map's trail renders nothing until the engine has ticked.
// Block count translates to simulated time: N blocks × 512 samples / 48 kHz.
static void pumpTrajectories (OpenSpatialDelayProcessor& processor, int numBlocks)
{
    const int blockSize = 512;
    juce::AudioBuffer<float> buffer (16, blockSize);  // plugin may route up to 16 out channels
    juce::MidiBuffer midi;
    for (int n = 0; n < numBlocks; ++n)
    {
        buffer.clear();
        processor.processBlock (buffer, midi);
    }
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
    bool showcase = false;
    juce::String outputFormatArg;   // --output-format "9.1.6 Atmos"
    juce::String surroundAlgoArg;   // --surround-algo "VBAP"
    juce::String stereoModeArg;     // --stereo-mode "XY Pair"
    juce::String hrtfProfileArg;    // --hrtf-profile "Studio Reference"
    juce::String trajectoryArg;     // --trajectory "Bounce"
    int          tapArg = 1;        // --tap 1..12 (1-indexed)

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
        else if (a == "--preset"        && i + 1 < argc) { presetArg       = juce::String (argv[++i]); }
        else if (a == "--mode"          && i + 1 < argc) { mode            = juce::String (argv[++i]); }
        else if (a == "--showcase")                      { showcase        = true; }
        else if (a == "--output-format" && i + 1 < argc) { outputFormatArg = juce::String (argv[++i]); }
        else if (a == "--surround-algo" && i + 1 < argc) { surroundAlgoArg = juce::String (argv[++i]); }
        else if (a == "--stereo-mode"   && i + 1 < argc) { stereoModeArg   = juce::String (argv[++i]); }
        else if (a == "--hrtf-profile"  && i + 1 < argc) { hrtfProfileArg  = juce::String (argv[++i]); }
        else if (a == "--trajectory"    && i + 1 < argc) { trajectoryArg   = juce::String (argv[++i]); }
        else if (a == "--tap"           && i + 1 < argc) { tapArg          = juce::String (argv[++i]).getIntValue(); }
        else if (! a.startsWith ("--"))                  { positional.push_back (a); }
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

    // ── Output format / algorithm / HRTF overrides ────────────────────────
    // Applied AFTER preset load so CLI flags win over the preset's stored
    // values (presets do not touch configOutputFormat or configAlgorithm).
    int selectedOutputIdx = -1;
    if (outputFormatArg.isNotEmpty())
    {
        selectedOutputIdx = resolveOutputFormatIndex (outputFormatArg);
        if (selectedOutputIdx < 0)
        {
            std::cerr << "Error: unknown --output-format: "
                      << outputFormatArg.toStdString() << "\n";
            return 1;
        }
        processor.configOutputFormat.store (selectedOutputIdx, std::memory_order_relaxed);
        processor.markConfigStateDirty();
        processor.requestOutputFormatChange (selectedOutputIdx);
    }
    else
    {
        selectedOutputIdx = processor.configOutputFormat.load (std::memory_order_relaxed);
    }

    // configAlgorithm is multiplexed by output format:
    //   - Surround: 0..6 (Ambisonics, Constant Power, DBAP, KNN, MDAP, VBAP, VBIP)
    //   - Stereo:   7..11 (Equal Power, VBAP, XY Pair, MS Encode, Blumlein)
    //   - Binaural: configHrtfProfile is a separate field
    const auto& selectedInfo
        = OpenSpatialDelayProcessor::outputFormatRegistry[(size_t) juce::jlimit (
            0, OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS - 1, selectedOutputIdx)];
    const bool isStereo   = selectedInfo.isStereoVariant;
    const bool isBinaural = (selectedInfo.format == OpenSpatialDelayProcessor::OutputFormat::Binaural);

    if (stereoModeArg.isNotEmpty())
    {
        if (! isStereo)
        {
            std::cerr << "Error: --stereo-mode only applies when --output-format is Stereo.\n";
            return 1;
        }
        int m = resolveStereoModeIndex (stereoModeArg);
        if (m < 0)
        {
            std::cerr << "Error: unknown --stereo-mode: "
                      << stereoModeArg.toStdString() << "\n";
            return 1;
        }
        processor.configAlgorithm.store (7 + m, std::memory_order_relaxed);
        processor.markConfigStateDirty();
    }
    else if (surroundAlgoArg.isNotEmpty())
    {
        if (isStereo || isBinaural)
        {
            std::cerr << "Error: --surround-algo does not apply when --output-format is Stereo or Binaural.\n";
            return 1;
        }
        int a = resolveSurroundAlgoIndex (surroundAlgoArg);
        if (a < 0)
        {
            std::cerr << "Error: unknown --surround-algo: "
                      << surroundAlgoArg.toStdString() << "\n";
            return 1;
        }
        processor.configAlgorithm.store (a, std::memory_order_relaxed);
        processor.markConfigStateDirty();
    }

    if (hrtfProfileArg.isNotEmpty())
    {
        if (! isBinaural)
        {
            std::cerr << "Error: --hrtf-profile only applies when --output-format is Binaural.\n";
            return 1;
        }
        int h = resolveHrtfProfileIndex (hrtfProfileArg);
        if (h < 0)
        {
            std::cerr << "Error: unknown --hrtf-profile: "
                      << hrtfProfileArg.toStdString() << "\n";
            return 1;
        }
        processor.configHrtfProfile.store (h, std::memory_order_relaxed);
        processor.markConfigStateDirty();
    }

    // OSC Receive: leave enabled for visual completeness (--showcase overrides
    // this and flips Send on / Receive off to reflect the demo state).
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

    if (showcase)
        applyShowcaseState (processor, *osd);

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
        auto mock = buildOutputDropdownMock (osd->getOSDLookAndFeel(), processor, 170);
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
        // history. Build a linear forward history so only the undo arrow is
        // active (redo inactive) — matches spec from issue #168 round 2.
        populateUndoHistory (processor);
        osd->syncForScreenshot();  // triggers timerCallback → updateUndoButtons
        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "preset-showcase")
    {
        // Per-preset showcase: render the plugin as configured by the loaded
        // preset, advance trajectories so any animated shape has a visible
        // trail, and draw the full path at peak brightness for a still frame.
        // Output format / algorithm / stereo-mode / HRTF come from CLI flags
        // applied above — no synthetic state, no drawer, no showcase glow.
        // 60 blocks @ 512 samples / 48 kHz ≈ 0.64 s of ticking, which puts
        // trajectories comfortably past their origin so their shape is legible.
        pumpTrajectories (processor, 60);
        osd->syncForScreenshot();
        auto& map = osd->getSpatialMapForScreenshot();
        map.setDrawFullTrajectoryForScreenshot (true);
        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "annotated-source")
    {
        // Source image for the callout overlay (issue #168 r3). Combines the
        // showcase state (features-on demo) with the Global Drawer open and a
        // populated undo history so the annotated overlay can point to every
        // major UI region — including the drawer tab and the highlighted undo
        // arrow. Always applies showcase, so --showcase is implicit here.
        if (! showcase)
            applyShowcaseState (processor, *osd);
        const float demoKnobs[6] = { 0.0f, 0.0f, 0.15f, -0.1f, 0.0f, 0.0f };
        osd->configureGlobalDrawer (true, demoKnobs, 6);
        populateUndoHistory (processor);
        osd->resized();
        osd->syncForScreenshot();
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
    else if (mode == "hero")
    {
        // Issue #168 round 3: hero screenshot for docs/README. Combines the
        // showcase state (features-on demo) with the Global Drawer open, a
        // populated undo history (undo arrow lit), per-tap activity glow on
        // a subset of non-Tap-1 taps, and a custom preset-name label that
        // better matches the Round-3 Tap-1 characterisation. --showcase is
        // implicit so the script doesn't need both flags.
        if (! showcase)
            applyShowcaseState (processor, *osd);
        const float demoKnobs[6] = { 0.0f, 0.0f, 0.15f, -0.1f, 0.0f, 0.0f };
        osd->configureGlobalDrawer (true, demoKnobs, 6);
        populateUndoHistory (processor);
        osd->resized();
        osd->syncForScreenshot();

        // Post-sync overrides — syncForScreenshot calls timerCallback which
        // (a) resets presetNameButton text from getCurrentPresetIndex(), and
        // (b) zeroes spatialMap activity to getTapActivityRMS() (== 0 in the
        // offline tool). Any manual overrides must therefore run AFTER that
        // sync, right before the snapshot.
        osd->setPresetNameForScreenshot ("Infinity Halo");
        auto& map = osd->getSpatialMapForScreenshot();
        // Light up a handful of non-Tap-1 echoes so the hero image reads as
        // a plugin in motion. Indices are 0-based: 1 = Tap 2, 4 = Tap 5,
        // 7 = Tap 8, 10 = Tap 11.
        map.setObjectActivityLevel (1,  0.75f);
        map.setObjectActivityLevel (4,  0.65f);
        map.setObjectActivityLevel (7,  0.85f);
        map.setObjectActivityLevel (10, 0.70f);

        // The trajectory trail (PluginEditor.cpp ~L1087) only draws when
        // the selected object's TrajectoryState has shape != 0. That state
        // is normally set from trajectory.tick() inside processBlock, which
        // never runs in the offline tool — so we seed it manually here.
        // Tap 1 (index 0) is the default selected object.
        TrajectoryState infinityState;
        infinityState.originAzDeg = 0.0f;
        infinityState.originElDeg = 20.0f;
        infinityState.originDist  = 0.25f;
        infinityState.shape       = 7;       // Infinity (Lemniscate)
        infinityState.phase       = 0.25f;   // mid-loop — animated dot sits ~¼ along the path
        infinityState.reverse     = false;
        infinityState.randomTime  = 0.0f;
        map.setTrajectoryState (0, infinityState);

        // Round-3 update 2: draw the entire sampled path at peak brightness
        // so the figure-∞ reads as a complete shape (the live view's
        // proximity-based glow would render most of the curve near-invisible
        // at 0.05 alpha in a still frame).
        map.setDrawFullTrajectoryForScreenshot (true);

        result = snapshotComponent (*osd, scaleFactor);
    }
    else if (mode == "elevation-map")
    {
        // Configure all 12 taps as a spiral: azimuths evenly spread around the
        // circle, distances progressing outward, elevations running from −90°
        // (Tap 1) up to +90° (Tap 12). Issue #168 round 2 corrects the prior
        // direction. Labels are now rendered by the plugin itself via the
        // screenshot-mode flag on SpatialMapComponent, so every enabled tap
        // shows its in-plugin label in the tap's own colour (no tool overlay).
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

            float t    = (float) i / 11.0f;              // 0..1
            float az   = -180.0f + t * 360.0f;           // full azimuth sweep
            float dist = 0.38f + t * 0.55f;              // 0.38 → 0.93
            float elev = -90.0f + t * 180.0f;            // −90 (Tap 1) → +90 (Tap 12)

            setBool  ("enabled",    true);
            setFloat ("azimuth",    az);
            setFloat ("elevation",  elev);
            setFloat ("distance",   dist);
        }

        auto& map = osd->getSpatialMapForScreenshot();
        map.setLabelAllEnabledObjectsForScreenshot (true);
        osd->syncForScreenshot();
        result = snapshotComponent (map, scaleFactor);
    }
    else if (mode == "trajectory")
    {
        // Reference capture for one trajectory shape applied to one tap.
        // Output is just the SpatialMap (no chrome). Every tap except the
        // chosen one is disabled; the chosen tap sits at (az=0, el=0, dist=0.5)
        // so every shape is centred and fully visible on the map. The trail
        // is drawn at peak brightness via setDrawFullTrajectoryForScreenshot.
        int shapeIdx = resolveTrajectoryIndex (trajectoryArg);
        if (shapeIdx < 0)
        {
            std::cerr << "Error: --mode trajectory requires --trajectory <name>.\n"
                         "Valid names: None, Bounce, Circle, Cross, Figure-8, Heart,\n"
                         "             Helix, Infinity, Line, Orbit, Random, Spiral,\n"
                         "             Square, Triangle\n";
            return 1;
        }
        const int tapZero = juce::jlimit (1, 12, tapArg) - 1;  // 0-indexed

        // Per-shape origin overrides. Default is (az=0, el=0, dist=0.5) so
        // every shape is centred at the top of the map. Cross's on-map
        // projection happens to match Bounce (its elevation arms are invisible
        // in the 2D top-down view), so its tap is placed at the bottom of the
        // map (az=180) to visually differentiate the two.
        float originAz   = 0.0f;
        float originEl   = 0.0f;
        float originDist = 0.5f;
        if (shapeIdx == 3)  // Cross
            originAz = 180.0f;

        auto& apvts = processor.apvts;
        for (int i = 0; i < 12; ++i)
        {
            auto pre = "object" + juce::String (i + 1) + "_";
            auto setFloat = [&] (const juce::String& id, float v) {
                if (auto* p = apvts.getParameter (pre + id))
                    p->setValueNotifyingHost (p->convertTo0to1 (v));
            };
            auto setChoice = [&] (const juce::String& id, int idx) {
                if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (pre + id)))
                {
                    int n = p->choices.size();
                    if (n > 1)
                        p->setValueNotifyingHost ((float) idx / (float) (n - 1));
                }
            };
            auto setBool = [&] (const juce::String& id, bool v) {
                if (auto* p = apvts.getParameter (pre + id))
                    p->setValueNotifyingHost (v ? 1.0f : 0.0f);
            };

            if (i == tapZero)
            {
                setBool   ("enabled",            true);
                setFloat  ("azimuth",            originAz);
                setFloat  ("elevation",          originEl);
                setFloat  ("distance",           originDist);
                setChoice ("trajectoryShape",    shapeIdx);
                setFloat  ("trajectorySpeed",    0.3f);
            }
            else
            {
                setBool   ("enabled",            false);
                setChoice ("trajectoryShape",    0);
            }
        }

        // Pump the processor ONLY for Random (shape 10). Its trail sampling
        // reads live noise state from the engine, so it needs a populated
        // randomTime. For every other shape, pumping advances trajectory.tick()
        // which flips trajectory.isActive(t) to true — and that makes
        // processor.getObjectState() return the animated position instead of
        // the APVTS origin, so the tap dot drifts off (az=0, el=0, dist=0.5).
        // Skipping the pump keeps the dot pinned to the origin while
        // setDrawFullTrajectoryForScreenshot still samples the entire path
        // directly from the seeded TrajectoryState below.
        if (shapeIdx == 10)
            pumpTrajectories (processor, 60);
        osd->syncForScreenshot();

        auto& map = osd->getSpatialMapForScreenshot();

        // syncForScreenshot() writes the map's selectedObject from the
        // editor's currentObjectIndex (== 0 by default) and overwrites
        // trajectoryStates from processor.getTrajectoryState(). We must
        // therefore (a) redirect the map's selection to the active tap,
        // and (b) re-seed its TrajectoryState explicitly, after sync.
        map.setSelectedObject (tapZero);

        TrajectoryState ts;
        ts.originAzDeg = originAz;
        ts.originElDeg = originEl;
        ts.originDist  = originDist;
        ts.shape       = shapeIdx;
        ts.phase       = 0.25f;
        ts.reverse     = false;
        ts.randomTime  = 0.0f;
        map.setTrajectoryState (tapZero, ts);

        // Random pumped the processor, which makes trajectory.isActive(t)
        // true and therefore getObjectState() returns the live animated
        // position — so the dot drifts off the origin. Force the map to
        // show the tap at the configured base position regardless.
        map.setObjectState (tapZero, originAz, originEl, originDist, true);

        map.setDrawFullTrajectoryForScreenshot (true);
        result = snapshotComponent (map, scaleFactor);
    }
    else
    {
        std::cerr << "Error: unknown --mode: " << mode.toStdString() << "\n";
        std::cerr << "Valid modes: full, drawer-open, tone-section, osc-section,\n"
                     "             save-overlay, preset-menu, output-dropdown,\n"
                     "             undo-active, annotated-source, hero,\n"
                     "             preset-showcase, spatial-map, elevation-map,\n"
                     "             trajectory\n";
        return 1;
    }

    if (! savePng (result, outFile, scaleFactor)) return 1;

    editor.reset();
    processor.releaseResources();
    return 0;
}
