#include "PluginEditor.h"
#include "FontData.h"

//==============================================================================
// Object colours for the spatial map
//==============================================================================
const juce::Colour SpatialMapComponent::objectColours[SpatialMapComponent::MAX_OBJECTS] = {
    juce::Colour (0xffED5E5E),  //  1 red        — hsl(0, 80%, 65%)
    juce::Colour (0xffEDA65E),  //  2 orange     — hsl(30, 80%, 65%)
    juce::Colour (0xffACD435),  //  3 lime       — hsl(75, 65%, 52%)
    juce::Colour (0xff3BCE6C),  //  4 green      — hsl(140, 60%, 52%)
    juce::Colour (0xff3CDDA7),  //  5 teal       — hsl(160, 70%, 55%)
    juce::Colour (0xff52E0E0),  //  6 cyan       — hsl(180, 70%, 60%)
    juce::Colour (0xff4DB3E6),  //  7 light blue — hsl(200, 75%, 60%)
    juce::Colour (0xff6390E9),  //  8 blue       — hsl(220, 75%, 65%)
    juce::Colour (0xff6767E4),  //  9 indigo     — hsl(240, 70%, 65%)
    juce::Colour (0xffA667E4),  // 10 violet     — hsl(270, 70%, 65%)
    juce::Colour (0xffE467E4),  // 11 magenta    — hsl(300, 70%, 65%)
    juce::Colour (0xffE963A6),  // 12 rose       — hsl(330, 75%, 65%)
};

//==============================================================================
// Colour constants
//==============================================================================
namespace Colours_OSD
{
    // === Backgrounds (OKLCH hue 260 = blue-tinted darks) ===
    // Hex values from browser canvas rendering of oklch() CSS values
    static const juce::Colour bgVoid       (0xff03060b);  // oklch(12% 0.015 260) — main bg
    static const juce::Colour bgPanel      (0xff0a0d12);  // oklch(16% 0.012 260) — right/bottom panels
    static const juce::Colour bgHeader     (0xff06090f);  // oklch(14% 0.014 260) — header bar
    static const juce::Colour bgRecessed   (0xff010205);  // oklch(9% 0.012 260) — inputs, dropdowns, knob bg
    static const juce::Colour bgWell       (0xff020307);  // oklch(10% 0.013 260) — filter graph bg

    // === Borders ===
    static const juce::Colour borderSubtle (0xff252930);  // oklch(28% 0.015 260) — primary borders
    static const juce::Colour borderDim    (0xff171b20);  // oklch(22% 0.012 260) — dim borders, panel edges

    // === Accent Colors (browser canvas-verified OKLCH→sRGB) ===
    static const juce::Colour accentStellar    (0xff80d8ff);  // oklch(85% 0.12 240) — delay section
    static const juce::Colour accentStellarDim (0xff5187ab);  // oklch(60% 0.08 240)
    static const juce::Colour accentViolet     (0xff7457d1);  // oklch(55% 0.18 290) — tone section
    static const juce::Colour accentVioletDim  (0xff493883);  // oklch(40% 0.12 290)
    static const juce::Colour accentAmber      (0xfff0a646);  // oklch(78% 0.14 70) — mix section
    static const juce::Colour accentAmberDim   (0xff936831);  // oklch(55% 0.09 70)
    static const juce::Colour accentGreen      (0xff3bce6c);  // oklch(75% 0.16 155) — OSC section
    static const juce::Colour accentGreenDim   (0xff277a42);  // oklch(50% 0.10 155)
    static const juce::Colour accentRose       (0xffe467a6);  // oklch(70% 0.14 350) — mod section
    static const juce::Colour accentSync       (0xffe1c34b);  // oklch(82% 0.14 95)  — sync buttons (gold/yellow)
    static const juce::Colour accentSyncDim    (0xff8b7a36);  // oklch(58% 0.09 95)
    static const juce::Colour accentChannelL   (0xff4499ff);  // standard audio blue — Left channel
    static const juce::Colour accentChannelR   (0xffff4444);  // standard audio red — Right channel
    static const juce::Colour accentGlobal     (0xffc8d8e8);  // silver/ice — global tap drawer
    static const juce::Colour accentGlobalDim  (0xff7d8894);  // dimmed variant

    // === Text (browser canvas-verified OKLCH→sRGB) ===
    static const juce::Colour textPrimary   (0xffe1e5ea);  // oklch(92% 0.008 260)
    static const juce::Colour textSecondary (0xff9fa5ae);  // oklch(72% 0.015 260)
    static const juce::Colour textDim       (0xff6d7279);  // oklch(55% 0.012 260) — labels, inactive
    static const juce::Colour textEtched    (0xff5a5e63);  // oklch(48% 0.01 260) — cardinals

    // === Spatial map ===
    static const juce::Colour mapVoid      (0xff010204);  // oklch(8% 0.015 260)
    static const juce::Colour mapRing      (0x28506e8c);  // rgba(80,110,140, 0.16) — semi-transparent with glow
    static const juce::Colour mapCross     (0x0d3c5a78);  // rgba(60,90,120, 0.05)
    static const juce::Colour mapReticle   (0x2e8caac8);  // rgba(140,170,200, 0.18)

    // === Backward-compat aliases (used in a few spots) ===
    static const juce::Colour selectionBox (0x40ffffff);
}

// OSC port validation bounds
static constexpr int kMinPort = 1024;
static constexpr int kMaxPort = 65535;

// Helper: create a Font from a specific Typeface with height and optional kerning
static juce::Font makeFont (juce::Typeface::Ptr tf, float height, float kerning = 0.0f)
{
    return juce::Font (juce::FontOptions (tf).withHeight (height).withKerningFactor (kerning));
}

// Helper: create SML molecular icon path at 250×250 SVG scale
static juce::Path createSMLIconPath()
{
    juce::Path p;
    // Lines (connecting stems) — drawn first so circles paint over endpoints
    p.addLineSegment (juce::Line<float> (61.364f, 149.021f, 86.020f, 145.051f), 8.0f);
    p.addLineSegment (juce::Line<float> (62.664f,  82.016f, 95.593f, 111.421f), 8.0f);
    p.addLineSegment (juce::Line<float> (127.0f,    63.0f,  127.0f,  107.146f), 8.0f);
    p.addLineSegment (juce::Line<float> (114.861f, 166.925f, 100.761f, 208.759f), 8.0f);
    p.addLineSegment (juce::Line<float> (200.387f, 129.244f, 156.488f, 133.905f), 8.0f);
    p.addLineSegment (juce::Line<float> (173.532f,  83.158f, 144.538f, 116.449f), 8.0f);
    p.addLineSegment (juce::Line<float> (143.990f, 159.343f, 173.316f, 192.341f), 8.0f);
    // Circles (nodes) — central hub + 7 satellites
    auto addCircle = [&] (float cx, float cy, float r) { p.addEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f); };
    addCircle (124.5f, 137.865f, 43.703f);   // central hub
    addCircle (216.5f, 126.5f,   23.5f);     // right
    addCircle (127.0f,  41.5f,   27.0f);     // top (rx≈ry≈27 in SVG)
    addCircle ( 35.5f, 151.5f,   26.5f);     // left
    addCircle (181.5f, 196.5f,   23.5f);     // bottom-right
    addCircle ( 98.5f, 213.5f,   20.5f);     // bottom
    addCircle ( 55.0f,  75.0f,   16.0f);     // top-left (small)
    addCircle (176.0f,  80.0f,   12.0f);     // top-right (small)
    return p;
}

// Helper: create dotted eighth note icon path (viewBox 0 0 32 36)
static juce::Path createDottedNoteIconPath()
{
    juce::Path p;
    // ViewBox anchors — extend bounding box to match SVG viewBox for correct scaling
    p.startNewSubPath (0.0f, 0.0f);
    p.startNewSubPath (32.0f, 36.0f);
    // Flag (cubic bezier)
    p.startNewSubPath (16.5f, 6.0f);
    p.cubicTo (16.5f, 6.0f, 20.0f, 9.0f, 20.0f, 13.5f);
    p.cubicTo (20.0f, 16.0f, 17.5f, 17.0f, 16.5f, 15.0f);
    p.lineTo (16.5f, 6.0f);
    p.closeSubPath();
    // Stem
    p.addRectangle (15.5f, 6.0f, 1.2f, 20.0f);
    // Note head (rotated ellipse)
    juce::Path head;
    head.addEllipse (11.0f - 5.0f, 26.0f - 3.5f, 10.0f, 7.0f);
    p.addPath (head, juce::AffineTransform::rotation (juce::degreesToRadians (-20.0f), 11.0f, 26.0f));
    // Augmentation dot
    p.addEllipse (24.5f - 2.5f, 26.0f - 2.5f, 5.0f, 5.0f);
    return p;
}

// Helper: create triplet beamed eighth notes icon path (viewBox 0 0 30 34)
static juce::Path createTripletNoteIconPath()
{
    juce::Path p;
    // ViewBox anchors — extend bounding box to match SVG viewBox for correct scaling
    p.startNewSubPath (0.0f, 0.0f);
    p.startNewSubPath (30.0f, 34.0f);
    // Stems
    p.addRectangle (7.0f, 5.0f, 1.1f, 18.0f);
    p.addRectangle (17.0f, 5.0f, 1.1f, 18.0f);
    p.addRectangle (27.0f, 5.0f, 1.1f, 18.0f);
    // Beam
    p.addRoundedRectangle (7.0f, 4.0f, 21.1f, 2.2f, 0.4f);
    // Note heads (rotated ellipses)
    auto addRotatedHead = [&] (float cx, float cy)
    {
        juce::Path head;
        head.addEllipse (cx - 4.2f, cy - 3.0f, 8.4f, 6.0f);
        p.addPath (head, juce::AffineTransform::rotation (juce::degreesToRadians (-20.0f), cx, cy));
    };
    addRotatedHead (5.0f, 23.5f);
    addRotatedHead (15.0f, 23.5f);
    addRotatedHead (25.0f, 23.5f);
    return p;
}

//==============================================================================
// IndicatorToggle — reusable toggle pill with indicator dot
//==============================================================================
IndicatorToggle::IndicatorToggle (const juce::String& lbl, const juce::Colour& col,
                                   juce::Typeface::Ptr tf)
    : juce::Button (lbl), label (lbl), accent (col), typeface (tf)
{
    setClickingTogglesState (true);
}

int IndicatorToggle::getPreferredWidth (juce::Typeface::Ptr tf, const juce::String& text)
{
    auto font = makeFont (tf, kFontSize, kKerning);
    juce::GlyphArrangement gl;
    gl.addLineOfText (font, text, 0.0f, 0.0f);
    return (int) kLeftPad + (int) (kDotRadius * 2.0f) + (int) kDotGap
         + juce::roundToInt (gl.getBoundingBox (0, gl.getNumGlyphs(), true).getWidth())
         + (int) kRightPad;
}

// Shared background + border painting for toggle-style buttons
static void paintToggleButtonBg (juce::Graphics& g, juce::Rectangle<float> bounds,
                                  const juce::Colour& accent, bool isOn, bool isHover, float cornerR)
{
    float bgAlpha = isOn ? 0.08f : 0.0f;
    if (isHover) bgAlpha += 0.06f;
    g.setColour (isOn ? accent.withAlpha (bgAlpha)
                      : (isHover ? accent.withAlpha (0.04f) : Colours_OSD::bgRecessed));
    g.fillRoundedRectangle (bounds, cornerR);

    float borderAlpha = isOn ? 0.6f : 0.0f;
    if (isHover && !isOn)  borderAlpha = 0.3f;
    else if (isHover && isOn) borderAlpha = 0.85f;
    g.setColour (isOn || isHover ? accent.withAlpha (borderAlpha) : Colours_OSD::borderSubtle);
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerR, 1.0f);
}

void IndicatorToggle::paintButton (juce::Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() <= 0.0f) return;

    bool isOn = getToggleState();
    paintToggleButtonBg (g, bounds, accent, isOn, isMouseOverButton, kCornerR);

    // Indicator dot
    float dotX = bounds.getX() + kLeftPad + kDotRadius;
    float dotY = bounds.getCentreY();
    g.setColour (isOn ? accent
                      : (isMouseOverButton ? Colours_OSD::textDim.withAlpha (0.6f)
                                           : Colours_OSD::textDim.withAlpha (0.4f)));
    g.fillEllipse (dotX - kDotRadius, dotY - kDotRadius, kDotRadius * 2.0f, kDotRadius * 2.0f);
    if (isOn)
    {
        g.setColour (accent.withAlpha (0.25f));
        g.fillEllipse (dotX - kDotRadius - 2.0f, dotY - kDotRadius - 2.0f,
                       (kDotRadius + 2.0f) * 2.0f, (kDotRadius + 2.0f) * 2.0f);
    }

    // Text label
    g.setColour (isOn ? accent
                      : (isMouseOverButton ? Colours_OSD::textSecondary : Colours_OSD::textDim));
    g.setFont (makeFont (typeface, kFontSize, kKerning));
    auto textBounds = bounds.withLeft (dotX + kDotRadius + kDotGap);
    g.drawText (label, textBounds.toNearestInt(), juce::Justification::centredLeft);
}

//==============================================================================
// StyledButton — centred-text button, same visual language as IndicatorToggle
//==============================================================================
StyledButton::StyledButton (const juce::String& lbl, const juce::Colour& col,
                            juce::Typeface::Ptr tf)
    : juce::Button (lbl), label (lbl), accent (col), typeface (tf)
{
    setClickingTogglesState (true);
}

int StyledButton::getPreferredWidth (juce::Typeface::Ptr tf, const juce::String& text)
{
    auto font = makeFont (tf, kFontSize, kKerning);
    juce::GlyphArrangement gl;
    gl.addLineOfText (font, text, 0.0f, 0.0f);
    return (int) kHPad
         + juce::roundToInt (gl.getBoundingBox (0, gl.getNumGlyphs(), true).getWidth())
         + (int) kHPad;
}

void StyledButton::paintButton (juce::Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() <= 0.0f) return;

    bool isOn = alwaysActive || getToggleState();

    paintToggleButtonBg (g, bounds, accent, isOn, isMouseOverButton, kCornerR);

    // Text (+ optional icon) — centred
    float fontSize = getEffectiveFontSize();
    auto textCol = isOn ? accent
                        : (isMouseOverButton ? Colours_OSD::textSecondary : Colours_OSD::textDim);
    g.setColour (textCol);
    g.setFont (makeFont (typeface, fontSize, kKerning));

    if (iconScale > 0.0f)
    {
        auto pathBounds = iconPath.getBounds();
        float iconH = (fixedIconHeight > 0.0f) ? fixedIconHeight : (bounds.getHeight() - 6.0f);
        float s = iconH / pathBounds.getHeight();
        float iconW = pathBounds.getWidth() * s;

        if (label.isEmpty())
        {
            // Icon-only: centre the icon
            float startX = bounds.getCentreX() - iconW * 0.5f;
            g.fillPath (iconPath,
                        juce::AffineTransform::translation (-pathBounds.getX(), -pathBounds.getY())
                            .scaled (s)
                            .translated (startX, bounds.getCentreY() - iconH * 0.5f));
        }
        else
        {
            // Icon + text: compute combined width, centre both
            constexpr float gap = 3.0f;
            juce::GlyphArrangement gl;
            auto font = makeFont (typeface, fontSize, kKerning);
            gl.addLineOfText (font, label, 0.0f, 0.0f);
            float textW = gl.getBoundingBox (0, gl.getNumGlyphs(), true).getWidth();
            float totalW = iconW + gap + textW;
            float startX = bounds.getCentreX() - totalW * 0.5f;

            g.fillPath (iconPath,
                        juce::AffineTransform::translation (-pathBounds.getX(), -pathBounds.getY())
                            .scaled (s)
                            .translated (startX, bounds.getCentreY() - iconH * 0.5f));

            auto textRect = juce::Rectangle<float> (startX + iconW + gap, bounds.getY(),
                                                     textW + 2.0f, bounds.getHeight());
            g.drawText (label, textRect.toNearestInt(), juce::Justification::centredLeft);
        }
    }
    else
    {
        g.drawText (label, bounds.toNearestInt(), juce::Justification::centred);
    }
}

//==============================================================================
// PresetSaveOverlay — native popup window for saving presets
// Uses addToDesktop() to bypass host keyboard interception (Issue #35)
//==============================================================================
PresetSaveOverlay::PresetSaveOverlay()
{
    setWantsKeyboardFocus (true);
    // v1.0: Do NOT mark opaque — rounded rectangle leaves corner pixels unpainted.
    // Opaque flag with unpainted regions causes corrupted CoreAnimation backing store
    // → Metal GPU crash (EXC_BAD_ACCESS in AGXMetalG16X). See issue #37.

    // Name editor — DM Sans Regular 13px, recessed bg, cyan focus outline
    nameEditor.setMultiLine (false);
    nameEditor.setReturnKeyStartsNewLine (false);
    nameEditor.setColour (juce::TextEditor::backgroundColourId, Colours_OSD::bgRecessed);
    nameEditor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    nameEditor.setColour (juce::TextEditor::outlineColourId, Colours_OSD::borderDim);
    nameEditor.setColour (juce::TextEditor::focusedOutlineColourId, Colours_OSD::accentStellar);
    nameEditor.setColour (juce::TextEditor::highlightColourId, Colours_OSD::accentStellar.withAlpha (0.35f));
    nameEditor.setColour (juce::TextEditor::highlightedTextColourId, juce::Colours::white);
    nameEditor.setJustification (juce::Justification::centred);
    nameEditor.setTextToShowWhenEmpty ("Enter preset name...", Colours_OSD::textDim);
    nameEditor.onReturnKey = [this]
    {
        auto name = nameEditor.getText().trim();
        if (name.isNotEmpty() && onSave)
        {
            onSave (name);
            dismiss();
        }
    };
    addAndMakeVisible (nameEditor);

    // Save button — filled cyan, dark text (primary action)
    saveBtn.setColour (juce::TextButton::buttonColourId, Colours_OSD::accentStellar);
    saveBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff0a0d12));
    saveBtn.onClick = [this]
    {
        auto name = nameEditor.getText().trim();
        if (name.isNotEmpty() && onSave)
        {
            onSave (name);
            dismiss();
        }
    };
    addAndMakeVisible (saveBtn);

    // Cancel button — unfilled, dark red-tinted bg, red text (destructive/dismiss action)
    cancelBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff120508));
    cancelBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffed5e5e));
    cancelBtn.onClick = [this] { dismiss(); };
    addAndMakeVisible (cancelBtn);
}

void PresetSaveOverlay::show (const juce::String& existingName, juce::Component* parentEditor)
{
    nameEditor.setText (existingName, false);

    // Inherit LookAndFeel from parent editor for consistent styling
    if (parentEditor != nullptr)
        setLookAndFeel (&parentEditor->getLookAndFeel());

    // Set bounds BEFORE addToDesktop — ensures valid peer creation (Issue #35 fix)
    setSize (cardW, cardH);
    if (parentEditor != nullptr)
    {
        auto editorBounds = parentEditor->getScreenBounds();
        int x = editorBounds.getX() + (editorBounds.getWidth()  - cardW) / 2;
        int y = editorBounds.getY() + (editorBounds.getHeight() - cardH) / 2;
        setTopLeftPosition (x, y);
    }

    // Create native OS window — bypasses host keyboard interception
    addToDesktop (juce::ComponentPeer::windowIsTemporary
                | juce::ComponentPeer::windowHasDropShadow);

    setAlwaysOnTop (true);  // prevent z-order issues in some hosts (Issue #35 fix)
    setVisible (true);
    toFront (true);
    enterModalState (true);  // non-blocking modal; inputAttemptWhenModal() fires on outside clicks
    nameEditor.grabKeyboardFocus();
    nameEditor.setHighlightedRegion ({ 0, nameEditor.getText().length() });
}

void PresetSaveOverlay::showForSnapshot (const juce::String& existingName,
                                         juce::Component* parentEditor)
{
    nameEditor.setText (existingName, false);

    if (parentEditor != nullptr)
        setLookAndFeel (&parentEditor->getLookAndFeel());

    setSize (cardW, cardH);

    if (parentEditor != nullptr)
    {
        setTopLeftPosition ((parentEditor->getWidth()  - cardW) / 2,
                            (parentEditor->getHeight() - cardH) / 2);
        parentEditor->addAndMakeVisible (this);
    }

    setVisible (true);
}

void PresetSaveOverlay::dismiss()
{
    if (isCurrentlyModal())
        exitModalState (0);

    if (isOnDesktop())
        removeFromDesktop();

    nameEditor.clear();
}

void PresetSaveOverlay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Fill entire bounds first for CoreAnimation safety (issue #37)
    g.fillAll (Colours_OSD::bgPanel);

    // Card background (rounded corners painted over the fill)
    g.setColour (Colours_OSD::bgPanel);
    g.fillRoundedRectangle (bounds, 6.0f);

    // Card border
    g.setColour (Colours_OSD::borderSubtle);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    // Title — SAVE PRESET
    g.setColour (Colours_OSD::textDim);
    auto titleFont = juce::Font (juce::FontOptions (13.0f).withStyle ("Bold"));
    g.setFont (titleFont);
    auto titleArea = bounds.withHeight (28.0f).translated (0.0f, 10.0f);
    g.drawText ("SAVE PRESET", titleArea.toNearestInt(), juce::Justification::centred);
}

void PresetSaveOverlay::resized()
{
    int pad = 24;

    // Name editor
    nameEditor.setBounds (pad, 42, getWidth() - pad * 2, 26);

    // Buttons
    int btnW = 76, btnH = 26, btnGap = 10;
    int totalBtnW = btnW * 2 + btnGap;
    int btnX = (getWidth() - totalBtnW) / 2;
    int btnY = getHeight() - btnH - 14;
    saveBtn.setBounds (btnX, btnY, btnW, btnH);
    cancelBtn.setBounds (btnX + btnW + btnGap, btnY, btnW, btnH);
}

bool PresetSaveOverlay::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        dismiss();
        return true;
    }
    return false;
}

void PresetSaveOverlay::inputAttemptWhenModal()
{
    // Click outside the native window → dismiss without saving
    dismiss();
}

//==============================================================================
// OSDLookAndFeel — Observatory v6 LookAndFeel implementation
//==============================================================================
OSDLookAndFeel::OSDLookAndFeel()
{
    // Load embedded fonts
    dmSansRegular    = juce::Typeface::createSystemTypefaceFor (FontData::DM_SansRegular_ttf,    FontData::DM_SansRegular_ttfSize);
    dmSansMedium     = juce::Typeface::createSystemTypefaceFor (FontData::DM_SansMedium_ttf,     FontData::DM_SansMedium_ttfSize);
    dmSansBold       = juce::Typeface::createSystemTypefaceFor (FontData::DM_SansBold_ttf,       FontData::DM_SansBold_ttfSize);
    jetbrainsRegular = juce::Typeface::createSystemTypefaceFor (FontData::JetBrains_MonoRegular_ttf, FontData::JetBrains_MonoRegular_ttfSize);
    jetbrainsMedium  = juce::Typeface::createSystemTypefaceFor (FontData::JetBrains_MonoMedium_ttf,  FontData::JetBrains_MonoMedium_ttfSize);
    jetbrainsBold    = juce::Typeface::createSystemTypefaceFor (FontData::JetBrains_MonoBold_ttf,    FontData::JetBrains_MonoBold_ttfSize);
    robotoMedium     = juce::Typeface::createSystemTypefaceFor (FontData::RobotoMedium_ttf,          FontData::RobotoMedium_ttfSize);

    // Popup menu colors — match ComboBox for visual consistency
    setColour (juce::PopupMenu::backgroundColourId,            juce::Colour (0xff010205));
    setColour (juce::PopupMenu::textColourId,                  juce::Colour (0xff9fa2b0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff7cc8f0));
    setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::black);
    setColour (juce::ComboBox::backgroundColourId,             juce::Colour (0xff010205));
    setColour (juce::ComboBox::textColourId,                   juce::Colour (0xff9fa2b0));
    setColour (juce::ComboBox::outlineColourId,                juce::Colour (0xff252930));
    setColour (juce::ComboBox::arrowColourId,                  juce::Colour (0xff6d7080));
}

void OSDLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, const float rotaryStartAngle,
                                       const float rotaryEndAngle, juce::Slider& slider)
{
    // Observatory v6: knobs adapt to available space (42-46px typical)
    float knobDiameter = std::min ((float) std::min (width, height), 46.0f);
    auto knobBounds = juce::Rectangle<float> (x + (width - knobDiameter) * 0.5f,
                                              y + (height - knobDiameter) * 0.5f,
                                              knobDiameter, knobDiameter);

    auto cx = knobBounds.getCentreX();
    auto cy = knobBounds.getCentreY();
    auto arcR = knobDiameter * 0.5f - 5.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = 2.2f;

    // 1. Dark circle background with subtle border
    g.setColour (Colours_OSD::bgRecessed);
    g.fillEllipse (cx - arcR - 1.5f, cy - arcR - 1.5f, (arcR + 1.5f) * 2.0f, (arcR + 1.5f) * 2.0f);
    g.setColour (Colours_OSD::borderDim);
    g.drawEllipse (cx - arcR - 1.5f, cy - arcR - 1.5f, (arcR + 1.5f) * 2.0f, (arcR + 1.5f) * 2.0f, 0.8f);

    // 2. Background track arc
    juce::Path backgroundTrack;
    backgroundTrack.addCentredArc (cx, cy, arcR, arcR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Colours_OSD::borderSubtle);
    g.strokePath (backgroundTrack, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 3. Active value arc with glow
    if (slider.isEnabled())
    {
        auto accentColour = slider.findColour (juce::Slider::thumbColourId);
        if (accentColour == juce::Colours::transparentBlack)
            accentColour = juce::Colour (0xff7cc8f0);

        juce::Path valueArc;
        valueArc.addCentredArc (cx, cy, arcR, arcR, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (accentColour.withAlpha (0.3f));
        g.strokePath (valueArc, juce::PathStrokeType (lineW + 3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (accentColour);
        g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Tick marks at 0%, 50%, 100%
    g.setColour (Colours_OSD::borderSubtle);
    for (float t : { 0.0f, 0.5f, 1.0f })
    {
        float tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        float cosA = std::cos (tickAngle - juce::MathConstants<float>::halfPi);
        float sinA = std::sin (tickAngle - juce::MathConstants<float>::halfPi);
        float inner = arcR + 2.0f;
        float outer = arcR + 5.0f;
        g.drawLine (cx + cosA * inner, cy + sinA * inner,
                    cx + cosA * outer, cy + sinA * outer, 0.7f);
    }

    // 5. Center dot
    g.setColour (juce::Colour (0xff3e4150));
    g.fillEllipse (cx - 1.8f, cy - 1.8f, 3.6f, 3.6f);

    // 6. Indicator needle
    if (slider.isEnabled())
    {
        auto accentColour = slider.findColour (juce::Slider::thumbColourId);
        if (accentColour == juce::Colours::transparentBlack)
            accentColour = juce::Colour (0xff7cc8f0);

        float cosA = std::cos (angle - juce::MathConstants<float>::halfPi);
        float sinA = std::sin (angle - juce::MathConstants<float>::halfPi);
        float innerR = arcR - 4.0f;
        float outerR = arcR + 1.0f;
        g.setColour (accentColour.withAlpha (0.85f));
        g.drawLine (cx + cosA * innerR, cy + sinA * innerR,
                    cx + cosA * outerR, cy + sinA * outerR, 1.3f);
    }
}

void OSDLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& backgroundColour,
                                           bool isMouseOverButton, bool isButtonDown)
{
    if (backgroundColour.isTransparent())
        return;

    // v0.9: Preset button — match drawComboBox exactly for visual consistency
    if (button.getComponentID() == "presetButton")
    {
        auto bounds = juce::Rectangle<float> (0, 0, (float) button.getWidth(), (float) button.getHeight());
        auto baseColour = backgroundColour;
        if (isButtonDown)            baseColour = baseColour.brighter (0.15f);
        else if (isMouseOverButton)  baseColour = baseColour.brighter (0.12f);

        g.setColour (baseColour);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (Colours_OSD::borderSubtle);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

        // Dropdown arrow — same geometry as drawComboBox
        float arrowX = (float) button.getWidth() - 12.0f;
        float arrowY = (float) button.getHeight() * 0.5f;
        juce::Path arrow;
        arrow.addTriangle (arrowX - 3.0f, arrowY - 1.5f,
                           arrowX + 3.0f, arrowY - 1.5f,
                           arrowX,        arrowY + 2.5f);
        g.setColour (Colours_OSD::textDim);
        g.fillPath (arrow);
        return;
    }

    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto baseColour = backgroundColour;
    if (isButtonDown)            baseColour = baseColour.brighter (0.15f);
    else if (isMouseOverButton)  baseColour = baseColour.brighter (0.12f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 4.0f);

    // 1px border stroke — brighter on hover
    float borderBright = isMouseOverButton ? 0.5f : 0.3f;
    auto borderCol = baseColour.brighter (borderBright).withAlpha (isMouseOverButton ? 0.8f : 0.6f);
    g.setColour (borderCol);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void OSDLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                     bool isMouseOverButton, bool isButtonDown)
{
    // Preset button — left-aligned text to match ComboBox label positioning
    if (button.getComponentID() == "presetButton")
    {
        g.setFont (getTextButtonFont (button, button.getHeight()));
        g.setColour (button.findColour (juce::TextButton::textColourOffId));
        // 6px left pad, reserve 20px for dropdown arrow (matches positionComboBoxText)
        g.drawText (button.getButtonText(),
                    6, 0, button.getWidth() - 20, button.getHeight(),
                    juce::Justification::centredLeft, true);
        return;
    }

    // All other TextButtons — default centered rendering
    LookAndFeel_V4::drawButtonText (g, button, isMouseOverButton, isButtonDown);
}

//==============================================================================
// v0.9: Custom PopupMenu rendering — matches ComboBox dropdown appearance
//==============================================================================
void OSDLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
    g.setColour (juce::Colour (0xff252930));  // borderDim
    g.drawRect (0, 0, width, height, 1);
}

void OSDLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted,
                                        bool isTicked, bool hasSubMenu,
                                        const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                        const juce::Drawable* /*icon*/, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto sepArea = area.reduced (8, 0);
        g.setColour (juce::Colour (0xff252930));
        g.fillRect (sepArea.getX(), area.getCentreY(), sepArea.getWidth(), 1);
        return;
    }

    auto textBounds = area.reduced (8, 0);

    if (isHighlighted && isActive)
    {
        g.setColour (findColour (juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRect (area);
        g.setColour (findColour (juce::PopupMenu::highlightedTextColourId));
    }
    else
    {
        g.setColour (textColour != nullptr ? *textColour
                     : (isActive ? findColour (juce::PopupMenu::textColourId)
                                 : findColour (juce::PopupMenu::textColourId).withAlpha (0.4f)));
    }

    g.setFont (getPopupMenuFont());

    // Tick mark for current preset
    if (isTicked)
    {
        auto tickBounds = area.withWidth (20);
        g.drawText (juce::String::charToString (0x2713), tickBounds,
                    juce::Justification::centred);
        textBounds = textBounds.withTrimmedLeft (14);
    }

    g.drawFittedText (text, textBounds, juce::Justification::centredLeft, 1);

    // Submenu arrow — 6×4px triangle matching ComboBox dropdown arrow size
    if (hasSubMenu)
    {
        float arrowX = (float) (area.getRight() - 12);
        float arrowY = (float) area.getCentreY();
        juce::Path arrow;
        arrow.addTriangle (arrowX - 1.5f, arrowY - 3.0f,
                           arrowX - 1.5f, arrowY + 3.0f,
                           arrowX + 2.5f, arrowY);
        g.fillPath (arrow);
    }
}

void OSDLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                                int /*standardMenuItemHeight*/,
                                                int& idealWidth, int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = 8;
        return;
    }

    auto font = getPopupMenuFont();
    juce::GlyphArrangement ga;
    ga.addLineOfText (font, text, 0.0f, 0.0f);
    idealWidth = static_cast<int> (std::ceil (ga.getBoundingBox (0, -1, true).getWidth())) + 32;
    idealHeight = 24;  // match ComboBox item height
}

void OSDLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                   int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                   juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height);

    // Recessed background
    g.setColour (findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);

    // Subtle border
    g.setColour (findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    // Small dropdown arrow — hidden when combo is disabled (e.g. Ambisonics mode)
    if (box.isEnabled())
    {
        float arrowX = (float) width - 12.0f;
        float arrowY = (float) height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle (arrowX - 3.0f, arrowY - 1.5f,
                           arrowX + 3.0f, arrowY - 1.5f,
                           arrowX,        arrowY + 2.5f);
        g.setColour (findColour (juce::ComboBox::arrowColourId));
        g.fillPath (arrow);
    }
}

void OSDLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    // Reserve 20px for arrow when enabled; use full width when disabled (no arrow)
    if (box.isEnabled())
        label.setBounds (1, 1, box.getWidth() - 20, box.getHeight() - 2);
    else
        label.setBounds (0, 1, box.getWidth(), box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void OSDLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    // Override slider text-box labels to use JetBrains Mono 12px
    // Only match labels whose parent is a Slider (the value readout text box)
    if (dynamic_cast<juce::Slider*> (label.getParentComponent()) != nullptr
        && jetbrainsRegular)
    {
        g.setFont (juce::Font (juce::FontOptions (jetbrainsRegular).withHeight (12.0f)));
        g.setColour (label.findColour (juce::Label::textColourId));
        g.drawText (label.getText(), label.getLocalBounds(),
                    label.getJustificationType(), true);
        return;
    }

    // v0.9: Custom label rendering with sharp-rect outlines (matches editing state shape)
    auto bounds = label.getLocalBounds().toFloat();
    auto bg = label.findColour (juce::Label::backgroundColourId);
    if (! bg.isTransparent())
    {
        g.setColour (bg);
        g.fillRect (bounds);
    }
    auto outline = label.findColour (juce::Label::outlineColourId);
    if (! outline.isTransparent())
    {
        g.setColour (outline);
        g.drawRect (bounds.reduced (0.5f), 1.0f);
    }
    if (! label.isBeingEdited())
    {
        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        g.setColour (label.findColour (juce::Label::textColourId));
        g.setFont (label.getFont());
        g.drawFittedText (label.getText(), textArea,
                          label.getJustificationType(),
                          juce::jmax (1, (int) ((float) textArea.getHeight() / label.getFont().getHeight())),
                          label.getMinimumHorizontalScale());
    }
}

// v0.9: Thin 1px outline for text editors — replaces JUCE's thick default
// Uses focusedOutlineColourId when editor has focus (Label propagates outlineWhenEditingColourId here)
void OSDLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int w, int h,
                                            juce::TextEditor& editor)
{
    auto colour = editor.hasKeyboardFocus (true)
                    ? editor.findColour (juce::TextEditor::focusedOutlineColourId)
                    : editor.findColour (juce::TextEditor::outlineColourId);
    if (colour.isTransparent())
        return;
    g.setColour (colour);

    // Knob text editors (parent chain: TextEditor → Label → Slider) get rounded rect;
    // OSC input fields and others get sharp rect
    auto* parent = editor.getParentComponent();
    bool isKnobEditor = parent != nullptr
                        && dynamic_cast<juce::Slider*> (parent->getParentComponent()) != nullptr;

    if (isKnobEditor)
        g.drawRoundedRectangle (0.5f, 0.5f, static_cast<float> (w) - 1.0f,
                                static_cast<float> (h) - 1.0f, 3.0f, 1.0f);
    else
        g.drawRect (0.5f, 0.5f, static_cast<float> (w) - 1.0f,
                    static_cast<float> (h) - 1.0f, 1.0f);
}

//==============================================================================
// SpatialMapComponent
//==============================================================================
SpatialMapComponent::SpatialMapComponent() { generateStars(); }

