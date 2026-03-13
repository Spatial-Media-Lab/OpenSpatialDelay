#include "PluginEditor.h"

//==============================================================================
// Object colours for the spatial map
//==============================================================================
const juce::Colour SpatialMapComponent::objectColours[SpatialMapComponent::MAX_OBJECTS] = {
    juce::Colour::fromHSV (0.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  1 red
    juce::Colour::fromHSV (1.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  2 orange
    juce::Colour::fromHSV (2.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  3 yellow
    juce::Colour::fromHSV (3.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  4 chartreuse
    juce::Colour::fromHSV (4.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  5 green
    juce::Colour::fromHSV (5.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  6 spring
    juce::Colour::fromHSV (6.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  7 cyan
    juce::Colour::fromHSV (7.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  8 azure
    juce::Colour::fromHSV (8.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  //  9 blue
    juce::Colour::fromHSV (9.0f  / 12.0f, 0.85f, 0.95f, 1.0f),  // 10 violet
    juce::Colour::fromHSV (10.0f / 12.0f, 0.85f, 0.95f, 1.0f),  // 11 magenta
    juce::Colour::fromHSV (11.0f / 12.0f, 0.85f, 0.95f, 1.0f),  // 12 rose
};

//==============================================================================
// Colour constants
//==============================================================================
namespace Colours_OSD
{
    static const juce::Colour bg          (0xff0a0a14);
    static const juce::Colour headerBg    (0xff111827);
    static const juce::Colour panelBorder (0xff1e293b);
    static const juce::Colour textPrimary (0xffe2e8f0);
    static const juce::Colour textSecondary (0xffb0bec5);
    static const juce::Colour textDim     (0xff64748b);
    static const juce::Colour accentCyan  (0xff00d4ff);
    static const juce::Colour accentPurple (0xff8B5CF6);
    static const juce::Colour accentAmber (0xfff5c542);
    static const juce::Colour sectionText (0xff7c8da0);
    static const juce::Colour knobBg      (0xff1e293b);
}


//==============================================================================
// SpatialMapComponent
//==============================================================================
SpatialMapComponent::SpatialMapComponent() {}

juce::Point<float> SpatialMapComponent::spatialToPixel (float azimuthDeg, float dist) const
{
    float radius = std::min (getWidth(), getHeight()) * 0.45f;
    float cx = getWidth()  * 0.5f;
    float cy = getHeight() * 0.5f;

    float azRad = juce::degreesToRadians (azimuthDeg);
    float x = cx - std::sin (azRad) * dist * radius;
    float y = cy - std::cos (azRad) * dist * radius;
    return { x, y };
}

std::pair<float, float> SpatialMapComponent::pixelToSpatial (juce::Point<float> pixel) const
{
    float radius = std::min (getWidth(), getHeight()) * 0.45f;
    float cx = getWidth()  * 0.5f;
    float cy = getHeight() * 0.5f;

    float dx = (pixel.x - cx) / radius;
    float dy = (pixel.y - cy) / radius;
    float dist = std::sqrt (dx * dx + dy * dy);
    dist = juce::jlimit (0.0f, 1.0f, dist);

    float azRad = std::atan2 (-dx, -dy);
    float azDeg = juce::radiansToDegrees (azRad);

    return { azDeg, dist };
}

int SpatialMapComponent::findObjectAt (juce::Point<float> pos) const
{
    for (int i = MAX_OBJECTS - 1; i >= 0; --i)
    {
        if (! objects[(size_t)i].enabled) continue;
        auto p = spatialToPixel (objects[(size_t)i].azimuthDeg, objects[(size_t)i].distance);

        // Variable hit radius based on elevation-dependent dot size
        float elDeg = objects[(size_t)i].elevationDeg;
        float z = std::sin (juce::degreesToRadians (elDeg));
        float baseDiam = (i == selectedObject) ? 18.0f : 14.0f;
        float currentDotSize = baseDiam + 3.0f * z;
        float hitRadius = std::max (currentDotSize * 0.5f + 2.0f, 10.0f);

        if (p.getDistanceFrom (pos) < hitRadius)
            return (int)i;
    }
    return -1;
}

void SpatialMapComponent::setObjectState (int index, float azimuthDeg, float elevationDeg,
                                        float distance, bool enabled)
{
    if (index < 0 || index >= MAX_OBJECTS) return;
    objects[(size_t)index].azimuthDeg   = azimuthDeg;
    objects[(size_t)index].elevationDeg = elevationDeg;
    objects[(size_t)index].distance     = distance;
    objects[(size_t)index].enabled      = enabled;
    repaint();
}

void SpatialMapComponent::paint (juce::Graphics& g)
{
    float radius = std::min (getWidth(), getHeight()) * 0.45f;
    float cx = getWidth()  * 0.5f;
    float cy = getHeight() * 0.5f;

    g.fillAll (Colours_OSD::bg);

    g.setColour (Colours_OSD::panelBorder);
    for (float r = 0.25f; r <= 1.0f; r += 0.25f)
    {
        float ringR = r * radius;
        g.drawEllipse (cx - ringR, cy - ringR, ringR * 2.0f, ringR * 2.0f, 1.0f);
    }

    g.drawLine (cx - radius, cy, cx + radius, cy, 0.5f);
    g.drawLine (cx, cy - radius, cx, cy + radius, 0.5f);

    g.setColour (juce::Colour (0xff8899aa));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText ("F",  (int)(cx - 10), (int)(cy - radius - 20), 20, 16, juce::Justification::centred);
    g.drawText ("B",  (int)(cx - 10), (int)(cy + radius + 4),  20, 16, juce::Justification::centred);
    g.drawText ("L",  (int)(cx - radius - 22), (int)(cy - 8),  20, 16, juce::Justification::centred);
    g.drawText ("R",  (int)(cx + radius + 4),  (int)(cy - 8),  20, 16, juce::Justification::centred);

    g.setColour (Colours_OSD::textPrimary);
    g.fillEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f);

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        if (! objects[(size_t)i].enabled) continue;

        auto pos = spatialToPixel (objects[(size_t)i].azimuthDeg, objects[(size_t)i].distance);

        // v0.6: IEM-faithful elevation visualization
        // Elevation encoded through dot visual properties: size, opacity, outline, text
        // (no stems — matches IEM StereoEncoder / Nuendo / Pro Tools industry standard)
        float elDeg = objects[(size_t)i].elevationDeg;
        bool isAbove = (elDeg >= 0.0f);
        float z = std::sin (juce::degreesToRadians (elDeg));  // -1..+1

        float baseDiam = (i == selectedObject) ? 18.0f : 14.0f;
        float dotSize = baseDiam + 3.0f * z;  // IEM-adapted: ±3px range
        float half = dotSize * 0.5f;

        // 1. Selection halo (scales with dot, alpha adapts by hemisphere)
        if (i == selectedObject)
        {
            g.setColour (objectColours[i].withAlpha (isAbove ? 0.3f : 0.15f));
            float haloSize = dotSize + 8.0f;
            float haloHalf = haloSize * 0.5f;
            g.fillEllipse (pos.x - haloHalf, pos.y - haloHalf, haloSize, haloSize);
        }

        // 2. Dot outline at full colour (IEM: always visible regardless of hemisphere)
        juce::Path dotPath;
        dotPath.addEllipse (pos.x - half, pos.y - half, dotSize, dotSize);
        g.setColour (objectColours[i]);
        g.strokePath (dotPath, juce::PathStrokeType (1.2f));

        // 3. Dot fill with hemisphere alpha (IEM: 1.0 above, 0.3 below)
        g.setColour (objectColours[i].withAlpha (isAbove ? 1.0f : 0.3f));
        g.fillPath (dotPath);

        // 4. Number label — Path-based faux bold with pixel-perfect centering
        //    Converts glyphs to a Path, then fills + strokes for guaranteed visual weight.
        //    GlyphArrangement getBoundingBox() centers on actual pixel bounds (no descent offset).
        {
            auto labelColour = isAbove ? juce::Colours::black : objectColours[i];
            juce::Font labelFont (juce::FontOptions (11.0f));
            juce::GlyphArrangement glyphs;
            juce::String numText (i + 1);
            glyphs.addLineOfText (labelFont, numText, 0.0f, 0.0f);
            auto glyphBounds = glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true);
            float gx = pos.x - glyphBounds.getWidth() * 0.5f - glyphBounds.getX();
            float gy = pos.y - glyphBounds.getHeight() * 0.5f - glyphBounds.getY();
            glyphs.moveRangeOfGlyphs (0, -1, gx, gy);

            // Convert to Path and fill+stroke for faux bold effect
            juce::Path textPath;
            glyphs.createPath (textPath);
            g.setColour (labelColour);
            g.fillPath (textPath);
            g.strokePath (textPath, juce::PathStrokeType (0.8f));
        }

        // 5. Elevation degree label (selected object only, non-zero elevation)
        if (i == selectedObject && std::abs (elDeg) > 1.0f)
        {
            float labelOffsetY = isAbove ? -(half + 10.0f) : (half + 2.0f);
            g.setColour (objectColours[i].withAlpha (0.85f));
            g.setFont (juce::FontOptions (9.0f));
            juce::String elText = (elDeg > 0.0f ? "+" : "")
                                + juce::String (juce::roundToInt (elDeg))
                                + juce::String::charToString (0x00B0);
            g.drawText (elText, (int)(pos.x - 18), (int)(pos.y + labelOffsetY),
                        36, 12, juce::Justification::centred);
        }

        // 6. OSC override label (collision-aware positioning)
        if (oscOverride[(size_t)i])
        {
            float oscLabelY = (i == selectedObject && elDeg < -1.0f)
                            ? pos.y + half + 14.0f    // below elevation label
                            : pos.y + half + 1.0f;    // normal position
            g.setColour (Colours_OSD::accentCyan);
            g.setFont (juce::FontOptions (8.0f).withStyle ("Bold"));
            g.drawText ("OSC", (int)(pos.x - half - 2), (int)(oscLabelY),
                        (int)(dotSize + 4), 10, juce::Justification::centred);
        }
    }
}

void SpatialMapComponent::mouseDown (const juce::MouseEvent& e)
{
    draggedObject = findObjectAt (e.position);
    if (draggedObject >= 0)
        listeners.call ([this](Listener& l) { l.objectSelected (draggedObject); });
}

void SpatialMapComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (draggedObject < 0) return;

    auto result = pixelToSpatial (e.position);
    float azDeg = result.first;
    float dist  = result.second;
    objects[(size_t)draggedObject].azimuthDeg = azDeg;
    objects[(size_t)draggedObject].distance   = dist;
    repaint();

    listeners.call ([this, azDeg, dist](Listener& l) {
        l.objectPositionChanged (draggedObject, azDeg, dist);
    });
}

//==============================================================================
// Helper: create a rotary slider with consistent styling
//==============================================================================
static void styleSlider (juce::Slider& slider, juce::LookAndFeel& lf,
                         juce::Slider::SliderStyle style = juce::Slider::RotaryVerticalDrag)
{
    slider.setLookAndFeel (&lf);
    slider.setSliderStyle (style);
    slider.setWantsKeyboardFocus (true);
    slider.setMouseClickGrabsKeyboardFocus (true);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 14);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

static void styleLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (11.0f));  // (#13) consistent font
    label.setColour (juce::Label::textColourId, Colours_OSD::textSecondary);
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================
// Editor constructor
//==============================================================================
OpenSpatialDelayEditor::OpenSpatialDelayEditor (OpenSpatialDelayProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&ableton12Look);
    setSize (820, 580);

    // --- Title ---------------------------------------------------------------
    titleLabel.setText ("OpenSpatialDelay v0.6", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, Colours_OSD::accentCyan);
    addAndMakeVisible (titleLabel);

    // --- Spatial map ---------------------------------------------------------
    addAndMakeVisible (spatialMap);
    spatialMap.addListener (this);

    // --- Global knobs --------------------------------------------------------
    auto addKnob = [&](juce::Slider& s, juce::Label& l, const juce::String& name,
                        const juce::String& paramId, std::unique_ptr<SliderAttachment>& attach) {
        styleSlider (s, ableton12Look);
        addAndMakeVisible (s);
        styleLabel (l, name);
        addAndMakeVisible (l);
        attach = std::make_unique<SliderAttachment> (processorRef.apvts, paramId, s);
    };

    addKnob (inputGainSlider,  inputGainLabel,  "INPUT",    "inputGain",  inputGainAttach);
    addKnob (delayTimeSlider,  delayTimeLabel,  "TIME",     "delayTime",  delayTimeAttach);
    addKnob (noteDivisionSlider, delayTimeLabel, "TIME",    "noteDivision", noteDivisionAttach);
    addKnob (feedbackSlider,   feedbackLabel,   "FEEDBACK", "feedback",   feedbackAttach);
    addKnob (pitchShiftSlider, pitchShiftLabel, "PITCH",    "pitchShift", pitchShiftAttach);
    addKnob (filterHPSlider,   filterHPLabel,   "HP",       "filterHP",   filterHPAttach);
    addKnob (filterLPSlider,   filterLPLabel,   "LP",       "filterLP",   filterLPAttach);
    addKnob (dryWetSlider,     dryWetLabel,     "DRY/WET",  "dryWet",     dryWetAttach);
    addKnob (outputGainSlider, outputGainLabel, "OUTPUT",   "outputGain", outputGainAttach);

    // Purple accent for tone section
    filterHPSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentPurple);
    filterLPSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentPurple);
    pitchShiftSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentPurple);

    // --- Dropdowns (#13: consistent font) ------------------------------------
    auto setupCombo = [&](juce::ComboBox& box, juce::Label& label, const juce::String& name,
                          const juce::String& paramId, const juce::StringArray& items,
                          std::unique_ptr<ComboBoxAttachment>& attach)
    {
        for (int i = 0; i < items.size(); ++i)
            box.addItem (items[i], i + 1);
        // (#9) Colors inherited from Ableton12Look for consistency
        box.setLookAndFeel (&ableton12Look);
        addAndMakeVisible (box);
        if (&label != &delayTimeLabel) {
            styleLabel (label, name);
            addAndMakeVisible (label);
        }
        attach = std::make_unique<ComboBoxAttachment> (processorRef.apvts, paramId, box);
    };

    // Algorithm combo — manually managed (no ComboBoxParameterAttachment)
    // Items dynamically populated based on output format (stereo vs surround)
    algorithmBox.setLookAndFeel (&ableton12Look);
    addAndMakeVisible (algorithmBox);
    styleLabel (algorithmLabel, "ALGORITHM");
    addAndMakeVisible (algorithmLabel);
    algorithmBox.onChange = [this]
    {
        int selectedId = algorithmBox.getSelectedId();
        if (selectedId > 0)
        {
            int paramIdx = selectedId - 1;  // IDs are 1-based, param indices are 0-based
            auto* param = processorRef.apvts.getParameter ("algorithm");
            float normVal = static_cast<float> (paramIdx) / 10.0f;  // 11 items (0..10)
            param->setValueNotifyingHost (normVal);
        }
    };
    setupCombo (hrtfProfileBox, hrtfProfileLabel, "PROFILE",
                "hrtfProfile", { "Simple", "Studio Ref", "Immersive", "Natural", "Precise", "Spatial" },
                hrtfProfileAttach);
    setupCombo (syncModeBox, delayTimeLabel, "SYNC MODE",
                "syncMode", { "Notes", "Triplet", "Dotted", "16th" }, syncModeAttach);

    // --- Output format dropdown (header bar) ---------------------------------
    {
        juce::StringArray formatNames;
        for (const auto& info : OpenSpatialDelayProcessor::outputFormatRegistry)
            formatNames.add (info.name);
        setupCombo (outputFormatBox, outputFormatLabel, "OUTPUT",
                    "outputFormat", formatNames, outputFormatAttach);
    }

    // --- Header dropdown labels: left-aligned, section-header style ----------
    {
        auto headerLblFont   = juce::FontOptions (10.0f).withStyle ("Bold");
        auto headerLblColour = Colours_OSD::sectionText;

        for (auto* lbl : { &algorithmLabel, &outputFormatLabel, &hrtfProfileLabel })
        {
            lbl->setFont (headerLblFont);
            lbl->setColour (juce::Label::textColourId, headerLblColour);
            lbl->setJustificationType (juce::Justification::centredLeft);
        }
    }

    // --- Tempo sync button (#5/#6) -------------------------------------------
    tempoSyncButton.setClickingTogglesState (true);
    tempoSyncButton.setLookAndFeel (&ableton12Look);
    // (#6) Fixed colors: purple when Sync (on), blue when Time (off)
    tempoSyncButton.setColour (juce::TextButton::buttonColourId,   Colours_OSD::accentCyan);    // Time = blue
    tempoSyncButton.setColour (juce::TextButton::buttonOnColourId,  Colours_OSD::accentPurple);  // Sync = purple
    tempoSyncButton.setColour (juce::TextButton::textColourOffId,  juce::Colours::black);
    tempoSyncButton.setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
    addAndMakeVisible (tempoSyncButton);
    tempoSyncAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, "tempoSync", tempoSyncButton);

    auto updateSyncUI = [this] {
        bool isSynced = tempoSyncButton.getToggleState();
        delayTimeSlider.setVisible (!isSynced);
        noteDivisionSlider.setVisible (isSynced);
        syncModeBox.setVisible (isSynced);

        // Toggle button text: "Sync" when synced, "Time" when free
        tempoSyncButton.setButtonText (isSynced ? "Sync" : "Time");
        delayTimeLabel.setText ("TIME", juce::dontSendNotification);
    };

    tempoSyncButton.onStateChange = updateSyncUI;
    updateSyncUI();

    // --- Object selector buttons ---------------------------------------------
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto& btn = objectButtons[(size_t)i];
        btn.setButtonText (juce::String (i + 1));
        btn.setClickingTogglesState (false);
        btn.setLookAndFeel (&ableton12Look);  // (#13)
        btn.onClick = [this, i] { selectObject (i); };
        addAndMakeVisible (btn);
    }
    updateObjectButtonColours();

    // --- Per-object controls -------------------------------------------------
    // (#3) Azimuth: rotary knob — use RotaryHorizontalVerticalDrag for natural direction
    styleSlider (objAzimuthSlider, ableton12Look, juce::Slider::RotaryHorizontalVerticalDrag);
    objAzimuthSlider.setRotaryParameters (juce::MathConstants<float>::pi, 3.0f * juce::MathConstants<float>::pi, false);
    objAzimuthSlider.setReversed (true);  // (#8) IEM StereoEncoder convention: clockwise knob = clockwise on map
    addAndMakeVisible (objAzimuthSlider);
    styleLabel (objAzLabel, "AZIMUTH");
    addAndMakeVisible (objAzLabel);

    // (#4) Elevation: vertical slider with wider text box
    styleSlider (objElevationSlider, ableton12Look, juce::Slider::LinearVertical);
    objElevationSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 14);  // (#4) wider
    addAndMakeVisible (objElevationSlider);
    styleLabel (objElLabel, "ELEV");
    addAndMakeVisible (objElLabel);

    // Distance: rotary knob
    styleSlider (objDistanceSlider, ableton12Look, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objDistanceSlider);
    styleLabel (objDistLabel, "DIST");
    addAndMakeVisible (objDistLabel);

    // --- Enabled toggle (#1: styled power button, no default rendering) ------
    objEnabledButton.setButtonText ("");  // we draw text ourselves
    objEnabledButton.setClickingTogglesState (true);
    objEnabledButton.setLookAndFeel (&ableton12Look);  // (#13)
    // Make the default button rendering invisible — we paint over it
    objEnabledButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    objEnabledButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    objEnabledButton.setColour (juce::TextButton::textColourOnId, juce::Colours::transparentBlack);
    objEnabledButton.setColour (juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    addAndMakeVisible (objEnabledButton);

    // --- v0.4: Per-object Doppler amount knob --------------------------------
    styleSlider (objDopplerSlider, ableton12Look, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objDopplerSlider);
    styleLabel (objDopplerLabel, "DOPPLER");
    addAndMakeVisible (objDopplerLabel);
    objDopplerSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentAmber);

    // --- v0.6: Per-object trajectory controls (bottom panel, bound in selectObject) ---
    objTrajectoryBox.setLookAndFeel (&ableton12Look);
    objTrajectoryBox.addItem ("None",     1);
    objTrajectoryBox.addItem ("Spiral",   2);
    objTrajectoryBox.addItem ("Orbit",    3);
    objTrajectoryBox.addItem ("Bounce",   4);
    objTrajectoryBox.addItem ("Figure-8", 5);
    objTrajectoryBox.addItem ("Random",   6);
    addAndMakeVisible (objTrajectoryBox);
    styleLabel (objTrajectoryLabel, "TRAJ");
    addAndMakeVisible (objTrajectoryLabel);
    // Attachment created in selectObject()

    styleSlider (objTrajectorySpeedSlider, ableton12Look, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objTrajectorySpeedSlider);
    styleLabel (objTrajectorySpeedLabel, "SPEED");
    addAndMakeVisible (objTrajectorySpeedLabel);
    objTrajectorySpeedSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentAmber);
    // Attachment created in selectObject()

    // --- v0.6: ADM-OSC toggle button (header bar) ---------------------------
    oscToggleButton.setButtonText ("OSC");
    oscToggleButton.setClickingTogglesState (true);
    oscToggleButton.setLookAndFeel (&ableton12Look);
    oscToggleButton.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff222230));
    oscToggleButton.setColour (juce::TextButton::buttonOnColourId,  juce::Colour (0xff00a86b));  // green when active
    oscToggleButton.setColour (juce::TextButton::textColourOffId,  Colours_OSD::textDim);
    oscToggleButton.setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
    addAndMakeVisible (oscToggleButton);
    oscToggleAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, "admOscEnabled", oscToggleButton);

    // v0.6: Editable OSC port label (double-click to edit, Enter to commit)
    oscPortLabel.setText (juce::String (processorRef.getOscReceivePort()), juce::dontSendNotification);
    oscPortLabel.setEditable (false, true, false);  // single-click no, double-click yes, return-key commits
    oscPortLabel.setFont (juce::FontOptions (11.0f));
    oscPortLabel.setColour (juce::Label::textColourId, Colours_OSD::textSecondary);
    oscPortLabel.setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
    oscPortLabel.setColour (juce::Label::backgroundWhenEditingColourId, juce::Colour (0xff1a1a2e));
    oscPortLabel.setColour (juce::Label::outlineWhenEditingColourId, Colours_OSD::accentCyan);
    oscPortLabel.setJustificationType (juce::Justification::centred);
    oscPortLabel.onTextChange = [this]
    {
        auto text = oscPortLabel.getText().trim();
        int port = text.getIntValue();
        if (port >= 1024 && port <= 65535)
        {
            processorRef.setOscReceivePort (port);
        }
        else
        {
            // Revert to current valid port
            oscPortLabel.setText (juce::String (processorRef.getOscReceivePort()), juce::dontSendNotification);
        }
    };
    addAndMakeVisible (oscPortLabel);

    // --- v0.4: Global Air Absorption toggle ----------------------------------
    airAbsorptionButton.setButtonText ("AIR");
    airAbsorptionButton.setClickingTogglesState (true);
    airAbsorptionButton.setLookAndFeel (&ableton12Look);
    airAbsorptionButton.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff222230));
    airAbsorptionButton.setColour (juce::TextButton::buttonOnColourId,  Colours_OSD::accentCyan);
    airAbsorptionButton.setColour (juce::TextButton::textColourOffId,  Colours_OSD::textDim);
    airAbsorptionButton.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
    addAndMakeVisible (airAbsorptionButton);
    airAbsorptionAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, "airAbsorption", airAbsorptionButton);

    // --- v0.6: Preset browser (header bar) ------------------------------------
    presetBox.setLookAndFeel (&ableton12Look);
    presetBox.setTextWhenNothingSelected ("Preset...");
    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        int sel = presetBox.getSelectedId();
        if (sel > 0)
            processorRef.loadPreset (sel - 1);  // ComboBox IDs are 1-based
    };

    auto stylePresetButton = [&] (juce::TextButton& btn, const juce::String& text)
    {
        btn.setButtonText (text);
        btn.setLookAndFeel (&ableton12Look);
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff222230));
        btn.setColour (juce::TextButton::textColourOffId, Colours_OSD::textSecondary);
        addAndMakeVisible (btn);
    };

    stylePresetButton (presetPrevButton, "<");
    presetPrevButton.onClick = [this]
    {
        processorRef.loadPreviousPreset();
        presetBox.setSelectedId (processorRef.getCurrentPresetIndex() + 1, juce::dontSendNotification);
    };

    stylePresetButton (presetNextButton, ">");
    presetNextButton.onClick = [this]
    {
        processorRef.loadNextPreset();
        presetBox.setSelectedId (processorRef.getCurrentPresetIndex() + 1, juce::dontSendNotification);
    };

    stylePresetButton (presetSaveButton, "Save");
    presetSaveButton.onClick = [this]
    {
        auto* aw = new juce::AlertWindow ("Save Preset",
                                           "Enter a name for this preset:",
                                           juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor ("presetName", "", "Name:");
        aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
        aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
        aw->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, aw] (int result)
            {
                if (result == 1)
                {
                    auto name = aw->getTextEditorContents ("presetName").trim();
                    if (name.isNotEmpty())
                    {
                        processorRef.saveUserPreset (name);
                        refreshPresetBox();
                        presetBox.setSelectedId (processorRef.getCurrentPresetIndex() + 1,
                                                 juce::dontSendNotification);
                    }
                }
                delete aw;
            }), true);
    };

    selectObject (0);
    startTimerHz (30);
}

