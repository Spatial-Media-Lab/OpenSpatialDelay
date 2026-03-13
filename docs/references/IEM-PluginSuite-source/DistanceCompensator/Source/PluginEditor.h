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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//Plugin Design Essentials
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

//Custom juce::Components
#include "../../resources/LabelAttachment.h"
#include "../../resources/customComponents/MailBox.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/RoundButton.h"
#include "../../resources/customComponents/SimpleLabel.h"

typedef ReverseSlider::SliderAttachment
    SliderAttachment; // all ReverseSliders will make use of the parameters' valueToText() function
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
 */
class DistanceCompensatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                private juce::Timer,
                                                private juce::Button::Listener
{
public:
    DistanceCompensatorAudioProcessorEditor (DistanceCompensatorAudioProcessor&,
                                             juce::AudioProcessorValueTreeState&);
    ~DistanceCompensatorAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void buttonClicked (juce::Button* button) override;
    void buttonStateChanged (juce::Button* button) override;

private:
    // ====================== begin essentials ==================
    LaF globalLaF;

    DistanceCompensatorAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    TitleBar<AudioChannelsIOWidget<64, true>, NoIOWidget> title;
    OSCFooter footer;
    // ====================== end essentials ====================

    void updateEnableSetting (const int ch);
    void showControls (const int nCh);

    std::unique_ptr<ComboBoxAttachment> cbInputChannelsSettingAttachment;

    juce::Label lbSpeedOfSound;
    std::unique_ptr<LabelAttachment> lbSpeedOfSoundAttachment;
    SimpleLabel slbSpeedOfSound;

    juce::Label lbDistanceExponent;
    std::unique_ptr<LabelAttachment> lbDistanceExponentAttachment;
    SimpleLabel slbDistanceExponent;

    juce::ComboBox cbGainNormalization;
    SimpleLabel slbGainNormalization;
    std::unique_ptr<ComboBoxAttachment> cbGainNormalizationAttachment;

    juce::Label lbReferenceX, lbReferenceY, lbReferenceZ;
    std::unique_ptr<LabelAttachment> lbReferenceXAttachment, lbReferenceYAttachment,
        lbReferenceZAttachment;
    SimpleLabel slbReference, slbReferenceX, slbReferenceY, slbReferenceZ;

    juce::TooltipWindow toolTipWin;

    // load
    juce::GroupComponent gcLayout;
    juce::TextButton btLoadFile;
    juce::TextButton btReference;

    // buttons
    juce::GroupComponent gcCompensation;
    juce::ToggleButton tbEnableGains;
    juce::ToggleButton tbEnableDelays;
    std::unique_ptr<ButtonAttachment> tbEnableGainsAttachment;
    std::unique_ptr<ButtonAttachment> tbEnableDelaysAttachment;
    std::unique_ptr<ButtonAttachment> tbEnableFiltersAttachment;

    int maxNumInputs = -1;
    int lastSetNumChIn = -1;

    // distances
    juce::GroupComponent gcDistances;

    juce::OwnedArray<RoundButton> tbEnableCompensation;
    juce::OwnedArray<ButtonAttachment> tbEnableCompensationAttachment;

    juce::OwnedArray<juce::Label> slDistance;
    juce::OwnedArray<LabelAttachment> slDistanceAttachment;
    juce::OwnedArray<SimpleLabel> lbDistance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistanceCompensatorAudioProcessorEditor)
};
