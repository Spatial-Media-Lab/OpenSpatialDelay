/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
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
RoomEncoderAudioProcessorEditor::RoomEncoderAudioProcessorEditor (
    RoomEncoderAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    footer (p.getOSCParameterInterface()),
    processor (p),
    valueTreeState (vts),
    sourceElement (*valueTreeState.getParameter ("sourceX"),
                   valueTreeState.getParameterRange ("sourceX"),
                   *valueTreeState.getParameter ("sourceY"),
                   valueTreeState.getParameterRange ("sourceY"),
                   *valueTreeState.getParameter ("sourceZ"),
                   valueTreeState.getParameterRange ("sourceZ")),
    listenerElement (*valueTreeState.getParameter ("listenerX"),
                     valueTreeState.getParameterRange ("listenerX"),
                     *valueTreeState.getParameter ("listenerY"),
                     valueTreeState.getParameterRange ("listenerY"),
                     *valueTreeState.getParameter ("listenerZ"),
                     valueTreeState.getParameterRange ("listenerZ"))

{
    setSize (820, 600);
    setLookAndFeel (&globalLaF);

    toolTipWin.setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    toolTipWin.setOpaque (false);

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Room"), juce::String ("Encoder"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    cbNormalizationAttachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getOutputWidgetPtr()->getNormCbPointer()));
    cbOrderAttachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getOutputWidgetPtr()->getOrderCbPointer()));
    cbDirectivityOrderAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "directivityOrderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));
    cbDirectivityNormalizationAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputIsSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    addAndMakeVisible (&footer);

    addAndMakeVisible (&gcRoomDimensions);
    gcRoomDimensions.setText ("Room Dimensions");
    gcRoomDimensions.setTextLabelPosition (juce::Justification::left);
    gcRoomDimensions.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcRoomDimensions.setColour (juce::GroupComponent::textColourId, juce::Colours::white);

    addAndMakeVisible (&gcListenerPosition);
    gcListenerPosition.setText ("Listener Position");
    gcListenerPosition.setTextLabelPosition (juce::Justification::left);
    gcListenerPosition.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcListenerPosition.setColour (juce::GroupComponent::textColourId, juce::Colours::white);

    addAndMakeVisible (&gcSourcePosition);
    gcSourcePosition.setText ("Source Position");
    gcSourcePosition.setTextLabelPosition (juce::Justification::left);
    gcSourcePosition.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcSourcePosition.setColour (juce::GroupComponent::textColourId, juce::Colours::white);

    addAndMakeVisible (&gcReflectionProperties);
    gcReflectionProperties.setText ("Reflection Properties");
    gcReflectionProperties.setTextLabelPosition (juce::Justification::centredLeft);
    gcReflectionProperties.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcReflectionProperties.setColour (juce::GroupComponent::textColourId, juce::Colours::white);

    addAndMakeVisible (&gcSync);
    gcSync.setText ("Synchronize Room Settings");
    gcSync.setTextLabelPosition (juce::Justification::left);
    gcSync.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcSync.setColour (juce::GroupComponent::textColourId, juce::Colours::white);

    addAndMakeVisible (&lbRoomX);
    lbRoomX.setText ("Depth");
    addAndMakeVisible (&lbRoomY);
    lbRoomY.setText ("Width");
    addAndMakeVisible (&lbRoomZ);
    lbRoomZ.setText ("Height");

    addAndMakeVisible (&lbListenerX);
    lbListenerX.setText ("x");
    addAndMakeVisible (&lbListenerY);
    lbListenerY.setText ("y");
    addAndMakeVisible (&lbListenerZ);
    lbListenerZ.setText ("z");

    addAndMakeVisible (&lbSourceX);
    lbSourceX.setText ("x");
    addAndMakeVisible (&lbSourceY);
    lbSourceY.setText ("y");
    addAndMakeVisible (&lbSourceZ);
    lbSourceZ.setText ("z");

    addAndMakeVisible (&slRoomX);
    slRoomXAttachment.reset (new SliderAttachment (valueTreeState, "roomX", slRoomX));
    slRoomX.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRoomX.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slRoomX.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomX.setTextValueSuffix (" m");
    slRoomX.setTooltip ("room size x");
    slRoomX.addListener (this);

    addAndMakeVisible (&slRoomY);
    slRoomYAttachment.reset (new SliderAttachment (valueTreeState, "roomY", slRoomY));
    slRoomY.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRoomY.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slRoomY.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomY.setTextValueSuffix (" m");
    slRoomY.setTooltip ("room size y");
    slRoomY.addListener (this);

    addAndMakeVisible (&slRoomZ);
    slRoomZAttachment.reset (new SliderAttachment (valueTreeState, "roomZ", slRoomZ));
    slRoomZ.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRoomZ.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slRoomZ.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slRoomZ.setTextValueSuffix (" m");
    slRoomZ.setTooltip ("room size z");
    slRoomZ.addListener (this);

    addAndMakeVisible (&xyPlane);
    xyPlane.addElement (&sourceElement);
    xyPlane.addElement (&listenerElement);
    xyPlane.useAutoScale (false);

    addAndMakeVisible (&zyPlane);
    zyPlane.setPlane (PositionPlane::Planes::zy);
    zyPlane.addElement (&sourceElement);
    zyPlane.addElement (&listenerElement);
    zyPlane.useAutoScale (false);

    addAndMakeVisible (&slSourceX);
    slSourceXAttachment.reset (new SliderAttachment (valueTreeState, "sourceX", slSourceX));
    slSourceX.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slSourceX.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slSourceX.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceX.setTextValueSuffix (" m");
    slSourceX.setTooltip ("source position x");
    slSourceX.addListener (this);

    addAndMakeVisible (&slSourceY);
    slSourceYAttachment.reset (new SliderAttachment (valueTreeState, "sourceY", slSourceY));
    slSourceY.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slSourceY.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slSourceY.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceY.setTextValueSuffix (" m");
    slSourceY.setTooltip ("source position y");
    slSourceY.addListener (this);

    addAndMakeVisible (&slSourceZ);
    slSourceZAttachment.reset (new SliderAttachment (valueTreeState, "sourceZ", slSourceZ));
    slSourceZ.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slSourceZ.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slSourceZ.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slSourceZ.setTextValueSuffix (" m");
    slSourceZ.setTooltip ("source position z");
    slSourceZ.addListener (this);

    addAndMakeVisible (&slListenerX);
    slListenerXAttachment.reset (new SliderAttachment (valueTreeState, "listenerX", slListenerX));
    slListenerX.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slListenerX.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slListenerX.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerX.setRotaryParameters (juce::MathConstants<float>::pi,
                                     3 * juce::MathConstants<float>::pi,
                                     false);
    slListenerX.setTextValueSuffix (" m");
    slListenerX.setTooltip ("listener position x");
    slListenerX.addListener (this);

    addAndMakeVisible (&slListenerY);
    slListenerYAttachment.reset (new SliderAttachment (valueTreeState, "listenerY", slListenerY));
    slListenerY.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slListenerY.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slListenerY.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerY.setTextValueSuffix (" m");
    slListenerY.setTooltip ("listener position y");
    slListenerY.addListener (this);

    addAndMakeVisible (&slListenerZ);
    slListenerZAttachment.reset (new SliderAttachment (valueTreeState, "listenerZ", slListenerZ));
    slListenerZ.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slListenerZ.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slListenerZ.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slListenerZ.setTextValueSuffix (" m");
    slListenerZ.setTooltip ("listener position z");
    slListenerZ.addListener (this);

    sourceElement.setColour (globalLaF.ClWidgetColours[2]);
    listenerElement.setColour (globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&lbNumReflections);
    lbNumReflections.setText ("Number of Reflections");
    addAndMakeVisible (&slNumReflections);
    slNumReflectionsAttachment.reset (
        new SliderAttachment (valueTreeState, "numRefl", slNumReflections));
    slNumReflections.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slNumReflections.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slNumReflections.setColour (juce::Slider::rotarySliderOutlineColourId,
                                globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (lbWallAttenuation);
    lbWallAttenuation.setText ("Additional Attenuation", true, juce::Justification::left);

    addAndMakeVisible (slWallAttenuationFront);
    slWallAttenuationFrontAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationFront", slWallAttenuationFront));
    slWallAttenuationFront.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationFront.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationFront.setColour (juce::Slider::rotarySliderOutlineColourId,
                                      globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationFront);
    lbWallAttenuationFront.setText ("Front");

    addAndMakeVisible (slWallAttenuationBack);
    slWallAttenuationBackAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationBack", slWallAttenuationBack));
    slWallAttenuationBack.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationBack.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationBack.setColour (juce::Slider::rotarySliderOutlineColourId,
                                     globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationBack);
    lbWallAttenuationBack.setText ("Back");

    addAndMakeVisible (slWallAttenuationLeft);
    slWallAttenuationLeftAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationLeft", slWallAttenuationLeft));
    slWallAttenuationLeft.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationLeft.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationLeft.setColour (juce::Slider::rotarySliderOutlineColourId,
                                     globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationLeft);
    lbWallAttenuationLeft.setText ("Left");

    addAndMakeVisible (slWallAttenuationRight);
    slWallAttenuationRightAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationRight", slWallAttenuationRight));
    slWallAttenuationRight.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationRight.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationRight.setColour (juce::Slider::rotarySliderOutlineColourId,
                                      globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationRight);
    lbWallAttenuationRight.setText ("Right");

    addAndMakeVisible (slWallAttenuationCeiling);
    slWallAttenuationCeilingAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationCeiling", slWallAttenuationCeiling));
    slWallAttenuationCeiling.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationCeiling.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationCeiling.setColour (juce::Slider::rotarySliderOutlineColourId,
                                        globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationCeiling);
    lbWallAttenuationCeiling.setText ("Ceiling");

    addAndMakeVisible (slWallAttenuationFloor);
    slWallAttenuationFloorAttachment.reset (
        new SliderAttachment (valueTreeState, "wallAttenuationFloor", slWallAttenuationFloor));
    slWallAttenuationFloor.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWallAttenuationFloor.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWallAttenuationFloor.setColour (juce::Slider::rotarySliderOutlineColourId,
                                      globalLaF.ClWidgetColours[2]);
    addAndMakeVisible (lbWallAttenuationFloor);
    lbWallAttenuationFloor.setText ("Floor");

    addAndMakeVisible (&lbReflCoeff);
    lbReflCoeff.setText ("Overall Attenuation");

    addAndMakeVisible (&slReflCoeff);
    slReflCoeffAttachment.reset (new SliderAttachment (valueTreeState, "reflCoeff", slReflCoeff));
    slReflCoeff.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slReflCoeff.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slReflCoeff.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::lightgrey);
    slReflCoeff.setTextValueSuffix (" dB");

    addAndMakeVisible (&lbLSF);
    lbLSF.setText ("Freq.");
    addAndMakeVisible (&lbLSG);
    lbLSG.setText ("Gain");
    addAndMakeVisible (&lbHSF);
    lbHSF.setText ("Freq.");
    addAndMakeVisible (&lbHSG);
    lbHSG.setText ("Gain");

    addAndMakeVisible (&slLowShelfFreq);
    slLowShelfFreqAttachment.reset (
        new SliderAttachment (valueTreeState, "lowShelfFreq", slLowShelfFreq));
    slLowShelfFreq.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slLowShelfFreq.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slLowShelfFreq.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::cyan);
    slLowShelfFreq.setTooltip ("low shelf freq");

    addAndMakeVisible (&slLowShelfGain);
    slLowShelfGainAttachment.reset (
        new SliderAttachment (valueTreeState, "lowShelfGain", slLowShelfGain));
    slLowShelfGain.setTextValueSuffix (" dB");
    slLowShelfGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slLowShelfGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slLowShelfGain.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::cyan);

    addAndMakeVisible (&slHighShelfFreq);
    slHighShelfFreqAttachment.reset (
        new SliderAttachment (valueTreeState, "highShelfFreq", slHighShelfFreq));
    slHighShelfFreq.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slHighShelfFreq.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slHighShelfFreq.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::orangered);
    slHighShelfFreq.setTooltip ("high shelf freq");

    addAndMakeVisible (&slHighShelfGain);
    slHighShelfGainAttachment.reset (
        new SliderAttachment (valueTreeState, "highShelfGain", slHighShelfGain));
    slHighShelfGain.setTextValueSuffix (" dB");
    slHighShelfGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slHighShelfGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slHighShelfGain.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::orangered);

    addAndMakeVisible (&fv);
    fv.addCoefficients (processor.lowShelfCoefficients,
                        juce::Colours::cyan,
                        &slLowShelfFreq,
                        &slLowShelfGain);
    fv.addCoefficients (processor.highShelfCoefficients,
                        juce::Colours::orangered,
                        &slHighShelfFreq,
                        &slHighShelfGain);

    addAndMakeVisible (&rv);
    rv.setDataPointers (p.allGains, p.mRadius, p.numRefl);

    juce::Vector3D<float> dims (slRoomX.getValue(), slRoomY.getValue(), slRoomZ.getValue());
    float scale = juce::jmin (xyPlane.setDimensions (dims), zyPlane.setDimensions (dims));
    xyPlane.setScale (scale);
    zyPlane.setScale (scale);

    addAndMakeVisible (&cbSyncChannel);
    cbSyncChannel.setJustificationType (juce::Justification::centred);
    cbSyncChannel.addItem ("none", 1);
    cbSyncChannel.addItem ("1", 2);
    cbSyncChannel.addItem ("2", 3);
    cbSyncChannel.addItem ("3", 4);
    cbSyncChannel.addItem ("4", 5);
    cbSyncChannelAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "syncChannel", cbSyncChannel));

    addAndMakeVisible (&lbSyncChannel);
    lbSyncChannel.setText ("Synchronize to Channel");

    addAndMakeVisible (&tbSyncRoomSize);
    tbSyncRoomSizeAttachment.reset (
        new ButtonAttachment (valueTreeState, "syncRoomSize", tbSyncRoomSize));
    tbSyncRoomSize.setButtonText ("Room Dimensions");
    tbSyncRoomSize.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&tbSyncListener);
    tbSyncListenerAttachment.reset (
        new ButtonAttachment (valueTreeState, "syncListener", tbSyncListener));
    tbSyncListener.setButtonText ("Listener Position");
    tbSyncListener.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&tbSyncReflection);
    tbSyncReflectionAttachment.reset (
        new ButtonAttachment (valueTreeState, "syncReflection", tbSyncReflection));
    tbSyncReflection.setButtonText ("Reflection Properties");
    tbSyncReflection.setColour (juce::ToggleButton::tickColourId, juce::Colours::orangered);

    addAndMakeVisible (&tbDirectPathUnityGain);
    tbDirectPathUnityGainAttachment.reset (
        new ButtonAttachment (valueTreeState, "directPathUnityGain", tbDirectPathUnityGain));
    tbDirectPathUnityGain.setButtonText ("Direct Path Unity Gain");
    tbDirectPathUnityGain.setColour (juce::ToggleButton::tickColourId,
                                     globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&tbDirectPathZeroDelay);
    tbDirectPathZeroDelayAttachment.reset (
        new ButtonAttachment (valueTreeState, "directPathZeroDelay", tbDirectPathZeroDelay));
    tbDirectPathZeroDelay.setButtonText ("Direct Path Zero Delay");
    tbDirectPathZeroDelay.setColour (juce::ToggleButton::tickColourId,
                                     globalLaF.ClWidgetColours[1]);
    rv.setZeroDelay (tbDirectPathZeroDelay.getToggleState());
    tbDirectPathZeroDelay.addListener (this);

    addAndMakeVisible (&tbRenderDirectPath);
    tbRenderDirectPathAttachment.reset (
        new ButtonAttachment (valueTreeState, "renderDirectPath", tbRenderDirectPath));
    tbRenderDirectPath.setButtonText ("Render Direct Path");
    tbRenderDirectPath.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[2]);
    tbRenderDirectPath.addListener (this);

    startTimer (20);
}