OpenSpatialDelayEditor::~OpenSpatialDelayEditor()
{
    spatialMap.removeListener (this);
    setLookAndFeel (nullptr);
    stopTimer();
}

//==============================================================================
// Preset browser helpers
//==============================================================================
void OpenSpatialDelayEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    auto names = processorRef.getPresetNames();
    for (int i = 0; i < names.size(); ++i)
        presetBox.addItem (names[i], i + 1);  // ComboBox IDs are 1-based

    int current = processorRef.getCurrentPresetIndex();
    if (current >= 0 && current < names.size())
        presetBox.setSelectedId (current + 1, juce::dontSendNotification);
}

//==============================================================================
// Object selection
//==============================================================================
void OpenSpatialDelayEditor::selectObject (int index)
{
    currentObjectIndex = juce::jlimit (0, SpatialMapComponent::MAX_OBJECTS - 1, index);
    objAzAttach.reset();
    objElAttach.reset();
    objDistAttach.reset();
    objEnabledAttach.reset();
    objDopplerAttach.reset();
    objTrajectoryAttach.reset();
    objTrajectorySpeedAttach.reset();

    auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
    objAzAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "azimuth",   objAzimuthSlider);
    objElAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "elevation", objElevationSlider);
    objDistAttach    = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "distance",  objDistanceSlider);
    objEnabledAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, prefix + "enabled",   objEnabledButton);
    objDopplerAttach = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "dopplerAmount", objDopplerSlider);
    objTrajectoryAttach      = std::make_unique<ComboBoxAttachment> (processorRef.apvts, prefix + "trajectoryShape", objTrajectoryBox);
    objTrajectorySpeedAttach = std::make_unique<SliderAttachment>   (processorRef.apvts, prefix + "trajectorySpeed", objTrajectorySpeedSlider);

    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
        objectButtons[(size_t)i].setToggleState (i == currentObjectIndex, juce::dontSendNotification);

    // (#2) Set per-object knob arc color to match the selected object's color
    auto objCol = SpatialMapComponent::objectColours[currentObjectIndex];
    objAzimuthSlider.setColour  (juce::Slider::thumbColourId, objCol);
    objElevationSlider.setColour (juce::Slider::thumbColourId, objCol);
    objDistanceSlider.setColour (juce::Slider::thumbColourId, objCol);

    spatialMap.setSelectedObject (currentObjectIndex);
    updateObjectButtonColours();
}

