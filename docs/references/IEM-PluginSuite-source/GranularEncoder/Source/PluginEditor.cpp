/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Stefan Riedel
 Copyright (c) 2022 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
GranularEncoderAudioProcessorEditor::GranularEncoderAudioProcessorEditor (
    GranularEncoderAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    footer (p.getOSCParameterInterface()),
    processor (p),
    valueTreeState (vts),
    centerElement (*valueTreeState.getParameter ("azimuth"),
                   valueTreeState.getParameterRange ("azimuth"),
                   *valueTreeState.getParameter ("elevation"),
                   valueTreeState.getParameterRange ("elevation"))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // setSize(500, 325);
    // setSize(500, 470);
    setSize (500, 480);
    setLookAndFeel (&globalLaF);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible (&sphere);
    sphere.addListener (this);

    centerElement.setColour (juce::Colours::white);
    sphere.addElement (&centerElement);
    centerElement.setGrabPriority (1);
    // ======================================

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Granular"), juce::String ("Encoder"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    addAndMakeVisible (&footer);

    toolTipWin.setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    toolTipWin.setOpaque (false);

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getOutputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getOutputWidgetPtr()->getOrderCbPointer()));

    // ======================== AZIMUTH ELEVATION ROLL WIDTH GROUP
    ypGroup.setText ("Azimuth, Elevation, Size, Shape");
    ypGroup.setTextLabelPosition (juce::Justification::centredLeft);
    ypGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible (&ypGroup);
    ypGroup.setVisible (true);

    addAndMakeVisible (&azimuthSlider);
    azimuthAttachment.reset (new SliderAttachment (valueTreeState, "azimuth", azimuthSlider));
    azimuthSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    azimuthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    azimuthSlider.setReverse (true);
    azimuthSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[0]);
    azimuthSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                       3 * juce::MathConstants<float>::pi,
                                       false);
    azimuthSlider.setTooltip ("Azimuth angle");
    azimuthSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&elevationSlider);
    elevationAttachment.reset (new SliderAttachment (valueTreeState, "elevation", elevationSlider));
    elevationSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    elevationSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    elevationSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                               globalLaF.ClWidgetColours[1]);
    elevationSlider.setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                         2.5 * juce::MathConstants<float>::pi,
                                         false);
    elevationSlider.setTooltip ("Elevation angle");
    elevationSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&shapeSlider);
    shapeAttachment.reset (new SliderAttachment (valueTreeState, "shape", shapeSlider));
    shapeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    shapeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    shapeSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    shapeSlider.setReverse (false);
    shapeSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                     3 * juce::MathConstants<float>::pi,
                                     true);
    shapeSlider.setTooltip ("Shape the grain distribution: edgy(-) / uniform(0) / peaky(+)");
    shapeSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"()"));

    addAndMakeVisible (&sizeSlider);
    sizeAttachment.reset (new SliderAttachment (valueTreeState, "size", sizeSlider));
    sizeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sizeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sizeSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    sizeSlider.setReverse (false);
    sizeSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                    3 * juce::MathConstants<float>::pi,
                                    true);
    sizeSlider.setTooltip ("Set the maximum spread of the grain distribution");
    sizeSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    // ======================== TEMPORAL GRAIN PARAMETERS GROUP
    grainGroup.setText ("Grain Parameters");
    grainGroup.setTextLabelPosition (juce::Justification::centredLeft);
    grainGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    grainGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible (&grainGroup);
    grainGroup.setVisible (true);

    // Delta Time
    addAndMakeVisible (&deltaTimeSlider);
    deltaTimeAttachment.reset (new SliderAttachment (valueTreeState, "deltaTime", deltaTimeSlider));
    deltaTimeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    deltaTimeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    deltaTimeSlider.setReverse (false);
    deltaTimeSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                               globalLaF.ClWidgetColours[0]);
    deltaTimeSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                         3 * juce::MathConstants<float>::pi,
                                         true);
    deltaTimeSlider.setTooltip ("Time between grains");
    deltaTimeSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(s)"));
    // Delta Time Modulation Amount
    addAndMakeVisible (&deltaTimeModSlider);
    deltaTimeModAttachment.reset (
        new SliderAttachment (valueTreeState, "deltaTimeMod", deltaTimeModSlider));
    deltaTimeModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    deltaTimeModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    deltaTimeModSlider.setReverse (false);
    deltaTimeModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                  globalLaF.ClWidgetColours[0]);
    deltaTimeModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                            3 * juce::MathConstants<float>::pi,
                                            true);
    deltaTimeModSlider.setTooltip (
        "Spread in percent of the current time between grains (bipolar)");
    deltaTimeModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Grain Length
    addAndMakeVisible (&grainLengthSlider);
    grainLengthAttachment.reset (
        new SliderAttachment (valueTreeState, "grainLength", grainLengthSlider));
    grainLengthSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    grainLengthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    grainLengthSlider.setReverse (false);
    grainLengthSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[0]);
    grainLengthSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                           3 * juce::MathConstants<float>::pi,
                                           true);
    grainLengthSlider.setTooltip ("Length of grains");
    grainLengthSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(s)"));
    // Grain Length Modulation Amount
    addAndMakeVisible (&grainLengthModSlider);
    grainLengthModAttachment.reset (
        new SliderAttachment (valueTreeState, "grainLengthMod", grainLengthModSlider));
    grainLengthModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    grainLengthModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    grainLengthModSlider.setReverse (false);
    grainLengthModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                    globalLaF.ClWidgetColours[0]);
    grainLengthModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                              3 * juce::MathConstants<float>::pi,
                                              true);
    grainLengthModSlider.setTooltip ("Spread in percent of the current grain length (bipolar)");
    grainLengthModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Buffer Position
    addAndMakeVisible (&positionSlider);
    positionAttachment.reset (new SliderAttachment (valueTreeState, "position", positionSlider));
    positionSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    positionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    positionSlider.setReverse (false);
    positionSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[0]);
    positionSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                        3 * juce::MathConstants<float>::pi,
                                        true);
    positionSlider.setTooltip ("Read position in the buffer (relative to write head)");
    positionSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(s)"));
    // Buffer Position Modulation Amount
    addAndMakeVisible (&positionModSlider);
    positionModAttachment.reset (
        new SliderAttachment (valueTreeState, "positionMod", positionModSlider));
    positionModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    positionModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    positionModSlider.setReverse (false);
    positionModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[0]);
    positionModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                           3 * juce::MathConstants<float>::pi,
                                           true);
    positionModSlider.setTooltip ("Spread amount for the read position in the buffer (unipolar)");
    positionModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Pitch
    addAndMakeVisible (&pitchSlider);
    pitchAttachment.reset (new SliderAttachment (valueTreeState, "pitch", pitchSlider));
    pitchSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    pitchSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    pitchSlider.setReverse (false);
    pitchSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    pitchSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                     3 * juce::MathConstants<float>::pi,
                                     true);
    pitchSlider.setTooltip (
        "Pitch of grains in semitones: -12 to +12 in Freeze mode, -12 to 0 in real-time input mode");
    pitchSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(st)"));
    // Pitch Modulation Amount
    addAndMakeVisible (&pitchModSlider);
    pitchModAttachment.reset (new SliderAttachment (valueTreeState, "pitchMod", pitchModSlider));
    pitchModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    pitchModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    pitchModSlider.setReverse (false);
    pitchModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[0]);
    pitchModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                        3 * juce::MathConstants<float>::pi,
                                        true);
    pitchModSlider.setTooltip ("Spread amount for the pitch of grains (bipolar)");
    pitchModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Window Attack
    addAndMakeVisible (&windowAttackSlider);
    windowAttackAttachment.reset (
        new SliderAttachment (valueTreeState, "windowAttack", windowAttackSlider));
    windowAttackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    windowAttackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    windowAttackSlider.setReverse (false);
    windowAttackSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                  globalLaF.ClWidgetColours[1]);
    windowAttackSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                            3 * juce::MathConstants<float>::pi,
                                            true);
    windowAttackSlider.setTooltip ("Window attack time in percent of grain length");
    windowAttackSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));
    addAndMakeVisible (&windowAttackModSlider);
    windowAttackModAttachment.reset (
        new SliderAttachment (valueTreeState, "windowAttackMod", windowAttackModSlider));
    windowAttackModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    windowAttackModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    windowAttackModSlider.setReverse (false);
    windowAttackModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                     globalLaF.ClWidgetColours[1]);
    windowAttackModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                               3 * juce::MathConstants<float>::pi,
                                               true);
    windowAttackModSlider.setTooltip ("Spread amount for window attack time");
    windowAttackModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Window Decay
    addAndMakeVisible (&windowDecaySlider);
    windowDecayAttachment.reset (
        new SliderAttachment (valueTreeState, "windowDecay", windowDecaySlider));
    windowDecaySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    windowDecaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    windowDecaySlider.setReverse (false);
    windowDecaySlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[1]);
    windowDecaySlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                           3 * juce::MathConstants<float>::pi,
                                           true);
    windowDecaySlider.setTooltip ("Window decay time in percent of grain length");
    windowDecaySlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));
    addAndMakeVisible (&windowDecayModSlider);
    windowDecayModAttachment.reset (
        new SliderAttachment (valueTreeState, "windowDecayMod", windowDecayModSlider));
    windowDecayModSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    windowDecayModSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    windowDecayModSlider.setReverse (false);
    windowDecayModSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                    globalLaF.ClWidgetColours[1]);
    windowDecayModSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                              3 * juce::MathConstants<float>::pi,
                                              true);
    windowDecayModSlider.setTooltip ("Spread amount for window decay time");
    windowAttackModSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Mix between Dry Encoding and Granular Encoding
    addAndMakeVisible (&mixSlider);
    mixAttachment.reset (new SliderAttachment (valueTreeState, "mix", mixSlider));
    mixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    mixSlider.setReverse (false);
    mixSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    mixSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                   3 * juce::MathConstants<float>::pi,
                                   true);
    mixSlider.setTooltip ("Mix between standard encoding (0) and granular encoding (100).");
    mixSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // Blend between seeding left or right channel (only relevant for stereo input)
    addAndMakeVisible (&sourceSlider);
    sourceAttachment.reset (
        new SliderAttachment (valueTreeState, "sourceProbability", sourceSlider));
    sourceSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sourceSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sourceSlider.setReverse (false);
    sourceSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                            globalLaF.ClWidgetColours[2]);
    sourceSlider.setRotaryParameters (juce::MathConstants<float>::pi,
                                      3 * juce::MathConstants<float>::pi,
                                      true);
    sourceSlider.setTooltip (
        "Seed grains from left (-1), both (0), or right (+1) input channel (for stereo input).");
    sourceSlider.setTextValueSuffix (juce::CharPointer_UTF8 (R"(%)"));

    // ====================== QUATERNION GROUP
    quatGroup.setText ("Quaternions");
    quatGroup.setTextLabelPosition (juce::Justification::centredLeft);
    quatGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    quatGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible (&quatGroup);
    quatGroup.setVisible (true);

    addAndMakeVisible (&qwSlider);
    qwAttachment.reset (new SliderAttachment (valueTreeState, "qw", qwSlider));
    qwSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    qwSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    qwSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&qxSlider);
    qxAttachment.reset (new SliderAttachment (valueTreeState, "qx", qxSlider));
    qxSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    qxSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    qxSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&qySlider);
    qyAttachment.reset (new SliderAttachment (valueTreeState, "qy", qySlider));
    qySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    qySlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    qySlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&qzSlider);
    qzAttachment.reset (new SliderAttachment (valueTreeState, "qz", qzSlider));
    qzSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    qzSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    qzSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    // FREEZE STATE
    addAndMakeVisible (tbFreeze);
    tbFreezeAttachment.reset (new ButtonAttachment (valueTreeState, "freeze", tbFreeze));
    tbFreeze.setButtonText ("Freeze Audio");
    tbFreeze.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[2]);
    tbFreeze.setTooltip (
        "Toggle to switch between a freezed audio buffer and realtime audio input.");

    // 2D/3D Spatialization Toggle
    /*
    addAndMakeVisible(tb2D);
    tb2DAttachment.reset(new ButtonAttachment(valueTreeState, "spatialize2D", tb2D));
    tb2D.setButtonText("");
    tb2D.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
    tb2D.setTooltip("Toggle for 2D spatialization mode (azimuth spread only).");*/

    addAndMakeVisible (cb2D3D);
    cb2D3D.setJustificationType (juce::Justification::centred);
    cb2D3D.addSectionHeading ("Distribution");
    // cb2D3D.addItem("3D", 1);
    // cb2D3D.addItem("2D", 2);
    cb2D3D.addItem ("Spherical", 1);
    cb2D3D.addItem ("Circular", 2);
    cb2D3DAtachement.reset (new ComboBoxAttachment (valueTreeState, "spatialize2D", cb2D3D));

    // addAndMakeVisible(&lb2D);
    // lb2D.setText("2D");

    // ================ LABELS ===================
    addAndMakeVisible (&lbAzimuth);
    lbAzimuth.setText ("Azimuth");

    addAndMakeVisible (&lbElevation);
    lbElevation.setText ("Elevation");

    addAndMakeVisible (&lbShape);
    lbShape.setText ("Shape");

    addAndMakeVisible (&lbSize);
    lbSize.setText ("Size");

    addAndMakeVisible (&lbDeltaTime);
    lbDeltaTime.setText ("Delta-t");
    addAndMakeVisible (&lbDeltaTimeMod);
    lbDeltaTimeMod.setText ("t-Mod");

    addAndMakeVisible (&lbGrainLength);
    lbGrainLength.setText ("Length");
    addAndMakeVisible (&lbGrainLengthMod);
    lbGrainLengthMod.setText ("L-Mod");

    addAndMakeVisible (&lbPosition);
    lbPosition.setText ("Position");
    addAndMakeVisible (&lbPositionMod);
    lbPositionMod.setText ("Pos-Mod");

    addAndMakeVisible (&lbPitch);
    lbPitch.setText ("Pitch");
    addAndMakeVisible (&lbPitchMod);
    lbPitchMod.setText ("Pitch-Mod");

    addAndMakeVisible (&lbWindowAttack);
    lbWindowAttack.setText ("Attack");

    addAndMakeVisible (&lbWindowDecay);
    lbWindowDecay.setText ("Decay");

    addAndMakeVisible (&lbMix);
    lbMix.setText ("Mix");

    addAndMakeVisible (&lbSource);
    lbSource.setText ("Source");

    addAndMakeVisible (&lbW);
    lbW.setText ("W");

    addAndMakeVisible (&lbX);
    lbX.setText ("X");

    addAndMakeVisible (&lbY);
    lbY.setText ("Y");

    addAndMakeVisible (&lbZ);
    lbZ.setText ("Z");

    // KeyListener
    addKeyListener (this);

    startTimer (20);
}