RoomEncoderAudioProcessorEditor::~RoomEncoderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void RoomEncoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void RoomEncoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 15;

    juce::Rectangle<int> area (getLocalBounds());

    {
        juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
        footer.setBounds (footerArea);
    }

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);

    {
        juce::Rectangle<int> bottomStrip (area.removeFromBottom (120));
        rv.setBounds (bottomStrip.removeFromRight (540));
        bottomStrip.removeFromRight (20);
        gcSync.setBounds (bottomStrip);
        bottomStrip.removeFromTop (25);
        juce::Rectangle<int> channelRow (bottomStrip.removeFromTop (20));
        lbSyncChannel.setBounds (channelRow.removeFromLeft (130));
        cbSyncChannel.setBounds (channelRow.removeFromLeft (50));
        bottomStrip.removeFromLeft (5);
        tbSyncRoomSize.setBounds (bottomStrip.removeFromTop (20));
        tbSyncListener.setBounds (bottomStrip.removeFromTop (20));
        tbSyncReflection.setBounds (bottomStrip.removeFromTop (20));
    }

    area.removeFromBottom (10);

    juce::Rectangle<int> propArea (
        area.removeFromRight (9 * rotSliderWidth + 8 * rotSliderSpacing + 20));
    {
        juce::Rectangle<int> coordinateArea (propArea.removeFromTop (100));

        juce::Rectangle<int> roomArea (
            coordinateArea.removeFromLeft (3 * rotSliderWidth + 2 * rotSliderSpacing));
        gcRoomDimensions.setBounds (roomArea);
        roomArea.removeFromTop (25);

        juce::Rectangle<int> sliderRow = (roomArea.removeFromTop (rotSliderHeight));
        slRoomX.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slRoomY.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slRoomZ.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        lbRoomX.setBounds (roomArea.removeFromLeft (rotSliderWidth));
        roomArea.removeFromLeft (rotSliderSpacing);
        lbRoomY.setBounds (roomArea.removeFromLeft (rotSliderWidth));
        roomArea.removeFromLeft (rotSliderSpacing);
        lbRoomZ.setBounds (roomArea.removeFromLeft (rotSliderWidth));

        coordinateArea.removeFromLeft (20);
        juce::Rectangle<int> sourceArea (
            coordinateArea.removeFromLeft (3 * rotSliderWidth + 2 * rotSliderSpacing));
        gcSourcePosition.setBounds (sourceArea);
        sourceArea.removeFromTop (25);

        sliderRow = (sourceArea.removeFromTop (rotSliderHeight));
        slSourceX.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slSourceY.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slSourceZ.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        lbSourceX.setBounds (sourceArea.removeFromLeft (rotSliderWidth));
        sourceArea.removeFromLeft (rotSliderSpacing);
        lbSourceY.setBounds (sourceArea.removeFromLeft (rotSliderWidth));
        sourceArea.removeFromLeft (rotSliderSpacing);
        lbSourceZ.setBounds (sourceArea.removeFromLeft (rotSliderWidth));

        coordinateArea.removeFromLeft (20);
        juce::Rectangle<int> listenerArea (
            coordinateArea.removeFromLeft (3 * rotSliderWidth + 2 * rotSliderSpacing));
        gcListenerPosition.setBounds (listenerArea);
        listenerArea.removeFromTop (25);

        sliderRow = (listenerArea.removeFromTop (rotSliderHeight));
        slListenerX.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slListenerY.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slListenerZ.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        lbListenerX.setBounds (listenerArea.removeFromLeft (rotSliderWidth));
        listenerArea.removeFromLeft (rotSliderSpacing);
        lbListenerY.setBounds (listenerArea.removeFromLeft (rotSliderWidth));
        listenerArea.removeFromLeft (rotSliderSpacing);
        lbListenerZ.setBounds (listenerArea.removeFromLeft (rotSliderWidth));
    }

    propArea.removeFromTop (10);
    gcReflectionProperties.setBounds (propArea);
    propArea.removeFromTop (20);

    auto buttonRow = propArea.removeFromBottom (20);
    tbDirectPathZeroDelay.setBounds (buttonRow.removeFromLeft (120));
    buttonRow.removeFromLeft (10);
    tbDirectPathUnityGain.setBounds (buttonRow.removeFromLeft (120));
    buttonRow.removeFromLeft (10);
    tbRenderDirectPath.setBounds (buttonRow.removeFromLeft (120));

    juce::Rectangle<int> fvCol (propArea.removeFromLeft (330));

    { // 120
        juce::Rectangle<int> fvRow (fvCol.removeFromTop (130));
        fv.setBounds (fvRow);

        fvCol.removeFromTop (10);

        juce::Rectangle<int> sliderRow (fvCol.removeFromTop (rotSliderHeight));
        sliderRow.removeFromLeft (20);
        slLowShelfFreq.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (sliderSpacing);
        slLowShelfGain.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow.removeFromRight (10);
        slHighShelfGain.setBounds (sliderRow.removeFromRight (rotSliderWidth));
        sliderRow.removeFromRight (sliderSpacing);
        slHighShelfFreq.setBounds (sliderRow.removeFromRight (rotSliderWidth));

        slReflCoeff.setBounds (sliderRow);

        sliderRow = fvCol.removeFromTop (labelHeight);
        sliderRow.removeFromLeft (20);
        lbLSF.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (sliderSpacing);
        lbLSG.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow.removeFromRight (10);
        lbHSG.setBounds (sliderRow.removeFromRight (rotSliderWidth));
        sliderRow.removeFromRight (sliderSpacing);
        lbHSF.setBounds (sliderRow.removeFromRight (rotSliderWidth));

        lbReflCoeff.setBounds (sliderRow);
    }

    auto wallAttenArea = propArea.removeFromTop (135);
    lbWallAttenuation.setBounds (wallAttenArea.removeFromTop (20));
    auto sliderRow = wallAttenArea.removeFromTop (rotSliderHeight - 10);
    slWallAttenuationFront.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    slWallAttenuationLeft.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    slWallAttenuationRight.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderRow = wallAttenArea.removeFromTop (12);
    lbWallAttenuationFront.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    lbWallAttenuationLeft.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    lbWallAttenuationRight.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderRow = wallAttenArea.removeFromTop (rotSliderHeight - 10);
    slWallAttenuationBack.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    slWallAttenuationCeiling.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    slWallAttenuationFloor.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderRow = wallAttenArea.removeFromTop (12);
    lbWallAttenuationBack.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    lbWallAttenuationCeiling.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (5);
    lbWallAttenuationFloor.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    propArea.removeFromTop (5);
    slNumReflections.setBounds (propArea.removeFromTop (rotSliderHeight));
    lbNumReflections.setBounds (propArea.removeFromTop (labelHeight));

    area.removeFromRight (10);

    {
        juce::Rectangle<int> planeArea (area.removeFromLeft (300));
        int height = planeArea.getHeight() / 2;

        xyPlane.setBounds (planeArea.removeFromTop (height));
        zyPlane.setBounds (planeArea.removeFromTop (height));
    }
}

void RoomEncoderAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &slRoomX || slider == &slRoomY || slider == &slRoomZ)
    {
        juce::Vector3D<float> dims (slRoomX.getValue(), slRoomY.getValue(), slRoomZ.getValue());
        float scale = juce::jmin (xyPlane.setDimensions (dims), zyPlane.setDimensions (dims));
        xyPlane.setScale (scale);
        zyPlane.setScale (scale);
    }
}

void RoomEncoderAudioProcessorEditor::buttonStateChanged (juce::Button* button)
{
    if (button == &tbDirectPathZeroDelay)
        rv.setZeroDelay (tbDirectPathZeroDelay.getToggleState());
}

void RoomEncoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.updateFv)
    {
        fv.setOverallGainInDecibels (*valueTreeState.getRawParameterValue ("reflCoeff"));
        fv.setSampleRate (processor.getSampleRate());
        processor.updateFv = false;
    }

    if (processor.repaintPositionPlanes.get())
    {
        processor.repaintPositionPlanes = false;
        xyPlane.repaint();
        zyPlane.repaint();
    }
}
