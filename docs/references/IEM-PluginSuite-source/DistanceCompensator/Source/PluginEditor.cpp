/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
DistanceCompensatorAudioProcessorEditor::DistanceCompensatorAudioProcessorEditor (
    DistanceCompensatorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Distance"), juce::String ("Compensator"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    // create the connection between title component's comboBoxes and parameters
    cbInputChannelsSettingAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputChannelsSetting",
                                *title.getInputWidgetPtr()->getChannelsCbPointer()));

    addAndMakeVisible (lbSpeedOfSound);
    lbSpeedOfSound.setEditable (true);
    lbSpeedOfSound.setJustificationType (juce::Justification::centred);
    lbSpeedOfSoundAttachment.reset (
        new LabelAttachment (valueTreeState, "speedOfSound", lbSpeedOfSound));

    addAndMakeVisible (slbSpeedOfSound);
    slbSpeedOfSound.setText ("Speed of sound");
    slbSpeedOfSound.setJustification (juce::Justification::left);

    addAndMakeVisible (lbDistanceExponent);
    lbDistanceExponent.setEditable (true);
    lbDistanceExponent.setJustificationType (juce::Justification::centred);
    lbDistanceExponentAttachment.reset (
        new LabelAttachment (valueTreeState, "distanceExponent", lbDistanceExponent));

    addAndMakeVisible (slbDistanceExponent);
    slbDistanceExponent.setText ("Distance-Gain exponent");
    slbDistanceExponent.setJustification (juce::Justification::left);

    addAndMakeVisible (cbGainNormalization);
    cbGainNormalization.addSectionHeading ("Gain normalization");
    cbGainNormalization.addItem ("Attenuation only", 1);
    cbGainNormalization.addItem ("Zero-mean", 2);
    cbGainNormalization.setJustificationType (juce::Justification::centred);
    cbGainNormalizationAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "gainNormalization", cbGainNormalization));

    addAndMakeVisible (slbGainNormalization);
    slbGainNormalization.setText ("Normalization");
    slbGainNormalization.setJustification (juce::Justification::left);

    addAndMakeVisible (lbReferenceX);
    lbReferenceX.setEditable (true);
    lbReferenceX.setJustificationType (juce::Justification::centred);
    lbReferenceXAttachment.reset (new LabelAttachment (valueTreeState, "referenceX", lbReferenceX));

    addAndMakeVisible (lbReferenceY);
    lbReferenceY.setEditable (true);
    lbReferenceY.setJustificationType (juce::Justification::centred);
    lbReferenceYAttachment.reset (new LabelAttachment (valueTreeState, "referenceY", lbReferenceY));

    addAndMakeVisible (lbReferenceZ);
    lbReferenceZ.setEditable (true);
    lbReferenceZ.setJustificationType (juce::Justification::centred);
    lbReferenceZAttachment.reset (new LabelAttachment (valueTreeState, "referenceZ", lbReferenceZ));

    addAndMakeVisible (slbReference);
    slbReference.setText ("Reference position", true);
    slbReference.setJustification (juce::Justification::left);

    addAndMakeVisible (slbReferenceX);
    slbReferenceX.setText ("x");
    slbReferenceX.setJustification (juce::Justification::centred);

    addAndMakeVisible (slbReferenceY);
    slbReferenceY.setText ("y");
    slbReferenceY.setJustification (juce::Justification::centred);

    addAndMakeVisible (slbReferenceZ);
    slbReferenceZ.setText ("z");
    slbReferenceZ.setJustification (juce::Justification::centred);

    addAndMakeVisible (gcLayout);
    gcLayout.setText ("From loudspeaker layout");

    addAndMakeVisible (btLoadFile);
    btLoadFile.setButtonText ("LOAD LAYOUT");
    btLoadFile.addListener (this);
    btLoadFile.setColour (juce::TextButton::buttonColourId, juce::Colours::orange);

    addAndMakeVisible (btReference);
    btReference.setButtonText ("UPDATE REFERENCE");
    btReference.addListener (this);
    btReference.setColour (juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);

    addAndMakeVisible (gcCompensation);
    gcCompensation.setText ("Settings");

    addAndMakeVisible (tbEnableGains);
    tbEnableGainsAttachment.reset (
        new ButtonAttachment (valueTreeState, "enableGains", tbEnableGains));
    tbEnableGains.setButtonText ("Gain compensation");
    tbEnableGains.setColour (juce::ToggleButton::tickColourId, juce::Colours::limegreen);

    addAndMakeVisible (tbEnableDelays);
    tbEnableDelaysAttachment.reset (
        new ButtonAttachment (valueTreeState, "enableDelays", tbEnableDelays));
    tbEnableDelays.setButtonText ("Delay compensation");
    tbEnableDelays.setColour (juce::ToggleButton::tickColourId, juce::Colours::orange);

    addAndMakeVisible (gcDistances);
    gcDistances.setText ("Loudspeaker Distances");

    for (int i = 0; i < 64; ++i)
    {
        auto enHandle = tbEnableCompensation.add (new RoundButton());
        addAndMakeVisible (enHandle);
        enHandle->setColour (juce::ToggleButton::tickColourId, juce::Colours::cornflowerblue);
        enHandle->setButtonText ("C");
        enHandle->setTooltip (
            "Enable compensation and \n factoring in the distance in gain/delay-calculation.");
        tbEnableCompensationAttachment.add (
            new ButtonAttachment (valueTreeState,
                                  "enableCompensation" + juce::String (i),
                                  *enHandle));
        enHandle->onStateChange = [this, i]() { updateEnableSetting (i); };

        bool isOn = enHandle->getToggleState();

        auto handle = slDistance.add (new juce::Label());
        addAndMakeVisible (handle);
        handle->setJustificationType (juce::Justification::centred);
        handle->setEditable (true);
        handle->setEnabled (isOn);
        handle->setExplicitFocusOrder (i + 1);
        slDistanceAttachment.add (
            new LabelAttachment (valueTreeState, "distance" + juce::String (i), *handle));

        auto lbHandle = lbDistance.add (new SimpleLabel());
        addAndMakeVisible (lbHandle);
        lbHandle->setEnabled (isOn);
        lbHandle->setText (juce::String (i + 1), true, juce::Justification::right);
    }

    toolTipWin.setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    toolTipWin.setOpaque (false);

    setResizeLimits (500, 650, 500, 650); // use this to create a resizable GUI

    // start timer after everything is set up properly
    startTimer (20);
}