void SpatialMapComponent::generateStars()
{
    // ~80 stars with deterministic LCG random positions (increased from 50 for visibility)
    stars.clear();
    uint32_t seed = 0x5ACEA10u;  // deterministic seed
    auto lcg = [&seed]() -> float {
        seed = seed * 1103515245u + 12345u;
        return (float) ((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    // Star colours from prototype: 82% cyan-white, 15% amber, 3% violet
    auto cyanWhite = juce::Colour (0xffe0f0fa);   // oklch(90% 0.02 240)
    auto amber     = juce::Colour (0xfff0d8a8);   // oklch(85% 0.05 70)
    auto violet    = juce::Colour (0xffc0a8f0);   // oklch(80% 0.06 290)

    for (int i = 0; i < 80; ++i)
    {
        Star s;
        s.x = lcg();
        s.y = lcg();
        s.phase = lcg() * juce::MathConstants<float>::twoPi;
        s.speed = 0.7f + lcg() * 1.4f;  // 3-9s cycle (2π/2.1 ≈ 3s, 2π/0.7 ≈ 9s)
        s.size = lcg() < 0.80f ? 1.0f : 2.0f;  // 20% are 2px (increased from 12%)

        float colRoll = lcg();
        if (colRoll < 0.82f)       s.colour = cyanWhite;
        else if (colRoll < 0.97f)  s.colour = amber;
        else                       s.colour = violet;

        stars.push_back (s);
    }
}

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
    // Issue #98: Check selected tap first (it renders on top)
    auto hitTest = [&](int i) -> bool
    {
        if (! objects[(size_t)i].enabled) return false;
        auto p = spatialToPixel (objects[(size_t)i].azimuthDeg, objects[(size_t)i].distance);

        // Variable hit radius based on elevation-dependent dot size
        float elDeg = objects[(size_t)i].elevationDeg;
        float z = std::sin (juce::degreesToRadians (elDeg));
        float baseDiam = (i == selectedObject) ? 18.0f : 14.0f;
        float elevScale = (z >= 0.0f) ? 5.0f : 3.0f;  // asymmetric: bigger up, gentler down
        float currentDotSize = baseDiam + elevScale * z;
        float hitRadius = std::max (currentDotSize * 0.5f + 2.0f, 10.0f);

        return p.getDistanceFrom (pos) < hitRadius;
    };

    // Selected tap is visually on top, so check it first
    if (selectedObject >= 0 && selectedObject < MAX_OBJECTS && hitTest (selectedObject))
        return selectedObject;

    for (int i = MAX_OBJECTS - 1; i >= 0; --i)
    {
        if (i == selectedObject) continue;
        if (hitTest (i))
            return i;
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
    auto* lf = dynamic_cast<OSDLookAndFeel*> (&getLookAndFeel());
    float radius = std::min (getWidth(), getHeight()) * 0.45f;
    float cx = getWidth()  * 0.5f;
    float cy = getHeight() * 0.5f;

    // Observatory v6: nearly pure black void
    g.fillAll (Colours_OSD::mapVoid);

    // Subtle radial vignette
    {
        juce::ColourGradient vignette (Colours_OSD::mapVoid.withAlpha (0.0f), cx, cy,
                                       juce::Colour (0xff000000).withAlpha (0.5f),
                                       cx + (float) getWidth() * 0.5f, cy, true);
        g.setGradientFill (vignette);
        g.fillRect (getLocalBounds());
    }

    // Star field — twinkling dots with forward parallax (expanding from center)
    {
        float w = (float) getWidth();
        float h = (float) getHeight();
        float globalPhase = std::fmod (starTime * 0.04f, 1.0f);  // continuous 0→1 cycle, ~25s

        for (auto& s : stars)
        {
            float twinkle = 0.15f + 0.50f * (0.5f + 0.5f * std::sin (starTime * s.speed + s.phase));

            // Per-star phase offset for depth staggering
            float starPhase = std::fmod (globalPhase + s.phase * 0.159f, 1.0f);  // 0.159 ≈ 1/2π

            // Expand from center: at phase≈0 star is near center, at phase≈1 near edge
            float baseX = s.x - 0.5f;   // -0.5..+0.5 relative to center
            float baseY = s.y - 0.5f;
            float expand = 0.3f + starPhase * 0.7f;  // scale 0.3→1.0

            float sx = cx + baseX * expand * w;
            float sy = cy + baseY * expand * h;

            // Grow slightly as they approach (0.8x → 1.5x)
            float sizeMult = 0.8f + starPhase * 0.7f;
            float drawSize = s.size * sizeMult;

            // Fade: appear faint near center, brighten mid-distance, fade at edge
            float fadeMult = std::sin (starPhase * juce::MathConstants<float>::pi);
            g.setColour (s.colour.withAlpha (twinkle * fadeMult));

            g.fillEllipse (sx, sy, drawSize, drawSize);
        }
    }

    // Crosshairs — subtle dashed lines
    {
        float dashLengths[] = { 2.0f, 6.0f };

        // Horizontal dashed line
        juce::Path hSrc, hDashed;
        hSrc.startNewSubPath (cx - radius, cy);
        hSrc.lineTo (cx + radius, cy);
        juce::PathStrokeType (0.5f).createDashedStroke (hDashed, hSrc, dashLengths, 2);
        g.setColour (Colours_OSD::textEtched.withAlpha (0.6f));
        g.fillPath (hDashed);

        // Vertical dashed line
        juce::Path vSrc, vDashed;
        vSrc.startNewSubPath (cx, cy - radius);
        vSrc.lineTo (cx, cy + radius);
        juce::PathStrokeType (0.5f).createDashedStroke (vDashed, vSrc, dashLengths, 2);
        g.fillPath (vDashed);

        // Diagonal crosshairs (±45°, subtler)
        float diagDash[] = { 1.0f, 8.0f };
        float diagR = radius * 0.707f;
        g.setColour (Colours_OSD::textEtched.withAlpha (0.12f));

        juce::Path d1Src, d1Dashed;
        d1Src.startNewSubPath (cx - diagR, cy - diagR);
        d1Src.lineTo (cx + diagR, cy + diagR);
        juce::PathStrokeType (0.25f).createDashedStroke (d1Dashed, d1Src, diagDash, 2);
        g.fillPath (d1Dashed);

        juce::Path d2Src, d2Dashed;
        d2Src.startNewSubPath (cx + diagR, cy - diagR);
        d2Src.lineTo (cx - diagR, cy + diagR);
        juce::PathStrokeType (0.25f).createDashedStroke (d2Dashed, d2Src, diagDash, 2);
        g.fillPath (d2Dashed);
    }

    // Distance rings — thin stroke matching center reticle weight
    for (float r = 0.25f; r <= 1.0f; r += 0.25f)
    {
        float ringR = r * radius;
        g.setColour (Colours_OSD::textEtched.withAlpha (0.6f));
        g.drawEllipse (cx - ringR, cy - ringR, ringR * 2.0f, ringR * 2.0f, 0.7f);
    }

    // Ring labels: distance values at 30° (lower-right), meter values at 210° (upper-left)
    g.setColour (Colours_OSD::textEtched.withAlpha (0.45f));
    if (lf) g.setFont (makeFont (lf->jetbrainsRegular, 9.0f));
    else    g.setFont (juce::FontOptions (9.0f));
    for (int ri = 1; ri <= 4; ++ri)
    {
        float r = ri * 0.25f;
        float ringR = r * radius;

        // Distance values at 30° (lower-right from center)
        float lrAngle = juce::MathConstants<float>::pi / 6.0f;  // 30° in screen coords
        float lx = cx + std::cos (lrAngle) * ringR + 5.0f;
        float ly = cy + std::sin (lrAngle) * ringR + 4.0f;
        g.drawText (juce::String (r, 2), juce::roundToInt (lx), juce::roundToInt (ly), 28, 10,
                    juce::Justification::centredLeft);

        // Meter values at 210° (upper-left from center)
        float ulAngle = juce::MathConstants<float>::pi * 7.0f / 6.0f;  // 210° in screen coords
        float mx = cx + std::cos (ulAngle) * ringR - 30.0f;
        float my = cy + std::sin (ulAngle) * ringR - 12.0f;
        int meters = juce::roundToInt (r * r * 20.0f);
        g.drawText (juce::String (meters) + "m", juce::roundToInt (mx), juce::roundToInt (my), 28, 10,
                    juce::Justification::centredRight);
    }

    // Center reticle (ring + dot + short cross arms)
    {
        float reticleR = 7.0f;
        // Reticle ring
        g.setColour (Colours_OSD::textEtched.withAlpha (0.6f));
        g.drawEllipse (cx - reticleR, cy - reticleR, reticleR * 2.0f, reticleR * 2.0f, 0.7f);
        // Center dot
        g.setColour (Colours_OSD::textEtched.withAlpha (0.6f));
        g.fillEllipse (cx - 1.5f, cy - 1.5f, 3.0f, 3.0f);
        // Short cross arms (4px each side)
        g.setColour (Colours_OSD::textEtched.withAlpha (0.6f));
        float armLen = 4.0f;
        g.drawLine (cx - reticleR - armLen, cy, cx - reticleR, cy, 0.5f);
        g.drawLine (cx + reticleR, cy, cx + reticleR + armLen, cy, 0.5f);
        g.drawLine (cx, cy - reticleR - armLen, cx, cy - reticleR, 0.5f);
        g.drawLine (cx, cy + reticleR, cx, cy + reticleR + armLen, 0.5f);
    }

    // Cardinals — JetBrains Mono Medium, wide tracking, larger text
    g.setColour (Colours_OSD::textEtched);
    if (lf) g.setFont (makeFont (lf->jetbrainsMedium, 14.0f, 0.2f));
    else    g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
    g.drawText ("F",  juce::Rectangle<float> (cx - 30.0f, cy - radius - 20.0f, 60.0f, 16.0f), juce::Justification::centred);
    g.drawText ("B",  juce::Rectangle<float> (cx - 30.0f, cy + radius + 4.0f,  60.0f, 16.0f), juce::Justification::centred);
    g.drawText ("L",  juce::roundToInt (cx - radius - 22), juce::roundToInt (cy - 8),  24, 16, juce::Justification::centred);
    g.drawText ("R",  juce::roundToInt (cx + radius + 2),  juce::roundToInt (cy - 8),  24, 16, juce::Justification::centred);

    // =========================================================================
    // v0.9 PROTOTYPE: Glow trail + origin markers for active trajectories
    // Renders behind object dots. Only drawn for selected tap with active trajectory.
    // =========================================================================
    if (selectedObject >= 0 && selectedObject < MAX_OBJECTS
        && objects[(size_t)selectedObject].enabled
        && trajectoryStates[(size_t)selectedObject].shape != 0)
    {
        auto& ts = trajectoryStates[(size_t)selectedObject];
        auto objCol = objectColours[selectedObject];

        // --- Glow trail: sample trajectory path at ~240 phase points ---
        // Higher sample count for smooth trails with large origin-relative shapes
        // Skip for Random (shape 10) — path is non-deterministic, can't be pre-sampled
        bool drawTrail = (ts.shape != 10);
        constexpr int kPathSamples = 240;
        struct PathPoint { juce::Point<float> px; float elDeg; float phase; };
        PathPoint pathPoints[kPathSamples];

        if (drawTrail)
        {
            for (int s = 0; s < kPathSamples; ++s)
            {
                float samplePhase = (float) s / (float) kPathSamples;
                auto result = OpenSpatialDelayProcessor::computeTrajectory (
                    ts.shape, samplePhase, ts.originAzDeg, ts.originElDeg, ts.originDist, ts.reverse);
                pathPoints[s].px    = spatialToPixel (result.azDeg, result.dist);
                pathPoints[s].elDeg = result.elDeg;
                pathPoints[s].phase = samplePhase;
            }

            // Draw glow trail segments
            for (int s = 0; s < kPathSamples; ++s)
            {
                int next = (s + 1) % kPathSamples;
                auto& p0 = pathPoints[s];
                auto& p1 = pathPoints[next];

                // Skip segments that wrap across the map (large pixel jumps)
                if (p0.px.getDistanceFrom (p1.px) > radius * 0.8f)
                    continue;

                // Spiral: skip the wrap-back segment from end (outer edge) to start (center)
                if (ts.shape == 11 && next == 0)
                    continue;

                // Brightness: proximity to current animated dot position
                // Tighter focus (×6) for concentrated glow near the moving dot.
                // Issue #168 r3: in hero-screenshot mode, keep the proximity
                // fade so the animated dot still has a bright focus, but
                // lift the baseline so the rest of the path reads as a faint
                // continuous curve rather than a near-invisible 0.05-alpha
                // outline. Peak brightness stays the same.
                float phaseDist = std::abs (p0.phase - ts.phase);
                if (phaseDist > 0.5f) phaseDist = 1.0f - phaseDist;
                float proximity = 1.0f - (phaseDist * 6.0f);
                proximity = juce::jlimit (0.0f, 1.0f, proximity);
                float glowAlpha = drawFullTrajectoryForScreenshot
                                ? (0.18f + proximity * 0.42f)
                                : (0.05f + proximity * 0.55f);

                // Elevation encoding: opacity + thickness
                float avgEl = (p0.elDeg + p1.elDeg) * 0.5f;
                float elNorm = (avgEl + 90.0f) / 180.0f;
                float elOpacity = 0.3f + elNorm * 0.7f;
                float thickness = 1.0f + elNorm * 4.5f;

                float finalAlpha = glowAlpha * elOpacity;
                g.setColour (objCol.withAlpha (finalAlpha));
                g.drawLine (p0.px.x, p0.px.y, p1.px.x, p1.px.y, thickness);
            }
        }
        else if (ts.shape == 10 && processor != nullptr)
        {
            // Symmetric time-windowed trail: look-back + look-ahead centered on randomTime
            // Unlike deterministic shapes (which outline a compact closed loop at 0.05 base alpha),
            // Random's sprawling path needs zero base alpha — only the proximity glow near the dot.
            constexpr int kRandomSamples = 120;
            constexpr float kHalfWindow = 1.0f;     // ±1s from current time (2s total)
            constexpr float kDecayRate = 3.0f;       // glow visible within ~±0.33s of dot

            struct RndPathPoint { juce::Point<float> px; float elDeg; float timeOffset; };
            RndPathPoint rndPath[kRandomSamples];

            for (int s = 0; s < kRandomSamples; ++s)
            {
                float timeOffset = -kHalfWindow + (float) s / (float) (kRandomSamples - 1) * (2.0f * kHalfWindow);
                float sampleTime = ts.randomTime + timeOffset;
                auto rp = processor->evaluateRandomNoise (selectedObject, sampleTime);
                float az   = ts.originAzDeg + rp.azDeg;
                float el   = juce::jlimit (-90.0f, 90.0f, ts.originElDeg + rp.elDeg);
                float distScaleR = 1.0f - ts.originDist;  // match tick() distance scaling
                float dist = juce::jlimit (0.0f, 1.0f, ts.originDist + rp.dist * distScaleR);
                az = wrapAzimuth (az);

                rndPath[s].px         = spatialToPixel (az, dist);
                rndPath[s].elDeg      = el;
                rndPath[s].timeOffset = timeOffset;
            }

            for (int s = 0; s < kRandomSamples - 1; ++s)
            {
                auto& p0 = rndPath[s];
                auto& p1 = rndPath[s + 1];

                // Skip segments that wrap across the map
                if (p0.px.getDistanceFrom (p1.px) > radius * 0.8f)
                    continue;

                // Brightness: proximity to current time (timeOffset == 0)
                float proximity = 1.0f - std::abs (p0.timeOffset) * kDecayRate;
                proximity = juce::jlimit (0.0f, 1.0f, proximity);
                float glowAlpha = proximity * 0.60f;

                // Elevation encoding: opacity + thickness (same as other shapes)
                float avgEl = (p0.elDeg + p1.elDeg) * 0.5f;
                float elNorm = (avgEl + 90.0f) / 180.0f;
                float elOpacity = 0.3f + elNorm * 0.7f;
                float thickness = 1.0f + elNorm * 4.5f;

                float finalAlpha = glowAlpha * elOpacity;
                g.setColour (objCol.withAlpha (finalAlpha));
                g.drawLine (p0.px.x, p0.px.y, p1.px.x, p1.px.y, thickness);
            }
        }

        // --- Origin marker: crosshair at captured base position ---
        auto originPx = spatialToPixel (ts.originAzDeg, ts.originDist);
        originPx = { std::round (originPx.x), std::round (originPx.y) };
        {
            float armLen = 8.0f;
            g.setColour (objCol.withAlpha (0.5f));
            g.drawLine (originPx.x - armLen, originPx.y, originPx.x + armLen, originPx.y, 1.2f);
            g.drawLine (originPx.x, originPx.y - armLen, originPx.x, originPx.y + armLen, 1.2f);
        }
    }

    // Issue #98: Build draw order so selected tap renders on top
    int drawOrder[MAX_OBJECTS];
    int drawCount = 0;
    for (int i = 0; i < MAX_OBJECTS; ++i)
        if (i != selectedObject) drawOrder[drawCount++] = i;
    if (selectedObject >= 0 && selectedObject < MAX_OBJECTS)
        drawOrder[drawCount++] = selectedObject;

    for (int di = 0; di < drawCount; ++di)
    {
        int i = drawOrder[di];
        if (! objects[(size_t)i].enabled) continue;

        auto pos = spatialToPixel (objects[(size_t)i].azimuthDeg, objects[(size_t)i].distance);
        pos = { std::round (pos.x), std::round (pos.y) };  // pixel-grid snap for HiDPI sharpness

        // v0.6: IEM-faithful elevation visualization
        // Elevation encoded through dot visual properties: size, opacity, outline, text
        // (no stems — matches IEM StereoEncoder / Nuendo / Pro Tools industry standard)
        float elDeg = objects[(size_t)i].elevationDeg;
        bool isAbove = (elDeg >= 0.0f);
        float z = std::sin (juce::degreesToRadians (elDeg));  // -1..+1

        float baseDiam = (i == selectedObject) ? 18.0f : 14.0f;
        float elevScale = (z >= 0.0f) ? 5.0f : 3.0f;  // asymmetric: +5px up, -3px down
        float dotSize = baseDiam + elevScale * z;
        float half = dotSize * 0.5f;

        // 1. Selection halo (scales with dot, alpha adapts by hemisphere)
        if (i == selectedObject)
        {
            g.setColour (objectColours[i].withAlpha (isAbove ? 0.3f : 0.15f));
            float haloSize = dotSize + 8.0f;
            float haloHalf = haloSize * 0.5f;
            g.fillEllipse (pos.x - haloHalf, pos.y - haloHalf, haloSize, haloSize);
        }

        // 1b. Activity glow — expanding pulse ring + core glow (v0.7)
        // Matches prototype tapPulse: ring expands scale(1)→scale(1.8), fades out
        if (activityLevel[(size_t)i] > 0.01f)
        {
            float act = juce::jlimit (0.0f, 1.0f, activityLevel[(size_t)i]);
            float phase = pulsePhase[(size_t)i];

            // Soft ambient glow (always present when active)
            float ambientR = half + 8.0f + act * 6.0f;
            g.setColour (objectColours[i].withAlpha (act * 0.20f));
            g.fillEllipse (pos.x - ambientR, pos.y - ambientR, ambientR * 2.0f, ambientR * 2.0f);

            // Expanding pulse ring (prototype tapPulse keyframes)
            float ringOpacity = 0.0f;
            float ringScale = 1.0f;
            if (phase < 0.03f)
            {
                // 0%→3%: opacity 0→0.7, scale 1.0
                float t = phase / 0.03f;
                ringOpacity = t * 0.7f;
                ringScale = 1.0f;
            }
            else if (phase < 0.08f)
            {
                // 3%→8%: opacity 0.7→0.6, scale 1.0→1.3
                float t = (phase - 0.03f) / 0.05f;
                ringOpacity = 0.7f - t * 0.1f;
                ringScale = 1.0f + t * 0.3f;
            }
            else if (phase < 0.20f)
            {
                // 8%→20%: opacity 0.6→0, scale 1.3→1.8
                float t = (phase - 0.08f) / 0.12f;
                ringOpacity = 0.6f * (1.0f - t);
                ringScale = 1.3f + t * 0.5f;
            }
            // 20%→100%: invisible

            if (ringOpacity > 0.01f)
            {
                float ringR = (half + 4.0f) * ringScale;
                g.setColour (objectColours[i].withAlpha (ringOpacity * act));
                g.drawEllipse (pos.x - ringR, pos.y - ringR, ringR * 2.0f, ringR * 2.0f, 1.5f);
            }
        }

        // 2. Dot outline at full colour (IEM: always visible regardless of hemisphere)
        juce::Path dotPath;
        dotPath.addEllipse (pos.x - half, pos.y - half, dotSize, dotSize);

        // Core brightness boost during activity pulse (prototype tapCorePulse: brightness 1→1.5→1.4→1)
        float coreBrightness = 1.0f;
        if (activityLevel[(size_t)i] > 0.01f)
        {
            float phase = pulsePhase[(size_t)i];
            if (phase < 0.03f)
                coreBrightness = 1.0f + (phase / 0.03f) * 0.5f;
            else if (phase < 0.08f)
                coreBrightness = 1.5f - ((phase - 0.03f) / 0.05f) * 0.1f;
            else if (phase < 0.18f)
                coreBrightness = 1.4f - ((phase - 0.08f) / 0.10f) * 0.4f;
        }
        auto coreColour = (coreBrightness > 1.01f)
                        ? objectColours[i].brighter (coreBrightness - 1.0f)
                        : objectColours[i];

        g.setColour (coreColour);
        g.strokePath (dotPath, juce::PathStrokeType (1.2f));

        // 3. Dot fill with hemisphere alpha (IEM: 1.0 above, 0.3 below)
        g.setColour (coreColour.withAlpha (isAbove ? 1.0f : 0.3f));
        g.fillPath (dotPath);

        // 4. Number label — Path-based faux bold with pixel-perfect centering
        //    Converts glyphs to a Path, then fills + strokes for guaranteed visual weight.
        //    GlyphArrangement getBoundingBox() centers on actual pixel bounds (no descent offset).
        {
            auto labelColour = isAbove ? juce::Colour (0xff161820) : objectColours[i];
            juce::Font labelFont = lf ? makeFont (lf->jetbrainsBold, 10.0f)
                                      : juce::Font (juce::FontOptions (10.0f));
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

        // 5. Elevation degree label
        //    - Normal UI: selected object only, non-zero elevation
        //    - Screenshot mode (#168 round 2): every enabled object regardless of magnitude
        bool drawElevationLabel = labelAllEnabledForScreenshot
                                    ? objects[(size_t)i].enabled
                                    : (i == selectedObject && std::abs (elDeg) > 1.0f);
        if (drawElevationLabel)
        {
            float labelOffsetY = isAbove ? -(half + 14.0f) : (half + 2.0f);
            g.setColour (objectColours[i].withAlpha (0.85f));
            if (lf) g.setFont (makeFont (lf->jetbrainsRegular, 11.0f));
            else    g.setFont (juce::FontOptions (11.0f));
            juce::String elText = (elDeg > 0.0f ? "+" : "")
                                + juce::String (juce::roundToInt (elDeg))
                                + juce::String::charToString (0x00B0);
            g.drawText (elText, juce::roundToInt (pos.x - 18), juce::roundToInt (pos.y + labelOffsetY),
                        36, 12, juce::Justification::centred);
        }

        // 6. OSC override label (collision-aware positioning)
        if (oscOverride[(size_t)i])
        {
            float oscLabelY = (i == selectedObject && elDeg < -1.0f)
                            ? pos.y + half + 14.0f    // below elevation label
                            : pos.y + half + 1.0f;    // normal position
            g.setColour (Colours_OSD::accentStellar);
            if (lf) g.setFont (makeFont (lf->jetbrainsBold, 10.0f));
            else    g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
            g.drawText ("OSC", juce::roundToInt (pos.x - half - 2), juce::roundToInt (oscLabelY),
                        juce::roundToInt (dotSize + 4), 10, juce::Justification::centred);
        }
    }
}

void SpatialMapComponent::mouseDown (const juce::MouseEvent& e)
{
    draggedObject = findObjectAt (e.position);
    if (draggedObject >= 0)
    {
        listeners.call ([this](Listener& l) { l.objectSelected (draggedObject); });
        // Issue E15: Signal drag start for gesture wrapping (undo grouping)
        if (onDragStarted) onDragStarted (draggedObject);
    }
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

void SpatialMapComponent::mouseUp (const juce::MouseEvent&)
{
    // Issue E15: Signal drag end for gesture wrapping (undo grouping)
    if (draggedObject >= 0 && onDragEnded)
        onDragEnded (draggedObject);
    draggedObject = -1;
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
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 12);
    slider.setColour (juce::Slider::textBoxTextColourId, Colours_OSD::textSecondary);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    // Disable mouse wheel parameter adjustment on all sliders.
    slider.setScrollWheelEnabled (false);
}

static void styleLabel (juce::Label& label, const juce::String& text, OSDLookAndFeel* lf = nullptr)
{
    label.setText (text, juce::dontSendNotification);
    if (lf && lf->jetbrainsMedium)
        label.setFont (makeFont (lf->jetbrainsMedium, 10.0f, 0.12f));
    else
        label.setFont (juce::FontOptions (10.0f));
    label.setColour (juce::Label::textColourId, Colours_OSD::textDim);
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================
// v0.7: FilterGraphComponent — Observatory v6 bandpass display
//==============================================================================
FilterGraphComponent::FilterGraphComponent() {}

float FilterGraphComponent::freqToX (float freqHz) const
{
    float w = static_cast<float> (getWidth());
    float logMin = std::log10 (kMinFreq);
    float logMax = std::log10 (kMaxFreq);
    float logF   = std::log10 (juce::jlimit (kMinFreq, kMaxFreq, freqHz));
    return (logF - logMin) / (logMax - logMin) * w;
}

float FilterGraphComponent::xToFreq (float x) const
{
    float w = static_cast<float> (getWidth());
    float logMin = std::log10 (kMinFreq);
    float logMax = std::log10 (kMaxFreq);
    float logF   = logMin + (x / w) * (logMax - logMin);
    return std::pow (10.0f, logF);
}

void FilterGraphComponent::setFrequencies (float hp, float lp)
{
    if (std::abs (hp - hpFreq) > 0.01f || std::abs (lp - lpFreq) > 0.01f)
    {
        hpFreq = hp;
        lpFreq = lp;
        repaint();
    }
}

void FilterGraphComponent::setQ (float newHPQ, float newLPQ)
{
    if (std::abs (newHPQ - hpQ) > 0.001f || std::abs (newLPQ - lpQ) > 0.001f)
    {
        hpQ = newHPQ;
        lpQ = newLPQ;
        repaint();
    }
}

// 2nd-order high-pass magnitude response
float FilterGraphComponent::hpMagnitude (float f, float fc, float q)
{
    if (fc < 1.0f) return 1.0f;
    float ratio = f / fc;
    float r2 = ratio * ratio;
    float denom = (1.0f - r2) * (1.0f - r2) + (ratio / q) * (ratio / q);
    return (denom > 0.0f) ? (r2 / std::sqrt (denom)) : 0.0f;
}

// 2nd-order low-pass magnitude response
float FilterGraphComponent::lpMagnitude (float f, float fc, float q)
{
    if (fc < 1.0f) return 0.0f;
    float ratio = f / fc;
    float r2 = ratio * ratio;
    float denom = (1.0f - r2) * (1.0f - r2) + (ratio / q) * (ratio / q);
    return (denom > 0.0f) ? (1.0f / std::sqrt (denom)) : 0.0f;
}

void FilterGraphComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float alpha = filterEnabled ? 1.0f : 0.3f;

    // Background
    g.setColour (Colours_OSD::bgRecessed);
    g.fillRoundedRectangle (bounds, 4.0f);

    // 1px border
    g.setColour (Colours_OSD::borderDim);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    // --- dB-to-Y mapping (used by both grid lines and curve) ---
    // Uniform ±18 dB scale with 0 dB centered — proportional like EQ8
    float plotTop = 2.0f;
    float plotVisBot = h - 2.0f;          // visible graph bottom (for grid line bounds)
    float passbaseY = plotTop + (plotVisBot - plotTop) * 0.5f;  // 0 dB at vertical center
    float dbPerPixel = 18.0f / (passbaseY - plotTop);           // 18 dB above and below

    auto dbToY = [&] (float dB) -> float {
        return std::max (plotTop, passbaseY - dB / dbPerPixel);  // clamp top only — curve clips off-screen at bottom
    };

    // --- Logarithmic frequency grid (EQ8-style per-decade subdivisions) ---
    // Tier 1: subdivision lines at every integer multiple per decade (faintest)
    g.setColour (Colours_OSD::borderDim.withAlpha (alpha * 0.25f));
    for (int decade : { 10, 100, 1000, 10000 })
    {
        for (int mult = 2; mult <= 9; ++mult)
        {
            float freq = static_cast<float> (decade * mult);
            if (freq < 20.0f || freq > 20000.0f) continue;
            if (mult == 5) continue;  // drawn brighter below
            float x = freqToX (freq);
            g.drawVerticalLine (juce::roundToInt (x), bounds.getY() + 2, bounds.getBottom() - 2);
        }
    }
    // Tier 2: half-decade anchors — 50, 500, 5k (medium brightness)
    g.setColour (Colours_OSD::borderDim.withAlpha (alpha * 0.5f));
    for (float freq : { 50.0f, 500.0f, 5000.0f })
    {
        float x = freqToX (freq);
        g.drawVerticalLine (juce::roundToInt (x), bounds.getY() + 2, bounds.getBottom() - 2);
    }
    // Tier 3: decade markers — 100, 1k, 10k (brightest)
    g.setColour (Colours_OSD::borderDim.withAlpha (alpha * 1.0f));
    for (float freq : { 100.0f, 1000.0f, 10000.0f })
    {
        float x = freqToX (freq);
        g.drawVerticalLine (juce::roundToInt (x), bounds.getY() + 2, bounds.getBottom() - 2);
    }

    // --- Horizontal dB grid lines with labels (EQ8-style) ---
    {
        auto* lfPtr = dynamic_cast<OSDLookAndFeel*> (&getLookAndFeel());
        if (lfPtr) g.setFont (makeFont (lfPtr->jetbrainsRegular, 7.0f));
        else       g.setFont (juce::FontOptions (7.0f));
    }
    for (float dB : { -12.0f, -6.0f, 0.0f, 6.0f, 12.0f })
    {
        float y = dbToY (dB);
        if (y < bounds.getY() + 2 || y > plotVisBot) continue;
        float lineAlpha = (dB == 0.0f) ? 0.6f : 0.3f;
        g.setColour (Colours_OSD::borderDim.withAlpha (alpha * lineAlpha));
        g.drawHorizontalLine (juce::roundToInt (y), bounds.getX() + 2, bounds.getRight() - 2);
        // dB label on left edge
        juce::String label = (dB > 0.0f) ? ("+" + juce::String ((int) dB))
                                          : juce::String ((int) dB);
        g.setColour (Colours_OSD::textDim.withAlpha (alpha * 0.5f));
        g.drawText (label, (int) bounds.getX() + 3, juce::roundToInt (y) - 5,
                    22, 10, juce::Justification::left);
    }

    // Frequency labels at bottom — decade markers only (EQ8-style)
    {
        auto* lfPtr = dynamic_cast<OSDLookAndFeel*> (&getLookAndFeel());
        if (lfPtr) g.setFont (makeFont (lfPtr->jetbrainsRegular, 8.0f));
        else       g.setFont (juce::FontOptions (8.0f));
    }
    g.setColour (Colours_OSD::textDim.withAlpha (alpha * 0.7f));
    for (auto& [text, freq] : std::initializer_list<std::pair<const char*, float>>{
             {"100", 100.0f}, {"1k", 1000.0f}, {"10k", 10000.0f}})
    {
        int lw = (std::strlen (text) >= 3) ? 24 : 20;
        g.drawText (text, juce::roundToInt (freqToX (freq)) - lw / 2,
                    juce::roundToInt (h) - 11, lw, 10, juce::Justification::centred);
    }

    // --- Compute combined HP+LP magnitude response curve ---
    // Build the magnitude response path (2px steps for smoothness)
    juce::Path curve;
    bool started = false;
    for (float px = 0; px <= w; px += 2.0f)
    {
        float f = xToFreq (px);
        float mag = hpMagnitude (f, hpFreq, hpQ) * lpMagnitude (f, lpFreq, lpQ);
        float dB = 20.0f * std::log10 (std::max (mag, 0.0001f));
        float y = dbToY (dB);

        if (! started)
        {
            curve.startNewSubPath (px, y);
            started = true;
        }
        else
        {
            curve.lineTo (px, y);
        }
    }

    // Fill under curve (extend past bottom so fill clips at component edge)
    juce::Path fillPath (curve);
    fillPath.lineTo (w, h + 20.0f);
    fillPath.lineTo (0.0f, h + 20.0f);
    fillPath.closeSubPath();
    g.setColour (Colours_OSD::accentViolet.withAlpha (0.10f * alpha));
    g.fillPath (fillPath);

    // Stroke
    g.setColour (Colours_OSD::accentViolet.withAlpha (0.8f * alpha));
    g.strokePath (curve, juce::PathStrokeType (1.5f));

    // --- HP handle dot (filled) at HP frequency, on the curve ---
    {
        float hpX = freqToX (hpFreq);
        float hpMag = hpMagnitude (hpFreq, hpFreq, hpQ) * lpMagnitude (hpFreq, lpFreq, lpQ);
        float hpdB = 20.0f * std::log10 (std::max (hpMag, 0.0001f));
        float hpY = dbToY (hpdB);
        g.setColour (Colours_OSD::accentViolet.withAlpha (alpha));
        g.fillEllipse (hpX - 5.0f, hpY - 5.0f, 10.0f, 10.0f);
    }

    // --- LP handle dot (hollow ring) at LP frequency, on the curve ---
    {
        float lpX = freqToX (lpFreq);
        float lpMag = hpMagnitude (lpFreq, hpFreq, hpQ) * lpMagnitude (lpFreq, lpFreq, lpQ);
        float lpdB = 20.0f * std::log10 (std::max (lpMag, 0.0001f));
        float lpY = dbToY (lpdB);
        g.setColour (Colours_OSD::accentViolet.withAlpha (alpha));
        g.drawEllipse (lpX - 5.0f, lpY - 5.0f, 10.0f, 10.0f, 1.5f);
    }
}

void FilterGraphComponent::mouseDown (const juce::MouseEvent& e)
{
    float hpX = freqToX (hpFreq);
    float lpX = freqToX (lpFreq);
    float mx = static_cast<float> (e.x);

    if (std::abs (mx - hpX) < 12.0f)
        currentDrag = HP;
    else if (std::abs (mx - lpX) < 12.0f)
        currentDrag = LP;
    else
        currentDrag = None;

    dragStartY = static_cast<float> (e.y);
    dragStartQ = (currentDrag == HP) ? hpQ : lpQ;

    // Issue E15: Signal drag start for gesture wrapping (undo grouping)
    if (currentDrag != None && onFilterDragStarted)
        onFilterDragStarted (currentDrag == HP);
}

void FilterGraphComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (currentDrag == None) return;

    // Horizontal: frequency — clamp to component bounds so handles can't go off-screen (issue #195)
    float clampedX = juce::jlimit (0.0f, static_cast<float> (getWidth()), static_cast<float> (e.x));
    float freq = xToFreq (clampedX);
    if (currentDrag == HP)
        hpFreq = juce::jlimit (20.0f, 20000.0f, freq);
    else
        lpFreq = juce::jlimit (20.0f, 20000.0f, freq);

    // Vertical: resonance Q — logarithmic scaling for uniform feel across full range
    float dy = dragStartY - static_cast<float> (e.y);  // positive = dragged up
    float newQ = juce::jlimit (0.1f, 8.0f, dragStartQ * std::exp (dy * 0.042f));

    if (currentDrag == HP)
    {
        hpQ = newQ;
        if (onHPQChanged) onHPQChanged (hpQ);
    }
    else
    {
        lpQ = newQ;
        if (onLPQChanged) onLPQChanged (lpQ);
    }

    repaint();
    if (onFrequencyChanged)
        onFrequencyChanged (hpFreq, lpFreq);
}

void FilterGraphComponent::mouseUp (const juce::MouseEvent&)
{
    // Issue E15: Signal drag end for gesture wrapping (undo grouping)
    if (currentDrag != None && onFilterDragEnded)
        onFilterDragEnded (currentDrag == HP);
    currentDrag = None;
}

//==============================================================================
// GlobalTapDrawerComponent — collapsible left-edge mini-drawer (v1.0)
// Architecture: outer component = handle + clipping viewport for the knob panel.
// KnobPanel is always full-sized; the outer component clips it by setting
// the panel's visible bounds, creating a natural slide-in/out effect.
//==============================================================================
GlobalTapDrawerComponent::GlobalTapDrawerComponent (OSDLookAndFeel& lf)
    : lookAndFeel (lf)
{
    setLookAndFeel (&lf);

    // KnobPanel is a child that holds all knobs. We clip it by setting its bounds
    // to only the visible portion of the panel area — this creates natural slide-in/out.
    addAndMakeVisible (knobPanel);

    setupKnob (azSlider,      azLabel,      "AZIM",    -180.0f, 180.0f, 0.1f, kAzimuth);
    azSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);  // match per-tap AZIM drag style
    azSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                  3.0f * juce::MathConstants<float>::pi, false);  // continuous wrap
    azSlider.setReversed (true);  // IEM convention: clockwise knob = clockwise on map
    azSlider.setTextValueSuffix (juce::CharPointer_UTF8 ("\xc2\xb0"));  // °
    setupKnob (elSlider,      elLabel,      "ELEV",    -90.0f,  90.0f,  0.1f, kElevation);
    elSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                  juce::MathConstants<float>::twoPi, true);  // 0° at 9 o'clock, +90° at 12, -90° at 6
    elSlider.setTextValueSuffix (juce::CharPointer_UTF8 ("\xc2\xb0"));  // °
    setupKnob (distSlider,    distLabel,    "DIST",    -1.0f,   1.0f,   0.01f, kDistance);
    setupKnob (dopplerSlider, dopplerLabel, "DOPPLER", -100.0f, 100.0f, 1.0f,  kDoppler);
    dopplerSlider.setTextValueSuffix ("%");
    setupKnob (pitchSlider,   pitchLabel,   "PITCH",   -12.0f,  12.0f,  1.0f,  kPitch);
    pitchSlider.setTextValueSuffix (" st");
    setupKnob (speedSlider,   speedLabel,   "SPEED",   -5.0f,   5.0f,   0.01f, kSpeed);
    speedSlider.setTextValueSuffix (" Hz");
}