void OpenSpatialDelayEditor::updateObjectButtonColours()
{
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto& btn = objectButtons[(size_t)i];
        auto colour = SpatialMapComponent::objectColours[i];

        auto prefix = "object" + juce::String (i + 1) + "_";
        bool enabled = false;
        if (auto* p = processorRef.apvts.getRawParameterValue (prefix + "enabled"))
            enabled = p->load() > 0.5f;

        if (i == currentObjectIndex)
        {
            // (#2) Selected = bright object color (both on/off colour IDs)
            btn.setColour (juce::TextButton::buttonColourId,   colour);
            btn.setColour (juce::TextButton::buttonOnColourId,  colour);
            btn.setColour (juce::TextButton::textColourOffId,  juce::Colours::black);
            btn.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        }
        else if (enabled)
        {
            btn.setColour (juce::TextButton::buttonColourId,   colour.withAlpha (0.35f));
            btn.setColour (juce::TextButton::buttonOnColourId,  colour.withAlpha (0.35f));
            btn.setColour (juce::TextButton::textColourOffId,  Colours_OSD::textPrimary);
            btn.setColour (juce::TextButton::textColourOnId,   Colours_OSD::textPrimary);
        }
        else
        {
            btn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff151520));
            btn.setColour (juce::TextButton::buttonOnColourId,  juce::Colour (0xff151520));
            btn.setColour (juce::TextButton::textColourOffId,  Colours_OSD::textDim);
            btn.setColour (juce::TextButton::textColourOnId,   Colours_OSD::textDim);
        }
    }
}

