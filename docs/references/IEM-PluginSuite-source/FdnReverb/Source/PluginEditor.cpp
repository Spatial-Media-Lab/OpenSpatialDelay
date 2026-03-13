/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
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
FdnReverbAudioProcessorEditor::FdnReverbAudioProcessorEditor (
    FdnReverbAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    tv (20.f, 20000.f, 0.1f, 25.f, 5.f),
    fv (20.f, 20000.f, -80.f, 5.f, 5.f, false)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setResizeLimits (600, 480, 1000, 950);
    setLookAndFeel (&globalLaF);

    freezeMode.addListener (this);

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("FDN"), juce::String ("Reverb"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    addAndMakeVisible (&footer);
    addAndMakeVisible (&delayGroup);
    delayGroup.setText ("General Settings");
    delayGroup.setTextLabelPosition (juce::Justification::centredLeft);
    delayGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    delayGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    delayGroup.setVisible (true);

    addAndMakeVisible (&filterGroup);
    filterGroup.setText ("Filter Settings");
    addAndMakeVisible (&t60Group);
    t60Group.setText ("Reverberation Time");
    t60Group.setTextLabelPosition (juce::Justification::centredLeft);
    t60Group.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    t60Group.setColour (juce::GroupComponent::textColourId, juce::Colours::red);
    t60Group.setVisible (true);

    addAndMakeVisible (&delayLengthSlider);
    delayAttachment.reset (new SliderAttachment (valueTreeState, "delayLength", delayLengthSlider));
    delayLengthSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delayLengthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    delayLengthSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[1]);
    delayLengthSlider.setTooltip ("Room Size");

    addAndMakeVisible (&revTimeSlider);
    feedbackAttachment.reset (new SliderAttachment (valueTreeState, "revTime", revTimeSlider));
    revTimeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    revTimeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    revTimeSlider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white);
    revTimeSlider.setTooltip ("Reverberation Time");
    revTimeSlider.addListener (this);

    addAndMakeVisible (&fadeInSlider);
    fadeInAttachment.reset (new SliderAttachment (valueTreeState, "fadeInTime", fadeInSlider));
    fadeInSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    fadeInSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    fadeInSlider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white);
    fadeInSlider.setTooltip ("FadeIn Time");

    addAndMakeVisible (&dryWetSlider);
    dryWetAttachment.reset (new SliderAttachment (valueTreeState, "dryWet", dryWetSlider));
    dryWetSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    dryWetSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    dryWetSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                            globalLaF.ClWidgetColours[2]);
    dryWetSlider.setTooltip ("Dry/Wet");

    addAndMakeVisible (&lowCutoffSlider);
    lowCutoffAttachment.reset (new SliderAttachment (valueTreeState, "lowCutoff", lowCutoffSlider));
    lowCutoffSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lowCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    lowCutoffSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                               globalLaF.ClWidgetColours[3]);
    lowCutoffSlider.setTooltip ("Low Shelf Cutoff Freq");
    lowCutoffSlider.addListener (this);

    addAndMakeVisible (&lowQSlider);
    lowQAttachment.reset (new SliderAttachment (valueTreeState, "lowQ", lowQSlider));
    lowQSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lowQSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    lowQSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);
    lowQSlider.setTooltip ("Low Shelf Q");
    lowQSlider.addListener (this);

    addAndMakeVisible (&lowGainSlider);
    lowGainAttachment.reset (new SliderAttachment (valueTreeState, "lowGain", lowGainSlider));
    lowGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lowGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    lowGainSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[3]);
    lowGainSlider.setTooltip ("Low Shelf Gain");
    lowGainSlider.addListener (this);

    addAndMakeVisible (&highCutoffSlider);
    highCutoffAttachment.reset (
        new SliderAttachment (valueTreeState, "highCutoff", highCutoffSlider));
    highCutoffSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    highCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    highCutoffSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                                globalLaF.ClWidgetColours[0]);
    ;
    highCutoffSlider.setTooltip ("High Shelf Cutoff Freq");
    highCutoffSlider.addListener (this);

    addAndMakeVisible (&highQSlider);
    highQAttachment.reset (new SliderAttachment (valueTreeState, "highQ", highQSlider));
    highQSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    highQSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    highQSlider.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    highQSlider.setTooltip ("High Shelf Q");
    highQSlider.addListener (this);

    addAndMakeVisible (&highGainSlider);
    highGainAttachment.reset (new SliderAttachment (valueTreeState, "highGain", highGainSlider));
    highGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    highGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    highGainSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[0]);
    highGainSlider.setTooltip ("High Shelf Gain");
    highGainSlider.addListener (this);

    addAndMakeVisible (cbFdnSize);
    cbFdnSize.addSectionHeading ("Fdn Size");
    cbFdnSize.addItem ("16", 1);
    cbFdnSize.addItem ("32", 2);
    cbFdnSize.addItem ("64", 3);
    cbFdnSize.setJustificationType (juce::Justification::centred);
    cbFdnSizeAttachment.reset (new ComboBoxAttachment (valueTreeState, "fdnSize", cbFdnSize));

    addAndMakeVisible (&freezeMode);
    freezeMode.setButtonText ("Freeze");
    addAndMakeVisible (&lbDelay);
    lbDelay.setText ("Room Size");
    addAndMakeVisible (&lbTime);
    lbTime.setText ("Rev. Time");
    addAndMakeVisible (&lbDryWet);
    lbDryWet.setText ("Dry/Wet");
    addAndMakeVisible (&lbHighCutoff);
    lbHighCutoff.setText ("Freq.");
    addAndMakeVisible (&lbHighQ);
    lbHighQ.setText ("Q");
    addAndMakeVisible (&lbHighGain);
    lbHighGain.setText ("Gain");
    addAndMakeVisible (&lbLowCutoff);
    lbLowCutoff.setText ("Freq.");
    addAndMakeVisible (&lbLowQ);
    lbLowQ.setText ("Q");
    addAndMakeVisible (&lbLowGain);
    lbLowGain.setText ("Gain");
    addAndMakeVisible (fdnSize);
    fdnSize.setText ("Fdn Size");
    addAndMakeVisible (&fdnLbTime);
    fdnLbTime.setText ("Fade In");

    // left side
    addAndMakeVisible (&tv);
    lowpassCoeffs = IIR::Coefficients<float>::makeLowShelf (
        48000,
        lowCutoffSlider.getValue(),
        lowQSlider.getValue(),
        juce::Decibels::decibelsToGain (lowGainSlider.getValue()));
    highpassCoeffs = IIR::Coefficients<float>::makeHighShelf (
        48000,
        highCutoffSlider.getValue(),
        highQSlider.getValue(),
        juce::Decibels::decibelsToGain (highGainSlider.getValue()));

    tv.addCoefficients (lowpassCoeffs, juce::Colours::orangered, &lowCutoffSlider, &lowGainSlider);
    tv.addCoefficients (highpassCoeffs, juce::Colours::cyan, &highCutoffSlider, &highGainSlider);

    float gain = pow (10.0, -3.0 / revTimeSlider.getValue());
    tv.setOverallGain (gain);

    addAndMakeVisible (&fv);
    fv.addCoefficients (lowpassCoeffs,
                        globalLaF.ClWidgetColours[3],
                        &lowCutoffSlider,
                        &lowGainSlider,
                        &lowQSlider);
    fv.addCoefficients (highpassCoeffs,
                        globalLaF.ClWidgetColours[0],
                        &highCutoffSlider,
                        &highGainSlider,
                        &highQSlider);
    fv.setOverallGain (gain);
}

