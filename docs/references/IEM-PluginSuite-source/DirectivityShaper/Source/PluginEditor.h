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
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//Plugin Design Essentials
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

//Custom juce::Components
#include "../../resources/customComponents/FilterVisualizer.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "DirectivityVisualizer.h"
#include "ShapeAndOrderXyPad.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    DirectivityShaperAudioProcessorEditor (DirectivityShaperAudioProcessor&,
                                           juce::AudioProcessorValueTreeState&);
    ~DirectivityShaperAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LaF globalLaF;

    DirectivityShaperAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    ShapeAndOrderXyPad xyPad;

    float weights[numberOfBands][8];

    void timerCallback() override;

    TitleBar<AudioChannelsIOWidget<1, false>, DirectivityIOWidget> title;
    OSCFooter footer;

    int maxPossibleOrder = -1;
    int ambisonicOrder = -1;

    juce::ComboBox cbFilterType[numberOfBands];
    ReverseSlider slFilterFrequency[numberOfBands];
    ReverseSlider slFilterQ[numberOfBands];
    ReverseSlider slFilterGain[numberOfBands];
    ReverseSlider slOrder[numberOfBands];
    ReverseSlider slShape[numberOfBands];
    ReverseSlider slAzimuth[numberOfBands];
    ReverseSlider slElevation[numberOfBands];
    ReverseSlider slProbeAzimuth;
    ReverseSlider slProbeElevation;
    ReverseSlider slProbeRoll;
    juce::ComboBox cbDirectivityNormalization;
    std::unique_ptr<ComboBoxAttachment> cbDirectivityNormalizationAttachment; // on-axis, energy

    SpherePanner sphere;
    std::unique_ptr<SpherePanner::AzimuthElevationParameterElement> sphereElements[numberOfBands];
    SpherePanner::AzimuthElevationParameterElement probeElement;

    SimpleLabel lbAzimuth, lvElevation, lbOrder, lbShape;
    SimpleLabel lbProbeAzimuth, lbProbeElevation, lbProbeRoll;
    SimpleLabel lbNormalization;

    juce::GroupComponent gcFilterBands, gcOrderAndShape, gcPanning, gcSettings;

    juce::ToggleButton tbProbeLock;
    std::unique_ptr<ComboBoxAttachment> cbFilterTypeAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slFilterFrequencyAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slFilterQAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slFilterGainAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slOrderAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slShapeAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slAzimuthAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slElevationAttachment[numberOfBands];
    std::unique_ptr<SliderAttachment> slProbeAzimuthAttachment;
    std::unique_ptr<SliderAttachment> slProbeElevationAttachment;
    std::unique_ptr<SliderAttachment> slProbeRollAttachment;
    std::unique_ptr<ButtonAttachment> tbProbeLockAttachment;
    DirectivityVisualizer dv;
    FilterVisualizer<float> fv;

    std::unique_ptr<SliderAttachment> slParam1Attachment, slParam2Attachment, slParam3Attachment;
    std::unique_ptr<ComboBoxAttachment> cbOrderSettingAttachment;
    std::unique_ptr<ComboBoxAttachment> cbNormalizationAttachment; // n3d, sn3d

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityShaperAudioProcessorEditor)
};
