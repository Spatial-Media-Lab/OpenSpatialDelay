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
SceneRotatorAudioProcessorEditor::SceneRotatorAudioProcessorEditor (
    SceneRotatorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (450, 320, 800, 500); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Scene"), juce::String ("Rotator"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    // create the connection between title component's comboBoxes and parameters
    cbOrderAttachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));
    cbNormalizationAttachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));

    // ======================== YAW, PITCH, ROLL GROUP
    yprGroup.setText ("Yaw, Pitch & Roll");
    yprGroup.setTextLabelPosition (juce::Justification::centredLeft);
    addAndMakeVisible (&yprGroup);

    addAndMakeVisible (&slYaw);
    slYawAttachment.reset (new SliderAttachment (valueTreeState, "yaw", slYaw));
    slYaw.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slYaw.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slYaw.setReverse (true);
    slYaw.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slYaw.setRotaryParameters (juce::MathConstants<float>::pi,
                               3 * juce::MathConstants<float>::pi,
                               false);
    slYaw.setTooltip ("Yaw angle: rotation around z-axis");
    slYaw.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&slPitch);
    slPitchAttachment.reset (new SliderAttachment (valueTreeState, "pitch", slPitch));
    slPitch.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slPitch.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slPitch.setReverse (true);
    slPitch.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slPitch.setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                 2.5 * juce::MathConstants<float>::pi,
                                 false);
    slPitch.setTooltip ("Pitch angle: rotation around y-axis");
    slPitch.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&slRoll);
    slRollAttachment.reset (new SliderAttachment (valueTreeState, "roll", slRoll));
    slRoll.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRoll.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slRoll.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slRoll.setReverse (false);
    slRoll.setRotaryParameters (juce::MathConstants<float>::pi,
                                3 * juce::MathConstants<float>::pi,
                                false);
    slRoll.setTooltip ("Roll angle: rotation around x-axis");
    slRoll.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (tbInvertYaw);
    tbInvertYawAttachment.reset (new ButtonAttachment (valueTreeState, "invertYaw", tbInvertYaw));
    tbInvertYaw.setColour (juce::ToggleButton::ColourIds::tickColourId,
                           globalLaF.ClWidgetColours[0]);
    tbInvertYaw.setButtonText ("Flip");

    addAndMakeVisible (tbInvertPitch);
    tbInvertPitchAttachment.reset (
        new ButtonAttachment (valueTreeState, "invertPitch", tbInvertPitch));
    tbInvertPitch.setColour (juce::ToggleButton::ColourIds::tickColourId,
                             globalLaF.ClWidgetColours[1]);
    tbInvertPitch.setButtonText ("Flip");

    addAndMakeVisible (tbInvertRoll);
    tbRollFlipAttachment.reset (new ButtonAttachment (valueTreeState, "invertRoll", tbInvertRoll));
    tbInvertRoll.setColour (juce::ToggleButton::ColourIds::tickColourId,
                            globalLaF.ClWidgetColours[2]);
    tbInvertRoll.setButtonText ("Flip");

    addAndMakeVisible (tbInvertQuaternion);
    tbInvertQuaternionAttachment.reset (
        new ButtonAttachment (valueTreeState, "invertQuaternion", tbInvertQuaternion));
    tbInvertQuaternion.setColour (juce::ToggleButton::ColourIds::tickColourId,
                                  globalLaF.ClWidgetColours[0]);
    tbInvertQuaternion.setButtonText ("Invert Quaternions");

    addAndMakeVisible (cbRotationSequence);
    cbRotationSequence.setTooltip ("Sequence of intrinsic rotations");
    cbRotationSequence.addSectionHeading ("Rotation sequence");
    cbRotationSequence.addItem ("Yaw -> Pitch -> Roll", 1);
    cbRotationSequence.addItem ("Roll -> Pitch -> Yaw", 2);
    cbRotationSequence.setJustificationType (juce::Justification::centred);
    cbRotationSequenceAttachment.reset (
        new ComboBoxAttachment (valueTreeState, "rotationSequence", cbRotationSequence));

    // ====================== QUATERNION GROUP
    quatGroup.setText ("Quaternions");
    quatGroup.setTextLabelPosition (juce::Justification::centredLeft);
    addAndMakeVisible (&quatGroup);

    addAndMakeVisible (&slQW);
    slQWAttachment.reset (new SliderAttachment (valueTreeState, "qw", slQW));
    slQW.setSliderStyle (juce::Slider::LinearHorizontal);
    slQW.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    slQW.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQX);
    slQXAttachment.reset (new SliderAttachment (valueTreeState, "qx", slQX));
    slQX.setSliderStyle (juce::Slider::LinearHorizontal);
    slQX.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    slQX.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQY);
    slQYAttachment.reset (new SliderAttachment (valueTreeState, "qy", slQY));
    slQY.setSliderStyle (juce::Slider::LinearHorizontal);
    slQY.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    slQY.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&slQZ);
    slQZAttachment.reset (new SliderAttachment (valueTreeState, "qz", slQZ));
    slQZ.setSliderStyle (juce::Slider::LinearHorizontal);
    slQZ.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 15);
    slQZ.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    // ================ LABELS ===================
    addAndMakeVisible (&lbYaw);
    lbYaw.setText ("Yaw");

    addAndMakeVisible (&lbPitch);
    lbPitch.setText ("Pitch");

    addAndMakeVisible (&lbRoll);
    lbRoll.setText ("Roll");

    addAndMakeVisible (&lbQW);
    lbQW.setText ("W");

    addAndMakeVisible (&lbQX);
    lbQX.setText ("X");

    addAndMakeVisible (&lbQY);
    lbQY.setText ("Y");

    addAndMakeVisible (&lbQZ);
    lbQZ.setText ("Z");

    // ====================== MIDI GROUP
    addAndMakeVisible (midiGroup);
    midiGroup.setText ("MIDI Connection");
    midiGroup.setTextLabelPosition (juce::Justification::centredLeft);

    addAndMakeVisible (cbMidiDevices);
    cbMidiDevices.setJustificationType (juce::Justification::centred);
    refreshMidiDeviceList();
    cbMidiDevices.addListener (this);

    addAndMakeVisible (cbMidiScheme);
    cbMidiScheme.setJustificationType (juce::Justification::centred);
    cbMidiScheme.addSectionHeading ("Select Device's MIDI Scheme");
    cbMidiScheme.addItemList (processor.getMidiSchemes(), 1);
    cbMidiScheme.setSelectedId (static_cast<int> (processor.getCurrentMidiScheme()) + 1);
    updateSelectedMidiScheme();
    cbMidiScheme.addListener (this);

    addAndMakeVisible (slMidiDevices);
    slMidiDevices.setText ("Device");

    addAndMakeVisible (slMidiScheme);
    slMidiScheme.setText ("Scheme");

    tooltipWin.setLookAndFeel (&globalLaF);
    tooltipWin.setMillisecondsBeforeTipAppears (500);
    tooltipWin.setOpaque (false);

    // start timer after everything is set up properly
    startTimer (20);
}