DistanceCompensatorAudioProcessorEditor::~DistanceCompensatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DistanceCompensatorAudioProcessorEditor::updateEnableSetting (const int ch)
{
    bool shouldBeEnabled = tbEnableCompensation.getUnchecked (ch)->getToggleState();
    slDistance.getUnchecked (ch)->setEnabled (shouldBeEnabled);
    lbDistance.getUnchecked (ch)->setEnabled (shouldBeEnabled);
}

void DistanceCompensatorAudioProcessorEditor::showControls (const int nCh)
{
    for (int i = 0; i < nCh; ++i)
    {
        lbDistance.getUnchecked (i)->setVisible (true);
        tbEnableCompensation.getUnchecked (i)->setVisible (true);
        slDistance.getUnchecked (i)->setVisible (true);
    }
    for (int i = nCh; i < 64; ++i)
    {
        lbDistance.getUnchecked (i)->setVisible (false);
        tbEnableCompensation.getUnchecked (i)->setVisible (false);
        slDistance.getUnchecked (i)->setVisible (false);
    }
}

//==============================================================================
void DistanceCompensatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void DistanceCompensatorAudioProcessorEditor::resized()
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

    juce::Rectangle<int> controls = area.removeFromTop (150);
    auto settingsArea = controls.removeFromLeft (200);
    gcCompensation.setBounds (settingsArea);
    settingsArea.removeFromTop (25);
    tbEnableGains.setBounds (settingsArea.removeFromTop (20));
    settingsArea.removeFromTop (3);

    auto rowDistanceGain (settingsArea.removeFromTop (18));
    rowDistanceGain.removeFromLeft (26);
    slbDistanceExponent.setBounds (rowDistanceGain.removeFromLeft (120));
    rowDistanceGain.removeFromLeft (5);
    lbDistanceExponent.setBounds (rowDistanceGain);
    settingsArea.removeFromTop (5);

    auto rowNormalization (settingsArea.removeFromTop (18));
    rowNormalization.removeFromLeft (26);
    slbGainNormalization.setBounds (rowNormalization.removeFromLeft (70));
    rowNormalization.removeFromLeft (5);
    cbGainNormalization.setBounds (rowNormalization);
    settingsArea.removeFromTop (8);

    tbEnableDelays.setBounds (settingsArea.removeFromTop (20));
    settingsArea.removeFromTop (5);
    auto rowSpeed (settingsArea.removeFromTop (18));
    rowSpeed.removeFromLeft (26);
    slbSpeedOfSound.setBounds (rowSpeed.removeFromLeft (80));
    rowSpeed.removeFromLeft (5);
    lbSpeedOfSound.setBounds (rowSpeed.removeFromLeft (60));

    controls.removeFromLeft (50);

    gcLayout.setBounds (controls);
    controls.removeFromTop (25);
    auto col = controls.removeFromLeft (130);
    btLoadFile.setBounds (col.removeFromTop (21));

    col.removeFromTop (20);

    slbReference.setBounds (col.removeFromTop (18));

    auto labelArea (col.removeFromTop (18));
    lbReferenceX.setBounds (labelArea.removeFromLeft (40));
    labelArea.removeFromLeft (5);
    lbReferenceY.setBounds (labelArea.removeFromLeft (40));
    labelArea.removeFromLeft (5);
    lbReferenceZ.setBounds (labelArea.removeFromLeft (40));
    labelArea = col.removeFromTop (12);
    slbReferenceX.setBounds (labelArea.removeFromLeft (40));
    labelArea.removeFromLeft (5);
    slbReferenceY.setBounds (labelArea.removeFromLeft (40));
    labelArea.removeFromLeft (5);
    slbReferenceZ.setBounds (labelArea.removeFromLeft (40));

    col.removeFromTop (5);
    btReference.setBounds (col.removeFromTop (21));

    area.removeFromTop (10);
    gcDistances.setBounds (area.removeFromTop (25));

    juce::Rectangle<int> sliderCol;

    for (int i = 0; i < 64; ++i)
    {
        if (i % 16 == 0)
            sliderCol = area.removeFromLeft (100);
        else if (i % 8 == 0)
            sliderCol.removeFromTop (15);

        auto sliderRow = sliderCol.removeFromTop (18);
        lbDistance.getUnchecked (i)->setBounds (sliderRow.removeFromLeft (20));
        sliderRow.removeFromLeft (8);
        tbEnableCompensation.getUnchecked (i)->setBounds (sliderRow.removeFromLeft (18));
        sliderRow.removeFromLeft (2);
        slDistance.getUnchecked (i)->setBounds (sliderRow);

        sliderCol.removeFromTop (2);

        if ((i - 1) % 16 == 0)
            area.removeFromLeft (10);
    }
}

void DistanceCompensatorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    // insert stuff you want to do be done at every timer callback

    const int selected = title.getInputWidgetPtr()->getChannelsCbPointer()->getSelectedId();
    int nChIn;
    if (selected > 1)
        nChIn = selected - 1;
    else
        nChIn = processor.input.getSize();

    if (nChIn != lastSetNumChIn)
    {
        showControls (nChIn);
        lastSetNumChIn = nChIn;
    }

    if (processor.updateMessage)
    {
        processor.updateMessage = false;
        juce::AlertWindow alert (processor.messageToEditor.headline,
                                 processor.messageToEditor.text,
                                 juce::AlertWindow::NoIcon);
        alert.setLookAndFeel (&globalLaF);
        alert.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
        alert.runModalLoop();
    }
}

void DistanceCompensatorAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &btLoadFile)
    {
        juce::FileChooser myChooser (
            "Load loudspeaker layout...",
            processor.getLastDir().exists()
                ? processor.getLastDir()
                : juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.json");
        if (myChooser.browseForFileToOpen())
        {
            juce::File configFile (myChooser.getResult());
            processor.setLastDir (configFile.getParentDirectory());
            processor.loadConfiguration (configFile);
        }
    }
    else if (button == &btReference)
    {
        processor.updateParameters();
    }
}

void DistanceCompensatorAudioProcessorEditor::buttonStateChanged (juce::Button* button)
{
}
