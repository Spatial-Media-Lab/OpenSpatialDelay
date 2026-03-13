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
BinauralDecoderAudioProcessorEditor::BinauralDecoderAudioProcessorEditor (
    BinauralDecoderAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setSize (450, 140); // use this to create a fixed-size GUI
    //setResizeLimits(500, 300, 800, 500); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle ("Binaural", "Decoder");
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    // create the connection between title component's comboBoxes and parameters
    cbOrderSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputOrderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));
    cbNormalizationSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));

    addAndMakeVisible (lbEq);
    lbEq.setText ("Headphone Equalization");

    addAndMakeVisible (cbEq);
    cbEq.addItem ("OFF", 1);
    cbEq.addItemList (processor.headphoneEQs, 2);
    cbEqAttachment.reset (new ComboBoxAttachment (valueTreeState, "applyHeadphoneEq", cbEq));

    // start timer after everything is set up properly
    startTimer (20);
}

BinauralDecoderAudioProcessorEditor::~BinauralDecoderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void BinauralDecoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void BinauralDecoderAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    auto area = getLocalBounds();

    auto footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    auto headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);
    // =========== END: header and footer =================

    auto sliderRow = area.removeFromTop (20);
    lbEq.setBounds (sliderRow.removeFromLeft (150));
    cbEq.setBounds (sliderRow.removeFromLeft (120));
}

void BinauralDecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    // insert stuff you want to do be done at every timer callback
}