void GlobalTapDrawerComponent::setupKnob (juce::Slider& s, juce::Label& l,
                                           const juce::String& name,
                                           float min, float max, float step, int knobIdx)
{
    styleSlider (s, lookAndFeel);
    s.setRange (min, max, step);
    s.textFromValueFunction = [&s](double value) {
        int dec = s.getNumDecimalPlacesToDisplay();
        juce::String text = dec > 0 ? juce::String (value, dec)
                                     : juce::String (juce::roundToInt (value));
        if (text[0] == '-' && text.getDoubleValue() == 0.0)
            return text.substring (1);
        return text;
    };
    s.setValue (0.0);
    s.updateText();  // force text refresh through textFromValueFunction (issue #126)
    s.setColour (juce::Slider::thumbColourId, Colours_OSD::accentGlobal);
    s.setDoubleClickReturnValue (true, 0.0);
    knobPanel.addAndMakeVisible (s);  // add to knobPanel, not directly to drawer

    s.onValueChange = [this, knobIdx, &s] {
        if (suppressCallbacks) return;
        float val = static_cast<float> (s.getValue());
        float delta = val - prevValues[knobIdx];
        if (knobIdx == kAzimuth)
        {
            delta = unwrapAzimuthDelta (delta);
        }
        prevValues[knobIdx] = val;
        if (onGlobalDelta && std::abs (delta) > 1e-6f)
            onGlobalDelta (knobIdx, delta);
    };
    // Issue E15: Signal drag start/end for gesture wrapping (undo grouping)
    s.onDragStart = [this, knobIdx] { if (onDragStarted) onDragStarted (knobIdx); };
    s.onDragEnd   = [this, knobIdx] { if (onDragEnded)  onDragEnded (knobIdx); };
    prevValues[knobIdx] = static_cast<float> (s.getValue());  // sync tracking to initial value

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, Colours_OSD::accentGlobal);
    if (lookAndFeel.jetbrainsMedium)
        l.setFont (juce::Font (juce::FontOptions (lookAndFeel.jetbrainsMedium).withHeight (9.0f)));
    knobPanel.addAndMakeVisible (l);  // add to knobPanel
}

void GlobalTapDrawerComponent::setOpen (bool shouldBeOpen, bool animate)
{
    if (open == shouldBeOpen) return;
    open = shouldBeOpen;
    targetWidth = open ? kOpenWidth : kClosedWidth;

    if (animate)
    {
        startTimerHz (60);
    }
    else
    {
        currentWidth = targetWidth;
        if (onToggle) onToggle();
    }
    repaint();
}

void GlobalTapDrawerComponent::timerCallback()
{
    int diff = targetWidth - currentWidth;
    if (std::abs (diff) <= 1)
    {
        currentWidth = targetWidth;
        stopTimer();
    }
    else
    {
        // Ease-out: 25% of remaining distance per frame (60fps ≈ 200ms settle)
        currentWidth += static_cast<int> (std::ceil (diff * 0.25f));
    }
    if (onToggle) onToggle();  // triggers parent resized() to update bounds
}

void GlobalTapDrawerComponent::resetToCenter()
{
    suppressCallbacks = true;
    azSlider.setValue (0.0, juce::dontSendNotification);
    elSlider.setValue (0.0, juce::dontSendNotification);
    distSlider.setValue (0.0, juce::dontSendNotification);
    dopplerSlider.setValue (0.0, juce::dontSendNotification);
    pitchSlider.setValue (0.0, juce::dontSendNotification);
    speedSlider.setValue (0.0, juce::dontSendNotification);
    std::fill (std::begin (prevValues), std::end (prevValues), 0.0f);
    suppressCallbacks = false;
}

float GlobalTapDrawerComponent::getKnobValue (int knobIdx) const
{
    const juce::Slider* sliders[] = { &azSlider, &elSlider, &distSlider,
                                       &pitchSlider, &dopplerSlider, &speedSlider };
    if (knobIdx < 0 || knobIdx >= kNumKnobs) return 0.0f;
    return static_cast<float> (sliders[knobIdx]->getValue());
}

void GlobalTapDrawerComponent::setKnobValueSilent (int knobIdx, float value)
{
    juce::Slider* sliders[] = { &azSlider, &elSlider, &distSlider,
                                 &pitchSlider, &dopplerSlider, &speedSlider };
    if (knobIdx < 0 || knobIdx >= kNumKnobs) return;
    suppressCallbacks = true;
    sliders[knobIdx]->setValue (value, juce::dontSendNotification);
    prevValues[knobIdx] = value;
    suppressCallbacks = false;
}

// KnobPanel paint — draws the panel background and divider
void GlobalTapDrawerComponent::KnobPanel::paint (juce::Graphics& g)
{
    g.setColour (Colours_OSD::bgPanel.withAlpha (0.92f));
    g.fillAll();

    // Group divider between position (AZIM/ELEV/DIST) and effect (DOPPLER/PITCH/SPEED)
    int knobUnit = (getHeight() - 10) / 6;
    int dividerY = 3 * knobUnit + 5;
    g.setColour (Colours_OSD::borderSubtle);
    g.drawHorizontalLine (dividerY, 4.0f, static_cast<float> (getWidth() - 4));
}

void GlobalTapDrawerComponent::paint (juce::Graphics& g)
{
    // Draw the handle strip (rightmost portion of drawer, or the whole thing when closed)
    int handleX = getWidth() - kHandleWidth;
    if (handleX < 0) handleX = 0;
    int handleW = getWidth() - handleX;
    auto handleBounds = juce::Rectangle<int> (handleX, 0, handleW, getHeight());

    g.setColour (juce::Colour (0xff10141c).withAlpha (0.6f));
    g.fillRect (handleBounds);
    g.setColour (Colours_OSD::borderDim);
    g.drawVerticalLine (handleX, 0.0f, static_cast<float> (getHeight()));
    if (handleX + handleW < getWidth())
        g.drawVerticalLine (handleX + handleW - 1, 0.0f, static_cast<float> (getHeight()));

    // "GLOBAL" text + arrow triangles
    auto hoverCol = handleHover ? Colours_OSD::accentGlobal : Colours_OSD::accentGlobalDim;
    g.setColour (hoverCol);

    auto font = lookAndFeel.dmSansBold
                    ? juce::Font (juce::FontOptions (lookAndFeel.dmSansBold).withHeight (12.5f))
                    : juce::Font (juce::FontOptions (12.5f).withStyle ("Bold"));
    g.setFont (font);

    float cx = handleX + handleW * 0.5f;
    float cy = getHeight() * 0.5f;

    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi, cx, cy));
    g.drawText ("GLOBAL", static_cast<int> (cx - 32), static_cast<int> (cy - 6), 64, 12,
                juce::Justification::centred, false);
    g.restoreState();

    // Arrow triangles above and below text (symmetric spacing)
    float arrowSize = 5.0f;
    float arrowGap  = 30.0f;
    float topArrowY = cy - arrowGap;
    float botArrowY = cy + arrowGap;
    bool pointRight = ! open;

    // Center arrow horizontally: shift tip so arrow midpoint aligns with handle center
    float arrowCx = cx + (pointRight ? arrowSize * 0.5f : -arrowSize * 0.5f);

    auto drawArrow = [&] (float tipX, float tipY, bool pr) {
        juce::Path arrow;
        float halfH = arrowSize * 0.55f;
        if (pr)
            arrow.addTriangle (tipX - arrowSize, tipY - halfH,
                               tipX - arrowSize, tipY + halfH, tipX, tipY);
        else
            arrow.addTriangle (tipX + arrowSize, tipY - halfH,
                               tipX + arrowSize, tipY + halfH, tipX, tipY);
        g.fillPath (arrow);
    };
    drawArrow (arrowCx, topArrowY, pointRight);
    drawArrow (arrowCx, botArrowY, pointRight);
}

void GlobalTapDrawerComponent::layoutKnobs()
{
    // Layout knobs inside the knobPanel (always at full panel size)
    int knobSize = 38;
    int textBoxH = 12;
    int labelH = 12;
    int dividerGap = 10;
    int totalH = knobPanel.getHeight();
    if (totalH <= 0) return;
    int knobUnit = (totalH - dividerGap) / 6;
    int panelW = knobPanel.getWidth();
    int contentH = labelH + knobSize + textBoxH;  // total content per knob slot

    juce::Slider* sliders[] = { &azSlider, &elSlider, &distSlider, &pitchSlider, &dopplerSlider, &speedSlider };
    juce::Label* labels[] = { &azLabel, &elLabel, &distLabel, &pitchLabel, &dopplerLabel, &speedLabel };

    for (int i = 0; i < kNumKnobs; ++i)
    {
        int yOffset = i * knobUnit;
        if (i >= 3) yOffset += dividerGap;
        int labelY = yOffset + (knobUnit - contentH) / 2;
        // Slider fills full panel width so TextBoxBelow has room for value text
        labels[i]->setBounds (0, labelY, panelW, labelH);
        sliders[i]->setBounds (0, labelY + labelH, panelW, knobSize + textBoxH);
    }
}

void GlobalTapDrawerComponent::resized()
{
    // The knobPanel is clipped to the visible panel area (left of handle).
    // Its internal size is always kPanelWidth × height, but we set its BOUNDS
    // to only show the visible portion, creating natural clipping.
    int visiblePanelW = getWidth() - kHandleWidth;
    if (visiblePanelW < 0) visiblePanelW = 0;

    // Position knobPanel so its RIGHT edge aligns with the handle's left edge.
    // When closing, the panel slides LEFT (negative x), hiding knobs progressively.
    int panelX = visiblePanelW - kPanelWidth;  // negative when partially closed
    knobPanel.setBounds (panelX, 0, kPanelWidth, getHeight());
    layoutKnobs();
}

void GlobalTapDrawerComponent::mouseDown (const juce::MouseEvent& e)
{
    // Handle click area: everything to the right of the knob panel
    int handleX = getWidth() - kHandleWidth;
    if (handleX < 0) handleX = 0;
    if (e.getPosition().x >= handleX)
        setOpen (! open, true);
}

void GlobalTapDrawerComponent::mouseEnter (const juce::MouseEvent& e)
{
    int handleX = juce::jmax (0, getWidth() - kHandleWidth);
    if (e.getPosition().x >= handleX && ! handleHover)
    {
        handleHover = true;
        repaint();
    }
}

void GlobalTapDrawerComponent::mouseExit (const juce::MouseEvent&)
{
    if (handleHover)
    {
        handleHover = false;
        repaint();
    }
}

void GlobalTapDrawerComponent::mouseMove (const juce::MouseEvent& e)
{
    int handleX = juce::jmax (0, getWidth() - kHandleWidth);
    bool newHover = e.getPosition().x >= handleX;
    if (newHover != handleHover)
    {
        handleHover = newHover;
        repaint();
    }
}