void OpenSpatialDelayEditor::objectPositionChanged (int objectIndex, float azimuthDeg, float distance)
{
    auto prefix = "object" + juce::String (objectIndex + 1) + "_";
    if (auto* param = processorRef.apvts.getParameter (prefix + "azimuth"))
        param->setValueNotifyingHost (param->convertTo0to1 (azimuthDeg));
    if (auto* param = processorRef.apvts.getParameter (prefix + "distance"))
        param->setValueNotifyingHost (param->convertTo0to1 (distance));
}

void OpenSpatialDelayEditor::objectSelected (int objectIndex) { selectObject (objectIndex); }

void OpenSpatialDelayEditor::updateMapFromParameters()
{
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto state = processorRef.getObjectState (i);
        spatialMap.setObjectState (i, state.azimuthDeg, state.elevationDeg, state.distance, state.enabled);
        // v0.6: Pass OSC override state to spatial map for indicator
        spatialMap.setOscOverride (i, processorRef.isOscOverrideActive (i));
    }
}

void OpenSpatialDelayEditor::timerCallback()
{
    updateMapFromParameters();
    updateObjectButtonColours();

    // v0.2: Bus-aware output format greying
    int maxCh = processorRef.getMaxBusChannels();
    for (int i = 0; i < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS; ++i)
    {
        const auto& info = OpenSpatialDelayProcessor::outputFormatRegistry[static_cast<size_t> (i)];
        bool available = (info.requiredChannels <= maxCh);
        outputFormatBox.setItemEnabled (i + 1, available);
    }

    // Trigger layout recomputation if format changed
    processorRef.requestOutputFormatChange (outputFormatBox.getSelectedItemIndex());

    // v0.5: Context-sensitive header dropdowns based on output format
    int fmtIdx = static_cast<int> (processorRef.getActiveOutputFormat());
    bool isBinaural = (processorRef.getActiveOutputFormat() == OpenSpatialDelayProcessor::OutputFormat::Binaural);
    bool isStereoVariant = (fmtIdx >= 0 && fmtIdx < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS)
                           && OpenSpatialDelayProcessor::outputFormatRegistry[static_cast<size_t> (fmtIdx)].isStereoVariant;
    bool isAmbiOutput = (fmtIdx >= 0 && fmtIdx < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS)
                        && OpenSpatialDelayProcessor::outputFormatRegistry[static_cast<size_t> (fmtIdx)].isAmbisonicsOutput;

    // HRTF Profile: only visible for binaural output
    hrtfProfileBox.setVisible (isBinaural);
    hrtfProfileLabel.setVisible (isBinaural);

    // Algorithm dropdown — dynamically populated based on output format category
    // Binaural: hide (rendering is always Direct Binaural / HRTF)
    // Ambisonics: show disabled, text = "Ambisonics Encode"
    // Stereo: show only stereo modes (param indices 6-10)
    // Surround: show only surround algorithms (param indices 0-5)
    int algoIdx = static_cast<int> (processorRef.apvts.getRawParameterValue ("algorithm")->load());

    // Determine current format category: 0=binaural, 1=ambi, 2=stereo, 3=surround
    int fmtCategory = isBinaural ? 0 : isAmbiOutput ? 1 : isStereoVariant ? 2 : 3;

    if (isBinaural)
    {
        algorithmBox.setVisible (false);
        algorithmLabel.setVisible (false);
    }
    else if (isAmbiOutput)
    {
        algorithmBox.setVisible (true);
        algorithmLabel.setVisible (true);
        algorithmBox.setEnabled (false);
        algorithmBox.setText ("Ambisonics Encode", juce::dontSendNotification);
    }
    else
    {
        algorithmBox.setVisible (true);
        algorithmLabel.setVisible (true);
        algorithmBox.setEnabled (true);

        // Rebuild combo items only when format category changes
        if (fmtCategory != lastAlgoCategoryShown)
        {
            lastAlgoCategoryShown = fmtCategory;
            algorithmBox.clear (juce::dontSendNotification);

            if (isStereoVariant)
            {
                // Stereo modes only — IDs match param indices + 1
                algorithmBox.addItem ("Equal Power",  7);   // param index 6
                algorithmBox.addItem ("Stereo VBAP",  8);   // param index 7
                algorithmBox.addItem ("XY Pair",      9);   // param index 8
                algorithmBox.addItem ("MS Encode",    10);  // param index 9
                algorithmBox.addItem ("Blumlein",     11);  // param index 10

                // Auto-snap if current param is a surround algorithm
                if (algoIdx < 6)
                {
                    processorRef.apvts.getParameter ("algorithm")
                        ->setValueNotifyingHost (6.0f / 10.0f);  // Equal Power
                    algoIdx = 6;
                }
            }
            else  // Surround
            {
                // Surround algorithms only — IDs match param indices + 1
                algorithmBox.addItem ("Ambisonics",   1);   // param index 0
                algorithmBox.addItem ("DBAP",         2);   // param index 1
                algorithmBox.addItem ("KNN",          3);   // param index 2
                algorithmBox.addItem ("MDAP",         4);   // param index 3
                algorithmBox.addItem ("VBAP",         5);   // param index 4
                algorithmBox.addItem ("VBIP",         6);   // param index 5

                // Auto-snap if current param is a stereo mode
                if (algoIdx > 5)
                {
                    processorRef.apvts.getParameter ("algorithm")
                        ->setValueNotifyingHost (4.0f / 10.0f);  // VBAP
                    algoIdx = 4;
                }
            }
        }

        // Sync combo selection from parameter (IDs are param index + 1)
        algorithmBox.setSelectedId (algoIdx + 1, juce::dontSendNotification);
    }

    // v0.6: Sync OSC port label from processor (e.g., after state restore)
    if (! oscPortLabel.isBeingEdited())
    {
        auto currentPort = juce::String (processorRef.getOscReceivePort());
        if (oscPortLabel.getText() != currentPort)
            oscPortLabel.setText (currentPort, juce::dontSendNotification);
    }

    // v0.6: Sync preset dropdown selection from processor
    {
        int expected = processorRef.getCurrentPresetIndex() + 1;  // 1-based ComboBox ID
        if (presetBox.getSelectedId() != expected)
            presetBox.setSelectedId (expected, juce::dontSendNotification);
    }

    repaint();
}

