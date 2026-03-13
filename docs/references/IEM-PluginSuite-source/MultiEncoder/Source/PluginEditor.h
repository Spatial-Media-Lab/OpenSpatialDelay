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

#include "../../resources/customComponents/MuteSoloButton.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "EncoderList.h"
#include "EnergySpherePanner.h"
#include "MasterControlWithText.h"
#include "PluginProcessor.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class MultiEncoderAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer,
                                         private SpherePanner::Listener
{
public:
    MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor&,
                                      juce::AudioProcessorValueTreeState&);
    ~MultiEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void importLayout();

private:
    LaF globalLaF;
    TitleBar<AudioChannelsIOWidget<maxNumberOfInputs>, AmbisonicIOWidget<>> title;
    OSCFooter footer;

    void timerCallback() override;
    void mouseWheelOnSpherePannerMoved (SpherePanner* sphere,
                                        const juce::MouseEvent& event,
                                        const juce::MouseWheelDetails& wheel) override;

    MultiEncoderAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::GroupComponent masterGroup, encoderGroup, rmsGroup;
    juce::TextButton tbImport;

    ReverseSlider slMasterAzimuth, slMasterElevation, slMasterRoll;
    juce::ToggleButton tbLockedToMaster;

    ReverseSlider slDynamicRange;
    std::unique_ptr<SliderAttachment> slDynamicRangeAttachment;
    SimpleLabel lbDynamicRange;

    ReverseSlider slPeakLevel;
    std::unique_ptr<SliderAttachment> slPeakLevelAttachment;
    SimpleLabel lbPeakLevel;

    juce::ToggleButton tbAnalyzeRMS;
    std::unique_ptr<ButtonAttachment> tbAnalyzeRMSAttachment;

    juce::ComboBox inputChooser;

    EnergySpherePanner sphere;
    SpherePanner::AzimuthElevationParameterElement masterElement;

    std::unique_ptr<SliderAttachment> slMasterAzimuthAttachment;
    std::unique_ptr<SliderAttachment> slMasterElevationAttachment;
    std::unique_ptr<SliderAttachment> slMasterRollAttachment;
    std::unique_ptr<ButtonAttachment> tbLockedToMasterAttachment;

    std::unique_ptr<ComboBoxAttachment> cbNumInputChannelsAttachment, cbNormalizationAtachment;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachment;

    juce::Viewport viewport;
    std::unique_ptr<EncoderList> encoderList;

    juce::TooltipWindow tooltipWin;

    int maxPossibleOrder = -1;
    int maxNumInputs = -1;
    int lastSetNumChIn = -1;

    // labels
    SimpleLabel lbNum;
    std::unique_ptr<MasterControlWithText> lbAzimuth, lbElevation, lbGain;
    SimpleLabel lbMasterAzimuth, lbMasterElevation, lbMasterRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEncoderAudioProcessorEditor)
};