SceneRotatorAudioProcessorEditor::~SceneRotatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void SceneRotatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void SceneRotatorAudioProcessorEditor::resized()
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

    const int sliderHeight = 17;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    const int sliderSpacing = 4;
    const int rotSliderWidth = 40;
    //const int labelHeight = 15;
    const int labelWidth = 20;

    auto topArea (area.removeFromTop (150));

    // -------------- Yaw Pitch Roll ------------------
    auto yprArea (topArea.removeFromLeft (160));
    yprGroup.setBounds (yprArea);
    yprArea.removeFromTop (25); //for box headline

    auto sliderRow (yprArea.removeFromTop (rotSliderHeight));
    slYaw.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slPitch.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slRoll.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

    sliderRow = yprArea.removeFromTop (20);
    lbYaw.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    lbPitch.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    lbRoll.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

    sliderRow = yprArea.removeFromTop (20);
    tbInvertYaw.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbInvertPitch.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbInvertRoll.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 5));

    yprArea.removeFromTop (5);

    sliderRow = yprArea.removeFromTop (20);
    sliderRow.reduce (10, 0);
    cbRotationSequence.setBounds (sliderRow);

    // ------------- Quaternions ------------------------
    auto quatArea (topArea.removeFromRight (190));
    quatGroup.setBounds (quatArea);
    quatArea.removeFromTop (25); //for box headline

    quatArea.removeFromTop (5);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQW.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQW.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQX.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQX.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQY.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQY.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (sliderHeight);
    slQZ.setBounds (sliderRow.removeFromRight (185 - labelWidth));
    lbQZ.setBounds (sliderRow);
    quatArea.removeFromTop (sliderSpacing);

    sliderRow = quatArea.removeFromTop (20);
    sliderRow.removeFromLeft (20);
    tbInvertQuaternion.setBounds (sliderRow);

    // ------------- MIDI Connection ------------------------
    area.removeFromTop (10);
    midiGroup.setBounds (area);
    area.removeFromTop (25);
    auto row = area.removeFromTop (20);
    auto leftSide = row.removeFromLeft (180);
    slMidiDevices.setBounds (leftSide.removeFromLeft (40));
    cbMidiDevices.setBounds (leftSide);

    row.removeFromLeft (10);
    slMidiScheme.setBounds (row.removeFromLeft (48));
    cbMidiScheme.setBounds (row.removeFromLeft (140));
}

void SceneRotatorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    // insert stuff you want to do be done at every timer callback
    if (processor.deviceHasChanged.get())
    {
        processor.deviceHasChanged = false;
        refreshMidiDeviceList();
    }

    if (processor.schemeHasChanged.get())
    {
        processor.schemeHasChanged = false;
        updateSelectedMidiScheme();
    }

    if (processor.showMidiOpenError.get())
    {
        processor.showMidiOpenError = false;
        juce::AlertWindow alert (
            "Could no open device",
            "The MIDI device could not be opened, although it's listed in the available device list. This can happen if this process has already opened that device. Please visit https://plugins.iem.at/docs/scenerotator/ for troubleshooting.",
            juce::AlertWindow::NoIcon);
        alert.setLookAndFeel (&globalLaF);
        alert.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
        alert.addButton ("Visit website", 2);
        if (alert.runModalLoop() == 2)
            juce::URL ("https://plugins.iem.at/docs/scenerotator/").launchInDefaultBrowser();
    }
}

void SceneRotatorAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &cbMidiDevices && ! refreshingMidiDevices.get())
    {
        auto id = cbMidiDevices.getSelectedId();

        if (id == -3) // refresh
            refreshMidiDeviceList();
        else if (id == -2)
        {
            processor.closeMidiInput();
            refreshMidiDeviceList();
        }
        else if (id > 0) // an actual device is selected!
        {
            juce::String deviceName = cbMidiDevices.getText();
            processor.openMidiInput (deviceName);
        }
    }
    else if (comboBoxThatHasChanged == &cbMidiScheme && ! updatingMidiScheme.get())
    {
        processor.setMidiScheme (
            SceneRotatorAudioProcessor::MidiScheme (cbMidiScheme.getSelectedId() - 1));
    }
}

void SceneRotatorAudioProcessorEditor::refreshMidiDeviceList()
{
    cbMidiDevices.clear();
    cbMidiDevices.addItem ("(refresh list...)", -3);
    cbMidiDevices.addItem ("none / use DAW input", -2);

    juce::MidiDeviceInfo currentDevice = processor.getCurrentMidiDeviceInfo();

    int select = -2;

    juce::Array<juce::MidiDeviceInfo> devices = juce::MidiInput::getAvailableDevices();
    if (! currentDevice.identifier.isEmpty())
    {
        if (devices.contains (currentDevice))
            select = devices.indexOf (currentDevice) + 1;
        else
        {
            cbMidiDevices.addItem (currentDevice.name + " (not available)", -1);
            select = -1;
        }
    }

    cbMidiDevices.addSeparator();
    cbMidiDevices.addSectionHeading ("Available Devices");
    for (int i = 0; i < devices.size(); ++i)
    {
        cbMidiDevices.addItem (devices[i].name, i + 1);
    }

    juce::ScopedValueSetter<juce::Atomic<bool>> refreshing (refreshingMidiDevices, true, false);
    cbMidiDevices.setSelectedId (select, juce::sendNotificationSync);
}

void SceneRotatorAudioProcessorEditor::updateSelectedMidiScheme()
{
    juce::ScopedValueSetter<juce::Atomic<bool>> refreshing (updatingMidiScheme, true, false);
    //cbMidiScheme.setSelectedId (select, juce::sendNotificationSync);
}
