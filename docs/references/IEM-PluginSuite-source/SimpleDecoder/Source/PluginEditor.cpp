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
SimpleDecoderAudioProcessorEditor::SimpleDecoderAudioProcessorEditor (
    SimpleDecoderAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    dcInfoBox (vts),
    fv (20, 20000, -20, 10, 5)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits (670, 300, 1000, 700); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Simple"), juce::String ("Decoder"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    valueTreeState.addParameterListener ("swChannel", this);
    valueTreeState.addParameterListener ("swMode", this);

    // create the connection between title component's comboBoxes and parameters
    cbOrderSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputOrderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));
    cbNormalizationSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));

    addAndMakeVisible (gcFilter);
    gcFilter.setText ("Frequency Bands");

    addAndMakeVisible (gcSw);
    gcSw.setText ("Subwoofer");

    addAndMakeVisible (gcConfiguration);
    gcConfiguration.setText ("Decoder Configuration");

    // ================= BEGIN: filter slider ================
    addAndMakeVisible (slLowPassFrequency);
    slLowPassFrequencyAttachment.reset (
        new SliderAttachment (valueTreeState, "lowPassFrequency", slLowPassFrequency));
    slLowPassFrequency.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slLowPassFrequency.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slLowPassFrequency.setColour (juce::Slider::rotarySliderOutlineColourId,
                                  juce::Colours::orangered);
    addAndMakeVisible (lbLowPassFrequency);
    lbLowPassFrequency.setText ("Frequency");

    addAndMakeVisible (slLowPassGain);
    slLowPassGainAttachment.reset (
        new SliderAttachment (valueTreeState, "lowPassGain", slLowPassGain));
    slLowPassGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slLowPassGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slLowPassGain.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::orangered);
    addAndMakeVisible (lbLowPassGain);
    lbLowPassGain.setText ("Gain");

    addAndMakeVisible (slHighPassFrequency);
    slHighPassFrequencyAttachment.reset (
        new SliderAttachment (valueTreeState, "highPassFrequency", slHighPassFrequency));
    slHighPassFrequency.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slHighPassFrequency.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slHighPassFrequency.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::cyan);
    addAndMakeVisible (lbHighPassFrequency);
    lbHighPassFrequency.setText ("Frequency");
    // ================= END: filter slider ==================

    // ================= BEGIN: Subwoofer mode =====================
    addAndMakeVisible (cbSwMode);
    cbSwMode.setName ("Subwoofer");
    cbSwMode.setJustificationType (juce::Justification::centred);
    cbSwMode.addItem ("none", 1);
    cbSwMode.addItem ("discrete", 2);
    cbSwMode.addItem ("virtual", 3);
    cbSwModeAttachment.reset (new ComboBoxAttachment (valueTreeState, "swMode", cbSwMode));
    const bool channelSelectShouldBeEnabled =
        (int) *valueTreeState.getRawParameterValue ("swMode") == 1;

    addAndMakeVisible (lbSwMode);
    lbSwMode.setText ("Subwoofer mode");

    addAndMakeVisible (lbSwChannel);
    lbSwChannel.setText ("Subwoofer Channel");
    lbSwChannel.setEnabled (channelSelectShouldBeEnabled);

    addAndMakeVisible (lbAlreadyUsed);
    lbAlreadyUsed.setText ("already used!");
    lbAlreadyUsed.setJustification (juce::Justification::centred);
    lbAlreadyUsed.setTextColour (juce::Colours::orangered);
    lbAlreadyUsed.setVisible (false);

    addAndMakeVisible (slSwChannel);
    slSwChannelAttachment.reset (new SliderAttachment (valueTreeState, "swChannel", slSwChannel));
    slSwChannel.setSliderStyle (juce::Slider::IncDecButtons);
    slSwChannel.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 200, 20);
    slSwChannel.setEnabled (channelSelectShouldBeEnabled);
    // ================= END: Subwoofer mode =======================

    addAndMakeVisible (btLoadFile);
    btLoadFile.setButtonText ("Load configuration");
    btLoadFile.onClick = [&]() { loadPresetFile(); };
    btLoadFile.setColour (juce::TextButton::buttonColourId, juce::Colours::orange);

    dcInfoBox.setErrorMessage (processor.getMessageForEditor());

    addAndMakeVisible (dcInfoBox);
    dcInfoBox.setDecoderConfig (processor.getCurrentDecoderConfig());

    addAndMakeVisible (fv);
    fv.setParallel (true);
    fv.addCoefficients (processor.cascadedLowPassCoeffs,
                        juce::Colours::orangered,
                        &slLowPassFrequency,
                        &slLowPassGain);
    fv.addCoefficients (processor.cascadedHighPassCoeffs,
                        juce::Colours::cyan,
                        &slHighPassFrequency);

    addAndMakeVisible (gcGain);
    gcGain.setText ("Overall Gain");

    addAndMakeVisible (slGain);
    slGainAttachment.reset (new SliderAttachment (valueTreeState, "overallGain", slGain));
    slGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);

    // start timer after everything is set up properly
    startTimer (20);
}

