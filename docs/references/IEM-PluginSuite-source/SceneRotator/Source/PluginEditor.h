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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//Plugin Design Essentials
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

//Custom juce::Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"

typedef ReverseSlider::SliderAttachment
    SliderAttachment; // all ReverseSliders will make use of the parameters' valueToText() function
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class SceneRotatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer,
                                         private juce::ComboBox::Listener
{
public:
    SceneRotatorAudioProcessorEditor (SceneRotatorAudioProcessor&,
                                      juce::AudioProcessorValueTreeState&);
    ~SceneRotatorAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

    void refreshMidiDeviceList();
    void updateSelectedMidiScheme();

private:
    // ====================== begin essentials ==================
    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;
    juce::TooltipWindow tooltipWin;

    // stored references to the AudioProcessor and juce::ValueTreeState holding all the parameters
    SceneRotatorAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    // title and footer component
    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;
    // =============== end essentials ============

    // Attachments to create a connection between IOWidgets comboboxes
    // and the associated parameters
    std::unique_ptr<ComboBoxAttachment> cbNormalizationAttachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAttachement;

    ReverseSlider slYaw, slPitch, slRoll, slQW, slQX, slQY, slQZ;

    std::unique_ptr<SliderAttachment> slYawAttachment;
    std::unique_ptr<SliderAttachment> slPitchAttachment;
    std::unique_ptr<SliderAttachment> slRollAttachment;
    std::unique_ptr<SliderAttachment> slQWAttachment;
    std::unique_ptr<SliderAttachment> slQXAttachment;
    std::unique_ptr<SliderAttachment> slQYAttachment;
    std::unique_ptr<SliderAttachment> slQZAttachment;

    juce::ComboBox cbRotationSequence;
    std::unique_ptr<ComboBoxAttachment> cbRotationSequenceAttachment;

    // juce::Labels and Groups
    SimpleLabel lbYaw, lbPitch, lbRoll, lbQW, lbQX, lbQY, lbQZ;
    juce::GroupComponent quatGroup, yprGroup;

    juce::ToggleButton tbInvertYaw, tbInvertPitch, tbInvertRoll, tbInvertQuaternion;
    std::unique_ptr<ButtonAttachment> tbInvertYawAttachment, tbInvertPitchAttachment,
        tbRollFlipAttachment, tbInvertQuaternionAttachment;

    // MIDI Section
    juce::GroupComponent midiGroup;
    SimpleLabel slMidiDevices, slMidiScheme;
    juce::ComboBox cbMidiDevices, cbMidiScheme;

    juce::Atomic<bool> refreshingMidiDevices = false;
    juce::Atomic<bool> updatingMidiScheme = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneRotatorAudioProcessorEditor)
};
