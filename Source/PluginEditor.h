#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// 2D top-down spatial map showing object positions
//==============================================================================
class SpatialMapComponent : public juce::Component
{
public:
    static constexpr int MAX_OBJECTS = OpenSpatialDelayProcessor::MAX_OBJECTS;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void objectPositionChanged (int objectIndex, float azimuthDeg, float distance) = 0;
        virtual void objectSelected (int objectIndex) = 0;
    };

    SpatialMapComponent();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    void setObjectState (int index, float azimuthDeg, float elevationDeg,
                         float distance, bool enabled);
    void setSelectedObject (int index) { selectedObject = index; repaint(); }

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    struct ObjectInfo
    {
        float azimuthDeg  = 0.0f;
        float elevationDeg = 0.0f;
        float distance    = 0.5f;
        bool  enabled     = false;
    };

    std::array<ObjectInfo, MAX_OBJECTS> objects;
    int selectedObject = -1;
    int draggedObject  = -1;

    juce::ListenerList<Listener> listeners;

    // Coordinate conversion
    juce::Point<float> spatialToPixel (float azimuthDeg, float distance) const;
    std::pair<float, float> pixelToSpatial (juce::Point<float> pixel) const;
    int findObjectAt (juce::Point<float> pos) const;

public:
    static const juce::Colour objectColours[MAX_OBJECTS];
private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialMapComponent)
};

//==============================================================================
// Reversed Slider (for azimuth — matches IEM StereoEncoder convention)
// Clockwise knob rotation = clockwise movement on spatial map
//==============================================================================
class ReverseSlider : public juce::Slider
{
public:
    void setReversed (bool r) { reversed = r; }
    bool isReversed() const { return reversed; }

    double proportionOfLengthToValue (double proportion) override
    {
        if (reversed)
            return juce::Slider::proportionOfLengthToValue (1.0 - proportion);
        return juce::Slider::proportionOfLengthToValue (proportion);
    }

    double valueToProportionOfLength (double value) override
    {
        if (reversed)
            return 1.0 - juce::Slider::valueToProportionOfLength (value);
        return juce::Slider::valueToProportionOfLength (value);
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (reversed)
        {
            auto rw = wheel;
            rw.deltaX = -wheel.deltaX;
            rw.deltaY = -wheel.deltaY;
            juce::Slider::mouseWheelMove (e, rw);
        }
        else
        {
            juce::Slider::mouseWheelMove (e, wheel);
        }
    }

private:
    bool reversed = false;
};

//==============================================================================
// Ableton 12 style LookAndFeel for knobs
//==============================================================================
class Ableton12Look : public juce::LookAndFeel_V4
{
public:
    Ableton12Look()
    {
        setDefaultSansSerifTypefaceName ("sans-serif");
        // (#9) Consistent dropdown / popup menu colors
        setColour (juce::PopupMenu::backgroundColourId,            juce::Colour (0xff111827));
        setColour (juce::PopupMenu::textColourId,                  juce::Colour (0xffe2e8f0));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff00d4ff));
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::black);
        setColour (juce::ComboBox::backgroundColourId,             juce::Colour (0xff111827));
        setColour (juce::ComboBox::textColourId,                   juce::Colour (0xffe2e8f0));
        setColour (juce::ComboBox::outlineColourId,                juce::Colour (0xff334155));
        setColour (juce::ComboBox::arrowColourId,                  juce::Colour (0xffb0bec5));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override
    {
        float knobDiameter = 48.0f;
        auto knobBounds = juce::Rectangle<float> (x + (width - knobDiameter) * 0.5f,
                                                  y + (height - knobDiameter) * 0.5f,
                                                  knobDiameter, knobDiameter);

        auto radius = knobDiameter * 0.5f;
        auto toX = knobBounds.getCentreX();
        auto toY = knobBounds.getCentreY();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 3.5f;

        // Background Track (#323232)
        juce::Path backgroundTrack;
        backgroundTrack.addCentredArc (toX, toY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xFF323232));
        g.strokePath (backgroundTrack, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active Value Arc (uses accent colour from slider property, default cyan)
        if (slider.isEnabled())
        {
            auto accentColour = slider.findColour (juce::Slider::thumbColourId);
            if (accentColour == juce::Colours::transparentBlack)
                accentColour = juce::Colour (0xFF32D2FF);  // default cyan

            juce::Path valueArc;
            valueArc.addCentredArc (toX, toY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
            g.setColour (accentColour);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Indicator Needle (White)
        g.setColour (juce::Colours::white);
        juce::Path needle;
        needle.addRectangle (-0.75f, -radius, 1.5f, radius * 0.45f);
        g.fillPath (needle, juce::AffineTransform::rotation (angle).translated (toX, toY));
    }

    // (#1) Clean button background — skip entirely for transparent buttons
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        if (backgroundColour.isTransparent())
            return;  // ON/OFF button is fully custom-painted in Editor::paint()

        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        auto baseColour = backgroundColour;
        if (isButtonDown)            baseColour = baseColour.darker (0.15f);
        else if (isMouseOverButton)  baseColour = baseColour.brighter (0.08f);

        g.setColour (baseColour);
        g.fillRoundedRectangle (bounds, 4.0f);
    }
};