SimpleDecoderAudioProcessorEditor::~SimpleDecoderAudioProcessorEditor()
{
    valueTreeState.removeParameterListener ("swChannel", this);
    valueTreeState.removeParameterListener ("swMode", this);
    juce::ModalComponentManager::getInstance()->cancelAllModalComponents();
    setLookAndFeel (nullptr);
}

//==============================================================================
void SimpleDecoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void SimpleDecoderAudioProcessorEditor::resized()
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

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int labelHeight = 12;

    juce::Rectangle<int> rightCol (area.removeFromRight (100));
    area.removeFromRight (20);
    juce::Rectangle<int> middleCol (area.removeFromRight (190));
    area.removeFromRight (20);
    juce::Rectangle<int> leftCol (area);

    { //====================== CONFIGURATION GROUP ==================================
        juce::Rectangle<int> configArea (leftCol);
        juce::Rectangle<int> buttonArea = configArea;

        gcConfiguration.setBounds (configArea);
        configArea.removeFromTop (25);

        buttonArea = configArea.removeFromTop (21).removeFromLeft (130);
        btLoadFile.setBounds (buttonArea);

        configArea.removeFromTop (5);

        dcInfoBox.setBounds (configArea);
    }

    { //====================== Subwoofer GROUP ==================================
        auto swArea = rightCol.removeFromTop (105);
        gcSw.setBounds (swArea);
        swArea.removeFromTop (25);

        cbSwMode.setBounds (swArea.removeFromTop (18));
        lbSwMode.setBounds (swArea.removeFromTop (labelHeight));

        swArea.removeFromTop (8);

        slSwChannel.setBounds (swArea.removeFromTop (20));
        lbSwChannel.setBounds (swArea.removeFromTop (labelHeight));
        lbAlreadyUsed.setBounds (swArea.removeFromTop (10));
    }

    { //====================== Gain GROUP ==================================
        auto gainArea = rightCol.removeFromTop (85);
        gcGain.setBounds (gainArea);
        gainArea.removeFromTop (25);
        slGain.setBounds (gainArea.removeFromTop (60));
    }

    { //====================== FILTER GROUP ==================================
        juce::Rectangle<int> filterArea (middleCol);
        gcFilter.setBounds (filterArea);
        filterArea.removeFromTop (25);

        const int rotSliderWidth = 50;

        fv.setBounds (filterArea.removeFromTop (110));

        juce::Rectangle<int> sliderRow (filterArea.removeFromTop (rotSliderHeight - 10));

        slLowPassGain.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slLowPassFrequency.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        slHighPassFrequency.setBounds (sliderRow.removeFromRight (rotSliderWidth));

        sliderRow = filterArea.removeFromTop (labelHeight);

        lbLowPassGain.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowPassFrequency.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        lbHighPassFrequency.setBounds (sliderRow.removeFromRight (rotSliderWidth));
    }
}

void SimpleDecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.messageChanged.get())
    {
        dcInfoBox.setErrorMessage (processor.getMessageForEditor());
        processor.messageChanged = false;
    }

    if (processor.updateDecoderInfo.get())
    {
        dcInfoBox.setDecoderConfig (processor.getCurrentDecoderConfig());
        processor.updateDecoderInfo = false;
    }

    ReferenceCountedDecoder::Ptr currentDecoder = processor.getCurrentDecoderConfig();
    const int swMode = *valueTreeState.getRawParameterValue ("swMode");
    if (lastDecoder != currentDecoder)
    {
        lastDecoder = currentDecoder;
        if (lastDecoder != nullptr)
        {
            int neededChannels = 0;
            if (swMode == 1)
                neededChannels =
                    juce::jmax (currentDecoder->getNumOutputChannels(),
                                (int) *valueTreeState.getRawParameterValue ("swChannel"));
            else
                neededChannels = currentDecoder->getNumOutputChannels();

            title.getInputWidgetPtr()->setMaxOrder (currentDecoder->getOrder());
            title.getOutputWidgetPtr()->setSizeIfUnselectable (neededChannels);
        }
        else
        {
            title.getInputWidgetPtr()->setMaxOrder (0);
            title.getOutputWidgetPtr()->setSizeIfUnselectable (0);
        }
    }

    if (updateChannelsInWidget)
    {
        const int swMode = *valueTreeState.getRawParameterValue ("swMode");
        int neededChannels = 0;
        if (swMode == 1)
            neededChannels = juce::jmax (currentDecoder->getNumOutputChannels(),
                                         (int) *valueTreeState.getRawParameterValue ("swChannel"));
        else
            neededChannels = currentDecoder->getNumOutputChannels();

        title.getOutputWidgetPtr()->setSizeIfUnselectable (neededChannels);
        updateChannelsInWidget = false;
    }

    if (swMode == 1 && currentDecoder != nullptr
        && currentDecoder->getRoutingArrayReference().contains (
            (int) *valueTreeState.getRawParameterValue ("swChannel") - 1))
    {
        lbAlreadyUsed.setVisible (true);
    }
    else
    {
        lbAlreadyUsed.setVisible (false);
    }

    if (processor.guiUpdateLowPassCoefficients.get())
    {
        fv.replaceCoefficients (0, processor.cascadedLowPassCoeffs);
        processor.guiUpdateLowPassCoefficients = false;
    }

    if (processor.guiUpdateHighPassCoefficients.get())
    {
        fv.replaceCoefficients (1, processor.cascadedHighPassCoeffs);
        processor.guiUpdateHighPassCoefficients = false;
    }

    if (processor.guiUpdateLowPassGain.get())
    {
        fv.repaint();
        processor.guiUpdateLowPassGain = false;
    }

    if (processor.guiUpdateSampleRate.get())
    {
        fv.setSampleRate (processor.getSampleRate());
        processor.guiUpdateSampleRate = false;
    }

    if (changeEnablement)
    {
        slSwChannel.setEnabled (enableSubwooferChannelControls);
        lbSwChannel.setEnabled (enableSubwooferChannelControls);
        changeEnablement = false;
    }
}

void SimpleDecoderAudioProcessorEditor::loadPresetFile()
{
    juce::FileChooser myChooser ("Please select the preset you want to load...",
                                 processor.getLastDir().exists() ? processor.getLastDir()
                                                                 : juce::File::getSpecialLocation (
                                                                     juce::File::userHomeDirectory),
                                 "*.json");
    if (myChooser.browseForFileToOpen())
    {
        juce::File presetFile (myChooser.getResult());
        processor.setLastDir (presetFile.getParentDirectory());
        processor.loadConfiguration (presetFile);

        dcInfoBox.setDecoderConfig (processor.getCurrentDecoderConfig());
    }
}

void SimpleDecoderAudioProcessorEditor::parameterChanged (const juce::String& parameterID,
                                                          float newValue)
{
    if (parameterID == "swChannel" || parameterID == "swMode")
    {
        ReferenceCountedDecoder::Ptr currentDecoder = processor.getCurrentDecoderConfig();
        if (currentDecoder != nullptr)
        {
            const int swMode = *valueTreeState.getRawParameterValue ("swMode");
            int neededChannels = 0;
            if (swMode == 1)
                neededChannels =
                    juce::jmax (currentDecoder->getNumOutputChannels(),
                                (int) *valueTreeState.getRawParameterValue ("swChannel"));
            else
                neededChannels = currentDecoder->getNumOutputChannels();

            updateChannelsInWidget = true;
        }
    }

    if (parameterID == "swMode")
    {
        const int swMode = *valueTreeState.getRawParameterValue ("swMode");
        enableSubwooferChannelControls = swMode == 1;
        changeEnablement = true;
    }
}