FdnReverbAudioProcessorEditor::~FdnReverbAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void FdnReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void FdnReverbAudioProcessorEditor::timerCallback()
{
}

void FdnReverbAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &freezeMode)
    {
        if (freezeMode.getToggleState())
            processor.setFreezeMode (true);
        else
            processor.setFreezeMode (false);
    }
}

void FdnReverbAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &highCutoffSlider || slider == &highQSlider || slider == &highGainSlider)
    {
        *highpassCoeffs = *IIR::Coefficients<float>::makeHighShelf (
            48000,
            highCutoffSlider.getValue(),
            highQSlider.getValue(),
            juce::Decibels::decibelsToGain (highGainSlider.getValue()));

        tv.repaint();
        fv.repaint();
    }
    else if (slider == &lowCutoffSlider || slider == &lowQSlider || slider == &lowGainSlider)
    {
        *lowpassCoeffs = *IIR::Coefficients<float>::makeLowShelf (
            48000,
            lowCutoffSlider.getValue(),
            lowQSlider.getValue(),
            juce::Decibels::decibelsToGain (lowGainSlider.getValue()));

        tv.repaint();
        fv.repaint();
    }
    else if (slider == &revTimeSlider)
    {
        float gain = pow (10.0, -3.0 / revTimeSlider.getValue());
        fv.setOverallGain (gain);
        tv.setOverallGain (gain);
    }
}

void FdnReverbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int labelHeight = 20;

    juce::Rectangle<int> area (getLocalBounds());
    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);

    { //====================== DELAY SETTINGS GROUP ==================================
        const int rotSliderWidth = 55;

        juce::Rectangle<int> settingsArea (area.removeFromRight (185));
        delayGroup.setBounds (settingsArea);
        settingsArea.removeFromTop (25); //for box headline

        juce::Rectangle<int> sliderRow (settingsArea.removeFromTop (rotSliderHeight));

        delayLengthSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        revTimeSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        dryWetSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = settingsArea.removeFromTop (labelHeight);
        lbDelay.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbTime.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbDryWet.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = settingsArea.removeFromTop (rotSliderHeight);
        cbFdnSize.setBounds (sliderRow.removeFromLeft (rotSliderWidth).removeFromBottom (18));
        sliderRow.removeFromLeft (rotSliderSpacing);
        fadeInSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = settingsArea.removeFromTop (labelHeight);
        fdnSize.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        fdnLbTime.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    }

    area.removeFromRight (10); //spacing

    const int height = (area.getHeight() - 10 - labelHeight - rotSliderHeight + 10) / 2;
    { //====================== T60 GROUP ==================================
        juce::Rectangle<int> t60Area (area.removeFromTop (height));
        t60Group.setBounds (t60Area);
        t60Area.removeFromTop (25);
        tv.setBounds (t60Area);
    }

    area.removeFromTop (10); //spacing

    { //====================== FILTER GROUP ==================================
        const int rotSliderWidth = 40;
        juce::Rectangle<int> filterArea (area);
        filterGroup.setBounds (filterArea);
        filterArea.removeFromTop (25);

        juce::Rectangle<int> sliderRow (filterArea.removeFromBottom (labelHeight));
        lbLowCutoff.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowQ.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowGain.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

        lbHighGain.setBounds (sliderRow.removeFromRight (rotSliderWidth + 5));
        sliderRow.removeFromRight (rotSliderSpacing);
        lbHighQ.setBounds (sliderRow.removeFromRight (rotSliderWidth));
        sliderRow.removeFromRight (rotSliderSpacing);
        lbHighCutoff.setBounds (sliderRow.removeFromRight (rotSliderWidth));

        sliderRow = filterArea.removeFromBottom (rotSliderHeight - 10);

        lowCutoffSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lowQSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lowGainSlider.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

        highGainSlider.setBounds (sliderRow.removeFromRight (rotSliderWidth + 5));
        sliderRow.removeFromRight (rotSliderSpacing);
        highQSlider.setBounds (sliderRow.removeFromRight (rotSliderWidth));
        sliderRow.removeFromRight (rotSliderSpacing);
        highCutoffSlider.setBounds (sliderRow.removeFromRight (rotSliderWidth));

        fv.setBounds (filterArea);
    }
}
