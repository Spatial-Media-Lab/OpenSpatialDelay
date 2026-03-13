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
EnergyVisualizerAudioProcessorEditor::EnergyVisualizerAudioProcessorEditor (
    EnergyVisualizerAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setResizeLimits (710, 410, 1500, 1200);
    setLookAndFeel (&globalLaF);

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Energy"), juce::String ("Visualizer"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));

    addAndMakeVisible (&slPeakLevel);
    slPeakLevelAttachment.reset (new SliderAttachment (valueTreeState, "peakLevel", slPeakLevel));
    slPeakLevel.setSliderStyle (juce::Slider::LinearVertical);
    slPeakLevel.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 12);
    slPeakLevel.setTextValueSuffix (" dB");
    slPeakLevel.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slPeakLevel.setReverse (false);
    slPeakLevel.addListener (this);

    addAndMakeVisible (&slDynamicRange);
    slDynamicRangeAttachment.reset (
        new SliderAttachment (valueTreeState, "dynamicRange", slDynamicRange));
    slDynamicRange.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slDynamicRange.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 12);
    slDynamicRange.setTextValueSuffix (" dB");
    slDynamicRange.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[0]);
    slDynamicRange.setReverse (false);
    slDynamicRange.addListener (this);

    addAndMakeVisible (&slRMStimeConstant);
    slRMStimeConstantAttachment.reset (
        new SliderAttachment (valueTreeState, "RMStimeConstant", slRMStimeConstant));
    slRMStimeConstant.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRMStimeConstant.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 12);
    slRMStimeConstant.setTextValueSuffix (" ms");
    slRMStimeConstant.setColour (juce::Slider::rotarySliderOutlineColourId,
                                 globalLaF.ClWidgetColours[2]);
    slRMStimeConstant.setReverse (false);
    slRMStimeConstant.addListener (this);

    addAndMakeVisible (tbHoldMax);
    tbHoldMaxAttachment.reset (new ButtonAttachment (valueTreeState, "holdMax", tbHoldMax));
    tbHoldMax.setButtonText ("Hold max");
    tbHoldMax.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (&lbPeakLevel);
    lbPeakLevel.setText ("Peak level");

    addAndMakeVisible (&lbDynamicRange);
    lbDynamicRange.setText ("Range");

    addAndMakeVisible (&lbRMStimeConstant);
    lbRMStimeConstant.setText ("Time Constant");

    addAndMakeVisible (&visualizer);
    visualizer.setRmsDataPtr (p.rms.data());

    addAndMakeVisible (&colormap);

    startTimer (20);
}

EnergyVisualizerAudioProcessorEditor::~EnergyVisualizerAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void EnergyVisualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void EnergyVisualizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
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
    area.removeFromBottom (5);

    juce::Rectangle<int> UIarea = area.removeFromRight (106);
    const juce::Point<int> UIareaCentre = UIarea.getCentre();
    UIarea.setHeight (320);
    UIarea.setCentre (UIareaCentre);

    juce::Rectangle<int> dynamicsArea = UIarea.removeFromTop (210);
    juce::Rectangle<int> sliderCol = dynamicsArea.removeFromRight (50);

    lbDynamicRange.setBounds (sliderCol.removeFromBottom (12));
    slDynamicRange.setBounds (sliderCol.removeFromBottom (50));

    sliderCol.removeFromBottom (20);

    lbPeakLevel.setBounds (sliderCol.removeFromBottom (12));
    slPeakLevel.setBounds (sliderCol);

    dynamicsArea.removeFromRight (6);
    sliderCol = dynamicsArea.removeFromRight (50);
    colormap.setBounds (sliderCol);
    colormap.setMaxLevel (processor.getPeakLevelSetting());
    colormap.setRange (processor.getDynamicRange());

    UIarea.removeFromTop (20);
    juce::Rectangle<int> sliderArea = UIarea.removeFromTop (45);
    sliderArea.removeFromRight (28);
    sliderArea.removeFromLeft (28);
    slRMStimeConstant.setBounds (sliderArea);
    lbRMStimeConstant.setBounds (UIarea.removeFromTop (12));

    UIarea.removeFromTop (10);
    UIarea.removeFromLeft (15);

    tbHoldMax.setBounds (UIarea.removeFromTop (20));

    area.removeFromRight (5);
    visualizer.setBounds (area);
}
void EnergyVisualizerAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &slPeakLevel)
        colormap.setMaxLevel ((float) slider->getValue());
    else if (slider == &slDynamicRange)
        colormap.setRange ((float) slider->getValue());
}

void EnergyVisualizerAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    visualizer.setColormap (colormap.getColormap());
    visualizer.setPeakLevel (processor.getPeakLevelSetting());
    visualizer.setDynamicRange (processor.getDynamicRange());
    visualizer.setHoldMax (processor.getHoldRMSSetting());

    processor.lastEditorTime = juce::Time::getCurrentTime();
}
