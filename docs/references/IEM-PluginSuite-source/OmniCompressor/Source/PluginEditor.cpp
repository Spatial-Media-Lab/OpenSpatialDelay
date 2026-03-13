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
OmniCompressorAudioProcessorEditor::OmniCompressorAudioProcessorEditor (
    OmniCompressorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    characteristic (&processor.compressor)
{
    setSize (330, 500);
    setLookAndFeel (&globalLaF);

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Omni"), juce::String ("Compressor"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);

    addAndMakeVisible (characteristic);

    addAndMakeVisible (&tbLookAhead);
    tbLookAheadAttachment.reset (new ButtonAttachment (valueTreeState, "lookAhead", tbLookAhead));
    tbLookAhead.setButtonText ("Look ahead (5ms)");
    tbLookAhead.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&sliderKnee);
    KnAttachment.reset (new SliderAttachment (valueTreeState, "knee", sliderKnee));
    sliderKnee.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderKnee.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderKnee.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    sliderKnee.setTextValueSuffix (" dB");

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));

    addAndMakeVisible (&sliderThreshold);
    ThAttachment.reset (new SliderAttachment (valueTreeState, "threshold", sliderThreshold));
    sliderThreshold.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderThreshold.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderThreshold.setColour (juce::Slider::rotarySliderOutlineColourId,
                               globalLaF.ClWidgetColours[2]);
    sliderThreshold.setTextValueSuffix (" dB");

    addAndMakeVisible (&sliderRatio);
    RaAttachment.reset (new SliderAttachment (valueTreeState, "ratio", sliderRatio));
    sliderRatio.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderRatio.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderRatio.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    //    sliderRatio.setTextValueSuffix("");

    addAndMakeVisible (&sliderAttackTime);
    ATAttachment.reset (new SliderAttachment (valueTreeState, "attack", sliderAttackTime));
    sliderAttackTime.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderAttackTime.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderAttackTime.setColour (juce::Slider::rotarySliderOutlineColourId,
                                globalLaF.ClWidgetColours[0]);
    sliderAttackTime.setTextValueSuffix (" ms");

    addAndMakeVisible (&sliderReleaseTime);
    RTAttachment.reset (new SliderAttachment (valueTreeState, "release", sliderReleaseTime));
    sliderReleaseTime.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderReleaseTime.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderReleaseTime.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[0]);
    sliderReleaseTime.setTextValueSuffix (" ms");

    addAndMakeVisible (&sliderMakeupGain);
    MGAttachment.reset (new SliderAttachment (valueTreeState, "outGain", sliderMakeupGain));
    sliderMakeupGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderMakeupGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    sliderMakeupGain.setColour (juce::Slider::rotarySliderOutlineColourId,
                                globalLaF.ClWidgetColours[1]);
    sliderMakeupGain.setTextValueSuffix (" dB");

    addAndMakeVisible (&dbGRmeter);
    dbGRmeter.setMinLevel (-25.0f);
    dbGRmeter.setColour (juce::Colours::red.withMultipliedAlpha (0.8f));
    dbGRmeter.setGainReductionMeter (true);

    addAndMakeVisible (&inpMeter);
    inpMeter.setMinLevel (-60.0f);
    inpMeter.setColour (juce::Colours::green.withMultipliedAlpha (0.8f));
    inpMeter.setGainReductionMeter (false);

    // ===== LABELS =====
    addAndMakeVisible (&lbKnee);
    lbKnee.setText ("Knee");

    addAndMakeVisible (&lbThreshold);
    lbThreshold.setText ("Threshold");

    addAndMakeVisible (&lbOutGain);
    lbOutGain.setText ("Makeup");

    addAndMakeVisible (&lbRatio);
    lbRatio.setText ("Ratio");

    addAndMakeVisible (&lbAttack);
    lbAttack.setText ("Attack");

    addAndMakeVisible (&lbRelease);
    lbRelease.setText ("Release");

    startTimer (50);
}

OmniCompressorAudioProcessorEditor::~OmniCompressorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void OmniCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void OmniCompressorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    characteristic.setMarkerLevels (processor.maxRMS.get(), processor.maxGR.get());
    characteristic.updateCharacteristic();
    characteristic.repaint();

    inpMeter.setLevel (processor.maxRMS.get());
    dbGRmeter.setLevel (processor.maxGR.get());
}

void OmniCompressorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int sliderHeight = 70;
    const int labelHeight = 15;
    const int sliderSpacing = 20;
    const int sliderWidth = 55;
    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);

    juce::Rectangle<int> ctrlPlane = area.removeFromBottom (180);
    ctrlPlane.setWidth (270);
    ctrlPlane.setCentre (area.getCentreX(), ctrlPlane.getCentreY());

    inpMeter.setBounds (ctrlPlane.removeFromLeft (20));
    ctrlPlane.removeFromLeft (10);
    dbGRmeter.setBounds (ctrlPlane.removeFromRight (20));
    ctrlPlane.removeFromRight (10);

    juce::Rectangle<int> sliderRow;

    sliderRow = ctrlPlane.removeFromTop (sliderHeight);

    sliderThreshold.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    sliderKnee.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    sliderMakeupGain.setBounds (sliderRow.removeFromLeft (sliderWidth));

    sliderRow = ctrlPlane.removeFromTop (labelHeight);
    lbThreshold.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbKnee.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbOutGain.setBounds (sliderRow.removeFromLeft (sliderWidth));

    ctrlPlane.removeFromTop (10);

    sliderRow = ctrlPlane.removeFromTop (sliderHeight);

    sliderRatio.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    sliderAttackTime.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    sliderReleaseTime.setBounds (sliderRow.removeFromLeft (sliderWidth));

    sliderRow = ctrlPlane.removeFromTop (labelHeight);
    lbRatio.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbAttack.setBounds (sliderRow.removeFromLeft (sliderWidth));
    sliderRow.removeFromLeft (sliderSpacing);
    lbRelease.setBounds (sliderRow.removeFromLeft (sliderWidth));

    area.removeFromBottom (10);
    tbLookAhead.setBounds (area.removeFromBottom (20).removeFromLeft (130));
    area.removeFromBottom (10);
    characteristic.setBounds (area);
}
