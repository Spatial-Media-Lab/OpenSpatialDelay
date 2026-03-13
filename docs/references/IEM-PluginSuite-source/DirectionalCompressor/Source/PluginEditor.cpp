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
DirectionalCompressorAudioProcessorEditor::DirectionalCompressorAudioProcessorEditor (
    DirectionalCompressorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    sphereElem (*valueTreeState.getParameter ("azimuth"),
                valueTreeState.getParameterRange ("azimuth"),
                *valueTreeState.getParameter ("elevation"),
                valueTreeState.getParameterRange ("elevation"))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (550, 445);
    setLookAndFeel (&globalLaF);

    addAndMakeVisible (&title);
    title.setTitle ("Directional", "Compressor");
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);

    addAndMakeVisible (&sphere);
    sphere.addElement (&sphereElem);

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));

    // ======== general components ===========
    addAndMakeVisible (&gcMask);
    gcMask.setText ("Spatial mask properties");
    gcMask.setTextLabelPosition (juce::Justification::centredLeft);
    gcMask.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcMask.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    gcMask.setVisible (true);

    addAndMakeVisible (&gcSettings);
    gcSettings.setText ("General settings");
    gcSettings.setTextLabelPosition (juce::Justification::centredLeft);
    gcSettings.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcSettings.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    gcSettings.setVisible (true);

    addAndMakeVisible (&slPreGain);
    slPreGainAttachment.reset (new SliderAttachment (valueTreeState, "preGain", slPreGain));
    slPreGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slPreGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slPreGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slPreGain.setTooltip ("PreGain");

    addAndMakeVisible (&slAzimuth);
    slAzimuthAttachment.reset (new SliderAttachment (valueTreeState, "azimuth", slAzimuth));
    slAzimuth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slAzimuth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slAzimuth.setReverse (true);
    slAzimuth.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slAzimuth.setRotaryParameters (juce::MathConstants<float>::pi,
                                   3 * juce::MathConstants<float>::pi,
                                   false);
    slAzimuth.setTooltip ("Azimuth angle of the spatial mask");

    addAndMakeVisible (&slElevation);
    slElevationAttachment.reset (new SliderAttachment (valueTreeState, "elevation", slElevation));
    slElevation.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slElevation.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slElevation.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slElevation.setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                     2.5 * juce::MathConstants<float>::pi,
                                     false);
    slElevation.setTooltip ("Elevation angle of the spatial mask");

    addAndMakeVisible (&slWidth);
    slWidthAttachment.reset (new SliderAttachment (valueTreeState, "width", slWidth));
    slWidth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slWidth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slWidth.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slWidth.setTooltip ("Width of the spatial mask");

    addAndMakeVisible (&cbListen);
    cbListenAttachment.reset (new ComboBoxAttachment (valueTreeState, "listen", cbListen));
    cbListen.setJustificationType (juce::Justification::centred);
    cbListen.addSectionHeading ("Listen to");
    cbListen.addItem ("Full", 1);
    cbListen.addItem ("Masked", 2);
    cbListen.addItem ("Unmasked", 3);
    cbListen.setSelectedId (*valueTreeState.getRawParameterValue ("listen") + 1);

    // ======== compressor 1 components ===========
    bool isOn = *valueTreeState.getRawParameterValue ("c1Enabled");

    addAndMakeVisible (&gcC1);
    gcC1.setText ("Compressor 1");
    gcC1.setTextLabelPosition (juce::Justification::centredLeft);
    gcC1.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcC1.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    gcC1.setVisible (true);

    addAndMakeVisible (&tbC1);
    tbC1Attachment.reset (new ButtonAttachment (valueTreeState, "c1Enabled", tbC1));
    tbC1.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);
    tbC1.setButtonText ("ON/OFF");
    tbC1.setName ("C1");
    tbC1.addListener (this);

    addAndMakeVisible (&cbC1Driving);
    cbC1DrivingAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "c1DrivingSignal", cbC1Driving));
    cbC1Driving.setJustificationType (juce::Justification::centred);
    cbC1Driving.addSectionHeading ("Driving signa");
    cbC1Driving.addItem ("Full", 1);
    cbC1Driving.addItem ("Masked", 2);
    cbC1Driving.addItem ("Unmasked", 3);
    cbC1Driving.setSelectedId (*valueTreeState.getRawParameterValue ("c1DrivingSignal") + 1);
    cbC1Driving.setEnabled (isOn);

    addAndMakeVisible (&cbC1Apply);
    cbC1ApplyAttachment.reset (new ComboBoxAttachment (valueTreeState, "c1Apply", cbC1Apply));
    cbC1Apply.setJustificationType (juce::Justification::centred);
    cbC1Apply.addSectionHeading ("Apply to");
    cbC1Apply.addItem ("Full", 1);
    cbC1Apply.addItem ("Masked", 2);
    cbC1Apply.addItem ("Unmasked", 3);
    cbC1Apply.setSelectedId (*valueTreeState.getRawParameterValue ("c1Apply") + 1);
    cbC1Apply.setEnabled (isOn);

    addAndMakeVisible (&slC1Threshold);
    slC1ThresholdAttachment.reset (
        new SliderAttachment (valueTreeState, "c1Threshold", slC1Threshold));
    slC1Threshold.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Threshold.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Threshold.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[2]);
    slC1Threshold.setTextValueSuffix (" dB");
    slC1Threshold.setEnabled (isOn);

    addAndMakeVisible (&slC1Knee);
    slC1KneeAttachment.reset (new SliderAttachment (valueTreeState, "c1Knee", slC1Knee));
    slC1Knee.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Knee.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Knee.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slC1Knee.setEnabled (isOn);

    addAndMakeVisible (&slC1Ratio);
    slC1RatioAttachment.reset (new SliderAttachment (valueTreeState, "c1Ratio", slC1Ratio));
    slC1Ratio.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Ratio.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Ratio.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    slC1Ratio.setTextValueSuffix (" : 1");
    slC1Ratio.setEnabled (isOn);

    addAndMakeVisible (&slC1Attack);
    slC1AttackAttachment.reset (new SliderAttachment (valueTreeState, "c1Attack", slC1Attack));
    slC1Attack.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Attack.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Attack.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slC1Attack.setTextValueSuffix (" ms");
    slC1Attack.setEnabled (isOn);

    addAndMakeVisible (&slC1Release);
    slC1ReleaseAttachment.reset (new SliderAttachment (valueTreeState, "c1Release", slC1Release));
    slC1Release.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Release.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Release.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slC1Release.setEnabled (isOn);

    addAndMakeVisible (&slC1Makeup);
    slC1MakeupAttachment.reset (new SliderAttachment (valueTreeState, "c1Makeup", slC1Makeup));
    slC1Makeup.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC1Makeup.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC1Makeup.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slC1Makeup.setEnabled (isOn);

    addAndMakeVisible (&dbC1GRmeter);
    dbC1GRmeter.setMinLevel (-25.0f);
    dbC1GRmeter.setColour (juce::Colours::red.withMultipliedAlpha (0.8f));
    dbC1GRmeter.setGainReductionMeter (true);
    dbC1GRmeter.setEnabled (isOn);

    addAndMakeVisible (&dbC1RMSmeter);
    dbC1RMSmeter.setMinLevel (-60.0f);
    dbC1RMSmeter.setColour (juce::Colours::green.withMultipliedAlpha (0.8f));
    dbC1RMSmeter.setGainReductionMeter (false);
    dbC1RMSmeter.setEnabled (isOn);

    addAndMakeVisible (&lbC1Threshold);
    lbC1Threshold.setText ("Threshold");
    lbC1Threshold.setEnabled (isOn);

    addAndMakeVisible (&lbC1Knee);
    lbC1Knee.setText ("Knee");
    lbC1Knee.setEnabled (isOn);

    addAndMakeVisible (&lbC1Ratio);
    lbC1Ratio.setText ("Ratio");
    lbC1Ratio.setEnabled (isOn);

    addAndMakeVisible (&lbC1Attack);
    lbC1Attack.setText ("Attack");
    lbC1Attack.setEnabled (isOn);

    addAndMakeVisible (&lbC1Release);
    lbC1Release.setText ("Release");
    lbC1Release.setEnabled (isOn);

    addAndMakeVisible (&lbC1Makeup);
    lbC1Makeup.setText ("MakeUp");
    lbC1Makeup.setEnabled (isOn);

    // ======== compressor 2 components ===========
    isOn = *valueTreeState.getRawParameterValue ("c2Enabled");

    addAndMakeVisible (&gcC2);
    gcC2.setText ("Compressor 2");
    gcC2.setTextLabelPosition (juce::Justification::centredLeft);
    gcC2.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    gcC2.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    gcC2.setVisible (true);

    addAndMakeVisible (&tbC2);
    tbC2Attachment.reset (new ButtonAttachment (valueTreeState, "c2Enabled", tbC2));
    tbC2.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);
    tbC2.setButtonText ("ON/OFF");
    tbC2.setName ("C2");
    tbC2.addListener (this);

    addAndMakeVisible (&cbC2Driving);
    cbC2DrivingAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "c2DrivingSignal", cbC2Driving));
    cbC2Driving.setJustificationType (juce::Justification::centred);
    cbC2Driving.addSectionHeading ("Driving signal");
    cbC2Driving.addItem ("Full", 1);
    cbC2Driving.addItem ("Masked", 2);
    cbC2Driving.addItem ("Unmasked", 3);
    cbC2Driving.setSelectedId (*valueTreeState.getRawParameterValue ("c2DrivingSignal") + 1);
    cbC2Driving.setEnabled (isOn);

    addAndMakeVisible (&cbC2Apply);
    cbC2ApplyAttachment.reset (new ComboBoxAttachment (valueTreeState, "c2Apply", cbC2Apply));
    cbC2Apply.setJustificationType (juce::Justification::centred);
    cbC2Apply.addSectionHeading ("Apply to");
    cbC2Apply.addItem ("Full", 1);
    cbC2Apply.addItem ("Masked", 2);
    cbC2Apply.addItem ("Unmasked", 3);
    cbC2Apply.setSelectedId (*valueTreeState.getRawParameterValue ("c2Apply") + 1);
    cbC2Apply.setEnabled (isOn);

    addAndMakeVisible (&slC2Threshold);
    slC2ThresholdAttachment.reset (
        new SliderAttachment (valueTreeState, "c2Threshold", slC2Threshold));
    slC2Threshold.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Threshold.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Threshold.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[2]);
    slC2Threshold.setTextValueSuffix (" dB");
    slC2Threshold.setEnabled (isOn);

    addAndMakeVisible (&slC2Knee);
    slC2KneeAttachment.reset (new SliderAttachment (valueTreeState, "c2Knee", slC2Knee));
    slC2Knee.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Knee.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Knee.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slC2Knee.setEnabled (isOn);

    addAndMakeVisible (&slC2Ratio);
    slC2RatioAttachment.reset (new SliderAttachment (valueTreeState, "c2Ratio", slC2Ratio));
    slC2Ratio.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Ratio.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Ratio.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    slC2Ratio.setTextValueSuffix (" : 1");
    slC2Ratio.setEnabled (isOn);

    addAndMakeVisible (&slC2Attack);
    slC2AttackAttachment.reset (new SliderAttachment (valueTreeState, "c2Attack", slC2Attack));
    slC2Attack.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Attack.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Attack.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slC2Attack.setTextValueSuffix (" ms");
    slC2Attack.setEnabled (isOn);

    addAndMakeVisible (&slC2Release);
    slC2ReleaseAttachment.reset (new SliderAttachment (valueTreeState, "c2Release", slC2Release));
    slC2Release.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Release.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Release.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slC2Release.setTextValueSuffix (" ms");
    slC2Release.setEnabled (isOn);

    addAndMakeVisible (&slC2Makeup);
    slC2MakeupAttachment.reset (new SliderAttachment (valueTreeState, "c2Makeup", slC2Makeup));
    slC2Makeup.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slC2Makeup.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slC2Makeup.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slC2Makeup.setTextValueSuffix (" dB");
    slC2Makeup.setEnabled (isOn);

    addAndMakeVisible (&dbC2GRmeter);
    dbC2GRmeter.setMinLevel (-25.0f);
    dbC2GRmeter.setColour (juce::Colours::red.withMultipliedAlpha (0.8f));
    dbC2GRmeter.setGainReductionMeter (true);
    dbC2GRmeter.setEnabled (isOn);

    addAndMakeVisible (&dbC2RMSmeter);
    dbC2RMSmeter.setMinLevel (-60.0f);
    dbC2RMSmeter.setColour (juce::Colours::green.withMultipliedAlpha (0.8f));
    dbC2RMSmeter.setGainReductionMeter (false);
    dbC2RMSmeter.setEnabled (isOn);

    addAndMakeVisible (&lbC2Threshold);
    lbC2Threshold.setText ("Threshold");
    lbC2Threshold.setEnabled (isOn);

    addAndMakeVisible (&lbC2Knee);
    lbC2Knee.setText ("Knee");
    lbC2Knee.setEnabled (isOn);

    addAndMakeVisible (&lbC2Ratio);
    lbC2Ratio.setText ("Ratio");
    lbC2Ratio.setEnabled (isOn);

    addAndMakeVisible (&lbC2Attack);
    lbC2Attack.setText ("Attack");
    lbC2Attack.setEnabled (isOn);

    addAndMakeVisible (&lbC2Release);
    lbC2Release.setText ("Release");
    lbC2Release.setEnabled (isOn);

    addAndMakeVisible (&lbC2Makeup);
    lbC2Makeup.setText ("MakeUp");
    lbC2Makeup.setEnabled (isOn);

    // ===== LABELS =====
    addAndMakeVisible (&lbPreGain);
    lbPreGain.setText ("Pre Gain");
    addAndMakeVisible (&lbAzimuth);
    lbAzimuth.setText ("Azimuth");
    addAndMakeVisible (&lbElevation);
    lbElevation.setText ("Elevation");
    addAndMakeVisible (&lbWidth);
    lbWidth.setText ("Width");

    startTimer (50);
}

