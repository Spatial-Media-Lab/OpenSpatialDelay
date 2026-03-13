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
#include "../../resources/customComponents/FilterVisualizer.h"
#include "../../resources/customComponents/OnOffButton.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"

typedef ReverseSlider::SliderAttachment
    SliderAttachment; // all ReverseSliders will make use of the parameters' valueToText() function
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class MultiEQAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer,
                                    private juce::Button::Listener,
                                    private juce::ComboBox::Listener
{
public:
    MultiEQAudioProcessorEditor (MultiEQAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~MultiEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void extracted();

    void timerCallback() override;
    void extracted (int f, bool state);

    void buttonClicked (juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

    void updateFilterVisualizer();
    void updateEnablement (const int idx, const bool shouldBeEnabled);

private:
    // ====================== begin essentials ==================
    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;

    // stored references to the AudioProcessor and juce::ValueTreeState holding all the parameters
    MultiEQAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    /* title and footer component
     title component can hold different widgets for in- and output:
        - NoIOWidget (if there's no need for an input or output widget)
        - AudioChannelsIOWidget<maxNumberOfChannels, isChoosable>
        - AmbisonicIOWidget<maxOrder>
        - DirectivitiyIOWidget
     */
    TitleBar<AudioChannelsIOWidget<64, true>, NoIOWidget> title;
    OSCFooter footer;
    // =============== end essentials ============

    // Attachments to create a connection between IOWidgets comboboxes
    // and the associated parameters
    std::unique_ptr<ComboBoxAttachment> cbNumInputChannelsAttachment;

    FilterVisualizer<double> fv;
    juce::TooltipWindow tooltipWin;
    OnOffButton tbFilterOn[numFilterBands];
    juce::ComboBox cbFilterType[numFilterBands];
    ReverseSlider slFilterFrequency[numFilterBands];
    ReverseSlider slFilterQ[numFilterBands];
    ReverseSlider slFilterGain[numFilterBands];
    std::unique_ptr<ButtonAttachment> tbFilterOnAttachment[numFilterBands];
    std::unique_ptr<ComboBoxAttachment> cbFilterTypeAttachment[numFilterBands];
    std::unique_ptr<SliderAttachment> slFilterFrequencyAttachment[numFilterBands];
    std::unique_ptr<SliderAttachment> slFilterQAttachment[numFilterBands];
    std::unique_ptr<SliderAttachment> slFilterGainAttachment[numFilterBands];

    bool gainEnabled[numFilterBands];
    bool qEnabled[numFilterBands];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEQAudioProcessorEditor)
};
