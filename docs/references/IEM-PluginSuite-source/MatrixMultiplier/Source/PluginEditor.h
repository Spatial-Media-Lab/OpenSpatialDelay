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
class MatrixMultiplierAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer,
                                             private juce::Button::Listener
{
public:
    MatrixMultiplierAudioProcessorEditor (MatrixMultiplierAudioProcessor&,
                                          juce::AudioProcessorValueTreeState&);
    ~MatrixMultiplierAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void buttonClicked (juce::Button* button) override;
    void buttonStateChanged (juce::Button* button) override;
    void loadConfigurationFile();

private:
    // ====================== beging essentials ==================
    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;

    // stored references to the AudioProcessor and juce::ValueTreeState holding all the parameters
    MatrixMultiplierAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    /* title and footer component
     title component can hold different widgets for in- and output:
        - NoIOWidget (if there's no need for an input or output widget)
        - AudioChannelsIOWidget<maxNumberOfChannels, isChoosable>
        - AmbisonicIOWidget
        - DirectivitiyIOWidget
     */
    TitleBar<AudioChannelsIOWidget<0, false>, AudioChannelsIOWidget<0, false>> title;
    OSCFooter footer;
    // =============== end essentials ============

    // Attachments to create a connection between IOWidgets comboboxes
    // and the associated parameters

    juce::TextButton btLoadFile;
    juce::TextEditor edOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatrixMultiplierAudioProcessorEditor)
};
