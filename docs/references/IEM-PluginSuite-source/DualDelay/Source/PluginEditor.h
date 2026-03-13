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

#include "../../resources/customComponents/DoubleSlider.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class DualDelayAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    DualDelayAudioProcessorEditor (DualDelayAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~DualDelayAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    void timerCallback() override;

    DualDelayAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;
    int maxPossibleOrder;

    ReverseSlider SlDryGain;
    std::unique_ptr<SliderAttachment> SlDryGainAttachment;

    // elements for left side
    DoubleSlider dblSlLeftFilter;
    ReverseSlider SlLeftRot;
    ReverseSlider SlLeftDelay, SlLeftLfoRate, SlLeftLfoDepth, SlLeftFb, SlLeftCrossFb, SlLeftGain;

    std::unique_ptr<SliderAttachment> dblSlLeftFilterHpAttachment, dblSlLeftFilterLpAttachment;
    std::unique_ptr<SliderAttachment> SlLeftRotAttachment;
    std::unique_ptr<SliderAttachment> SlLeftDelayAttachment, SlLeftLfoRateAttachment,
        SlLeftLfoDepthAttachment, SlLeftFbAttachment, SlLeftCrossFbAttachment, SlLeftGainAttachment;

    // elements for right side
    DoubleSlider dblSlRightFilter;
    ReverseSlider SlRightRot;
    ReverseSlider SlRightDelay, SlRightLfoRate, SlRightLfoDepth, SlRightFb, SlRightCrossFb,
        SlRightGain;

    std::unique_ptr<SliderAttachment> dblSlRightFilterHpAttachment, dblSlRightFilterLpAttachment;
    std::unique_ptr<SliderAttachment> SlRightRotAttachment;
    std::unique_ptr<SliderAttachment> SlRightDelayAttachment, SlRightLfoRateAttachment,
        SlRightLfoDepthAttachment, SlRightFbAttachment, SlRightCrossFbAttachment,
        SlRightGainAttachment;

    // labels and groups
    SimpleLabel lbRotL, lbDelL, lbFbL, lbXFbL;
    SimpleLabel lbRotR, lbDelR, lbFbR, lbXFbR;
    SimpleLabel lbGainL, lbGainR, lbGainDry;
    TripleLabel lbLfoL, lbLfoR, lbFilterL, lbFilterR;

    juce::GroupComponent gcRotDelL, gcRotDelR, gcFiltL, gcFiltR, gcFbL, gcFbR, gcOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessorEditor)
};