//==============================================================================
// Drawing helpers
//==============================================================================
void OpenSpatialDelayEditor::drawSelectionBox (juce::Graphics& g, juce::Component& label, juce::Component& slider)
{
    if (slider.isVisible() && slider.hasKeyboardFocus (false) && slider.isEnabled())
    {
        auto bounds = label.getBounds().getUnion (slider.getBounds()).toFloat().expanded (3.0f, 2.0f);
        g.setColour (juce::Colour (0xFFC6C6C6));
        float len = 4.0f, thickness = 1.0f;
        g.drawLine (bounds.getX(), bounds.getY(), bounds.getX() + len, bounds.getY(), thickness);
        g.drawLine (bounds.getX(), bounds.getY(), bounds.getX(), bounds.getY() + len, thickness);
        g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight() - len, bounds.getY(), thickness);
        g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + len, thickness);
        g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getX() + len, bounds.getBottom(), thickness);
        g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getX(), bounds.getBottom() - len, thickness);
        g.drawLine (bounds.getRight(), bounds.getBottom(), bounds.getRight() - len, bounds.getBottom(), thickness);
        g.drawLine (bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - len, thickness);
    }
}

void OpenSpatialDelayEditor::drawSectionHeader (juce::Graphics& g, int x, int y, int w, const juce::String& text)
{
    g.setColour (Colours_OSD::sectionText);
    g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    g.drawText (text, x, y, w, 12, juce::Justification::centredLeft);
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (juce::Font (juce::FontOptions (10.0f)), text, 0.0f, 0.0f);
    int textWidth = (int) glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth() + 6;
    g.setColour (Colours_OSD::panelBorder);
    g.drawLine ((float)(x + textWidth), (float)(y + 6), (float)(x + w), (float)(y + 6), 0.5f);
}