//==============================================================================
// Editor constructor
//==============================================================================
OpenSpatialDelayEditor::OpenSpatialDelayEditor (OpenSpatialDelayProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), globalTapDrawer (*osdLookAndFeel)
{
    setLookAndFeel (&*osdLookAndFeel);
    setSize (kWindowWidth, kWindowHeight);

    // --- Spatial map ---------------------------------------------------------
    addAndMakeVisible (spatialMap);
    spatialMap.addListener (this);
    // Issue E15: Gesture wrapping for spatial map drag (undo grouping)
    spatialMap.onDragStarted = [this] (int objectIndex) {
        auto prefix = "object" + juce::String (objectIndex + 1) + "_";
        activeSpatialGestureParams[0] = processorRef.apvts.getParameter (prefix + "azimuth");
        activeSpatialGestureParams[1] = processorRef.apvts.getParameter (prefix + "distance");
        for (auto* p : activeSpatialGestureParams)
            if (p != nullptr) p->beginChangeGesture();
    };
    spatialMap.onDragEnded = [this] (int /*objectIndex*/) {
        for (auto* p : activeSpatialGestureParams)
            if (p != nullptr) p->endChangeGesture();
        activeSpatialGestureParams = { nullptr, nullptr };
        processorRef.captureUndoState ("Move Object");
    };
    spatialMap.setProcessor (&processorRef);

    // --- Global tap drawer (overlays left edge of spatial map) ---------------
    addAndMakeVisible (globalTapDrawer);
    globalTapDrawer.toFront (false);
    globalTapDrawer.setOpen (processorRef.getGlobalDrawerOpen());
    // issue #95: Restore drawer knob values from APVTS params on editor (re)creation.
    // The globalTapAzimuth/etc. APVTS params hold the absolute knob value (written below
    // in onGlobalDelta) and survive save/restore via apvts.replaceState.
    {
        static const char* tapParamIds[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                             "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
        for (int i = 0; i < GlobalTapDrawerComponent::kNumKnobs; ++i)
        {
            if (auto* p = processorRef.apvts.getRawParameterValue (tapParamIds[i]))
                globalTapDrawer.setKnobValueSilent (i, p->load());
        }
    }
    // Issue E15: Begin gestures on all affected per-object params when a global knob drag starts
    globalTapDrawer.onDragStarted = [this] (int knobIndex) {
        static const char* suffixes[] = {
            "azimuth", "elevation", "distance",
            "pitchShift", "dopplerAmount", "trajectorySpeed"
        };
        if (knobIndex < 0 || knobIndex >= 6) return;
        activeGlobalGestureParams.clear();
        for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
        {
            auto enabledId = "object" + juce::String (i + 1) + "_enabled";
            if (processorRef.apvts.getRawParameterValue (enabledId)->load() < 0.5f)
                continue;
            auto paramId = "object" + juce::String (i + 1) + "_" + suffixes[knobIndex];
            if (auto* param = processorRef.apvts.getParameter (paramId))
            {
                param->beginChangeGesture();
                activeGlobalGestureParams.push_back (param);
            }
        }
        // Also begin gesture on the globalTap APVTS param itself
        static const char* tapParamIds[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                             "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
        if (auto* param = processorRef.apvts.getParameter (tapParamIds[knobIndex]))
        {
            param->beginChangeGesture();
            activeGlobalGestureParams.push_back (param);
        }
    };
    // Issue E15: End gestures when global knob drag ends
    globalTapDrawer.onDragEnded = [this] (int /*knobIndex*/) {
        for (auto* param : activeGlobalGestureParams)
            param->endChangeGesture();
        activeGlobalGestureParams.clear();
        processorRef.captureUndoState ("Adjust Global Tap");
    };
    globalTapDrawer.onGlobalDelta = [this] (int idx, float delta) {
        applyGlobalTapDelta (idx, delta);
        // Sync absolute knob value to processor atomic (for OSC Send)
        float absVal = static_cast<float> (globalTapDrawer.getKnobValue (idx));
        processorRef.globalTapOffset[idx].store (absVal, std::memory_order_relaxed);
        // issue #95: Also write to APVTS param so the value is saved with the session.
        // Without this, globalTapAzimuth etc. stay at 0.0 and knobs reset on UI reopen.
        static const char* tapParamIds[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                             "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
        if (auto* param = processorRef.apvts.getParameter (tapParamIds[idx]))
            param->setValueNotifyingHost (param->convertTo0to1 (absVal));
    };
    globalTapDrawer.onToggle = [this] {
        processorRef.setGlobalDrawerOpen (globalTapDrawer.isOpen());
        // v1.0: Only update drawer bounds during animation — calling full resized()
        // at 60Hz causes CoreAnimation layout thrashing → Metal GPU crash (issue #37).
        int drawerW = globalTapDrawer.getCurrentWidth();
        auto mapBounds = spatialMap.getBounds();
        globalTapDrawer.setBounds (mapBounds.getX(), mapBounds.getY(), drawerW, mapBounds.getHeight());
        globalTapDrawer.repaint();
    };

    // --- Global knobs --------------------------------------------------------
    auto addKnob = [&](juce::Slider& s, juce::Label& l, const juce::String& name,
                        const juce::String& paramId, std::unique_ptr<SliderAttachment>& attach) {
        styleSlider (s, *osdLookAndFeel);
        addAndMakeVisible (s);
        styleLabel (l, name, &*osdLookAndFeel);
        addAndMakeVisible (l);
        attach = std::make_unique<SliderAttachment> (processorRef.apvts, paramId, s);
    };

    addKnob (inputGainSlider,  inputGainLabel,  "INPUT",    "inputGain",  inputGainAttach);
    addKnob (delayTimeSlider,  delayTimeLabel,  "TIME",     "delayTime",  delayTimeAttach);
    addKnob (noteDivisionSlider, delayTimeLabel, "TIME",    "noteDivision", noteDivisionAttach);
    addKnob (feedbackSlider,   feedbackLabel,   "FEEDBACK", "feedback",   feedbackAttach);

    // Issue E15b: Force host state capture after APVTS slider drag gestures.
    // Uses gesture duration to distinguish drags (>100ms, force capture) from
    // mouse wheel ticks (<100ms, let host debounce/group naturally).
    auto makeGestureEndHandler = [this] (const char* name) {
        return [this, name] {
            double elapsed = juce::Time::getMillisecondCounterHiRes() - lastSliderDragStartTime;
            if (elapsed > 20.0)
            {
                processorRef.notifyHostStateChanged();
                processorRef.captureUndoState (juce::String ("Adjust ") + name);
            }
        };
    };
    auto makeDragStartHandler = [this] { return [this] { lastSliderDragStartTime = juce::Time::getMillisecondCounterHiRes(); }; };
    delayTimeSlider.onDragStart = makeDragStartHandler();
    delayTimeSlider.onDragEnd   = makeGestureEndHandler ("delayTime");
    feedbackSlider.onDragStart  = makeDragStartHandler();
    feedbackSlider.onDragEnd    = makeGestureEndHandler ("feedback");
    noteDivisionSlider.onDragStart = makeDragStartHandler();
    noteDivisionSlider.onDragEnd   = makeGestureEndHandler ("noteDivision");
    addKnob (filterHPSlider,   filterHPLabel,   "HP",       "filterHP",   filterHPAttach);
    addKnob (filterLPSlider,   filterLPLabel,   "LP",       "filterLP",   filterLPAttach);
    addKnob (dryWetSlider,     dryWetLabel,     "DRY/WET",  "dryWet",     dryWetAttach);
    dryWetSlider.onDragStart = makeDragStartHandler();
    dryWetSlider.onDragEnd   = makeGestureEndHandler ("dryWet");
    inputGainSlider.onDragStart  = makeDragStartHandler();
    inputGainSlider.onDragEnd    = makeGestureEndHandler ("inputGain");
    outputGainSlider.onDragStart = makeDragStartHandler();
    outputGainSlider.onDragEnd   = makeGestureEndHandler ("outputGain");
    addKnob (outputGainSlider, outputGainLabel, "OUTPUT",   "outputGain", outputGainAttach);

    // Section-based knob accent colors (prototype v6: DELAY=stellar, TONE=no knobs, MIX=amber)
    // DELAY section — cyan (stellar) — includes Input, Time, Feedback, Pitch
    inputGainSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentStellar);
    delayTimeSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentStellar);
    noteDivisionSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentStellar);
    feedbackSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentStellar);
    // TONE section — filter handles are violet (handled in FilterGraphComponent)
    filterHPSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentViolet);
    filterLPSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentViolet);
    // MIX section — amber
    dryWetSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentAmber);
    outputGainSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentAmber);

    // --- v0.8: Wobble modulation knobs (MOD section) -------------------------
    {
        styleSlider (wobbleAmountSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
        addAndMakeVisible (wobbleAmountSlider);
        styleLabel (wobbleAmountLabel, "AMOUNT", &*osdLookAndFeel);
        addAndMakeVisible (wobbleAmountLabel);
        wobbleAmountAttach = std::make_unique<SliderAttachment> (processorRef.apvts, "wobbleAmount", wobbleAmountSlider);
        wobbleAmountSlider.onDragStart = [this] { lastSliderDragStartTime = juce::Time::getMillisecondCounterHiRes(); };
        wobbleAmountSlider.onDragEnd = [this] {
            if (juce::Time::getMillisecondCounterHiRes() - lastSliderDragStartTime > 20.0)
            {
                processorRef.notifyHostStateChanged();
                processorRef.captureUndoState ("Adjust wobbleAmount");
            }
        };

        styleSlider (wobbleMorphSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
        addAndMakeVisible (wobbleMorphSlider);
        styleLabel (wobbleMorphLabel, "MORPH", &*osdLookAndFeel);
        addAndMakeVisible (wobbleMorphLabel);
        wobbleMorphAttach = std::make_unique<SliderAttachment> (processorRef.apvts, "wobbleMorph", wobbleMorphSlider);
        wobbleMorphSlider.onDragStart = [this] { lastSliderDragStartTime = juce::Time::getMillisecondCounterHiRes(); };
        wobbleMorphSlider.onDragEnd = [this] {
            if (juce::Time::getMillisecondCounterHiRes() - lastSliderDragStartTime > 20.0)
            {
                processorRef.notifyHostStateChanged();
                processorRef.captureUndoState ("Adjust wobbleMorph");
            }
        };

        // MOD section — rose/pink accent (using Colours_OSD::accentRose)
        wobbleAmountSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentRose);
        wobbleMorphSlider.setColour (juce::Slider::thumbColourId, Colours_OSD::accentRose);
    }

    // --- Dropdowns (#13: consistent font) ------------------------------------
    auto setupCombo = [&](juce::ComboBox& box, juce::Label& label, const juce::String& name,
                          const juce::String& paramId, const juce::StringArray& items,
                          std::unique_ptr<ComboBoxAttachment>& attach)
    {
        for (int i = 0; i < items.size(); ++i)
            box.addItem (items[i], i + 1);
        // (#9) Colors inherited from OSDLookAndFeel for consistency
        box.setLookAndFeel (&*osdLookAndFeel);
        addAndMakeVisible (box);
        if (&label != &delayTimeLabel) {
            styleLabel (label, name, &*osdLookAndFeel);
            addAndMakeVisible (label);
        }
        attach = std::make_unique<ComboBoxAttachment> (processorRef.apvts, paramId, box);
    };

    // Algorithm combo — manually managed (no ComboBoxParameterAttachment)
    // Items dynamically populated based on output format (stereo vs surround)
    algorithmBox.setLookAndFeel (&*osdLookAndFeel);
    addAndMakeVisible (algorithmBox);
    styleLabel (algorithmLabel, "ALGORITHM", &*osdLookAndFeel);
    addAndMakeVisible (algorithmLabel);
    algorithmBox.onChange = [this]
    {
        int selectedId = algorithmBox.getSelectedId();
        if (selectedId > 0)
        {
            int paramIdx = selectedId - 1;  // IDs are 1-based, param indices are 0-based
            processorRef.configAlgorithm.store (paramIdx, std::memory_order_relaxed);
            processorRef.markConfigStateDirty();
        }
    };
    {
        juce::StringArray hrtfItems { "Simple", "Immersive", "Natural", "Precise", "Spatial", "Studio Ref" };
        for (int i = 0; i < hrtfItems.size(); ++i)
            hrtfProfileBox.addItem (hrtfItems[i], i + 1);
        hrtfProfileBox.setLookAndFeel (&*osdLookAndFeel);
        addAndMakeVisible (hrtfProfileBox);
        styleLabel (hrtfProfileLabel, "PROFILE", &*osdLookAndFeel);
        addAndMakeVisible (hrtfProfileLabel);
        hrtfProfileBox.setSelectedItemIndex (processorRef.configHrtfProfile.load (std::memory_order_relaxed),
                                             juce::dontSendNotification);
        hrtfProfileBox.onChange = [this] {
            processorRef.configHrtfProfile.store (hrtfProfileBox.getSelectedItemIndex(), std::memory_order_relaxed);
            processorRef.markConfigStateDirty();
        };
    }
    // v0.7: syncMode reworked to 3-state (Straight/Dotted/Triplet)
    // Uses music note icons: straight = ♩, dotted = ♩., triplet = ♩³
    setupCombo (syncModeBox, delayTimeLabel, "SYNC MODE",
                "syncMode", { juce::String (juce::CharPointer_UTF8 ("\xe2\x99\xa9")),
                              juce::String (juce::CharPointer_UTF8 ("\xe2\x99\xa9.") ),
                              juce::String (juce::CharPointer_UTF8 ("\xe2\x99\xa9\xc2\xb3")) },
                syncModeAttach);

    // --- Output format dropdown (header bar) ---------------------------------
    {
        juce::StringArray formatNames;
        for (const auto& info : OpenSpatialDelayProcessor::outputFormatRegistry)
            formatNames.add (info.name);
        for (int i = 0; i < formatNames.size(); ++i)
            outputFormatBox.addItem (formatNames[i], i + 1);
        outputFormatBox.setLookAndFeel (&*osdLookAndFeel);
        addAndMakeVisible (outputFormatBox);
        styleLabel (outputFormatLabel, "OUTPUT", &*osdLookAndFeel);
        addAndMakeVisible (outputFormatLabel);
        outputFormatBox.setSelectedItemIndex (processorRef.configOutputFormat.load (std::memory_order_relaxed),
                                              juce::dontSendNotification);
        outputFormatBox.onChange = [this] {
            processorRef.configOutputFormat.store (outputFormatBox.getSelectedItemIndex(), std::memory_order_relaxed);
            processorRef.markConfigStateDirty();
            // Issue E15b: Activate layout immediately on user-driven format change
            // instead of waiting for the next timer tick. This eliminates the one-tick
            // race that corrupts undo state after format changes.
            processorRef.requestOutputFormatChange (outputFormatBox.getSelectedItemIndex());
        };
    }

    // --- Header dropdown labels: left-aligned, JetBrains Mono Medium ----------
    {
        auto headerLblFont = makeFont (osdLookAndFeel->jetbrainsMedium, 9.5f, 0.1f);
        auto headerLblColour = Colours_OSD::textDim;

        for (auto* lbl : { &algorithmLabel, &outputFormatLabel, &hrtfProfileLabel })
        {
            lbl->setFont (headerLblFont);
            lbl->setColour (juce::Label::textColourId, headerLblColour);
            lbl->setJustificationType (juce::Justification::centredLeft);
        }
    }

    // --- Tempo sync button (#5/#6) -------------------------------------------
    tempoSyncButton = std::make_unique<StyledButton> ("SYNC", Colours_OSD::accentSync,
                                                       osdLookAndFeel->jetbrainsMedium);
    tempoSyncButton->setAlwaysActive (true);
    tempoSyncButton->setClickingTogglesState (false);  // Issue E15b: timer drives visual state
    addAndMakeVisible (*tempoSyncButton);

    // Issue E15b: Manual gesture brackets instead of ButtonAttachment (prevents double-undo)
    tempoSyncButton->onClick = [this]
    {
        auto* syncParam = processorRef.apvts.getParameter ("tempoSync");
        if (syncParam != nullptr)
        {
            syncParam->beginChangeGesture();
            float current = processorRef.apvts.getRawParameterValue ("tempoSync")->load();
            syncParam->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
            syncParam->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Toggle Tempo Sync");
        }
    };

    auto updateSyncUI = [this] {
        bool isSynced = processorRef.apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
        delayTimeSlider.setVisible (!isSynced);
        noteDivisionSlider.setVisible (isSynced);
        syncModeBox.setVisible (false);  // hidden — replaced by toggle buttons
        if (syncDottedButton)  syncDottedButton->setVisible (isSynced);
        if (syncTripletButton) syncTripletButton->setVisible (isSynced);

        // Toggle button text: "Sync" when synced, "Time" when free
        tempoSyncButton->setLabel (isSynced ? "SYNC" : "TIME");
        tempoSyncButton->setAccentColour (isSynced ? Colours_OSD::accentSync : Colours_OSD::accentStellar);
        delayTimeLabel.setText ("TIME", juce::dontSendNotification);

        // Knob arc color matches button state: gold/yellow when synced, stellar when free
        auto knobCol = isSynced ? Colours_OSD::accentSync : Colours_OSD::accentStellar;
        delayTimeSlider.setColour (juce::Slider::thumbColourId, knobCol);
        noteDivisionSlider.setColour (juce::Slider::thumbColourId, knobCol);
    };

    // Issue E15b: call once to initialize UI, then timer drives updates
    updateSyncUI();
    tempoSyncUpdateUI = updateSyncUI;  // store for timer-driven refresh

    // v0.7: Dotted/Triplet toggle buttons (mutually exclusive, radio-style)
    syncDottedButton = std::make_unique<StyledButton> ("", Colours_OSD::accentSync,
                                                        osdLookAndFeel->jetbrainsMedium);
    syncDottedButton->setIcon (createDottedNoteIconPath(), 1.0f);
    syncDottedButton->setClickingTogglesState (false);
    addAndMakeVisible (*syncDottedButton);

    syncTripletButton = std::make_unique<StyledButton> ("", Colours_OSD::accentSync,
                                                         osdLookAndFeel->jetbrainsMedium);
    syncTripletButton->setIcon (createTripletNoteIconPath(), 1.0f);
    syncTripletButton->setClickingTogglesState (false);
    addAndMakeVisible (*syncTripletButton);

    // Dotted button: toggle → set syncMode to 1 (Dotted) or 0 (Straight)
    syncDottedButton->onClick = [this] {
        bool newState = !syncDottedButton->getToggleState();
        syncDottedButton->setToggleState (newState, juce::dontSendNotification);
        if (newState) syncTripletButton->setToggleState (false, juce::dontSendNotification);
        int mode = newState ? 1 : 0;
        if (auto* param = processorRef.apvts.getParameter ("syncMode"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 ((float) mode));
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Change Sync Mode");
        }
        repaint();
    };
    // Triplet button: toggle → set syncMode to 2 (Triplet) or 0 (Straight)
    syncTripletButton->onClick = [this] {
        bool newState = !syncTripletButton->getToggleState();
        syncTripletButton->setToggleState (newState, juce::dontSendNotification);
        if (newState) syncDottedButton->setToggleState (false, juce::dontSendNotification);
        int mode = newState ? 2 : 0;
        if (auto* param = processorRef.apvts.getParameter ("syncMode"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 ((float) mode));
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Change Sync Mode");
        }
        repaint();
    };

    updateSyncUI();

    // --- Object selector buttons ---------------------------------------------
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto& btn = objectButtons[(size_t)i];
        btn.setButtonText (juce::String (i + 1));
        btn.setClickingTogglesState (false);
        btn.setLookAndFeel (&*osdLookAndFeel);
        btn.onClick = [this, i] { selectObject (i); };
        addAndMakeVisible (btn);
    }
    updateObjectButtonColours();

    // --- Per-object controls -------------------------------------------------
    // (#3) Azimuth: rotary knob — use RotaryHorizontalVerticalDrag for natural direction
    styleSlider (objAzimuthSlider, *osdLookAndFeel, juce::Slider::RotaryHorizontalVerticalDrag);
    objAzimuthSlider.setRotaryParameters (juce::MathConstants<float>::pi, 3.0f * juce::MathConstants<float>::pi, false);
    objAzimuthSlider.setReversed (true);  // (#8) IEM StereoEncoder convention: clockwise knob = clockwise on map
    addAndMakeVisible (objAzimuthSlider);
    styleLabel (objAzLabel, "AZIMUTH", &*osdLookAndFeel);
    addAndMakeVisible (objAzLabel);

    // v0.7: Elevation: rotary knob — 0° at left (9 o'clock), -90° at bottom, +90° at top
    styleSlider (objElevationSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
    objElevationSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                            juce::MathConstants<float>::twoPi,
                                            true);
    addAndMakeVisible (objElevationSlider);
    styleLabel (objElLabel, "ELEVATION", &*osdLookAndFeel);
    addAndMakeVisible (objElLabel);

    // Distance: rotary knob
    styleSlider (objDistanceSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objDistanceSlider);
    styleLabel (objDistLabel, "DISTANCE", &*osdLookAndFeel);
    addAndMakeVisible (objDistLabel);

    // --- Enabled toggle (IndicatorToggle — accent matches selected tap color) --
    objEnabledButton = std::make_unique<IndicatorToggle> ("ON", SpatialMapComponent::objectColours[0],
                                                           osdLookAndFeel->jetbrainsMedium);
    addAndMakeVisible (*objEnabledButton);

    // --- v0.4: Per-object Doppler amount knob --------------------------------
    styleSlider (objDopplerSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objDopplerSlider);
    styleLabel (objDopplerLabel, "DOPPLER", &*osdLookAndFeel);
    addAndMakeVisible (objDopplerLabel);
    // Doppler color set per-tap in selectObject()

    // --- v0.6: Per-object trajectory controls (bottom panel, bound in selectObject) ---
    objTrajectoryBox.setLookAndFeel (&*osdLookAndFeel);
    objTrajectoryBox.addItem ("None",      1);
    objTrajectoryBox.addItem ("Bounce",    2);
    objTrajectoryBox.addItem ("Circle",    3);
    objTrajectoryBox.addItem ("Cross",     4);
    objTrajectoryBox.addItem ("Figure-8",  5);
    objTrajectoryBox.addItem ("Heart",     6);
    objTrajectoryBox.addItem ("Helix",     7);
    objTrajectoryBox.addItem ("Infinity",  8);
    objTrajectoryBox.addItem ("Line",      9);
    objTrajectoryBox.addItem ("Orbit",    10);
    objTrajectoryBox.addItem ("Random",   11);
    objTrajectoryBox.addItem ("Spiral",   12);
    objTrajectoryBox.addItem ("Square",   13);
    objTrajectoryBox.addItem ("Triangle", 14);
    addAndMakeVisible (objTrajectoryBox);
    styleLabel (objTrajectoryLabel, "TRAJECTORY", &*osdLookAndFeel);
    addAndMakeVisible (objTrajectoryLabel);
    // Attachment created in selectObject()

    // v0.8: Trajectory direction arrow buttons (← →)
    objTrajectoryFwdButton = std::make_unique<StyledButton> (juce::String::charToString (0x2192),
                                                              SpatialMapComponent::objectColours[0],
                                                              osdLookAndFeel->jetbrainsMedium);
    objTrajectoryFwdButton->setClickingTogglesState (false);
    addAndMakeVisible (*objTrajectoryFwdButton);

    objTrajectoryRevButton = std::make_unique<StyledButton> (juce::String::charToString (0x2190),
                                                              SpatialMapComponent::objectColours[0],
                                                              osdLookAndFeel->jetbrainsMedium);
    objTrajectoryRevButton->setClickingTogglesState (false);
    addAndMakeVisible (*objTrajectoryRevButton);

    // Hidden ComboBox for APVTS binding
    objTrajectoryDirBox.addItem ("Forward", 1);
    objTrajectoryDirBox.addItem ("Reverse", 2);
    addChildComponent (objTrajectoryDirBox);  // invisible
    // Issue #182: Use manual gesture brackets (like Dotted/Triplet) instead of
    // setSelectedId() which relies on ComboBoxAttachment auto-gestures that don't
    // create proper undo entries in Ableton.
    objTrajectoryFwdButton->onClick = [this] {
        auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
        if (auto* param = processorRef.apvts.getParameter (prefix + "trajectoryDirection"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (0.0f));  // 0 = Forward
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Change Trajectory Direction");
        }
        objTrajectoryFwdButton->setToggleState (true, juce::dontSendNotification);
        objTrajectoryRevButton->setToggleState (false, juce::dontSendNotification);
    };
    objTrajectoryRevButton->onClick = [this] {
        auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
        if (auto* param = processorRef.apvts.getParameter (prefix + "trajectoryDirection"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (1.0f));  // 1 = Reverse
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Change Trajectory Direction");
        }
        objTrajectoryRevButton->setToggleState (true, juce::dontSendNotification);
        objTrajectoryFwdButton->setToggleState (false, juce::dontSendNotification);
    };
    objTrajectoryDirBox.onChange = [this] {
        int sel = objTrajectoryDirBox.getSelectedId();  // 1=Forward, 2=Reverse
        if (objTrajectoryFwdButton) objTrajectoryFwdButton->setToggleState (sel == 1, juce::dontSendNotification);
        if (objTrajectoryRevButton) objTrajectoryRevButton->setToggleState (sel == 2, juce::dontSendNotification);
    };

    // Issue #182: Use manual gesture brackets (like Direction/InputChannel) instead of
    // ComboBoxAttachment auto-gestures that Ableton ignores.
    objTrajectoryBox.onChange = [this] {
        auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
        if (auto* param = processorRef.apvts.getParameter (prefix + "trajectoryShape"))
        {
            int selectedIdx = objTrajectoryBox.getSelectedId() - 1;  // ComboBox 1-based -> 0-based
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (selectedIdx)));
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Change Trajectory Shape");
        }
    };

    styleSlider (objTrajectorySpeedSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objTrajectorySpeedSlider);
    styleLabel (objTrajectorySpeedLabel, "SPEED", &*osdLookAndFeel);
    addAndMakeVisible (objTrajectorySpeedLabel);
    // Speed color set per-tap in selectObject()
    // Attachment created in selectObject()

    // --- v0.6: ADM-OSC Receive toggle (IndicatorToggle) ----------------------
    oscToggleButton = std::make_unique<IndicatorToggle> ("RECEIVE", Colours_OSD::accentGreen,
                                                          osdLookAndFeel->jetbrainsMedium);
    addAndMakeVisible (*oscToggleButton);
    oscToggleButton->setToggleState (processorRef.getOscReceiveEnabled(), juce::dontSendNotification);
    oscToggleButton->onClick = [this]
    {
        processorRef.setOscReceiveEnabled (oscToggleButton->getToggleState());
    };

    // v0.6: Editable OSC port label (single-click to edit, Enter to commit)
    oscPortLabel.setText (juce::String (processorRef.getOscReceivePort()), juce::dontSendNotification);
    oscPortLabel.setEditable (true, false, false);  // single-click yes, double-click no, loss-of-focus commits
    oscPortLabel.setFont (makeFont (osdLookAndFeel->jetbrainsRegular, 11.0f));
    oscPortLabel.setColour (juce::Label::textColourId, Colours_OSD::textSecondary);
    oscPortLabel.setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
    oscPortLabel.setColour (juce::Label::backgroundWhenEditingColourId, Colours_OSD::bgWell);
    oscPortLabel.setColour (juce::Label::outlineWhenEditingColourId, Colours_OSD::accentStellar);
    oscPortLabel.setColour (juce::TextEditor::highlightColourId, Colours_OSD::accentStellar.withAlpha (0.35f));
    oscPortLabel.setJustificationType (juce::Justification::centred);
    oscPortLabel.setColour (juce::Label::backgroundColourId, Colours_OSD::bgRecessed);
    oscPortLabel.setColour (juce::Label::outlineColourId, Colours_OSD::borderDim);  // matches ComboBox outline
    oscPortLabel.onEditorShow = [this]()
    {
        if (auto* ed = oscPortLabel.getCurrentTextEditor())
        {
            ed->setJustification (juce::Justification::centred);
            ed->setHighlightedRegion ({ 0, oscPortLabel.getText().length() });
        }
    };
    oscPortLabel.onTextChange = [this]
    {
        auto text = oscPortLabel.getText().trim();
        int port = text.getIntValue();
        if (port >= kMinPort && port <= kMaxPort)
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

    // --- v0.4: Global Air Absorption toggle (IndicatorToggle) ----------------
    // Issue #182: Use manual onClick with gesture brackets (like FLT/MOD) instead
    // of ButtonAttachment, which creates double undo entries.
    airAbsorptionButton = std::make_unique<IndicatorToggle> ("AIR", Colours_OSD::accentStellar,
                                                              osdLookAndFeel->jetbrainsMedium);
    airAbsorptionButton->setClickingTogglesState (false);  // timer drives visual state
    airAbsorptionButton->onClick = [this]
    {
        auto* airParam = processorRef.apvts.getParameter ("airAbsorption");
        if (airParam != nullptr)
        {
            airParam->beginChangeGesture();
            float current = processorRef.apvts.getRawParameterValue ("airAbsorption")->load();
            airParam->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
            airParam->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Toggle Air Absorption");
        }
    };
    addAndMakeVisible (*airAbsorptionButton);

    // --- v0.7: ADM-OSC Send toggle (IndicatorToggle) -------------------------
    oscSendToggleButton = std::make_unique<IndicatorToggle> ("SEND", Colours_OSD::accentGreen,
                                                              osdLookAndFeel->jetbrainsMedium);
    oscSendToggleButton->setToggleState (processorRef.getOscSendEnabled(), juce::dontSendNotification);
    oscSendToggleButton->onClick = [this]
    {
        processorRef.setOscSendEnabled (oscSendToggleButton->getToggleState());
    };
    addAndMakeVisible (*oscSendToggleButton);

    oscSendIPLabel.setText (processorRef.getOscSendIP(), juce::dontSendNotification);
    oscSendIPLabel.setEditable (true, false, false);
    oscSendIPLabel.setFont (makeFont (osdLookAndFeel->jetbrainsRegular, 11.0f));
    oscSendIPLabel.setColour (juce::Label::textColourId, Colours_OSD::textSecondary);
    oscSendIPLabel.setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
    oscSendIPLabel.setColour (juce::Label::backgroundWhenEditingColourId, Colours_OSD::bgWell);
    oscSendIPLabel.setColour (juce::Label::outlineWhenEditingColourId, Colours_OSD::accentStellar);
    oscSendIPLabel.setColour (juce::TextEditor::highlightColourId, Colours_OSD::accentStellar.withAlpha (0.35f));
    oscSendIPLabel.setColour (juce::Label::backgroundColourId, Colours_OSD::bgRecessed);
    oscSendIPLabel.setColour (juce::Label::outlineColourId, Colours_OSD::borderDim);
    oscSendIPLabel.setJustificationType (juce::Justification::centred);
    oscSendIPLabel.onEditorShow = [this]()
    {
        if (auto* ed = oscSendIPLabel.getCurrentTextEditor())
        {
            ed->setJustification (juce::Justification::centred);
            ed->setInputFilter (new juce::TextEditor::LengthAndCharacterRestriction (15, "0123456789."), true);
            ed->setHighlightedRegion ({ 0, oscSendIPLabel.getText().length() });
        }
    };
    oscSendIPLabel.onTextChange = [this]
    {
        auto text = oscSendIPLabel.getText().trim();
        auto octets = juce::StringArray::fromTokens (text, ".", "");
        bool valid = (octets.size() == 4);
        if (valid)
        {
            for (auto& o : octets)
            {
                int val = o.getIntValue();
                if (o.isEmpty() || o.length() > 3 || val < 0 || val > 255
                    || (o.length() > 1 && o[0] == '0'))
                {
                    valid = false;
                    break;
                }
            }
        }
        if (valid)
            processorRef.setOscSendIP (text);
        else
            oscSendIPLabel.setText (processorRef.getOscSendIP(), juce::dontSendNotification);
    };
    addAndMakeVisible (oscSendIPLabel);

    oscSendPortLabel.setText (juce::String (processorRef.getOscSendPort()), juce::dontSendNotification);
    oscSendPortLabel.setEditable (true, false, false);
    oscSendPortLabel.setFont (makeFont (osdLookAndFeel->jetbrainsRegular, 11.0f));
    oscSendPortLabel.setColour (juce::Label::textColourId, Colours_OSD::textSecondary);
    oscSendPortLabel.setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
    oscSendPortLabel.setColour (juce::Label::backgroundWhenEditingColourId, Colours_OSD::bgWell);
    oscSendPortLabel.setColour (juce::Label::outlineWhenEditingColourId, Colours_OSD::accentStellar);
    oscSendPortLabel.setColour (juce::TextEditor::highlightColourId, Colours_OSD::accentStellar.withAlpha (0.35f));
    oscSendPortLabel.setColour (juce::Label::backgroundColourId, Colours_OSD::bgRecessed);
    oscSendPortLabel.setColour (juce::Label::outlineColourId, Colours_OSD::borderDim);
    oscSendPortLabel.setJustificationType (juce::Justification::centred);
    oscSendPortLabel.onEditorShow = [this]()
    {
        if (auto* ed = oscSendPortLabel.getCurrentTextEditor())
        {
            ed->setJustification (juce::Justification::centred);
            ed->setHighlightedRegion ({ 0, oscSendPortLabel.getText().length() });
        }
    };
    oscSendPortLabel.onTextChange = [this]
    {
        int port = oscSendPortLabel.getText().trim().getIntValue();
        if (port >= kMinPort && port <= kMaxPort)
            processorRef.setOscSendPort (port);
        else
            oscSendPortLabel.setText (juce::String (processorRef.getOscSendPort()), juce::dontSendNotification);
    };
    addAndMakeVisible (oscSendPortLabel);

    // --- v0.9: FLT toggle — directly controls filterEnabled parameter ---------
    fltToggle = std::make_unique<IndicatorToggle> ("FLT", Colours_OSD::accentViolet,
                                                    osdLookAndFeel->jetbrainsMedium);
    fltToggle->setClickingTogglesState (false);  // timer drives visual state
    fltToggle->onClick = [this]
    {
        auto* fltParam = processorRef.apvts.getParameter ("filterEnabled");
        if (fltParam != nullptr)
        {
            fltParam->beginChangeGesture();
            float current = processorRef.apvts.getRawParameterValue ("filterEnabled")->load();
            fltParam->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
            fltParam->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Toggle Filter");
        }
    };
    addAndMakeVisible (*fltToggle);
    // Set initial visual state from filterEnabled parameter
    {
        filterIsActive = processorRef.apvts.getRawParameterValue ("filterEnabled")->load() > 0.5f;
        fltToggle->setToggleState (filterIsActive, juce::dontSendNotification);
    }

    // --- v0.8: MOD toggle (MOD section header) — timer-driven ----------------
    modToggle = std::make_unique<IndicatorToggle> ("MOD", Colours_OSD::accentRose,
                                                    osdLookAndFeel->jetbrainsMedium);
    modToggle->setClickingTogglesState (false);  // timer drives visual state
    modToggle->onClick = [this]
    {
        auto* param = processorRef.apvts.getParameter ("wobbleEnabled");
        if (param != nullptr)
        {
            param->beginChangeGesture();
            float cur = processorRef.apvts.getRawParameterValue ("wobbleEnabled")->load();
            param->setValueNotifyingHost (cur > 0.5f ? 0.0f : 1.0f);
            param->endChangeGesture();
            processorRef.notifyHostStateChanged();
            processorRef.captureUndoState ("Toggle Modulation");
        }
    };
    addAndMakeVisible (*modToggle);
    // Set initial visual state
    modIsActive = processorRef.apvts.getRawParameterValue ("wobbleEnabled")->load() > 0.5f;
    modToggle->setToggleState (modIsActive, juce::dontSendNotification);

    // --- v0.7: Filter graph component (TONE section, replaces HP/LP knobs) ----
    addAndMakeVisible (filterGraph);
    filterGraph.onFrequencyChanged = [this] (float hp, float lp)
    {
        if (auto* param = processorRef.apvts.getParameter ("filterHP"))
            param->setValueNotifyingHost (param->convertTo0to1 (hp));
        if (auto* param = processorRef.apvts.getParameter ("filterLP"))
            param->setValueNotifyingHost (param->convertTo0to1 (lp));
    };
    filterGraph.onHPQChanged = [this] (float q)
    {
        if (auto* param = processorRef.apvts.getParameter ("filterHPQ"))
            param->setValueNotifyingHost (param->convertTo0to1 (q));
        repaint();  // update painted readout
    };
    filterGraph.onLPQChanged = [this] (float q)
    {
        if (auto* param = processorRef.apvts.getParameter ("filterLPQ"))
            param->setValueNotifyingHost (param->convertTo0to1 (q));
        repaint();  // update painted readout
    };
    // Issue E15: Gesture wrapping for filter graph drag (undo grouping)
    filterGraph.onFilterDragStarted = [this] (bool isHP) {
        activeFilterGestureParams.clear();
        // Issue E15b: Only begin gestures on the params that will actually change.
        // Opening gestures on all 4 params causes the host to create per-parameter undo entries.
        const char* ids[2] = { isHP ? "filterHP" : "filterLP",
                               isHP ? "filterHPQ" : "filterLPQ" };
        for (auto* id : ids)
        {
            if (auto* param = processorRef.apvts.getParameter (id))
            {
                param->beginChangeGesture();
                activeFilterGestureParams.push_back (param);
            }
        }
    };
    filterGraph.onFilterDragEnded = [this] (bool /*isHP*/) {
        for (auto* param : activeFilterGestureParams)
            param->endChangeGesture();
        activeFilterGestureParams.clear();
        processorRef.notifyHostStateChanged();
        processorRef.captureUndoState ("Adjust Filter");
    };

    // --- v0.7: Per-object pitch shift knob (bottom panel) --------------------
    styleSlider (objPitchShiftSlider, *osdLookAndFeel, juce::Slider::RotaryVerticalDrag);
    addAndMakeVisible (objPitchShiftSlider);
    styleLabel (objPitchShiftLabel, "PITCH", &*osdLookAndFeel);
    addAndMakeVisible (objPitchShiftLabel);
    // Per-object pitch color set per-tap in selectObject()

    // --- v0.8: Per-object input channel cycling button -----------------------
    {
        // Hidden ComboBox for APVTS binding (items must be 1-based)
        objInputChannelBox.addItem ("L+R", 1);
        objInputChannelBox.addItem ("L",   2);
        objInputChannelBox.addItem ("R",   3);
        addChildComponent (objInputChannelBox);  // invisible

        // Cycling button — onClick advances to next item, wraps around
        objInputChannelButton = std::make_unique<StyledButton> ("L+R",
                                                                 SpatialMapComponent::objectColours[0],
                                                                 osdLookAndFeel->jetbrainsMedium);
        objInputChannelButton->setClickingTogglesState (false);
        objInputChannelButton->setAlwaysActive (true);
        addAndMakeVisible (*objInputChannelButton);

        // Update button text whenever hidden combo changes
        objInputChannelBox.onChange = [this] {
            int sel = objInputChannelBox.getSelectedId();
            juce::String labels[] = { "L+R", "L+R", "L", "R" };  // index 0 unused, 1-based
            if (sel >= 1 && sel <= 3)
            {
                objInputChannelButton->setLabel (labels[sel]);
                if (sel == 2)
                    objInputChannelButton->setAccentColour (Colours_OSD::accentChannelL);
                else if (sel == 3)
                    objInputChannelButton->setAccentColour (Colours_OSD::accentChannelR);
                else
                    objInputChannelButton->setAccentColour (
                        SpatialMapComponent::objectColours[currentObjectIndex]);
            }
        };

        // Cycle on click: L+R (1) → L (2) → R (3) → L+R (1)
        // Issue #182: Use manual gesture brackets (like Dotted/Triplet) instead of
        // setSelectedId() which relies on ComboBoxAttachment auto-gestures that don't
        // create proper undo entries in Ableton.
        objInputChannelButton->onClick = [this] {
            auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
            if (auto* param = processorRef.apvts.getParameter (prefix + "inputChannel"))
            {
                int cur = objInputChannelBox.getSelectedId();  // 1-based
                int next = (cur >= 3) ? 1 : cur + 1;
                param->beginChangeGesture();
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (next - 1)));
                param->endChangeGesture();
                processorRef.notifyHostStateChanged();
                processorRef.captureUndoState ("Change Input Channel");
            }
        };
    }

    // --- v0.7: Input format dropdown (header bar) ----------------------------
    inputFormatBox.setLookAndFeel (&*osdLookAndFeel);
    inputFormatBox.addItem ("Mono", 1);
    inputFormatBox.addItem ("Stereo", 2);
    addAndMakeVisible (inputFormatBox);
    inputFormatBox.setSelectedItemIndex (processorRef.configInputFormat.load (std::memory_order_relaxed),
                                        juce::dontSendNotification);
    inputFormatBox.onChange = [this] {
        processorRef.configInputFormat.store (inputFormatBox.getSelectedItemIndex(), std::memory_order_relaxed);
        processorRef.markConfigStateDirty();
    };

    // Header dropdown labels: JetBrains Mono Medium 7.5px, wide kerning
    {
        auto headerLblFont2 = makeFont (osdLookAndFeel->jetbrainsMedium, 9.5f, 0.1f);
        auto headerLblColour2 = Colours_OSD::textDim;

        presetLabel.setText ("PRESET", juce::dontSendNotification);
        presetLabel.setFont (headerLblFont2);
        presetLabel.setColour (juce::Label::textColourId, headerLblColour2);
        presetLabel.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (presetLabel);

        inputFormatLabel.setText ("INPUT", juce::dontSendNotification);
        inputFormatLabel.setFont (headerLblFont2);
        inputFormatLabel.setColour (juce::Label::textColourId, headerLblColour2);
        inputFormatLabel.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (inputFormatLabel);
    }

    // --- v0.9: Preset browser — TextButton + PopupMenu (replaces ComboBox) ---
    presetNameButton.setLookAndFeel (&*osdLookAndFeel);
    presetNameButton.setComponentID ("presetButton");
    presetNameButton.setColour (juce::TextButton::buttonColourId, Colours_OSD::bgRecessed);
    presetNameButton.setColour (juce::TextButton::textColourOffId, Colours_OSD::textSecondary);
    presetNameButton.onClick = [this] { showPresetMenu(); };
    addAndMakeVisible (presetNameButton);
    updatePresetButtonText();

    auto stylePresetButton = [&] (juce::TextButton& btn, const juce::String& text)
    {
        btn.setButtonText (text);
        btn.setLookAndFeel (&*osdLookAndFeel);
        btn.setColour (juce::TextButton::buttonColourId, Colours_OSD::bgRecessed);
        btn.setColour (juce::TextButton::textColourOffId, Colours_OSD::textSecondary);
        addAndMakeVisible (btn);
    };

    stylePresetButton (presetPrevButton, "<");
    presetPrevButton.onClick = [this]
    {
        globalTapDrawer.resetToCenter();
        resetGlobalTapAPVTSParams();
        processorRef.loadPreviousPreset();
        updatePresetButtonText();
        processorRef.captureUndoState ("Load Preset");
    };

    stylePresetButton (presetNextButton, ">");
    presetNextButton.onClick = [this]
    {
        globalTapDrawer.resetToCenter();
        resetGlobalTapAPVTSParams();
        processorRef.loadNextPreset();
        updatePresetButtonText();
        processorRef.captureUndoState ("Load Preset");
    };

    stylePresetButton (presetSaveButton, "Save");
    presetSaveButton.onClick = [this]
    {
        // Pre-fill name if current preset is user-made
        int currentIdx = processorRef.getCurrentPresetIndex();
        auto cats = processorRef.getCategorizedPresets();
        juce::String existingName;
        for (const auto& cp : cats)
        {
            if (cp.originalIndex == currentIdx && ! cp.isFactory)
            {
                auto names = processorRef.getPresetNames();
                if (currentIdx < names.size())
                    existingName = names[currentIdx];
                break;
            }
        }

        presetSaveOverlay.onSave = [this] (const juce::String& name)
        {
            processorRef.saveUserPreset (name);
            updatePresetButtonText();
        };

        presetSaveOverlay.show (existingName, this);  // native popup window
    };

    // --- SML badge button (header branding link) ---
    smlButton = std::make_unique<StyledButton> ("SML", Colours_OSD::accentStellar,
                                                  osdLookAndFeel->robotoMedium);
    smlButton->setClickingTogglesState (false);
    smlButton->onClick = [] { juce::URL ("https://spatialmedialab.org").launchInDefaultBrowser(); };
    smlButton->setAlwaysActive (true);
    smlButton->setIcon (createSMLIconPath(), 9.0f / 250.0f, 9.0f);  // scaled icon
    smlButton->setFontSize (11.5f);
    smlButton->setButtonHeight (17);
    addAndMakeVisible (*smlButton);

    // Issue E15b/182: Internal undo/redo buttons (FabFilter-style)
    undoButton = std::make_unique<juce::TextButton> (juce::CharPointer_UTF8 ("\xe2\x86\xa9"));  // arrow left hook
    undoButton->setTooltip ("Undo");
    undoButton->setColour (juce::TextButton::buttonColourId, Colours_OSD::bgRecessed);
    undoButton->setColour (juce::TextButton::textColourOffId, Colours_OSD::textSecondary);
    undoButton->onClick = [this] { processorRef.performInternalUndo(); updatePresetButtonText(); updateUndoButtons(); };
    addAndMakeVisible (*undoButton);

    redoButton = std::make_unique<juce::TextButton> (juce::CharPointer_UTF8 ("\xe2\x86\xaa"));  // arrow right hook
    redoButton->setTooltip ("Redo");
    redoButton->setColour (juce::TextButton::buttonColourId, Colours_OSD::bgRecessed);
    redoButton->setColour (juce::TextButton::textColourOffId, Colours_OSD::textSecondary);
    redoButton->onClick = [this] { processorRef.performInternalRedo(); updatePresetButtonText(); updateUndoButtons(); };
    addAndMakeVisible (*redoButton);

    selectObject (0);
    resized();  // re-layout now that all unique_ptr components are constructed

    // Issue E15b/182: Capture initial state for undo baseline
    processorRef.captureUndoState ("Initial State");
    updateUndoButtons();

    startTimerHz (30);
}