void GranularEncoderAudioProcessorEditor::mouseWheelOnSpherePannerMoved (
    SpherePanner* sphere,
    const juce::MouseEvent& event,
    const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isAltDown())
        elevationSlider.mouseWheelMove (event, wheel);
    else if (event.mods.isCommandDown())
        azimuthSlider.mouseWheelMove (event, wheel);
}

GranularEncoderAudioProcessorEditor::~GranularEncoderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void GranularEncoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void GranularEncoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        sphere.repaint();
    }
}

void GranularEncoderAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);

    juce::Rectangle<int> sliderRow;
    juce::Rectangle<int> sliderRowTwo;

    // ============== SIDEBAR RIGHT ====================
    // =================================================
    juce::Rectangle<int> sideBarArea (area.removeFromRight (190));
    const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int modSliderHeight = 40;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 15;
    const int labelWidth = 20;

    // -------------- Azimuth Elevation Roll Width ------------------

    juce::Rectangle<int> yprArea (sideBarArea.removeFromTop (25 + rotSliderHeight + labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop (25); // for box headline

    sliderRow = (yprArea.removeFromTop (rotSliderHeight));
    azimuthSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    elevationSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    sizeSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    shapeSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    lbAzimuth.setBounds (yprArea.removeFromLeft (rotSliderWidth));
    yprArea.removeFromLeft (rotSliderSpacing - 5);
    lbElevation.setBounds (yprArea.removeFromLeft (rotSliderWidth + 10));
    yprArea.removeFromLeft (rotSliderSpacing - 5);
    lbSize.setBounds (yprArea.removeFromLeft (rotSliderWidth));
    yprArea.removeFromLeft (rotSliderSpacing);
    lbShape.setBounds (yprArea.removeFromLeft (rotSliderWidth));

    sideBarArea.removeFromTop (5);
    juce::Rectangle<int> ModeArea2D (sideBarArea.removeFromTop (15));
    // lb2D.setBounds(ModeArea2D.removeFromRight(20));
    // tb2D.setBounds(ModeArea2D.removeFromRight(20));
    ModeArea2D.removeFromRight (5);
    cb2D3D.setBounds (ModeArea2D.removeFromRight (70));
    sideBarArea.removeFromTop (5);

    // -------------- DeltaTime GrainLength Position Pitch ------------------
    juce::Rectangle<int> grainArea (sideBarArea.removeFromTop (
        25 + 10 + 2 * rotSliderHeight + 2 * modSliderHeight + 2 * labelHeight));
    grainGroup.setBounds (grainArea);
    grainArea.removeFromTop (25); // for box headline

    sliderRow = (grainArea.removeFromTop (rotSliderHeight));
    deltaTimeSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    grainLengthSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    positionSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    pitchSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    juce::Rectangle<int> labelRow = (grainArea.removeFromTop (labelHeight));
    lbDeltaTime.setBounds (labelRow.removeFromLeft (rotSliderWidth));
    labelRow.removeFromLeft (rotSliderSpacing - 5);
    lbGrainLength.setBounds (labelRow.removeFromLeft (rotSliderWidth + 10));
    labelRow.removeFromLeft (rotSliderSpacing - 10);
    lbPosition.setBounds (labelRow.removeFromLeft (rotSliderWidth + 10));
    labelRow.removeFromLeft (rotSliderSpacing - 5);
    lbPitch.setBounds (labelRow.removeFromLeft (rotSliderWidth + 10));

    juce::Rectangle<int> grainModArea (grainArea.removeFromTop (modSliderHeight));
    sliderRow = (grainModArea.removeFromTop (modSliderHeight));
    deltaTimeModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    grainLengthModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    positionModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    pitchModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    grainArea.removeFromTop (10);
    sliderRowTwo = (grainArea.removeFromTop (rotSliderHeight));
    windowAttackSlider.setBounds (sliderRowTwo.removeFromLeft (rotSliderWidth));
    sliderRowTwo.removeFromLeft (rotSliderSpacing);
    windowDecaySlider.setBounds (sliderRowTwo.removeFromLeft (rotSliderWidth));
    sliderRowTwo.removeFromLeft (rotSliderSpacing);
    mixSlider.setBounds (sliderRowTwo.removeFromLeft (rotSliderWidth));
    sliderRowTwo.removeFromLeft (rotSliderSpacing);
    sourceSlider.setBounds (sliderRowTwo.removeFromLeft (rotSliderWidth));

    labelRow = (grainArea.removeFromTop (labelHeight));
    lbWindowAttack.setBounds (labelRow.removeFromLeft (rotSliderWidth));
    labelRow.removeFromLeft (rotSliderSpacing - 5);
    lbWindowDecay.setBounds (labelRow.removeFromLeft (rotSliderWidth + 10));
    labelRow.removeFromLeft (rotSliderSpacing - 5);
    lbMix.setBounds (labelRow.removeFromLeft (rotSliderWidth));
    labelRow.removeFromLeft (rotSliderSpacing);
    lbSource.setBounds (labelRow.removeFromLeft (rotSliderWidth));

    juce::Rectangle<int> grainModAreaTwo (grainArea.removeFromTop (modSliderHeight));
    sliderRow = (grainModAreaTwo.removeFromTop (modSliderHeight));
    windowAttackModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    windowDecayModSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbFreeze.setBounds (sliderRow.removeFromLeft (90));

    // ============== SIDEBAR LEFT ====================
    area.removeFromRight (10); // spacing
        // Sphere-Panner
    sphere.setBounds (area.getX(), area.getY(), area.getWidth() - 20, area.getWidth() - 20);

    // ------------- Quaternion ------------------------
    area.removeFromTop (10);
    area.removeFromTop (area.getWidth());
    juce::Rectangle<int> quatArea (area.getX(), area.getY(), area.getWidth() - 20, 165);
    quatGroup.setBounds (quatArea);
    quatArea.removeFromTop (25); // for box headline

    sliderRow = quatArea.removeFromTop (sliderHeight);
    qwSlider.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbW.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    qxSlider.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbX.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    qySlider.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbY.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    qzSlider.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbZ.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);
}

bool GranularEncoderAudioProcessorEditor::keyPressed (const juce::KeyPress& key,
                                                      juce::Component* originatingComponent)
{
    DBG ("Key pressed: " << key.getKeyCode());

    if (key.getModifiers().isShiftDown())
    {
        switch (key.getKeyCode())
        {
            case 90: // zenith
            case 84: // top
            case 85: // up
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (90.0);
                break;

            case 68: // down
            case 78: // nadir
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (-90.0);
                break;

            case 70: // front
                azimuthSlider.setValue (0.0);
                elevationSlider.setValue (0.0);
                break;

            case 66: // back
                azimuthSlider.setValue (-180.0);
                elevationSlider.setValue (0.0);
                break;

            case 76: // left
                azimuthSlider.setValue (90.0);
                elevationSlider.setValue (0.0);
                break;

            case 82: // right
                azimuthSlider.setValue (-90.0);
                elevationSlider.setValue (0.0);
                break;

            default:
                return false;
        }
        return true;
    }

    return false;
}