//==============================================================================
// Paint
//==============================================================================
void OpenSpatialDelayEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours_OSD::bg);

    // Header bar (56px)
    g.setColour (Colours_OSD::headerBg);
    g.fillRect (0, 0, getWidth(), 56);
    g.setColour (Colours_OSD::panelBorder);
    g.drawLine (0.0f, 56.0f, (float) getWidth(), 56.0f, 1.0f);
    titleLabel.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));

    // Right panel border (#3: wider panel)
    g.setColour (Colours_OSD::panelBorder);
    g.drawRoundedRectangle ((float)(getWidth() - 266), 58.0f, 262.0f,
                            (float)(getHeight() - 62), 8.0f, 1.0f);

    // Bottom panel border (#3: adjusted for wider right panel)
    g.drawRoundedRectangle (2.0f, (float)(getHeight() - 152),
                            (float)(getWidth() - 272), 148.0f, 8.0f, 1.0f);

    // --- Section headers (positions computed in resized) ---
    drawSectionHeader (g, rpX, delayHeaderY,  rpW, "DELAY");
    drawSectionHeader (g, rpX, toneHeaderY,   rpW, "TONE");
    drawSectionHeader (g, rpX, mixHeaderY,    rpW, "MIX");
    drawSectionHeader (g, 12,  objectHeaderY,    200, "OBJECT POSITION");

    // --- Styled enabled button (#1: clean custom paint, no JUCE chrome) ---
    {
        auto& btn = objEnabledButton;
        auto bounds = btn.getBounds().toFloat();
        if (bounds.getWidth() > 0)
        {
            bool isOn = btn.getToggleState();
            // (#2) Use the selected object's color when ON
            auto objCol = SpatialMapComponent::objectColours[currentObjectIndex];
            auto col = isOn ? objCol : juce::Colour (0xff222230);
            g.setColour (col);
            g.fillRoundedRectangle (bounds, 4.0f);
            // Border
            g.setColour (isOn ? objCol.brighter (0.2f) : juce::Colour (0xff3a3a4a));
            g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
            // Text
            g.setColour (isOn ? juce::Colours::black : Colours_OSD::textDim);
            g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
            g.drawText (isOn ? "ON" : "OFF", bounds.toNearestInt(), juce::Justification::centred);
        }
    }

    // --- v0.4: Styled Air Absorption toggle (in TONE section) ---
    {
        auto& btn = airAbsorptionButton;
        auto bounds = btn.getBounds().toFloat();
        if (bounds.getWidth() > 0)
        {
            bool isOn = btn.getToggleState();
            auto col = isOn ? Colours_OSD::accentCyan : juce::Colour (0xff222230);
            g.setColour (col);
            g.fillRoundedRectangle (bounds, 4.0f);
            g.setColour (isOn ? Colours_OSD::accentCyan.brighter (0.2f) : juce::Colour (0xff3a3a4a));
            g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
            g.setColour (isOn ? juce::Colours::black : Colours_OSD::textDim);
            g.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
            g.drawText ("AIR", bounds.toNearestInt(), juce::Justification::centred);
        }
    }

    // --- v0.6: OSC toggle button + status dot in header ---
    {
        auto& btn = oscToggleButton;
        auto bounds = btn.getBounds().toFloat();
        if (bounds.getWidth() > 0)
        {
            bool isOn = btn.getToggleState();
            auto col = isOn ? juce::Colour (0xff00a86b) : juce::Colour (0xff222230);
            g.setColour (col);
            g.fillRoundedRectangle (bounds, 4.0f);
            g.setColour (isOn ? juce::Colour (0xff00a86b).brighter (0.2f) : juce::Colour (0xff3a3a4a));
            g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
            g.setColour (isOn ? juce::Colours::white : Colours_OSD::textDim);
            g.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
            g.drawText ("OSC", bounds.toNearestInt(), juce::Justification::centred);

            // Status dot: green when connected, grey when off
            float dotX = bounds.getRight() - 7.0f;
            float dotY = bounds.getY() - 3.0f;
            g.setColour (isOn && processorRef.isOscConnected()
                         ? juce::Colour (0xff00ff00) : juce::Colour (0xff555555));
            g.fillEllipse (dotX, dotY, 6.0f, 6.0f);
        }
    }

    // --- Selection boxes for focused knobs ---
    drawSelectionBox (g, delayTimeLabel,  delayTimeSlider);
    drawSelectionBox (g, delayTimeLabel,  noteDivisionSlider);
    drawSelectionBox (g, feedbackLabel,   feedbackSlider);
    drawSelectionBox (g, dryWetLabel,     dryWetSlider);
    drawSelectionBox (g, pitchShiftLabel, pitchShiftSlider);
    drawSelectionBox (g, filterLPLabel,   filterLPSlider);
    drawSelectionBox (g, filterHPLabel,   filterHPSlider);
    drawSelectionBox (g, inputGainLabel,  inputGainSlider);
    drawSelectionBox (g, outputGainLabel, outputGainSlider);
    drawSelectionBox (g, objDopplerLabel, objDopplerSlider);
    drawSelectionBox (g, objTrajectorySpeedLabel, objTrajectorySpeedSlider);
}