DirectionalCompressorAudioProcessorEditor::~DirectionalCompressorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DirectionalCompressorAudioProcessorEditor::buttonStateChanged (juce::Button* button)
{
    bool isOn = button->getToggleState();
    if (button->getName() == "C1")
    {
        slC1Threshold.setEnabled (isOn);
        slC1Knee.setEnabled (isOn);
        slC1Ratio.setEnabled (isOn);
        slC1Attack.setEnabled (isOn);
        slC1Release.setEnabled (isOn);
        slC1Makeup.setEnabled (isOn);

        lbC1Threshold.setEnabled (isOn);
        lbC1Knee.setEnabled (isOn);
        lbC1Ratio.setEnabled (isOn);
        lbC1Attack.setEnabled (isOn);
        lbC1Release.setEnabled (isOn);
        lbC1Makeup.setEnabled (isOn);

        cbC1Driving.setEnabled (isOn);
        cbC1Apply.setEnabled (isOn);
        ;
    }
    else if (button->getName() == "C2")
    {
        slC2Threshold.setEnabled (isOn);
        slC2Knee.setEnabled (isOn);
        slC2Ratio.setEnabled (isOn);
        slC2Attack.setEnabled (isOn);
        slC2Release.setEnabled (isOn);
        slC2Makeup.setEnabled (isOn);

        lbC2Threshold.setEnabled (isOn);
        lbC2Knee.setEnabled (isOn);
        lbC2Ratio.setEnabled (isOn);
        lbC2Attack.setEnabled (isOn);
        lbC2Release.setEnabled (isOn);
        lbC2Makeup.setEnabled (isOn);

        cbC2Driving.setEnabled (isOn);
        cbC2Apply.setEnabled (isOn);
        ;
    }
};

