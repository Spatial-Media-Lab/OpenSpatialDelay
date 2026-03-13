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

#include "../../resources/customComponents/CompressorVisualizer.h"
#include "../../resources/customComponents/LevelMeter.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class OmniCompressorAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    OmniCompressorAudioProcessorEditor (OmniCompressorAudioProcessor&,
                                        juce::AudioProcessorValueTreeState&);
    ~OmniCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    OmniCompressorAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;

    void timerCallback() override;

    ReverseSlider sliderKnee, sliderThreshold, sliderRatio, sliderAttackTime, sliderReleaseTime,
        sliderMakeupGain;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;

    std::unique_ptr<SliderAttachment> KnAttachment;
    std::unique_ptr<SliderAttachment> ThAttachment;
    std::unique_ptr<SliderAttachment> RaAttachment;
    std::unique_ptr<SliderAttachment> ATAttachment;
    std::unique_ptr<SliderAttachment> RTAttachment;
    std::unique_ptr<SliderAttachment> MGAttachment;

    juce::ToggleButton tbLookAhead;
    std::unique_ptr<ButtonAttachment> tbLookAheadAttachment;

    CompressorVisualizer characteristic;
    LevelMeter inpMeter, dbGRmeter;

    SimpleLabel lbKnee, lbThreshold, lbOutGain, lbRatio, lbAttack, lbRelease;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmniCompressorAudioProcessorEditor)
};
