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
#include "../../resources/customComponents/HammerAitovGrid.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "VisualizerColormap.h"
#include "VisualizerComponent.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class EnergyVisualizerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer,
                                             juce::Slider::Listener
{
public:
    EnergyVisualizerAudioProcessorEditor (EnergyVisualizerAudioProcessor&,
                                          juce::AudioProcessorValueTreeState&);
    ~EnergyVisualizerAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    EnergyVisualizerAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    VisualizerComponent visualizer;
    VisualizerColormap colormap;

    void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;

    TitleBar<AmbisonicIOWidget<>, NoIOWidget> title;
    OSCFooter footer;

    ReverseSlider slPeakLevel, slDynamicRange, slRMStimeConstant;
    juce::ToggleButton tbHoldMax;

    SimpleLabel lbPeakLevel, lbDynamicRange, lbRMStimeConstant;
    std::unique_ptr<SliderAttachment> slPeakLevelAttachment, slDynamicRangeAttachment,
        slRMStimeConstantAttachment;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;
    std::unique_ptr<ButtonAttachment> tbHoldMaxAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnergyVisualizerAudioProcessorEditor)
};
