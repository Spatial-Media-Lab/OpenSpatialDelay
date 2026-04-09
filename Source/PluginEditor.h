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

    void setProcessor (OpenSpatialDelayProcessor* p) { processor = p; }
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void setObjectState (int index, float azimuthDeg, float elevationDeg,
                         float distance, bool enabled);
    void setOscOverride (int index, bool active) { if (index >= 0 && index < MAX_OBJECTS) oscOverride[(size_t)index] = active; }
    void setSelectedObject (int index) { selectedObject = index; repaint(); }

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    // Callbacks: fired on object drag start/end — used for gesture wrapping (issue E15)
    std::function<void (int objectIndex)> onDragStarted;
    std::function<void (int objectIndex)> onDragEnded;

private:
    struct ObjectInfo
    {
        float azimuthDeg  = 0.0f;
        float elevationDeg = 0.0f;
        float distance    = 0.5f;
        bool  enabled     = false;
    };

    std::array<ObjectInfo, MAX_OBJECTS> objects;
    std::array<TrajectoryState, MAX_OBJECTS> trajectoryStates = {};  // v0.9: trajectory origin + state
    std::array<bool, MAX_OBJECTS> oscOverride = {};  // v0.6: per-object OSC override indicator
    int selectedObject = -1;
    int draggedObject  = -1;
    OpenSpatialDelayProcessor* processor = nullptr;  // for Random trail look-ahead

    juce::ListenerList<Listener> listeners;

    // Coordinate conversion
    juce::Point<float> spatialToPixel (float azimuthDeg, float distance) const;
    std::pair<float, float> pixelToSpatial (juce::Point<float> pixel) const;
    int findObjectAt (juce::Point<float> pos) const;

    // v0.7: Star field — deterministic twinkling dots
    struct Star { float x, y, phase, speed, size; juce::Colour colour; };
    std::vector<Star> stars;
    float starTime = 0.0f;
    void generateStars();

    // v0.7: Per-object activity level (0..1) for glow animation
    std::array<float, MAX_OBJECTS> activityLevel = {};
    // v0.7: Per-object pulse phase (0..1, cycles at ~3s) for expanding ring effect
    std::array<float, MAX_OBJECTS> pulsePhase = {};

public:
    static const juce::Colour objectColours[MAX_OBJECTS];
    void setTrajectoryState (int index, const TrajectoryState& ts) { if (index >= 0 && index < MAX_OBJECTS) trajectoryStates[(size_t)index] = ts; }
    void advanceStarAnimation (float dt) { starTime += dt; }
    void setObjectActivityLevel (int index, float level) { if (index >= 0 && index < MAX_OBJECTS) activityLevel[(size_t)index] = level; }
    void advancePulsePhases (float dt)
    {
        for (int i = 0; i < MAX_OBJECTS; ++i)
        {
            if (activityLevel[(size_t)i] > 0.01f)
            {
                pulsePhase[(size_t)i] += dt / 3.0f;  // 3-second cycle
                if (pulsePhase[(size_t)i] >= 1.0f) pulsePhase[(size_t)i] -= 1.0f;
            }
            else
            {
                pulsePhase[(size_t)i] = 0.0f;
            }
        }
    }
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
        // Proportion overrides already handle the reversed value mapping,
        // so pass mousewheel through without negating deltas (fixes #157)
        juce::Slider::mouseWheelMove (e, wheel);
    }

private:
    bool reversed = false;
};