//==============================================================================
// Main plugin editor
//==============================================================================
class OpenSpatialDelayEditor : public juce::AudioProcessorEditor,
                                private juce::Timer,
                                private SpatialMapComponent::Listener
{
public:
    explicit OpenSpatialDelayEditor (OpenSpatialDelayProcessor&);
    ~OpenSpatialDelayEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // Helper for drawing Ableton-style corner selection box
    void drawSelectionBox (juce::Graphics& g, juce::Component& label, juce::Component& slider);
    // Helper for drawing section headers
    void drawSectionHeader (juce::Graphics& g, int x, int y, int w, const juce::String& text);

    // SpatialMapComponent::Listener
    void objectPositionChanged (int objectIndex, float azimuthDeg, float distance) override;
    void objectSelected (int objectIndex) override;

    OpenSpatialDelayProcessor& processorRef;

    Ableton12Look ableton12Look;

    // UI components
    SpatialMapComponent spatialMap;

    // Global controls — ordered by signal flow
    juce::Slider inputGainSlider, outputGainSlider;
    juce::Slider delayTimeSlider, noteDivisionSlider, feedbackSlider;
    juce::Slider pitchShiftSlider;
    juce::Slider filterHPSlider, filterLPSlider;
    juce::Slider dryWetSlider;
    juce::ComboBox algorithmBox, hrtfProfileBox, syncModeBox;
    juce::TextButton tempoSyncButton;

    // Object controls
    std::array<juce::TextButton, OpenSpatialDelayProcessor::MAX_OBJECTS> objectButtons;
    ReverseSlider objAzimuthSlider;
    juce::Slider objElevationSlider, objDistanceSlider;
    juce::TextButton objEnabledButton;  // styled power button instead of checkbox
    int currentObjectIndex = 0;

    // Labels
    juce::Label titleLabel;
    juce::Label inputGainLabel, outputGainLabel;
    juce::Label delayTimeLabel, feedbackLabel;
    juce::Label pitchShiftLabel;
    juce::Label filterHPLabel, filterLPLabel;
    juce::Label dryWetLabel;
    juce::Label algorithmLabel, hrtfProfileLabel;
    juce::Label objAzLabel, objElLabel, objDistLabel;

    // APVTS attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> inputGainAttach, outputGainAttach;
    std::unique_ptr<SliderAttachment> delayTimeAttach, noteDivisionAttach, feedbackAttach;
    std::unique_ptr<SliderAttachment> pitchShiftAttach;
    std::unique_ptr<SliderAttachment> filterHPAttach, filterLPAttach;
    std::unique_ptr<SliderAttachment> dryWetAttach;
    std::unique_ptr<ComboBoxAttachment> algorithmAttach, hrtfProfileAttach, syncModeAttach;
    std::unique_ptr<ButtonAttachment> tempoSyncAttach;

    // Per-object attachments (for the currently selected object)
    std::unique_ptr<SliderAttachment> objAzAttach, objElAttach, objDistAttach;
    std::unique_ptr<ButtonAttachment> objEnabledAttach;

    void selectObject (int index);
    void updateObjectButtonColours();
    void updateMapFromParameters();

    // Section header positions (computed in resized, drawn in paint)
    int rpX = 0, rpW = 0;
    int configHeaderY = 0, delayHeaderY = 0, toneHeaderY = 0, mixHeaderY = 0;
    int objectHeaderY = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayEditor)
};
