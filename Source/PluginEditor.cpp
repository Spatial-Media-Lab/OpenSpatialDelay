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

// (#13) Consistent font name used everywhere
static const char* const kFontName = "sans-serif";

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
    float hitRadius = 12.0f;
    for (int i = MAX_OBJECTS - 1; i >= 0; --i)
    {
        if (! objects[(size_t)i].enabled) continue;
        auto p = spatialToPixel (objects[(size_t)i].azimuthDeg, objects[(size_t)i].distance);
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
        float dotSize = (i == selectedObject) ? 18.0f : 14.0f;
        float half = dotSize * 0.5f;

        if (i == selectedObject)
        {
            g.setColour (objectColours[i].withAlpha (0.3f));
            g.fillEllipse (pos.x - half - 4, pos.y - half - 4,
                           dotSize + 8, dotSize + 8);
        }

        g.setColour (objectColours[i]);
        g.fillEllipse (pos.x - half, pos.y - half, dotSize, dotSize);

        g.setColour (juce::Colours::black);
        g.setFont (juce::FontOptions (10.0f));
        g.drawText (juce::String (i + 1),
                    (int)(pos.x - half), (int)(pos.y - half),
                    (int)dotSize, (int)dotSize,
                    juce::Justification::centred);
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
    titleLabel.setText ("OpenSpatialDelay v0.2", juce::dontSendNotification);
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

    setupCombo (algorithmBox,   algorithmLabel,   "ALGORITHM",
                "algorithm", { "Direct Binaural", "VBAP", "Ambisonics", "VBIP", "KNN" }, algorithmAttach);
    setupCombo (hrtfProfileBox, hrtfProfileLabel, "HRTF",
                "hrtfProfile", { "Studio Ref", "Immersive", "Natural", "Precise", "Spatial" },
                hrtfProfileAttach);
    setupCombo (syncModeBox, delayTimeLabel, "SYNC MODE",
                "syncMode", { "Notes", "Triplet", "Dotted", "16th" }, syncModeAttach);

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
// Object selection
//==============================================================================
void OpenSpatialDelayEditor::selectObject (int index)
{
    currentObjectIndex = juce::jlimit (0, SpatialMapComponent::MAX_OBJECTS - 1, index);
    objAzAttach.reset();
    objElAttach.reset();
    objDistAttach.reset();
    objEnabledAttach.reset();

    auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
    objAzAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "azimuth",   objAzimuthSlider);
    objElAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "elevation", objElevationSlider);
    objDistAttach    = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "distance",  objDistanceSlider);
    objEnabledAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, prefix + "enabled",   objEnabledButton);

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
    }
}

void OpenSpatialDelayEditor::timerCallback()
{
    updateMapFromParameters();
    updateObjectButtonColours();

    // v0.2: Context-sensitive UI — disable Direct Binaural and HRTF Profile for surround tracks
    bool isBinaural = (processorRef.getActiveOutputFormat() == OpenSpatialDelayProcessor::OutputFormat::Binaural);

    // Enable/disable Direct Binaural option (item ID 1 = first item)
    algorithmBox.setItemEnabled (1, isBinaural);
    // If Direct Binaural is selected on a surround track, switch to VBAP
    if (! isBinaural && algorithmBox.getSelectedId() == 1)
        algorithmBox.setSelectedId (2);

    // HRTF Profile is only relevant for binaural output
    hrtfProfileBox.setEnabled (isBinaural);
    hrtfProfileLabel.setEnabled (isBinaural);
    hrtfProfileBox.setAlpha (isBinaural ? 1.0f : 0.4f);
    hrtfProfileLabel.setAlpha (isBinaural ? 1.0f : 0.4f);

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

    // Header bar
    g.setColour (Colours_OSD::headerBg);
    g.fillRect (0, 0, getWidth(), 40);
    g.setColour (Colours_OSD::panelBorder);
    g.drawLine (0.0f, 40.0f, (float) getWidth(), 40.0f, 1.0f);
    titleLabel.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));

    // Right panel border (#3: wider panel)
    g.setColour (Colours_OSD::panelBorder);
    g.drawRoundedRectangle ((float)(getWidth() - 266), 42.0f, 262.0f,
                            (float)(getHeight() - 46), 8.0f, 1.0f);

    // Bottom panel border (#3: adjusted for wider right panel)
    g.drawRoundedRectangle (2.0f, (float)(getHeight() - 152),
                            (float)(getWidth() - 272), 148.0f, 8.0f, 1.0f);

    // --- Section headers (positions computed in resized) ---
    drawSectionHeader (g, rpX, configHeaderY, rpW, "CONFIG");
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
}

//==============================================================================
// Layout
//==============================================================================
void OpenSpatialDelayEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (40);
    titleLabel.setBounds (header.reduced (10, 8));

    // === RIGHT PANEL (264px — #3: enlarged for breathing space) ==============
    auto rightPanel = area.removeFromRight (264).reduced (10, 4);

    rpX = rightPanel.getX();
    rpW = rightPanel.getWidth();
    int panelW = rpW;

    // --- CONFIG section (#3: wider labels for "Algorithm" readability) ---
    configHeaderY = rightPanel.getY();
    rightPanel.removeFromTop (16);

    auto algoRow = rightPanel.removeFromTop (22);
    algorithmLabel.setBounds (algoRow.removeFromLeft (70));
    algorithmBox.setBounds (algoRow.reduced (2, 0));
    rightPanel.removeFromTop (4);

    auto hrtfRow = rightPanel.removeFromTop (22);
    hrtfProfileLabel.setBounds (hrtfRow.removeFromLeft (70));
    hrtfProfileBox.setBounds (hrtfRow.reduced (2, 0));
    rightPanel.removeFromTop (8);

    // --- DELAY section ---
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
    toneHeaderY = row1Y + knobH + 4;

    int row2Y = toneHeaderY + 16;
    placeKnob (filterHPSlider, filterHPLabel, kx0, row2Y);
    placeKnob (filterLPSlider, filterLPLabel, kx1, row2Y);

    // --- MIX section (#7: properly spaced below filters) ---
    mixHeaderY = row2Y + knobH + 4;

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

    // Per-object controls
    auto objCtrlArea = bottomPanel;
    int ctrlY = objCtrlArea.getY();
    int ctrlX = objCtrlArea.getX();

    objEnabledButton.setBounds (ctrlX, ctrlY + 18, 38, 24);

    int spatialX = ctrlX + 50;
    objAzLabel.setBounds (spatialX, ctrlY, 80, 14);
    objAzimuthSlider.setBounds (spatialX, ctrlY + 14, 80, 72);

    int elX = spatialX + 90;
    objElLabel.setBounds (elX, ctrlY, 60, 14);
    objElevationSlider.setBounds (elX + 5, ctrlY + 14, 50, 72);

    int distX = elX + 70;
    objDistLabel.setBounds (distX, ctrlY, 80, 14);
    objDistanceSlider.setBounds (distX, ctrlY + 14, 80, 72);

    // === SPATIAL MAP =========================================================
    spatialMap.setBounds (area.reduced (4));
}
