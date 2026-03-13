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
ToolBoxAudioProcessorEditor::ToolBoxAudioProcessorEditor (ToolBoxAudioProcessor& p,
                                                          juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (370, 250, 800, 500); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Tool"), juce::String ("Box"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    // create the connection between title component's comboBoxes and parameters
    cbInputOrderSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputOrderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));
    cbInputNormalizationSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSn3dInput",
                                *title.getInputWidgetPtr()->getNormCbPointer()));

    cbOutputOrderSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "outputOrderSetting",
                                *title.getOutputWidgetPtr()->getOrderCbPointer()));
    cbOutputNormalizationSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSn3dOutput",
                                *title.getOutputWidgetPtr()->getNormCbPointer()));

    // ================= FLIPs ==================
    addAndMakeVisible (gcFlip);
    gcFlip.setText ("Flip");

    addAndMakeVisible (tbFlipX);
    tbFlipXAttachment.reset (new ButtonAttachment (valueTreeState, "flipX", tbFlipX));
    tbFlipX.setButtonText ("Flip X (front/back)");
    tbFlipX.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (tbFlipY);
    tbFlipYAttachment.reset (new ButtonAttachment (valueTreeState, "flipY", tbFlipY));
    tbFlipY.setButtonText ("Flip Y (left/right)");
    tbFlipY.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (tbFlipZ);
    tbFlipZAttachment.reset (new ButtonAttachment (valueTreeState, "flipZ", tbFlipZ));
    tbFlipZ.setButtonText ("Flip Z (bottom/top)");
    tbFlipZ.setColour (juce::ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);

    // ================= LOA WEIGHTS ==================
    addAndMakeVisible (gcLOAWeighting);
    gcLOAWeighting.setText ("LOA Weighting");

    addAndMakeVisible (cbLoaWeights);
    cbLoaWeights.setJustificationType (juce::Justification::centred);
    cbLoaWeights.addSectionHeading ("Target Decoder Weights");
    cbLoaWeights.addItem ("none", 1);
    cbLoaWeights.addItem ("maxrE", 2);
    cbLoaWeights.addItem ("inPhase", 3);
    cbLoaWeightsAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "loaWeights", cbLoaWeights));

    addAndMakeVisible (lbLoaWeights);
    lbLoaWeights.setText ("Weights");

    // ================= GAIN =========================
    addAndMakeVisible (gcGain);
    gcGain.setText ("Overall Gain");

    addAndMakeVisible (slGain);
    slGainAttachment.reset (new SliderAttachment (valueTreeState, "gain", slGain));
    slGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);

    // start timer after everything is set up properly
    startTimer (20);
}

ToolBoxAudioProcessorEditor::~ToolBoxAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void ToolBoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void ToolBoxAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
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
    // =========== END: header and footer =================

    juce::Rectangle<int> leftColumn = area.removeFromLeft (150);
    {
        juce::Rectangle<int> flipArea = leftColumn.removeFromTop (85);
        gcFlip.setBounds (flipArea);
        flipArea.removeFromTop (25);
        tbFlipX.setBounds (flipArea.removeFromTop (20));
        tbFlipY.setBounds (flipArea.removeFromTop (20));
        tbFlipZ.setBounds (flipArea.removeFromTop (20));

        leftColumn.removeFromTop (10);

        juce::Rectangle<int> loaArea = leftColumn.removeFromTop (45);
        gcLOAWeighting.setBounds (loaArea);
        loaArea.removeFromTop (25);
        juce::Rectangle<int> row = loaArea.removeFromTop (20);
        lbLoaWeights.setBounds (row.removeFromLeft (60));
        cbLoaWeights.setBounds (row);
    }

    juce::Rectangle<int> rightColumn = area.removeFromRight (120);
    {
        juce::Rectangle<int> gainArea = rightColumn.removeFromTop (85);
        gcGain.setBounds (gainArea);
        gainArea.removeFromTop (25);
        juce::Rectangle<int> row = gainArea.removeFromTop (80);
        slGain.setBounds (row.removeFromLeft (100));
    }
}

void ToolBoxAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    // insert stuff you want to do be done at every timer callback
}