OpenSpatialDelayEditor::~OpenSpatialDelayEditor()
{
    presetSaveOverlay.dismiss();  // tear down native window before editor destruction
    spatialMap.removeListener (this);
    setLookAndFeel (nullptr);
    stopTimer();
}

void OpenSpatialDelayEditor::visibilityChanged()
{
    if (isVisible())
        startTimerHz (30);
    else
        startTimerHz (5);    // keep param sync alive, skip rendering
}

//==============================================================================
// Issue E15b/182: Internal undo/redo support
//==============================================================================
void OpenSpatialDelayEditor::updateUndoButtons()
{
    if (undoButton)
    {
        undoButton->setEnabled (processorRef.pluginUndo.canUndo());
        undoButton->setAlpha (processorRef.pluginUndo.canUndo() ? 1.0f : 0.3f);
    }
    if (redoButton)
    {
        redoButton->setEnabled (processorRef.pluginUndo.canRedo());
        redoButton->setAlpha (processorRef.pluginUndo.canRedo() ? 1.0f : 0.3f);
    }
}

bool OpenSpatialDelayEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier, 0))
    {
        processorRef.performInternalUndo();
        updatePresetButtonText();
        updateUndoButtons();
        return true;
    }
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        processorRef.performInternalRedo();
        updatePresetButtonText();
        updateUndoButtons();
        return true;
    }
    return false;
}

//==============================================================================
// v0.9: Preset browser helpers (PopupMenu + category submenus)
//==============================================================================
void OpenSpatialDelayEditor::showPresetMenu()
{
    auto presets = processorRef.getCategorizedPresets();
    juce::PopupMenu mainMenu;

    juce::String lastCategory;
    juce::PopupMenu currentSubMenu;
    bool hasFactoryInSub = false;
    bool hasUserInSub = false;

    auto flushSubMenu = [&]()
    {
        if (lastCategory.isNotEmpty() && (hasFactoryInSub || hasUserInSub))
            mainMenu.addSubMenu (lastCategory, currentSubMenu);
        currentSubMenu = juce::PopupMenu();
        hasFactoryInSub = false;
        hasUserInSub = false;
    };

    for (const auto& p : presets)
    {
        if (p.category != lastCategory)
        {
            flushSubMenu();
            lastCategory = p.category;
        }

        // Separator between factory and user presets within a category
        if (! p.isFactory && ! hasUserInSub && hasFactoryInSub)
            currentSubMenu.addSeparator();

        bool isCurrent = (p.originalIndex == processorRef.getCurrentPresetIndex());
        currentSubMenu.addItem (p.originalIndex + 1,  // PopupMenu IDs are 1-based
                                p.name,
                                true,       // enabled
                                isCurrent); // ticked if current

        if (p.isFactory) hasFactoryInSub = true;
        else             hasUserInSub = true;
    }
    flushSubMenu();  // flush last category

    // v0.9: Always show "User" category even if empty (Issue #2)
    if (lastCategory != "User")
    {
        bool userFound = false;
        for (const auto& p : presets)
            if (p.category == "User") { userFound = true; break; }
        if (! userFound)
        {
            juce::PopupMenu emptyUserMenu;
            emptyUserMenu.addItem (-1, "(empty)", false);
            mainMenu.addSubMenu ("User", emptyUserMenu);
        }
    }

    mainMenu.setLookAndFeel (&*osdLookAndFeel);
    mainMenu.showMenuAsync (
        juce::PopupMenu::Options()
            .withTargetComponent (&presetNameButton)
            .withMinimumWidth (160)
            .withPreferredPopupDirection (
                juce::PopupMenu::Options::PopupDirection::downwards),
        [this] (int result)
        {
            if (result > 0)
            {
                globalTapDrawer.resetToCenter();
                resetGlobalTapAPVTSParams();
                processorRef.loadPreset (result - 1);
                updatePresetButtonText();
                processorRef.captureUndoState ("Load Preset");
            }
        });
}

void OpenSpatialDelayEditor::updatePresetButtonText()
{
    int idx = processorRef.getCurrentPresetIndex();
    auto names = processorRef.getPresetNames();
    if (idx >= 0 && idx < names.size())
        presetNameButton.setButtonText (names[idx]);
    else
        presetNameButton.setButtonText ("Preset...");
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
    objPitchShiftAttach.reset();
    objInputChannelAttach.reset();
    objTrajectoryAttach.reset();
    objTrajectorySpeedAttach.reset();
    objTrajectoryDirAttach.reset();

    auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
    objAzAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "azimuth",   objAzimuthSlider);
    objElAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "elevation", objElevationSlider);
    objDistAttach    = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "distance",  objDistanceSlider);
    objEnabledAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, prefix + "enabled",   *objEnabledButton);
    objDopplerAttach = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "dopplerAmount", objDopplerSlider);
    objPitchShiftAttach      = std::make_unique<SliderAttachment> (processorRef.apvts, prefix + "pitchShift", objPitchShiftSlider);
    objInputChannelAttach    = std::make_unique<ComboBoxAttachment> (processorRef.apvts, prefix + "inputChannel", objInputChannelBox);

    // Issue E15b: Force host state capture after per-object slider drag gestures
    auto makeDragStart = [this] { return [this] { lastSliderDragStartTime = juce::Time::getMillisecondCounterHiRes(); }; };
    auto makeDragEnd = [this] (const char* name) {
        return [this, name] {
            double elapsed = juce::Time::getMillisecondCounterHiRes() - lastSliderDragStartTime;
            if (elapsed > 20.0)
            {
                processorRef.notifyHostStateChanged();
                processorRef.captureUndoState (juce::String ("Adjust ") + name);
            }
        };
    };
    for (auto* s : std::initializer_list<juce::Slider*> { &objAzimuthSlider, &objElevationSlider, &objDistanceSlider,
                     &objDopplerSlider, &objPitchShiftSlider, &objTrajectorySpeedSlider })
    {
        s->onDragStart = makeDragStart();
        s->onDragEnd   = makeDragEnd ("perObject");
    }
    // Issue #182: Manual sync replaces ComboBoxAttachment (auto-gestures fail in Ableton)
    {
        auto* raw = processorRef.apvts.getRawParameterValue (prefix + "trajectoryShape");
        if (raw)
            objTrajectoryBox.setSelectedId (static_cast<int> (raw->load()) + 1, juce::dontSendNotification);
    }
    objTrajectorySpeedAttach = std::make_unique<SliderAttachment>   (processorRef.apvts, prefix + "trajectorySpeed", objTrajectorySpeedSlider);
    objTrajectoryDirAttach   = std::make_unique<ComboBoxAttachment> (processorRef.apvts, prefix + "trajectoryDirection", objTrajectoryDirBox);

    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
        objectButtons[(size_t)i].setToggleState (i == currentObjectIndex, juce::dontSendNotification);

    // Set ALL bottom panel controls to match the selected object's tap color
    auto objCol = SpatialMapComponent::objectColours[currentObjectIndex];
    objEnabledButton->setAccentColour (objCol);
    objAzimuthSlider.setColour  (juce::Slider::thumbColourId, objCol);
    objElevationSlider.setColour (juce::Slider::thumbColourId, objCol);
    objDistanceSlider.setColour (juce::Slider::thumbColourId, objCol);
    objDopplerSlider.setColour (juce::Slider::thumbColourId, objCol);
    objPitchShiftSlider.setColour (juce::Slider::thumbColourId, objCol);
    objTrajectorySpeedSlider.setColour (juce::Slider::thumbColourId, objCol);

    // Trajectory dropdown — outline and text match tap color
    objTrajectoryBox.setColour (juce::ComboBox::outlineColourId, objCol.withAlpha (0.5f));
    objTrajectoryBox.setColour (juce::ComboBox::textColourId, objCol);
    objTrajectoryBox.setColour (juce::ComboBox::arrowColourId, objCol.withAlpha (0.6f));

    // v0.8: Update trajectory direction button highlights
    objTrajectoryFwdButton->setAccentColour (objCol);
    objTrajectoryRevButton->setAccentColour (objCol);
    {
        int dir = static_cast<int> (processorRef.apvts.getRawParameterValue (prefix + "trajectoryDirection")->load());
        objTrajectoryFwdButton->setToggleState (dir == 0, juce::dontSendNotification);
        objTrajectoryRevButton->setToggleState (dir == 1, juce::dontSendNotification);
    }

    // v0.8: Update input channel button text + color per mode
    {
        int ich = static_cast<int> (processorRef.apvts.getRawParameterValue (prefix + "inputChannel")->load());
        juce::String labels[] = { "L+R", "L", "R" };
        objInputChannelButton->setLabel (labels[juce::jlimit (0, 2, ich)]);
        if (ich == 1)
            objInputChannelButton->setAccentColour (Colours_OSD::accentChannelL);
        else if (ich == 2)
            objInputChannelButton->setAccentColour (Colours_OSD::accentChannelR);
        else
            objInputChannelButton->setAccentColour (objCol);

        // Visibility: only show when Input Format = Stereo (index 1)
        int inputFmt = processorRef.configInputFormat.load (std::memory_order_relaxed);
        objInputChannelButton->setVisible (inputFmt == 1);
    }

    spatialMap.setSelectedObject (currentObjectIndex);
    updateObjectButtonColours();
}

void OpenSpatialDelayEditor::updateObjectButtonColours()
{
    // Observatory v6: 4-state tap buttons (enabled/disabled × selected/unselected)
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto& btn = objectButtons[(size_t)i];
        auto colour = SpatialMapComponent::objectColours[i];

        auto prefix = "object" + juce::String (i + 1) + "_";
        bool enabled = false;
        if (auto* p = processorRef.apvts.getRawParameterValue (prefix + "enabled"))
            enabled = p->load() > 0.5f;

        bool selected = (i == currentObjectIndex);

        juce::Colour bg, border, text;

        if (enabled && selected)
        {
            // Enabled + Selected: bright tap color bg, tap color border, primary text
            bg     = colour.withAlpha (0.4f);
            border = colour;
            text   = Colours_OSD::textPrimary;
        }
        else if (enabled && !selected)
        {
            // Enabled + Unselected: dim tap color bg, dim border, tap color text
            bg     = colour.withAlpha (0.25f);
            border = colour.withAlpha (0.4f);
            text   = colour;
        }
        else if (!enabled && selected)
        {
            // Disabled + Selected: recessed bg, tap color border, tap color text
            bg     = Colours_OSD::bgRecessed;
            border = colour;
            text   = colour;
        }
        else
        {
            // Disabled + Unselected: recessed bg, dim border, dim text
            bg     = Colours_OSD::bgRecessed;
            border = Colours_OSD::borderDim;
            text   = Colours_OSD::textDim;
        }

        btn.setColour (juce::TextButton::buttonColourId,   bg);
        btn.setColour (juce::TextButton::buttonOnColourId,  bg);
        btn.setColour (juce::TextButton::textColourOffId,  text);
        btn.setColour (juce::TextButton::textColourOnId,   text);
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

//==============================================================================
// v1.0: Global Tap Delta Application (IEM MultiEncoder pattern)
//==============================================================================
void OpenSpatialDelayEditor::applyGlobalTapDelta (int knobIndex, float delta)
{
    static const char* suffixes[] = {
        "azimuth", "elevation", "distance",
        "pitchShift", "dopplerAmount", "trajectorySpeed"
    };
    if (knobIndex < 0 || knobIndex >= 6) return;
    const bool wraps = (knobIndex == GlobalTapDrawerComponent::kAzimuth);

    // Scale factors: knob display range → APVTS denormalized range
    // Doppler: knob shows -100..+100 (%), APVTS is 0..1 → scale by 0.01
    static const float scaleFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.01f, 1.0f };
    float scaledDelta = delta * scaleFactors[knobIndex];

    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        // Only affect enabled taps
        auto enabledId = "object" + juce::String (i + 1) + "_enabled";
        if (processorRef.apvts.getRawParameterValue (enabledId)->load() < 0.5f)
            continue;

        auto paramId = "object" + juce::String (i + 1) + "_" + suffixes[knobIndex];
        auto* param = processorRef.apvts.getParameter (paramId);
        if (param == nullptr) continue;

        // Read current denormalized value, add delta
        float current = param->convertFrom0to1 (param->getValue());
        float newVal = current + scaledDelta;

        // Azimuth wrapping
        if (wraps)
        {
            newVal = wrapAzimuth (newVal);
        }

        // convertTo0to1 handles NormalisableRange clamping for non-wrapping params
        param->setValueNotifyingHost (param->convertTo0to1 (newVal));
    }
}