//==============================================================================
// Observatory v6 LookAndFeel — custom fonts, knobs, buttons, dropdowns
//==============================================================================
class OSDLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Custom typefaces loaded from embedded binary data
    juce::Typeface::Ptr dmSansRegular, dmSansMedium, dmSansBold;
    juce::Typeface::Ptr jetbrainsRegular, jetbrainsMedium, jetbrainsBold;
    juce::Typeface::Ptr robotoMedium;

    OSDLookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override
    {
        if (f.getStyleFlags() & juce::Font::bold)
            return dmSansBold;
        return dmSansRegular;
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (juce::FontOptions (dmSansRegular).withHeight (13.0f));
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font (juce::FontOptions (dmSansRegular).withHeight (13.0f));
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override;

    // v0.9: Custom popup menu rendering to match ComboBox dropdown appearance
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;
    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                    int standardMenuItemHeight, int& idealWidth,
                                    int& idealHeight) override;

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    // v0.9: Thin 1px outline for all text editors (knob values + OSC fields)
    void drawTextEditorOutline (juce::Graphics& g, int width, int height,
                                juce::TextEditor& editor) override;

    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override
    {
        // Preset button → DM Sans Regular 13px (match ComboBox font)
        if (button.getComponentID() == "presetButton")
            return juce::Font (juce::FontOptions (dmSansRegular).withHeight (13.0f));
        // Object selector buttons (22px) → JetBrains Mono Medium 12px
        // Other buttons → JetBrains Mono Medium 10px
        float h = (buttonHeight >= 22) ? 12.0f : 10.0f;
        if (jetbrainsMedium)
            return juce::Font (juce::FontOptions (jetbrainsMedium).withHeight (h));
        return juce::Font (juce::FontOptions (h));
    }

    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* label = new juce::Label();
        // Copy essential setup from LookAndFeel_V4::createSliderTextBox
        label->setJustificationType (juce::Justification::centred);
        label->setKeyboardType (juce::TextInputTarget::decimalKeyboard);
        label->setColour (juce::Label::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
        label->setColour (juce::Label::backgroundColourId,
                          (slider.getSliderStyle() == juce::Slider::LinearBar
                           || slider.getSliderStyle() == juce::Slider::LinearBarVertical)
                              ? juce::Colours::transparentBlack
                              : slider.findColour (juce::Slider::textBoxBackgroundColourId));
        label->setColour (juce::Label::outlineColourId, slider.findColour (juce::Slider::textBoxOutlineColourId));
        // Match the small knob value font (JetBrains Mono 11px)
        if (jetbrainsRegular)
            label->setFont (juce::Font (juce::FontOptions (jetbrainsRegular).withHeight (11.0f)));
        else
            label->setFont (juce::Font (juce::FontOptions (11.0f)));
        label->setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
        label->setColour (juce::Label::backgroundWhenEditingColourId, juce::Colour (0xff0A0A14));
        // v0.9: Selection highlight matches knob accent color at 35% opacity
        label->setColour (juce::TextEditor::highlightColourId,
                          slider.findColour (juce::Slider::thumbColourId).withAlpha (0.35f));
        // v0.9: Thin outline matching knob accent color when editing
        label->setColour (juce::Label::outlineWhenEditingColourId,
                          slider.findColour (juce::Slider::thumbColourId));
        // v0.9: Center text and select-all when editor opens
        label->onEditorShow = [label]()
        {
            if (auto* ed = label->getCurrentTextEditor())
            {
                ed->setJustification (juce::Justification::centred);
                ed->setHighlightedRegion ({ 0, label->getText().length() });
            }
        };
        return label;
    }
};

//==============================================================================
// v0.7: Observatory v6 filter graph with two XY-draggable dots
// HP dot: X=HP freq, Y=HP Q.  LP dot: X=LP freq, Y=LP Q.
// Draws actual magnitude response curves with resonance peaks.
//==============================================================================
class FilterGraphComponent : public juce::Component
{
public:
    FilterGraphComponent();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    /** Set the HP/LP frequency values (Hz) for display. Call from timer. */
    void setFrequencies (float hpHz, float lpHz);

    /** Set separate HP and LP resonance Q. Call from timer. */
    void setQ (float hpQ, float lpQ);

    /** Callback when user drags a handle horizontally (frequency). */
    std::function<void (float hpHz, float lpHz)> onFrequencyChanged;

    /** Callback when user drags HP Q vertically. */
    std::function<void (float q)> onHPQChanged;

    /** Callback when user drags LP Q vertically. */
    std::function<void (float q)> onLPQChanged;

    // Callbacks: fired on filter handle drag start/end — used for gesture wrapping (issue E15)
    std::function<void()> onFilterDragStarted;
    std::function<void()> onFilterDragEnded;

