/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Stefan Riedel
 Copyright (c) 2022 - Institute of Electronic Music and Acoustics (IEM)
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
 */
class GranularEncoderAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer,
                                            public SpherePanner::Listener,
                                            private juce::KeyListener
{
public:
    GranularEncoderAudioProcessorEditor (GranularEncoderAudioProcessor&,
                                         juce::AudioProcessorValueTreeState&);
    ~GranularEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseWheelOnSpherePannerMoved (SpherePanner* sphere,
                                        const juce::MouseEvent& event,
                                        const juce::MouseWheelDetails& wheel) override;

    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;

private:
    LaF globalLaF;

    TitleBar<AudioChannelsIOWidget<2, false>, AmbisonicIOWidget<>> title;
    OSCFooter footer;

    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GranularEncoderAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::GroupComponent quatGroup, ypGroup, grainGroup;
    ReverseSlider azimuthSlider, elevationSlider, shapeSlider, sizeSlider, qwSlider, qxSlider,
        qySlider, qzSlider;
    ReverseSlider deltaTimeSlider, deltaTimeModSlider, grainLengthSlider, grainLengthModSlider;
    ReverseSlider positionSlider, positionModSlider, pitchSlider, pitchModSlider;
    ReverseSlider windowAttackSlider, windowAttackModSlider, windowDecaySlider,
        windowDecayModSlider;
    ReverseSlider mixSlider, sourceSlider;
    juce::ComboBox inputChooser;

    SpherePanner sphere;
    SpherePanner::AzimuthElevationParameterElement centerElement;

    std::unique_ptr<SliderAttachment> qwAttachment;
    std::unique_ptr<SliderAttachment> qxAttachment;
    std::unique_ptr<SliderAttachment> qyAttachment;
    std::unique_ptr<SliderAttachment> qzAttachment;
    std::unique_ptr<SliderAttachment> azimuthAttachment;
    std::unique_ptr<SliderAttachment> elevationAttachment;

    std::unique_ptr<SliderAttachment> shapeAttachment;
    std::unique_ptr<SliderAttachment> sizeAttachment;

    std::unique_ptr<SliderAttachment> rollAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;

    std::unique_ptr<SliderAttachment> deltaTimeAttachment;
    std::unique_ptr<SliderAttachment> deltaTimeModAttachment;

    std::unique_ptr<SliderAttachment> grainLengthAttachment;
    std::unique_ptr<SliderAttachment> grainLengthModAttachment;

    std::unique_ptr<SliderAttachment> positionAttachment;
    std::unique_ptr<SliderAttachment> positionModAttachment;

    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> pitchModAttachment;

    std::unique_ptr<SliderAttachment> windowAttackAttachment;
    std::unique_ptr<SliderAttachment> windowAttackModAttachment;

    std::unique_ptr<SliderAttachment> windowDecayAttachment;
    std::unique_ptr<SliderAttachment> windowDecayModAttachment;

    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> sourceAttachment;

    std::unique_ptr<ComboBoxAttachment> cbNormalizationAtachement;
    std::unique_ptr<ComboBoxAttachment> cbOrderAtachement;

    juce::TooltipWindow toolTipWin;

    // labels
    SimpleLabel lbAzimuth, lbElevation, lbShape, lbSize, lbW, lbX, lbY, lbZ;
    SimpleLabel lbDeltaTime, lbDeltaTimeMod, lbGrainLength, lbGrainLengthMod;
    SimpleLabel lbPosition, lbPositionMod, lbPitch, lbPitchMod;
    SimpleLabel lbWindowAttack, lbWindowAttackMod, lbWindowDecay, lbWindowDecayMod;
    SimpleLabel lbMix, lbSource;

    juce::ToggleButton tbFreeze;

    // juce::ToggleButton tb2D;
    // SimpleLabel lb2D;

    juce::ComboBox cb2D3D;
    std::unique_ptr<ComboBoxAttachment> cb2D3DAtachement;

    std::unique_ptr<ButtonAttachment> tbFreezeAttachment;
    std::unique_ptr<ButtonAttachment> tb2DAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularEncoderAudioProcessorEditor)
};