// issue #95: Zero out APVTS global tap params + processor atomics on preset change
void OpenSpatialDelayEditor::resetGlobalTapAPVTSParams()
{
    static const char* tapParamIds[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                         "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
    // Issue E15: Wrap in gestures so the host groups the reset as one undo entry
    std::array<juce::RangedAudioParameter*, 6> gestures {};
    for (int i = 0; i < OpenSpatialDelayProcessor::kNumGlobalTapOffsets; ++i)
    {
        gestures[static_cast<size_t> (i)] = processorRef.apvts.getParameter (tapParamIds[i]);
        if (gestures[static_cast<size_t> (i)] != nullptr)
            gestures[static_cast<size_t> (i)]->beginChangeGesture();
    }
    for (int i = 0; i < OpenSpatialDelayProcessor::kNumGlobalTapOffsets; ++i)
    {
        processorRef.globalTapOffset[i].store (0.0f, std::memory_order_relaxed);
        if (gestures[static_cast<size_t> (i)] != nullptr)
            gestures[static_cast<size_t> (i)]->setValueNotifyingHost (gestures[static_cast<size_t> (i)]->convertTo0to1 (0.0f));
    }
    for (int i = 0; i < OpenSpatialDelayProcessor::kNumGlobalTapOffsets; ++i)
    {
        if (gestures[static_cast<size_t> (i)] != nullptr)
            gestures[static_cast<size_t> (i)]->endChangeGesture();
    }
}

void OpenSpatialDelayEditor::syncGlobalTapOffsetsFromOSC()
{
    for (int i = 0; i < GlobalTapDrawerComponent::kNumKnobs; ++i)
    {
        float oscVal = processorRef.globalTapOffset[i].load (std::memory_order_relaxed);
        float curVal = globalTapDrawer.getKnobValue (i);
        float delta  = oscVal - curVal;

        // Azimuth wrapping
        if (i == GlobalTapDrawerComponent::kAzimuth)
        {
            delta = unwrapAzimuthDelta (delta);
        }

        if (std::abs (delta) > 1e-6f)
        {
            applyGlobalTapDelta (i, delta);
            globalTapDrawer.setKnobValueSilent (i, oscVal);
        }
    }
}

// issue #164: Sync global tap knobs from APVTS on host undo / external param change.
// Unlike syncGlobalTapOffsetsFromOSC(), this does NOT call applyGlobalTapDelta() —
// per-object params are already correct after undo, we only need the UI + atomics.
void OpenSpatialDelayEditor::syncGlobalTapKnobsFromAPVTS()
{
    static const char* tapParamIds[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                         "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
    for (int i = 0; i < GlobalTapDrawerComponent::kNumKnobs; ++i)
    {
        auto* rawParam = processorRef.apvts.getRawParameterValue (tapParamIds[i]);
        if (rawParam == nullptr) continue;
        float apvtsVal = rawParam->load();
        float knobVal  = globalTapDrawer.getKnobValue (i);
        if (std::abs (apvtsVal - knobVal) > 1e-6f)
        {
            globalTapDrawer.setKnobValueSilent (i, apvtsVal);
            processorRef.globalTapOffset[i].store (apvtsVal, std::memory_order_relaxed);
        }
    }
}

void OpenSpatialDelayEditor::syncForScreenshot()
{
    updateMapFromParameters();
    timerCallback();
}

void OpenSpatialDelayEditor::configureGlobalDrawer (bool open, const float* knobValues, int numKnobs)
{
    globalTapDrawer.setOpen (open);
    processorRef.setGlobalDrawerOpen (open);
    for (int i = 0; i < numKnobs && i < GlobalTapDrawerComponent::kNumKnobs; ++i)
        globalTapDrawer.setKnobValueSilent (i, knobValues[i]);
}

void OpenSpatialDelayEditor::applyShowcaseHeaderForScreenshot()
{
    // Issue #168 round 2: after the tool has written directly to
    // processor.configOutputFormat / configAlgorithm, refresh the header
    // combo boxes so the snapshot reflects the new selection. The output-
    // format combo needs an explicit setSelectedItemIndex (it's only
    // initialised from processor state in the constructor), and the
    // algorithm combo needs its category list rebuilt — force this by
    // invalidating lastAlgoCategoryShown so timerCallback repopulates it.
    outputFormatBox.setSelectedItemIndex (processorRef.configOutputFormat.load (std::memory_order_relaxed),
                                          juce::dontSendNotification);
    lastAlgoCategoryShown = -1;

    // Issue #168 round 3: the OSC Receive / Send toggle buttons only read
    // the processor state once, in the editor constructor. By the time
    // applyShowcaseState() has flipped Receive off and Send on, the
    // buttons still carry their construction-time state — so the snapshot
    // shows the wrong lit/dim pattern. Re-pull processor state here so
    // the buttons reflect the showcase.
    if (oscToggleButton     != nullptr)
        oscToggleButton->setToggleState     (processorRef.getOscReceiveEnabled(), juce::dontSendNotification);
    if (oscSendToggleButton != nullptr)
        oscSendToggleButton->setToggleState (processorRef.getOscSendEnabled(),    juce::dontSendNotification);

    syncForScreenshot();
}

void OpenSpatialDelayEditor::setPresetNameForScreenshot (const juce::String& name)
{
    // Issue #168 round 3: manual override for the preset-name button. Used
    // by the screenshot tool's hero mode to show a custom label instead of
    // the name derived from getCurrentPresetIndex(). Call AFTER
    // syncForScreenshot() — the timerCallback inside that sync resets the
    // label from processorRef.getPresetNames(), so any override must run
    // last, right before snapshotComponent().
    presetNameButton.setButtonText (name);
}

juce::Rectangle<int> OpenSpatialDelayEditor::getToneSectionBoundsForScreenshot() const
{
    // TONE section spans: header (14px) + 6px pad + filter graph (104px) + 22px readout
    // Round-2 review (#168): padding tightened to 6px top/bottom so the filter
    // graph fills the frame and doesn't look small inside a large crop.
    // Round-3 review (#168 item 16): padBottom dropped to 0 to cut flush at
    // the readout — at 6px the top corner of MIX/AIR was bleeding in.
    // Horizontal 10px padding retained for breathing room.
    const int padTop    = 6;
    const int padBottom = 0;
    const int padX      = 10;
    int top    = toneHeaderY - padTop;
    int height = padTop + 14 + 6 + 104 + 22 + padBottom;
    int x      = juce::jmax (0, rpX - padX);
    int w      = juce::jmin (getWidth() - x, rpW + padX * 2);
    return { x, top, w, height };
}

juce::Rectangle<int> OpenSpatialDelayEditor::getOscSectionBoundsForScreenshot() const
{
    // OSC section: header at oscHeaderY + 16px gap + 2 x 20px control rows.
    // Add left breathing room so "OSC" isn't flush against the crop edge.
    const int padTop    = 8;
    const int padBottom = 8;
    const int padLeft   = 12;
    const int padRight  = 10;
    int top    = oscHeaderY - padTop;
    int height = padTop + 14 + 16 + 20 + 4 + 20 + padBottom;
    int x      = juce::jmax (0, rpX - padLeft);
    int w      = juce::jmin (getWidth() - x, rpW + padLeft + padRight);
    return { x, top, w, height };
}

juce::Rectangle<int> OpenSpatialDelayEditor::getPresetNameButtonBounds() const
{
    return presetNameButton.getBounds();
}

juce::Rectangle<int> OpenSpatialDelayEditor::getOutputFormatBoxBounds() const
{
    return outputFormatBox.getBounds();
}

void OpenSpatialDelayEditor::updateMapFromParameters()
{
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        auto state = processorRef.getObjectState (i);
        spatialMap.setObjectState (i, state.azimuthDeg, state.elevationDeg, state.distance, state.enabled);
        // v0.6: Pass OSC override state to spatial map for indicator
        spatialMap.setOscOverride (i, processorRef.isOscOverrideActive (i));
        // v0.9 PROTOTYPE: Pass trajectory state for origin marker + glow trail
        spatialMap.setTrajectoryState (i, processorRef.getTrajectoryState (i));
    }
}

void OpenSpatialDelayEditor::timerCallback()
{
    // issue #131: skip animation + repaint when editor is not visible
    if (isVisible())
    {
        float dt = 1.0f / 30.0f;
        spatialMap.advanceStarAnimation (dt);

        for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
        {
            float rms = processorRef.getTapActivityRMS (i);
            smoothedActivity[i] = rms * 0.7f + smoothedActivity[i] * 0.3f;
            spatialMap.setObjectActivityLevel (i, smoothedActivity[i]);
        }
        spatialMap.advancePulsePhases (dt);
        spatialMap.repaint();
    }

    updateMapFromParameters();
    updateObjectButtonColours();

    // v0.8: Show/hide input channel button based on Input Format (Mono=hide, Stereo=show)
    {
        int inputFmt = processorRef.configInputFormat.load (std::memory_order_relaxed);
        if (objInputChannelButton) objInputChannelButton->setVisible (inputFmt == 1);
    }

    // v0.8: Show/hide trajectory direction arrows — only when a trajectory is active (not None)
    {
        auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
        int trajShape = static_cast<int> (processorRef.apvts.getRawParameterValue (prefix + "trajectoryShape")->load());
        bool hasTraj = (trajShape > 0);  // 0 = None
        if (objTrajectoryFwdButton) objTrajectoryFwdButton->setVisible (hasTraj);
        if (objTrajectoryRevButton) objTrajectoryRevButton->setVisible (hasTraj);
        // Issue #182: Sync ComboBox from APVTS (no ComboBoxAttachment)
        if (objTrajectoryBox.getSelectedId() != trajShape + 1)
            objTrajectoryBox.setSelectedId (trajShape + 1, juce::dontSendNotification);
    }

    // v0.8: MOD enable/disable — dim wobble knobs + labels when disabled
    {
        bool modActive = processorRef.apvts.getRawParameterValue ("wobbleEnabled")->load() > 0.5f;
        // Always apply on first timer tick (modIsActive starts false, but UI defaults to enabled look)
        bool forceUpdate = (wobbleAmountSlider.isEnabled() != modActive);
        if (modActive != modIsActive || forceUpdate)
        {
            modIsActive = modActive;
            modToggle->setToggleState (modIsActive, juce::dontSendNotification);
            float alpha = modActive ? 1.0f : 0.3f;
            wobbleAmountSlider.setEnabled (modActive);
            wobbleMorphSlider.setEnabled (modActive);
            wobbleAmountSlider.setAlpha (alpha);
            wobbleMorphSlider.setAlpha (alpha);
            wobbleAmountLabel.setAlpha (alpha);
            wobbleMorphLabel.setAlpha (alpha);
            repaint();
        }
    }

    // v0.8: Grey out all per-tap controls when the selected tap is disabled
    {
        auto prefix = "object" + juce::String (currentObjectIndex + 1) + "_";
        bool tapOn = processorRef.apvts.getRawParameterValue (prefix + "enabled")->load() > 0.5f;
        float tapAlpha = tapOn ? 1.0f : 0.3f;

        // Knobs + labels
        objAzimuthSlider.setEnabled (tapOn);        objAzimuthSlider.setAlpha (tapAlpha);
        objElevationSlider.setEnabled (tapOn);      objElevationSlider.setAlpha (tapAlpha);
        objDistanceSlider.setEnabled (tapOn);       objDistanceSlider.setAlpha (tapAlpha);
        objDopplerSlider.setEnabled (tapOn);        objDopplerSlider.setAlpha (tapAlpha);
        objPitchShiftSlider.setEnabled (tapOn);     objPitchShiftSlider.setAlpha (tapAlpha);
        objTrajectorySpeedSlider.setEnabled (tapOn); objTrajectorySpeedSlider.setAlpha (tapAlpha);
        objAzLabel.setAlpha (tapAlpha);
        objElLabel.setAlpha (tapAlpha);
        objDistLabel.setAlpha (tapAlpha);
        objDopplerLabel.setAlpha (tapAlpha);
        objPitchShiftLabel.setAlpha (tapAlpha);
        objTrajectorySpeedLabel.setAlpha (tapAlpha);

        // Trajectory dropdown + direction arrows + label
        objTrajectoryBox.setEnabled (tapOn);        objTrajectoryBox.setAlpha (tapAlpha);
        objTrajectoryLabel.setAlpha (tapAlpha);
        if (objTrajectoryFwdButton) { objTrajectoryFwdButton->setEnabled (tapOn);  objTrajectoryFwdButton->setAlpha (tapAlpha); }
        if (objTrajectoryRevButton) { objTrajectoryRevButton->setEnabled (tapOn);  objTrajectoryRevButton->setAlpha (tapAlpha); }

        // Input channel button
        if (objInputChannelButton) { objInputChannelButton->setEnabled (tapOn);   objInputChannelButton->setAlpha (tapAlpha); }
    }

    // v0.2: Bus-aware output format greying (v0.7: only re-check when bus count changes)
    int maxCh = processorRef.getMaxBusChannels();
    if (maxCh != lastMaxBusChannels)
    {
        lastMaxBusChannels = maxCh;
        for (int i = 0; i < OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS; ++i)
        {
            const auto& info = OpenSpatialDelayProcessor::outputFormatRegistry[static_cast<size_t> (i)];
            bool available = (info.requiredChannels <= maxCh);
            outputFormatBox.setItemEnabled (i + 1, available);
        }
    }

    // Issue E15b: Only re-resolve the output format from the timer when the host
    // changes the bus channel count (e.g., track routing change). User-driven format
    // changes now call requestOutputFormatChange() directly from outputFormatBox.onChange.
    // Polling every 30Hz tick created spurious layout activations after Undo/Redo that
    // corrupted the host undo stack.
    if (lastMaxBusChannels != processorRef.getMaxBusChannels())
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
    int algoIdx = processorRef.configAlgorithm.load (std::memory_order_relaxed);

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
        algorithmBox.setAlpha (0.5f);
        algorithmBox.setText ("Ambisonics Encode", juce::dontSendNotification);
        if (fmtCategory != lastAlgoCategoryShown)
        {
            lastAlgoCategoryShown = fmtCategory;
            algorithmBox.resized();   // re-trigger positionComboBoxText for full-width label
        }
    }
    else
    {
        algorithmBox.setVisible (true);
        algorithmLabel.setVisible (true);
        algorithmBox.setEnabled (true);
        algorithmBox.setAlpha (1.0f);

        // Rebuild combo items only when format category changes
        if (fmtCategory != lastAlgoCategoryShown)
        {
            lastAlgoCategoryShown = fmtCategory;
            algorithmBox.clear (juce::dontSendNotification);

            if (isStereoVariant)
            {
                // Stereo modes only — IDs match param indices + 1
                algorithmBox.addItem ("Equal Power",  8);   // param index 7
                algorithmBox.addItem ("Stereo VBAP",  9);   // param index 8
                algorithmBox.addItem ("XY Pair",      10);  // param index 9
                algorithmBox.addItem ("MS Encode",    11);  // param index 10
                algorithmBox.addItem ("Blumlein",     12);  // param index 11
            }
            else  // Surround
            {
                // Surround algorithms only — IDs match param indices + 1
                // Constant Power shown first (default for surround)
                algorithmBox.addItem ("Constant Power", 2); // param index 1
                algorithmBox.addItem ("Ambisonics",   1);   // param index 0
                algorithmBox.addItem ("DBAP",         3);   // param index 2
                algorithmBox.addItem ("KNN",          4);   // param index 3
                algorithmBox.addItem ("MDAP",         5);   // param index 4
                algorithmBox.addItem ("VBAP",         6);   // param index 5
                algorithmBox.addItem ("VBIP",         7);   // param index 6
            }

            algorithmBox.resized();   // restore arrow-reserved label width
        }

        // Validate algorithm against current output mode every tick
        // (preset loading can write an out-of-range value)
        if (isStereoVariant && algoIdx < 7)
        {
            algoIdx = 7;  // Equal Power
            processorRef.configAlgorithm.store (algoIdx, std::memory_order_relaxed);
            // Issue E15b: Do NOT call markConfigStateDirty() here — timer-driven
            // algorithm corrections must be silent. User-initiated format changes
            // already call markConfigStateDirty() from the combo box onChange handler.
            // Calling it here creates spurious updateHostDisplay() notifications that
            // corrupt the host undo stack after Undo/Redo operations.
        }
        else if (! isStereoVariant && algoIdx > 6)
        {
            algoIdx = 1;  // Constant Power (default for surround)
            processorRef.configAlgorithm.store (algoIdx, std::memory_order_relaxed);
        }

        // Sync combo selection from parameter (IDs are param index + 1)
        algorithmBox.setSelectedId (algoIdx + 1, juce::dontSendNotification);
    }

    // v0.6: Sync OSC port label from processor (v0.7: skip string compare when port unchanged)
    {
        int currentPort = processorRef.getOscReceivePort();
        if (currentPort != lastOscPort && ! oscPortLabel.isBeingEdited())
        {
            lastOscPort = currentPort;
            oscPortLabel.setText (juce::String (currentPort), juce::dontSendNotification);
        }
    }

    // v0.9: Sync preset name button text from processor
    // Issue #182: Also force sync after host state restore (Undo/Redo)
    if (processorRef.stateJustRestored.exchange (false, std::memory_order_relaxed))
        updatePresetButtonText();
    {
        int idx = processorRef.getCurrentPresetIndex();
        auto names = processorRef.getPresetNames();
        if (idx >= 0 && idx < names.size())
        {
            juce::String expected = names[idx];
            if (presetNameButton.getButtonText() != expected)
                presetNameButton.setButtonText (expected);
        }
    }

    // v0.9: Sync filter graph from parameters — always show live values (WYSIWYG)
    {
        float hp  = processorRef.apvts.getRawParameterValue ("filterHP")->load();
        float lp  = processorRef.apvts.getRawParameterValue ("filterLP")->load();
        float hpq = processorRef.apvts.getRawParameterValue ("filterHPQ")->load();
        float lpq = processorRef.apvts.getRawParameterValue ("filterLPQ")->load();

        filterIsActive = processorRef.apvts.getRawParameterValue ("filterEnabled")->load() > 0.5f;
        filterGraph.setEnabled (filterIsActive);
        if (fltToggle) fltToggle->setToggleState (filterIsActive, juce::dontSendNotification);

        // Issue #182: Sync Air button visual state (no ButtonAttachment — manual sync)
        if (airAbsorptionButton)
        {
            bool airActive = processorRef.apvts.getRawParameterValue ("airAbsorption")->load() > 0.5f;
            airAbsorptionButton->setToggleState (airActive, juce::dontSendNotification);
        }

        // Issue E15b: Sync tempoSync button visual state + UI (no ButtonAttachment)
        if (tempoSyncButton && tempoSyncUpdateUI)
        {
            bool synced = processorRef.apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
            if (tempoSyncButton->getToggleState() != synced)
            {
                tempoSyncButton->setToggleState (synced, juce::dontSendNotification);
                tempoSyncUpdateUI();
            }
        }

        // Always update graph with live values — dimming handles on/off visual
        filterGraph.setFrequencies (hp, lp);
        filterGraph.setQ (hpq, lpq);
    }

    // v0.7: Sync dotted/triplet buttons from syncMode parameter
    {
        int mode = static_cast<int> (processorRef.apvts.getRawParameterValue ("syncMode")->load());
        if (syncDottedButton)  syncDottedButton->setToggleState  (mode == 1, juce::dontSendNotification);
        if (syncTripletButton) syncTripletButton->setToggleState (mode == 2, juce::dontSendNotification);
    }

    // v1.0: Sync global tap offset knobs from OSC receive (processor → editor)
    if (processorRef.globalTapOffsetChanged.exchange (false, std::memory_order_relaxed))
    {
        syncGlobalTapOffsetsFromOSC();
    }

    // issue #164: Sync knobs from APVTS on host undo / external param change
    syncGlobalTapKnobsFromAPVTS();

    // Issue E15b/182: Refresh undo/redo button state
    updateUndoButtons();

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
        g.setColour (Colours_OSD::selectionBox);
        constexpr float len = 4.0f, t = 1.0f;
        float l = bounds.getX(), r = bounds.getRight(), top = bounds.getY(), bot = bounds.getBottom();
        // Four corner indicators (horizontal + vertical line each)
        for (auto [cx, cy, dx, dy] : { std::tuple{l,top,1.f,1.f}, {r,top,-1.f,1.f},
                                         {l,bot,1.f,-1.f}, {r,bot,-1.f,-1.f} })
        {
            g.drawLine (cx, cy, cx + len * dx, cy, t);
            g.drawLine (cx, cy, cx, cy + len * dy, t);
        }
    }
}

void OpenSpatialDelayEditor::drawSectionHeader (juce::Graphics& g, int x, int y, int w, const juce::String& text)
{
    // Observatory v6: JetBrains Mono Bold, wide tracking
    g.setColour (Colours_OSD::textDim);
    juce::Font headerFont = makeFont (osdLookAndFeel->jetbrainsBold, 11.0f, 0.15f);
    g.setFont (headerFont);
    g.drawText (text, x, y, w, 12, juce::Justification::centredLeft);
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (headerFont, text, 0.0f, 0.0f);
    int textWidth = (int) glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth() + 6;
    g.setColour (Colours_OSD::borderSubtle);
    g.drawLine ((float)(x + textWidth), (float)(y + 6), (float)(x + w), (float)(y + 6), 1.0f);
}