    /** Enable/disable the filter display (dims when off). */
    void setEnabled (bool enabled) { filterEnabled = enabled; repaint(); }

private:
    float hpFreq = 50.0f;      // Hz
    float lpFreq = 5000.0f;    // Hz
    float hpQ = 0.707f;        // HP resonance
    float lpQ = 0.707f;        // LP resonance
    bool  filterEnabled = true;

    static constexpr float kMinFreq = 20.0f;
    static constexpr float kMaxFreq = 20000.0f;

    enum DragTarget { None, HP, LP };
    DragTarget currentDrag = None;
    float dragStartY = 0.0f;
    float dragStartQ = 0.707f;

    float freqToX (float freqHz) const;
    float xToFreq (float x) const;

    /** Compute 2nd-order HP magnitude at frequency f given cutoff fc and Q. */
    static float hpMagnitude (float f, float fc, float q);
    /** Compute 2nd-order LP magnitude at frequency f given cutoff fc and Q. */
    static float lpMagnitude (float f, float fc, float q);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterGraphComponent)
};

//==============================================================================
// Reusable toggle pill: 5px indicator dot + uppercase label, ON/OFF/HOVER states
//==============================================================================
class IndicatorToggle : public juce::Button
{
public:
    IndicatorToggle (const juce::String& label, const juce::Colour& accentColour,
                     juce::Typeface::Ptr typeface);

    /** Compute preferred width for a given label (static for layout calculations). */
    static int getPreferredWidth (juce::Typeface::Ptr typeface, const juce::String& text);

    static constexpr int kHeight = 18;

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

    /** Update accent colour dynamically (e.g. tap-color-aware ON button). */
    void setAccentColour (const juce::Colour& newAccent) { accent = newAccent; repaint(); }

private:
    juce::String label;
    juce::Colour accent;
    juce::Typeface::Ptr typeface;

    static constexpr float kFontSize  = 10.0f;
    static constexpr float kKerning   = 0.08f;
    static constexpr float kDotRadius = 2.5f;
    static constexpr float kLeftPad   = 6.0f;
    static constexpr float kDotGap    = 4.0f;
    static constexpr float kRightPad  = 7.0f;
    static constexpr float kCornerR   = 4.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndicatorToggle)
};

//==============================================================================
/** Centred-text button with no indicator dot — same visual language as IndicatorToggle. */
class StyledButton : public juce::Button
{
public:
    StyledButton (const juce::String& label, const juce::Colour& accentColour,
                  juce::Typeface::Ptr typeface);

    static int getPreferredWidth (juce::Typeface::Ptr typeface, const juce::String& text);

    static constexpr int kHeight = 18;

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

    void setLabel (const juce::String& newLabel) { label = newLabel; repaint(); }
    void setAccentColour (const juce::Colour& newAccent) { accent = newAccent; repaint(); }
    void setAlwaysActive (bool active) { alwaysActive = active; repaint(); }
    void setFontSize (float size) { customFontSize = size; repaint(); }
    void setButtonHeight (int h) { customHeight = h; repaint(); }
    void setIcon (const juce::Path& path, float scale, float fixedHeight = 0.0f)
    {
        iconPath = path;
        iconScale = scale;
        fixedIconHeight = fixedHeight;
        repaint();
    }

    int getEffectiveHeight() const { return customHeight > 0 ? customHeight : kHeight; }
    float getEffectiveFontSize() const { return customFontSize > 0.0f ? customFontSize : kFontSize; }

private:
    juce::String label;
    juce::Colour accent;
    juce::Typeface::Ptr typeface;
    bool alwaysActive = false;
    juce::Path iconPath;
    float iconScale = 0.0f;
    float fixedIconHeight = 0.0f;
    float customFontSize = 0.0f;
    int customHeight = 0;

    static constexpr float kFontSize = 10.0f;
    static constexpr float kKerning  = 0.08f;
    static constexpr float kHPad     = 8.0f;
    static constexpr float kCornerR  = 4.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyledButton)
};

//==============================================================================
// PresetSaveOverlay — native popup window for saving presets
// Uses addToDesktop() to bypass host keyboard interception (Issue #35)
//==============================================================================
class PresetSaveOverlay : public juce::Component
{
public:
    PresetSaveOverlay();