//==============================================================================
void DirectionalCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void DirectionalCompressorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    auto sizes = processor.getMaxSize();
    sizes.first = juce::jmin (sizes.first, sizes.second);
    sizes.second = sizes.first;
    title.setMaxSize (sizes);
    // ==========================================

    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        sphere.repaint();
    }

    dbC1RMSmeter.setLevel (processor.c1MaxRMS);
    dbC1GRmeter.setLevel (processor.c1MaxGR);
    dbC2RMSmeter.setLevel (processor.c2MaxRMS);
    dbC2GRmeter.setLevel (processor.c2MaxGR);
}

void DirectionalCompressorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int sliderWidth = 45;
    //    const int labelHeight = 15;
    //    const int sliderHeight = 60;
    const int sliderSpacing = 15;

    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);

    juce::Rectangle<int> temp; //work area
    juce::Rectangle<int> sliderRow;
    juce::Rectangle<int> comprRow;
    comprRow = area.removeFromBottom (200);

    // ======== compressor 1
    {
        temp = comprRow.removeFromLeft (225);
        gcC1.setBounds (temp);
        sliderRow = temp.removeFromTop (15);
        sliderRow.removeFromRight (60);
        tbC1.setBounds (sliderRow.removeFromRight (40));
        temp.removeFromTop (10);

        // dbMeters
        dbC1RMSmeter.setBounds (temp.removeFromLeft (20));
        temp.removeFromLeft (10);
        dbC1GRmeter.setBounds (temp.removeFromRight (20));
        temp.removeFromRight (10);

        // comboBoxes
        sliderRow = temp.removeFromTop (15);
        cbC1Driving.setBounds (sliderRow.removeFromLeft (75));
        cbC1Apply.setBounds (sliderRow.removeFromRight (75));

        temp.removeFromTop (5); //spacing

        //first juce::Sliders
        sliderRow = temp.removeFromTop (60);
        slC1Threshold.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC1Knee.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC1Makeup.setBounds (sliderRow.removeFromLeft (45));

        sliderRow = temp.removeFromTop (15);
        lbC1Threshold.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (sliderSpacing - 5);
        lbC1Knee.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC1Makeup.setBounds (sliderRow.removeFromLeft (45));

        temp.removeFromTop (5); //spacing

        // next juce::Sliders
        sliderRow = temp.removeFromTop (60);
        slC1Ratio.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC1Attack.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC1Release.setBounds (sliderRow.removeFromLeft (45));

        sliderRow = temp.removeFromTop (15);
        lbC1Ratio.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC1Attack.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC1Release.setBounds (sliderRow.removeFromLeft (45));
    }

    // ======== compressor 2
    {
        temp = comprRow.removeFromRight (225);
        gcC2.setBounds (temp);
        sliderRow = temp.removeFromTop (15);
        sliderRow.removeFromRight (60);
        tbC2.setBounds (sliderRow.removeFromRight (40));
        temp.removeFromTop (10);

        // dbMeters
        dbC2RMSmeter.setBounds (temp.removeFromLeft (20));
        temp.removeFromLeft (10);
        dbC2GRmeter.setBounds (temp.removeFromRight (20));
        temp.removeFromRight (10);

        // comboBoxes
        sliderRow = temp.removeFromTop (15);
        cbC2Driving.setBounds (sliderRow.removeFromLeft (75));
        cbC2Apply.setBounds (sliderRow.removeFromRight (75));

        temp.removeFromTop (5); //spacing

        //first juce::Sliders
        sliderRow = temp.removeFromTop (60);
        slC2Threshold.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC2Knee.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC2Makeup.setBounds (sliderRow.removeFromLeft (45));

        sliderRow = temp.removeFromTop (15);
        lbC2Threshold.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (sliderSpacing - 5);
        lbC2Knee.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC2Makeup.setBounds (sliderRow.removeFromLeft (45));

        temp.removeFromTop (5); //spacing

        // next juce::Sliders
        sliderRow = temp.removeFromTop (60);
        slC2Ratio.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC2Attack.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        slC2Release.setBounds (sliderRow.removeFromLeft (45));

        sliderRow = temp.removeFromTop (15);
        lbC2Ratio.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC2Attack.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (sliderSpacing);
        lbC2Release.setBounds (sliderRow.removeFromLeft (45));
    }

    area.removeFromBottom (10); //spacing

    int height = area.getHeight();
    sphere.setBounds (area.removeFromLeft (height));

    area.removeFromLeft (10); //spacing
    temp = area.removeFromLeft (165);

    // MASK PROPERTIES
    gcMask.setBounds (temp);
    temp.removeFromTop (25); //spacing

    sliderRow = temp.removeFromTop (60);
    slAzimuth.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    slElevation.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    slWidth.setBounds (sliderRow.removeFromLeft (sliderWidth));

    sliderRow = temp.removeFromTop (15);
    lbAzimuth.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbElevation.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbWidth.setBounds (sliderRow.removeFromLeft (sliderWidth));

    area.removeFromLeft (30); //spacing

    // GENERAL SETTINGS

    gcSettings.setBounds (area);
    area.removeFromTop (25); //spacing

    temp = area.removeFromLeft (sliderWidth);
    sliderRow = temp.removeFromTop (60);
    slPreGain.setBounds (sliderRow.removeFromLeft (sliderWidth));

    sliderRow = temp.removeFromTop (15);
    lbPreGain.setBounds (sliderRow.removeFromLeft (sliderWidth));

    area.removeFromLeft (15); //spacing
    cbListen.setBounds (area.removeFromTop (15));
}