//==============================================================================
// Layout
//==============================================================================
void OpenSpatialDelayEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (56);

    // --- Header: Title (left) | Algorithm + Output + Monitor + HRTF (right-aligned) ---
    // Row 1: small labels.  Row 2: title + dropdown boxes, left-aligned vertically.
    const int hPad = 8, hGap = 8;
    const int algoBoxW = 120, outBoxW = 120;
    const int lblH = 14, boxH = 24;
    int lblY = header.getY() + 10;              // label row — balanced clearance from top
    int boxY = lblY + lblH + 4;                 // dropdown row — 4px gap below label

    int rx = header.getRight() - hPad;

    // Algorithm / HRTF Profile (rightmost — shared position, one visible at a time)
    rx -= algoBoxW;
    algorithmLabel.setBounds   (rx, lblY, algoBoxW, lblH);
    algorithmBox.setBounds     (rx, boxY, algoBoxW, boxH);
    hrtfProfileLabel.setBounds (rx, lblY, algoBoxW, lblH);
    hrtfProfileBox.setBounds   (rx, boxY, algoBoxW, boxH);
    rx -= hGap;

    // Output Format (to the left of Algorithm/Profile)
    rx -= outBoxW;
    outputFormatLabel.setBounds (rx, lblY, outBoxW, lblH);
    outputFormatBox.setBounds   (rx, boxY, outBoxW, boxH);
    rx -= hGap;

    // v0.6: OSC toggle button + port label (to the left of Output Format)
    rx -= 50;
    oscPortLabel.setBounds (rx, boxY + 2, 50, boxH - 4);
    rx -= 42;
    oscToggleButton.setBounds (rx, boxY + 2, 42, boxH - 4);
    rx -= hGap;

    // v0.6: Preset browser controls (to the left of OSC)
    rx -= 44;
    presetSaveButton.setBounds (rx, boxY + 2, 44, boxH - 4);
    rx -= 24;
    presetNextButton.setBounds (rx, boxY + 2, 24, boxH - 4);
    rx -= 24;
    presetPrevButton.setBounds (rx, boxY + 2, 24, boxH - 4);
    rx -= 130;
    presetBox.setBounds (rx, boxY + 2, 130, boxH - 4);

    // Title left-aligned with the dropdown row
    titleLabel.setBounds (header.getX() + 12, boxY, rx - header.getX() - 16, boxH);

    // === RIGHT PANEL (264px — #3: enlarged for breathing space) ==============
    auto rightPanel = area.removeFromRight (264).reduced (10, 4);

    rpX = rightPanel.getX();
    rpW = rightPanel.getWidth();
    int panelW = rpW;

    // --- DELAY section (now starts at top of right panel) ---
    delayHeaderY = rightPanel.getY();
    rightPanel.removeFromTop (16);

    int knobW = 90;
    int knobH = 80;   // 14 label + 66 slider
    int knobGap = 12;
    int pairW = knobW * 2 + knobGap;
    int centerOff = (panelW - pairW) / 2;  // (#5) center knob pairs
    int kx0 = rpX + centerOff;
    int kx1 = kx0 + knobW + knobGap;

    auto placeKnob = [knobW](juce::Slider& slider, juce::Label& label, int x, int y) {
        label.setBounds (x, y, knobW, 14);
        slider.setBounds (x, y + 14, knobW, 66);
    };

    int row0Y = rightPanel.getY();
    placeKnob (inputGainSlider,    inputGainLabel,  kx0, row0Y);
    placeKnob (delayTimeSlider,    delayTimeLabel,  kx1, row0Y);
    placeKnob (noteDivisionSlider, delayTimeLabel,  kx1, row0Y);

    // Sync button + mode below TIME knob
    int syncX = kx1 + 10;
    int syncY = row0Y + knobH + 2;
    tempoSyncButton.setBounds (syncX, syncY, 70, 20);
    syncModeBox.setBounds (syncX, syncY + 22, 70, 20);

    int row1Y = row0Y + knobH + 44;
    placeKnob (feedbackSlider,   feedbackLabel,   kx0, row1Y);
    placeKnob (pitchShiftSlider, pitchShiftLabel, kx1, row1Y);

    // --- TONE section (#4: positioned after Feedback/Pitch, not on top) ---
    toneHeaderY = row1Y + knobH + 20;

    int row2Y = toneHeaderY + 16;
    placeKnob (filterHPSlider, filterHPLabel, kx0, row2Y);
    placeKnob (filterLPSlider, filterLPLabel, kx1, row2Y);

    // v0.4: AIR toggle right-aligned in TONE section header row
    airAbsorptionButton.setBounds (rpX + rpW - 42, toneHeaderY - 3, 42, 18);

    // --- MIX section (#7: properly spaced below filters) ---
    mixHeaderY = row2Y + knobH + 20;

    int row3Y = mixHeaderY + 16;
    placeKnob (dryWetSlider,     dryWetLabel,     kx0, row3Y);
    placeKnob (outputGainSlider, outputGainLabel, kx1, row3Y);

    // === BOTTOM PANEL ========================================================
    auto bottomPanel = area.removeFromBottom (150).reduced (8, 4);

    objectHeaderY = getHeight() - 150;
    bottomPanel.removeFromTop (8);
    auto objBtnRow = bottomPanel.removeFromTop (28);
    int objBtnW = objBtnRow.getWidth() / SpatialMapComponent::MAX_OBJECTS;
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
        objectButtons[(size_t)i].setBounds (objBtnRow.getX() + i * objBtnW, objBtnRow.getY(),
                                         objBtnW - 2, 26);

    bottomPanel.removeFromTop (6);

    // Per-object controls: ON/OFF, AZIMUTH, ELEV, DIST, DOPPLER
    auto objCtrlArea = bottomPanel;
    int ctrlY = objCtrlArea.getY();
    int ctrlX = objCtrlArea.getX();

    // ON/OFF button
    objEnabledButton.setBounds (ctrlX, ctrlY + 18, 38, 24);

    // AZIMUTH knob
    int spatialX = ctrlX + 50;
    objAzLabel.setBounds (spatialX, ctrlY, 80, 14);
    objAzimuthSlider.setBounds (spatialX, ctrlY + 14, 80, 72);

    // ELEVATION slider
    int elX = spatialX + 90;
    objElLabel.setBounds (elX, ctrlY, 60, 14);
    objElevationSlider.setBounds (elX + 5, ctrlY + 14, 50, 72);

    // DISTANCE knob
    int distX = elX + 70;
    objDistLabel.setBounds (distX, ctrlY, 80, 14);
    objDistanceSlider.setBounds (distX, ctrlY + 14, 80, 72);

    // v0.4: Per-object DOPPLER amount knob (0=off, >0=on at that intensity)
    int dopX = distX + 90;
    objDopplerLabel.setBounds (dopX, ctrlY, 80, 14);
    objDopplerSlider.setBounds (dopX, ctrlY + 14, 80, 72);

    // v0.6: Per-object TRAJ dropdown + SPEED knob
    int trajX = dopX + 90;
    objTrajectoryLabel.setBounds (trajX, ctrlY, 80, 14);
    objTrajectoryBox.setBounds (trajX, ctrlY + 24, 80, 22);

    int speedX = trajX + 90;
    objTrajectorySpeedLabel.setBounds (speedX, ctrlY, 80, 14);
    objTrajectorySpeedSlider.setBounds (speedX, ctrlY + 14, 80, 72);

    // === SPATIAL MAP =========================================================
    spatialMap.setBounds (area.reduced (4));
}