//==============================================================================
// Paint
//==============================================================================
void OpenSpatialDelayEditor::paint (juce::Graphics& g)
{
    // === Observatory v6: Panel backgrounds and borders ===
    g.fillAll (Colours_OSD::bgVoid);

    // Header bar — full width, 52px
    g.setColour (Colours_OSD::bgHeader);
    g.fillRect (0, 0, getWidth(), kHeaderHeight);
    // Header bottom border
    g.setColour (Colours_OSD::borderSubtle);
    g.drawLine (0.0f, (float) kHeaderHeight, (float) getWidth(), (float) kHeaderHeight, 1.0f);
    // Subtle stellar glow on header border
    {
        juce::ColourGradient glow (Colours_OSD::accentStellar.withAlpha (0.0f), 0.0f, (float) kHeaderHeight,
                                   Colours_OSD::accentStellar.withAlpha (0.0f), (float) getWidth(), (float) kHeaderHeight,
                                   false);
        glow.addColour (0.2, Colours_OSD::accentStellar.withAlpha (0.12f));
        glow.addColour (0.5, Colours_OSD::accentStellar.withAlpha (0.2f));
        glow.addColour (0.8, Colours_OSD::accentStellar.withAlpha (0.12f));
        g.setGradientFill (glow);
        g.fillRect (0.0f, (float) kHeaderHeight - 1.0f, (float) getWidth(), 2.0f);
    }

    // Right panel — full height below header, bg fill + left border
    int rpLeft = getWidth() - kRightPanelWidth;
    g.setColour (Colours_OSD::bgPanel);
    g.fillRect (rpLeft, kHeaderHeight, kRightPanelWidth, getHeight() - kHeaderHeight);
    g.setColour (Colours_OSD::borderDim);
    g.drawLine ((float) rpLeft, (float) kHeaderHeight, (float) rpLeft, (float) getHeight(), 1.0f);

    // Bottom panel — column 1 only, bg fill + top border + right border
    int bpTop = getHeight() - kBottomPanelHeight;
    int bpRight = rpLeft;
    g.setColour (Colours_OSD::bgPanel);
    g.fillRect (0, bpTop, bpRight, kBottomPanelHeight);
    g.setColour (Colours_OSD::borderDim);
    g.drawLine (0.0f, (float) bpTop, (float) bpRight, (float) bpTop, 1.0f);
    g.drawLine ((float) bpRight, (float) bpTop, (float) bpRight, (float) getHeight(), 1.0f);

    // Title — SML button self-paints via StyledButton::paintButton() + "OpenSpatialDelay" + version
    {
        int leftX = 10;  // prototype: padding 0 10px
        // Compute SML button width from actual content (icon + gap + text at real font)
        auto smlFont = makeFont (osdLookAndFeel->robotoMedium, 11.5f, 0.08f);
        juce::GlyphArrangement smlGl;
        smlGl.addLineOfText (smlFont, "SML", 0.0f, 0.0f);
        float smlTextW = smlGl.getBoundingBox (0, smlGl.getNumGlyphs(), true).getWidth();
        float smlVPad = (17.0f - 11.5f) * 0.5f;  // vertical padding ≈ 2.75px
        int smlW = juce::roundToInt (9.0f + 3.0f + smlTextW + 2.0f * smlVPad);  // icon + gap + text + matched H pad

        // SML button self-paints via StyledButton::paintButton()

        // Plugin title — DM Sans Medium 14px
        int titleX = leftX + smlW + 6;
        g.setColour (Colours_OSD::textPrimary);
        auto titleFont = makeFont (osdLookAndFeel->dmSansMedium, 17.0f, 0.02f);
        g.setFont (titleFont);
        juce::String titleText = "OpenSpatialDelay";
        juce::GlyphArrangement titleGlyphs;
        titleGlyphs.addLineOfText (titleFont, titleText, 0.0f, 0.0f);
        float titleTextW = titleGlyphs.getBoundingBox (0, titleGlyphs.getNumGlyphs(), true).getWidth();
        g.drawText (titleText, titleX, 0, (int) titleTextW + 4, kHeaderHeight,
                    juce::Justification::centredLeft);

        // "v1.0" dim version tag — JetBrains Mono 9px
        int versionX = titleX + (int) titleTextW + 4;
        g.setColour (Colours_OSD::textDim);
        g.setFont (makeFont (osdLookAndFeel->jetbrainsRegular, 11.0f));
        g.drawText ("v1.0", versionX, 0, 40, kHeaderHeight,
                    juce::Justification::centredLeft);
    }

    // --- Header separator lines between dropdown groups ---
    {
        int sepH = 24;
        int sepY = kHeaderHeight / 2 - sepH / 2;
        g.setColour (Colours_OSD::borderDim);

        // Separator after Preset Save button (before flex spacer)
        int presetRight = presetSaveButton.getRight();
        if (presetRight > 0)
            g.fillRect (presetRight + 4, sepY, 1, sepH);

        // Separator before Input Format group
        int inputLeft = inputFormatBox.getX();
        if (inputLeft > 0)
            g.fillRect (inputLeft - 4, sepY, 1, sepH);

        // Separator before Output Format group
        int outputLeft = outputFormatBox.getX();
        if (outputLeft > 0)
            g.fillRect (outputLeft - 4, sepY, 1, sepH);

        // Separator before Algorithm/Profile group
        int algoLeft = algorithmBox.isVisible() ? algorithmBox.getX()
                     : hrtfProfileBox.isVisible() ? hrtfProfileBox.getX() : 0;
        if (algoLeft > 0)
            g.fillRect (algoLeft - 4, sepY, 1, sepH);
    }

    // --- Section headers (positions computed in resized) ---
    drawSectionHeader (g, rpX, delayHeaderY,  rpW, "DELAY");
    drawSectionHeader (g, rpX, modHeaderY,    rpW - (modToggle ? modToggle->getWidth() + 4 : 46), "MOD");
    drawSectionHeader (g, rpX, toneHeaderY,   rpW - (fltToggle ? fltToggle->getWidth() + 4 : 42), "TONE");
    drawSectionHeader (g, rpX, mixHeaderY,    rpW - (airAbsorptionButton ? airAbsorptionButton->getWidth() + 4 : 42), "MIX");
    drawSectionHeader (g, rpX, oscHeaderY,    rpW, "OSC");

    // Toggle pills (IndicatorToggle instances) self-paint via paintButton() — no manual calls needed

    // ON/OFF pill — now self-paints via IndicatorToggle::paintButton()

    // Input channel button self-paints via StyledButton::paintButton()
    // v0.8: "INPUT" label above input channel cycling button
    if (objInputChannelButton && objInputChannelButton->isVisible())
    {
        auto btnBounds = objInputChannelButton->getBounds();
        g.setColour (Colours_OSD::textDim);
        g.setFont (makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.12f));
        g.drawText ("INPUT", btnBounds.getX(), btnBounds.getY() - 14, btnBounds.getWidth(), 12,
                    juce::Justification::centred);
    }

    // SYNC/TIME button self-paints via StyledButton::paintButton()

    // Dotted/Triplet buttons self-paint via StyledButton::paintButton()

    // Trajectory direction arrow buttons self-paint via StyledButton::paintButton()
    // v0.8: "DIR" label above trajectory direction buttons
    if (objTrajectoryRevButton && objTrajectoryRevButton->isVisible())
    {
        auto revBounds = objTrajectoryRevButton->getBounds();
        auto fwdBounds = objTrajectoryFwdButton->getBounds();
        int dirLabelW = fwdBounds.getRight() - revBounds.getX();
        g.setColour (Colours_OSD::textDim);
        g.setFont (makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.12f));
        g.drawText ("DIRECTION", revBounds.getX(), revBounds.getY() - 14, dirLabelW, 12,
                    juce::Justification::centred);
    }

    // FLT and MOD toggles self-paint via IndicatorToggle::paintButton()

    // "Port" label for Receive row
    if (oscToggleButton)
    {
        auto oscBounds = oscToggleButton->getBounds().toFloat();
        if (oscBounds.getWidth() > 0)
        {
            g.setColour (Colours_OSD::textDim);
            g.setFont (makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.08f));
            g.drawText ("Port", (int)(oscBounds.getRight() + 4), (int)oscBounds.getY(),
                        28, (int)oscBounds.getHeight(), juce::Justification::centredLeft);
        }
    }

    // "IP" and "Port" labels for Send row
    if (oscSendToggleButton)
    {
        auto sendBounds = oscSendToggleButton->getBounds().toFloat();
        if (sendBounds.getWidth() > 0)
        {
            g.setColour (Colours_OSD::textDim);
            g.setFont (makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.08f));
            g.drawText ("IP", (int)(sendBounds.getRight() + 4), (int)sendBounds.getY(),
                        14, (int)sendBounds.getHeight(), juce::Justification::centredLeft);
            auto ipBounds = oscSendIPLabel.getBounds().toFloat();
            g.drawText ("Port", (int)(ipBounds.getRight() + 4), (int)ipBounds.getY(),
                        28, (int)ipBounds.getHeight(), juce::Justification::centredLeft);
        }
    }

    // v0.9: Filter readout — always shows live values (WYSIWYG), dims when disabled
    {
        float hp  = processorRef.apvts.getRawParameterValue ("filterHP")->load();
        float lp  = processorRef.apvts.getRawParameterValue ("filterLP")->load();
        float hpq = processorRef.apvts.getRawParameterValue ("filterHPQ")->load();
        float lpq = processorRef.apvts.getRawParameterValue ("filterLPQ")->load();
        auto hpStr = (hp >= 1000.0f) ? juce::String (hp / 1000.0f, 1) + "k"
                                     : juce::String (juce::roundToInt (hp));
        auto lpStr = (lp >= 1000.0f) ? juce::String (lp / 1000.0f, 1) + "k"
                                     : juce::String (juce::roundToInt (lp));
        auto filterBounds = filterGraph.getBounds();
        float readoutAlpha = filterIsActive ? 1.0f : 0.3f;
        g.setColour (Colours_OSD::textDim.withAlpha (readoutAlpha));
        g.setFont (makeFont (osdLookAndFeel->jetbrainsRegular, 11.0f, 0.04f));
        g.drawText ("HP " + hpStr + "  Res " + juce::String (hpq, 2)
                    + "    LP " + lpStr + "  Res " + juce::String (lpq, 2),
                    filterBounds.getX(), filterBounds.getBottom() + 2,
                    filterBounds.getWidth(), 12, juce::Justification::centred);
    }
}

//==============================================================================
// Layout
//==============================================================================
void OpenSpatialDelayEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (kHeaderHeight);

    // === HEADER: [Title block] [Preset group] ... [Input|Output|Profile] right-aligned ===
    const int hPad = 8, hGap = 6;
    const int algoBoxW = 110, outBoxW = 110, inBoxW = 70;
    const int lblH = 12, boxH = 22;
    int lblY = header.getY() + 8;
    int boxY = lblY + lblH + 2;

    // --- Right-aligned dropdown groups (Algorithm/Profile, Output, Input) ---
    int rx = header.getRight() - hPad;

    // Algorithm / HRTF Profile (rightmost)
    rx -= algoBoxW;
    algorithmLabel.setBounds   (rx, lblY, algoBoxW, lblH);
    algorithmBox.setBounds     (rx, boxY, algoBoxW, boxH);
    hrtfProfileLabel.setBounds (rx, lblY, algoBoxW, lblH);
    hrtfProfileBox.setBounds   (rx, boxY, algoBoxW, boxH);
    rx -= hGap;

    // Output Format
    rx -= outBoxW;
    outputFormatLabel.setBounds (rx, lblY, outBoxW, lblH);
    outputFormatBox.setBounds   (rx, boxY, outBoxW, boxH);
    rx -= hGap;

    // v0.7: Input Format
    rx -= inBoxW;
    inputFormatLabel.setBounds (rx, lblY, inBoxW, lblH);
    inputFormatBox.setBounds (rx, boxY, inBoxW, boxH);

    // --- Left-aligned preset group (after title block) ---
    // Title block: [10px pad][28px SML][6px][~155px title][4px][~35px v0.7] ≈ 238px
    int presetStartX = 240;
    presetLabel.setBounds (presetStartX, lblY, 120, lblH);
    presetNameButton.setBounds (presetStartX, boxY, 120, boxH);
    int px = presetStartX + 120 + 2;
    presetPrevButton.setBounds (px, boxY, 22, boxH);
    px += 22;
    presetNextButton.setBounds (px, boxY, 22, boxH);
    px += 22 + 2;
    presetSaveButton.setBounds (px, boxY, 40, boxH);

    // Issue E15b/182: Undo/Redo buttons in header (after preset save)
    {
        int undoX = px + 40 + 8;
        if (undoButton)  undoButton->setBounds  (undoX, boxY, 22, boxH);
        if (redoButton)  redoButton->setBounds  (undoX + 22, boxY, 22, boxH);
    }

    // Title is painted directly in paint() — no setBounds needed
    // SML badge button (header, left-aligned)
    {
        int smlH = smlButton ? smlButton->getEffectiveHeight() : StyledButton::kHeight;
        // Compute SML button width from actual content (icon + gap + text at real font)
        auto smlFont = makeFont (osdLookAndFeel->robotoMedium, 11.5f, 0.08f);
        juce::GlyphArrangement smlGl;
        smlGl.addLineOfText (smlFont, "SML", 0.0f, 0.0f);
        float smlTextW = smlGl.getBoundingBox (0, smlGl.getNumGlyphs(), true).getWidth();
        float smlVPad = (17.0f - 11.5f) * 0.5f;  // match vertical padding ≈ 2.75px
        int smlW = juce::roundToInt (9.0f + 3.0f + smlTextW + 2.0f * smlVPad);  // icon + gap + text + matched H pad
        int headerY = kHeaderHeight / 2;
        if (smlButton) smlButton->setBounds (10, headerY - smlH / 2, smlW, smlH);
    }

    // === RIGHT PANEL (Observatory v6: padding 16px H, 6px V) ==================
    auto rightPanel = area.removeFromRight (kRightPanelWidth).reduced (16, 6);

    rpX = rightPanel.getX();
    rpW = rightPanel.getWidth();
    int panelW = rpW;

    // Observatory v6: 72px knob columns, 8px gap
    int knobW = 72;
    int knobGap = 8;
    // 3-column layout for DELAY section
    int tripleW = knobW * 3 + knobGap * 2;
    int centerOff3 = (panelW - tripleW) / 2;
    int kx0 = rpX + centerOff3;
    int kx1 = kx0 + knobW + knobGap;
    int kx2 = kx1 + knobW + knobGap;
    // 2-column layout for MOD, MIX sections
    int pairW = knobW * 2 + knobGap;
    int centerOff2 = (panelW - pairW) / 2;
    int px0 = rpX + centerOff2;
    int px1 = px0 + knobW + knobGap;

    // placeKnob: 12px label + 54px slider (42px rotary + 12px text box) = 66px total
    auto placeKnob = [knobW](juce::Slider& slider, juce::Label& label, int x, int y) {
        label.setBounds (x, y, knobW, 12);
        slider.setBounds (x, y + 12, knobW, 54);
    };

    // --- DELAY section (3-knob single row: INPUT, TIME, FEEDBACK) ---
    int curY = rightPanel.getY();
    delayHeaderY = curY;
    curY += 18;  // header 12px + gap 6px

    placeKnob (inputGainSlider,    inputGainLabel,  kx0, curY);
    placeKnob (delayTimeSlider,    delayTimeLabel,  kx1, curY);
    placeKnob (noteDivisionSlider, delayTimeLabel,  kx1, curY);
    placeKnob (feedbackSlider,     feedbackLabel,   kx2, curY);

    // Time/Sync toggle below TIME knob — centered under the knob column
    int syncY = curY + 66 + 2;
    int syncBtnW = 50;
    int timeCenterX = kx1 + knobW / 2;
    if (tempoSyncButton) tempoSyncButton->setBounds (timeCenterX - syncBtnW / 2, syncY, syncBtnW, StyledButton::kHeight);
    // Dotted/Triplet toggle buttons — row below SYNC, half-width each
    int modRowY = syncY + StyledButton::kHeight + 2;
    int modBtnGap = 2;
    int modBtnW = (syncBtnW - modBtnGap) / 2;
    int modRowX = timeCenterX - syncBtnW / 2;
    if (syncDottedButton)  syncDottedButton->setBounds  (modRowX, modRowY, modBtnW, StyledButton::kHeight);
    if (syncTripletButton) syncTripletButton->setBounds (modRowX + modBtnW + modBtnGap, modRowY, modBtnW, StyledButton::kHeight);
    bool isSynced = tempoSyncButton ? tempoSyncButton->getToggleState() : false;
    if (syncDottedButton)  syncDottedButton->setVisible (isSynced);
    if (syncTripletButton) syncTripletButton->setVisible (isSynced);
    syncModeBox.setVisible (false);  // hidden — replaced by toggle buttons

    curY += 66 + 22 + 20 + 2;  // knob row + sync row + modifier row + gap

    // --- MOD section (v0.8: Wobble modulation) ---
    curY += 4;
    modHeaderY = curY;
    curY += 14;  // header + gap

    placeKnob (wobbleAmountSlider, wobbleAmountLabel, px0, curY);
    placeKnob (wobbleMorphSlider,  wobbleMorphLabel,  px1, curY);
    curY += 66;

    // --- TONE section ---
    curY += 4;
    toneHeaderY = curY;
    curY += 14;  // header + gap

    // FilterGraph — 6px top padding for breathing room below FLT toggle
    filterGraph.setBounds (rpX, curY + 6, panelW, 104);
    filterHPSlider.setVisible (false);
    filterLPSlider.setVisible (false);
    filterHPLabel.setVisible (false);
    filterLPLabel.setVisible (false);

    // (Q readout is painted in paint(), not a label component)

    curY += 110 + 22;  // filter graph + readout text below

    // --- MIX section ---
    curY += 4;
    mixHeaderY = curY;
    curY += 14;  // header + gap

    placeKnob (dryWetSlider,     dryWetLabel,     px0, curY);
    placeKnob (outputGainSlider, outputGainLabel, px1, curY);

    // Toggle pill sizes (IndicatorToggle::getPreferredWidth)
    auto jbm = osdLookAndFeel->jetbrainsMedium;
    int airW = IndicatorToggle::getPreferredWidth (jbm, "AIR");
    int fltW = IndicatorToggle::getPreferredWidth (jbm, "FLT");
    int modW = IndicatorToggle::getPreferredWidth (jbm, "MOD");
    int rcvW = IndicatorToggle::getPreferredWidth (jbm, "RECEIVE");
    int sndW = IndicatorToggle::getPreferredWidth (jbm, "SEND");

    // Place FLT and MOD toggles in their section headers
    // (null guards needed: setSize() triggers resized() before unique_ptrs are constructed)
    if (fltToggle)
        fltToggle->setBounds (rpX + panelW - fltW, toneHeaderY - 2, fltW, IndicatorToggle::kHeight);
    if (modToggle)
        modToggle->setBounds (rpX + panelW - modW, modHeaderY - 2, modW, IndicatorToggle::kHeight);
    if (airAbsorptionButton)
        airAbsorptionButton->setBounds (rpX + panelW - airW, mixHeaderY - 2, airW, IndicatorToggle::kHeight);

    // --- OSC section (pushed to bottom of right panel) ---
    int rpBottom = rightPanel.getBottom();
    oscHeaderY = rpBottom - 60;
    int oscCtrlY = oscHeaderY + 16;
    // Receive row
    if (oscToggleButton)
        oscToggleButton->setBounds (rpX, oscCtrlY, rcvW, 20);
    oscPortLabel.setBounds (rpX + rcvW + 34, oscCtrlY, 54, 20);
    // Send row
    int oscSendY = oscCtrlY + 24;
    if (oscSendToggleButton)
        oscSendToggleButton->setBounds (rpX, oscSendY, sndW, 20);
    oscSendIPLabel.setBounds (rpX + sndW + 4, oscSendY, 90, 20);
    oscSendPortLabel.setBounds (rpX + sndW + 4 + 94, oscSendY, 48, 20);

    // === BOTTOM PANEL (column 1 only, 106px) =================================
    auto bottomPanel = area.removeFromBottom (kBottomPanelHeight);

    objectHeaderY = bottomPanel.getY();
    int bpX = bottomPanel.getX() + 8;
    int bpY = bottomPanel.getY() + 8;

    // Tap selector — stretch to fill available width, 3px gap
    int bpRight = kWindowWidth - kRightPanelWidth;
    int bpAvailW = bpRight - bpX - 8;  // available width minus right padding
    int btnGap = 3;
    int totalGaps = (SpatialMapComponent::MAX_OBJECTS - 1) * btnGap;
    int btnBase = (bpAvailW - totalGaps) / SpatialMapComponent::MAX_OBJECTS;
    int btnRem  = (bpAvailW - totalGaps) % SpatialMapComponent::MAX_OBJECTS;
    int btnX = bpX;
    for (int i = 0; i < SpatialMapComponent::MAX_OBJECTS; ++i)
    {
        int w = btnBase + (i < btnRem ? 1 : 0);
        objectButtons[(size_t)i].setBounds (btnX, bpY, w, 22);
        btnX += w + btnGap;
    }
    int tapRightEdge = btnX - btnGap;  // right edge of tap #12

    // Controls row — dynamic knob width fills available space
    int ctrlY = bpY + 30;  // below tap buttons (8px gap)
    int ctrlX = bpX;
    int ctrlAvailW = tapRightEdge - bpX;  // match tap buttons width exactly
    int objKnobDiam = 46;  // slightly larger bottom-panel knobs

    // Compute trajectory column width — max of title and widest dropdown item
    auto trajFont = makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.12f);
    juce::GlyphArrangement trajGa;
    trajGa.addLineOfText (trajFont, "TRAJECTORY", 0.0f, 0.0f);
    int trajLabelW = juce::roundToInt (std::ceil (trajGa.getBoundingBox (0, -1, false).getWidth())) + 6;

    // Measure widest dropdown item (DM Sans Regular 13px + arrow/padding)
    auto comboFont = juce::Font (juce::FontOptions (osdLookAndFeel->dmSansRegular).withHeight (13.0f));
    juce::GlyphArrangement itemGa;
    itemGa.addLineOfText (comboFont, "Figure-8", 0.0f, 0.0f);
    int itemTextW = juce::roundToInt (std::ceil (itemGa.getBoundingBox (0, -1, false).getWidth())) + 24;
    int trajW = juce::jmax (trajLabelW, itemTextW);

    // ON pill width
    int onW = IndicatorToggle::getPreferredWidth (jbm, "ON");

    // Dynamic knob width: 6 knobs fill remaining space after fixed elements
    // Groups: ON/INPUT | gap | AZ EL DIST | gap | PITCH DOPPLER | gap | TRAJ/DIR SPEED
    int groupGap = 10;
    int fixedW = onW + 3 * groupGap + trajW + 4;  // ON + 3 gaps + TRAJ + TRAJ-to-SPEED gap
    int knobBase = (ctrlAvailW - fixedW) / 6;
    int knobRem  = (ctrlAvailW - fixedW) % 6;

    // Column width: first knobRem columns get +1px (remainder into columns, not gaps)
    auto knobColW = [knobBase, knobRem](int col) -> int {
        return knobBase + (col < knobRem ? 1 : 0);
    };

    // placeObjKnob: 12px label + 58px slider, knob circle centered in column
    auto placeObjKnob = [objKnobDiam, this](juce::Slider& slider, juce::Label& label,
                                             int x, int y, int colW) {
        label.setBounds (x, y, colW, 12);
        label.setFont (makeFont (osdLookAndFeel->jetbrainsMedium, 10.0f, 0.12f));
        slider.setBounds (x + (colW - objKnobDiam) / 2, y + 12, objKnobDiam, 58);
    };

    // ON/OFF pill — top-aligned with knob title labels
    int onBtnX = ctrlX;
    if (objEnabledButton)
        objEnabledButton->setBounds (ctrlX, ctrlY, onW, IndicatorToggle::kHeight);
    ctrlX += onW + groupGap;

    // v0.8: Input channel cycling button (L+R / L / R) — below ON/OFF pill
    if (objInputChannelButton)
        objInputChannelButton->setBounds (onBtnX, ctrlY + 38,
                                          juce::jmax (onW, 34), StyledButton::kHeight);

    // Position group: AZ (col 0), EL (col 1), DIST (col 2)
    { int w = knobColW (0); placeObjKnob (objAzimuthSlider,   objAzLabel,   ctrlX, ctrlY, w); ctrlX += w; }
    { int w = knobColW (1); placeObjKnob (objElevationSlider,  objElLabel,   ctrlX, ctrlY, w); ctrlX += w; }
    { int w = knobColW (2); placeObjKnob (objDistanceSlider,   objDistLabel, ctrlX, ctrlY, w); ctrlX += w; }

    ctrlX += groupGap;  // position / pitch-doppler boundary

    // Pitch (col 3) + Doppler (col 4)
    { int w = knobColW (3); placeObjKnob (objPitchShiftSlider, objPitchShiftLabel, ctrlX, ctrlY, w); ctrlX += w; }
    { int w = knobColW (4); placeObjKnob (objDopplerSlider,    objDopplerLabel,    ctrlX, ctrlY, w); ctrlX += w; }

    ctrlX += groupGap;  // pitch-doppler / motion boundary

    // Trajectory dropdown — width matches "TRAJECTORY" title
    int trajStartX = ctrlX;
    {
        objTrajectoryLabel.setFont (trajFont);
        objTrajectoryLabel.setJustificationType (juce::Justification::centred);
        objTrajectoryLabel.setBorderSize (juce::BorderSize<int> (0));  // remove default 5px internal padding
        objTrajectoryLabel.setBounds (ctrlX, ctrlY, trajW, 12);
        objTrajectoryBox.setBounds (ctrlX, ctrlY + 14, trajW, 20);   // 2px gap matches DIRECTION spacing
        ctrlX += trajW + 4;
    }

    // v0.8: Trajectory direction arrows (← →) — stretch to TRAJECTORY dropdown width
    {
        int dirBtnW = (trajW - 2) / 2, dirBtnH = StyledButton::kHeight;
        if (objTrajectoryRevButton) objTrajectoryRevButton->setBounds (trajStartX, ctrlY + 52, dirBtnW, dirBtnH);
        if (objTrajectoryFwdButton) objTrajectoryFwdButton->setBounds (trajStartX + dirBtnW + 2, ctrlY + 52, dirBtnW, dirBtnH);
    }

    // Speed knob — standard column width, right-aligned to tap button #12 edge
    {
        int speedW = juce::jmax (knobColW (5), objKnobDiam + 4);
        int speedX = tapRightEdge - speedW;
        placeObjKnob (objTrajectorySpeedSlider, objTrajectorySpeedLabel, speedX, ctrlY, speedW);
    }

    // === SPATIAL MAP (fills remaining area) ===================================
    spatialMap.setBounds (area);

    // === GLOBAL TAP DRAWER (overlays left edge of spatial map) ================
    {
        int drawerW = globalTapDrawer.getCurrentWidth();
        globalTapDrawer.setBounds (area.getX(), area.getY(), drawerW, area.getHeight());
        globalTapDrawer.toFront (false);
    }
}

//==============================================================================