    void show (const juce::String& existingName, juce::Component* parentEditor);
    void dismiss();

    // v1.0: callback with preset name only (always saves to User/)
    std::function<void (const juce::String&)> onSave;

    void paint (juce::Graphics& g) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    void inputAttemptWhenModal() override;  // click outside native window → dismiss

private:
    juce::TextEditor nameEditor;
    juce::TextButton saveBtn { "Save" }, cancelBtn { "Cancel" };

    static constexpr int cardW = 260, cardH = 130;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSaveOverlay)
};

//==============================================================================
// Global Tap Drawer — collapsible left-edge mini-drawer with 6 offset knobs
// Applies additive delta to all enabled taps (IEM MultiEncoder pattern)
//==============================================================================
class GlobalTapDrawerComponent : public juce::Component, private juce::Timer
{
public:
    GlobalTapDrawerComponent (OSDLookAndFeel& lookAndFeel);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

    bool isOpen() const { return open; }
    void setOpen (bool shouldBeOpen, bool animate = false);
    void resetToCenter();

    /** Get current knob value (for OSC sync delta computation). */
    float getKnobValue (int knobIdx) const;
    /** Set knob value without triggering onGlobalDelta callback (for OSC receive sync). */
    void setKnobValueSilent (int knobIdx, float value);

    /** Current animated width — use for layout in the parent editor. */
    int getCurrentWidth() const { return currentWidth; }

    // Callback: (knobIndex, delta) — fired when a global knob is turned
    std::function<void (int knobIndex, float delta)> onGlobalDelta;
    // Callback: fired when drawer toggles open/close (called on each animation frame)
    std::function<void()> onToggle;
    // Callbacks: fired on knob drag start/end — used for gesture wrapping (issue E15)
    std::function<void (int knobIndex)> onDragStarted;
    std::function<void (int knobIndex)> onDragEnded;

    static constexpr int kClosedWidth = 14;
    static constexpr int kOpenWidth   = 78;
    static constexpr int kHandleWidth = 16;
    static constexpr int kPanelWidth  = kOpenWidth - kHandleWidth;  // 62px knob area

    enum KnobID { kAzimuth = 0, kElevation, kDistance, kPitch, kDoppler, kSpeed, kNumKnobs };

private:
    bool open = false;
    bool suppressCallbacks = false;
    bool handleHover = false;
    OSDLookAndFeel& lookAndFeel;

    // Animation state
    int currentWidth = kClosedWidth;
    int targetWidth  = kClosedWidth;
    void timerCallback() override;

    // Knob panel — a child component that holds all knobs.
    // The drawer clips this panel by setting its bounds to only the visible portion,
    // so knobs slide in/out naturally during animation.
    struct KnobPanel : public juce::Component
    {
        void paint (juce::Graphics& g) override;
    } knobPanel;

    ReverseSlider azSlider;  // clockwise knob = clockwise on map (IEM convention)
    juce::Slider elSlider, distSlider, dopplerSlider, pitchSlider, speedSlider;
    juce::Label azLabel, elLabel, distLabel, dopplerLabel, pitchLabel, speedLabel;
    float prevValues[kNumKnobs] = {};

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                    float min, float max, float step, int knobIdx);
    void layoutKnobs();

    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalTapDrawerComponent)
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

    /** Sync all UI state from processor parameters (for screenshot tool).
        Normally the 30Hz timer handles this, but headless capture needs
        a single manual call before rendering. */
    void syncForScreenshot();

    /** Open/close global drawer and set knob values (for screenshot tool). */
    void configureGlobalDrawer (bool open, const float* knobValues = nullptr, int numKnobs = 0);

private:
    std::unique_ptr<StyledButton> smlButton;  // header branding link
    bool modIsActive = false;     // v0.8: MOD section enable state
    // Layout constants (v0.7: updated bottom panel height)
    static constexpr int kWindowWidth      = 820;
    static constexpr int kWindowHeight     = 580;
    static constexpr int kHeaderHeight     = 52;
    static constexpr int kRightPanelWidth  = 264;
    static constexpr int kBottomPanelHeight = 120;
    static constexpr int kPadding          = 8;

    void timerCallback() override;
    void visibilityChanged() override;

    // Helper for drawing Corner selection box
    void drawSelectionBox (juce::Graphics& g, juce::Component& label, juce::Component& slider);
    // Helper for drawing section headers
    void drawSectionHeader (juce::Graphics& g, int x, int y, int w, const juce::String& text);
    // SpatialMapComponent::Listener
    void objectPositionChanged (int objectIndex, float azimuthDeg, float distance) override;
    void objectSelected (int objectIndex) override;

    OpenSpatialDelayProcessor& processorRef;

    juce::SharedResourcePointer<OSDLookAndFeel> osdLookAndFeel;

    // UI components
    SpatialMapComponent spatialMap;
    GlobalTapDrawerComponent globalTapDrawer;  // v1.0: collapsible global offset knobs
    void applyGlobalTapDelta (int knobIndex, float delta);  // v1.0: IEM-style delta application
    void resetGlobalTapAPVTSParams();                       // issue #95: zero APVTS + atomics on preset change
    void syncGlobalTapOffsetsFromOSC();                     // v1.0: processor → editor OSC sync
    void syncGlobalTapKnobsFromAPVTS();                    // issue #164: sync knobs from APVTS on undo

    // Global controls — ordered by signal flow
    juce::Slider inputGainSlider, outputGainSlider;
    juce::Slider delayTimeSlider, noteDivisionSlider, feedbackSlider;
    juce::Slider filterHPSlider, filterLPSlider;
    juce::Slider dryWetSlider;
    juce::ComboBox algorithmBox, hrtfProfileBox, syncModeBox;
    juce::ComboBox outputFormatBox;
    juce::Label outputFormatLabel;
    std::unique_ptr<StyledButton> tempoSyncButton;
    std::unique_ptr<StyledButton> syncDottedButton, syncTripletButton;  // v0.7: note modifier toggles

    // Object controls
    std::array<juce::TextButton, OpenSpatialDelayProcessor::MAX_OBJECTS> objectButtons;
    ReverseSlider objAzimuthSlider;
    juce::Slider objElevationSlider, objDistanceSlider;
    std::unique_ptr<IndicatorToggle> objEnabledButton;  // tap ON/OFF — accent matches tap color
    int currentObjectIndex = 0;

    // v0.4: Per-object Doppler amount knob
    juce::Slider objDopplerSlider;
    juce::Label objDopplerLabel;

    // v0.7: Per-object pitch shift knob (bottom panel)
    juce::Slider objPitchShiftSlider;
    juce::Label objPitchShiftLabel;

    // v0.8: Per-object input channel cycling button (bottom panel)
    std::unique_ptr<StyledButton> objInputChannelButton;
    juce::ComboBox   objInputChannelBox;    // hidden, for APVTS binding

    // v0.6: Per-object trajectory controls (bottom panel)
    juce::ComboBox objTrajectoryBox;
    juce::Label objTrajectoryLabel;
    std::unique_ptr<StyledButton> objTrajectoryFwdButton, objTrajectoryRevButton;  // v0.8: direction arrows
    juce::ComboBox objTrajectoryDirBox;  // v0.8: hidden, for APVTS binding
    juce::Slider objTrajectorySpeedSlider;
    juce::Label objTrajectorySpeedLabel;

    // v0.6: Preset browser (header bar) — v0.9: PopupMenu replaces ComboBox
    juce::TextButton presetNameButton;  // shows current preset name, click opens popup
    juce::Label presetLabel;
    juce::TextButton presetPrevButton, presetNextButton, presetSaveButton;
    PresetSaveOverlay presetSaveOverlay;
    void showPresetMenu();
    void updatePresetButtonText();

    // v0.7: Input format dropdown (header bar)
    juce::ComboBox inputFormatBox;
    juce::Label inputFormatLabel;

    // v0.7: Filter graph component (replaces HP/LP knobs in TONE section)
    FilterGraphComponent filterGraph;

    // v0.6: ADM-OSC Receive toggle + editable port label
    std::unique_ptr<IndicatorToggle> oscToggleButton;
    juce::Label oscPortLabel;

    // v0.7: ADM-OSC Send toggle + IP/port fields
    std::unique_ptr<IndicatorToggle> oscSendToggleButton;
    juce::Label oscSendIPLabel;
    juce::Label oscSendPortLabel;

    // v0.8: Wobble modulation knobs (right panel, MOD section)
    juce::Slider wobbleAmountSlider, wobbleMorphSlider;
    juce::Label wobbleAmountLabel, wobbleMorphLabel;
    int modHeaderY = 0;  // section header Y position

    // v0.4: Global air absorption toggle
    std::unique_ptr<IndicatorToggle> airAbsorptionButton;

    // v0.8: IndicatorToggle instances for FLT and MOD section headers
    std::unique_ptr<IndicatorToggle> fltToggle;
    std::unique_ptr<IndicatorToggle> modToggle;

    // Labels
    juce::Label inputGainLabel, outputGainLabel;
    juce::Label delayTimeLabel, feedbackLabel;
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
    std::unique_ptr<SliderAttachment> filterHPAttach, filterLPAttach;
    std::unique_ptr<SliderAttachment> dryWetAttach;
    std::unique_ptr<ComboBoxAttachment> syncModeAttach;
    int lastAlgoCategoryShown = -1;  // Track format category to avoid redundant combo rebuilds
    int lastMaxBusChannels    = -1;  // v0.7: Gate format availability re-check
    int lastOscPort           = -1;  // v0.7: Gate OSC port label sync
    std::unique_ptr<ButtonAttachment> tempoSyncAttach;

    // v0.4: Global DSP attachments
    std::unique_ptr<ButtonAttachment> airAbsorptionAttach;

    // v0.8: Wobble modulation attachments
    std::unique_ptr<SliderAttachment> wobbleAmountAttach, wobbleMorphAttach;

    // Per-object attachments (for the currently selected object)
    std::unique_ptr<SliderAttachment> objAzAttach, objElAttach, objDistAttach;
    std::unique_ptr<ButtonAttachment> objEnabledAttach;
    std::unique_ptr<SliderAttachment> objDopplerAttach;  // v0.4: per-object Doppler amount

    // v0.7: Per-object pitch shift attachment (rebound in selectObject)
    std::unique_ptr<SliderAttachment> objPitchShiftAttach;

    // v0.7: Input format — no attachment, uses onChange callback (issue #68)

    // v0.8: Per-object input channel attachment (rebound in selectObject)
    std::unique_ptr<ComboBoxAttachment> objInputChannelAttach;

    // v0.6: Per-object trajectory attachments (rebound in selectObject)
    std::unique_ptr<ComboBoxAttachment> objTrajectoryAttach;
    std::unique_ptr<SliderAttachment>   objTrajectorySpeedAttach;
    std::unique_ptr<ComboBoxAttachment> objTrajectoryDirAttach;  // v0.8

    // v0.6: ADM-OSC toggle attachment
    std::unique_ptr<ButtonAttachment> oscToggleAttach;

    void selectObject (int index);
    void updateObjectButtonColours();
    void updateMapFromParameters();

    // v0.7: Smoothed per-tap activity for glow animation
    float smoothedActivity[OpenSpatialDelayProcessor::MAX_OBJECTS] = {};

    // Section header positions (computed in resized, drawn in paint)
    int rpX = 0, rpW = 0;
    int delayHeaderY = 0, toneHeaderY = 0, mixHeaderY = 0, oscHeaderY = 0;
    int objectHeaderY = 0;

    // v0.9: Filter active state — driven by filterEnabled parameter
    bool filterIsActive = false;  // default OFF (matches filterEnabled param default)

    // Issue E15: Gesture tracking for undo grouping
    std::vector<juce::RangedAudioParameter*> activeGlobalGestureParams;   // Global Tap Drawer drag
    std::array<juce::RangedAudioParameter*, 2> activeSpatialGestureParams { nullptr, nullptr }; // Spatial Map drag
    std::vector<juce::RangedAudioParameter*> activeFilterGestureParams;   // Filter Graph drag

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayEditor)
};
