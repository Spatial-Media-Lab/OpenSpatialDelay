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

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
MultiEQAudioProcessorEditor::MultiEQAudioProcessorEditor (MultiEQAudioProcessor& p,
                                                          juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (880, 330, 1000, 800); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Multi"), juce::String ("EQ"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    // create the connection between title component's comboBoxes and parameters
    cbNumInputChannelsAttachment.reset (
        new ComboBoxAttachment (valueTreeState,
                                "inputChannelsSetting",
                                *title.getInputWidgetPtr()->getChannelsCbPointer()));

    tooltipWin.setLookAndFeel (&globalLaF);
    tooltipWin.setMillisecondsBeforeTipAppears (500);
    tooltipWin.setOpaque (false);

    const juce::Colour colours[numFilterBands] = {
        juce::Colours::cadetblue, // make sure you have enough colours in here
        juce::Colours::mediumpurple, juce::Colours::cornflowerblue, juce::Colours::greenyellow,
        juce::Colours::yellow,       juce::Colours::orangered
    };

    for (int f = 0; f < numFilterBands; ++f)
    {
        gainEnabled[f] = true;
        qEnabled[f] = true;
    }
    gainEnabled[0] = false;
    gainEnabled[numFilterBands - 1] = false;

    addAndMakeVisible (fv);
    for (int f = 0; f < numFilterBands; ++f)
        fv.addCoefficients (processor.getCoefficientsForGui (f),
                            colours[f],
                            &slFilterFrequency[f],
                            &slFilterGain[f],
                            &slFilterQ[f]);

    fv.enableFilter (2, false);

    for (int i = 0; i < numFilterBands; ++i)
    {
        addAndMakeVisible (&tbFilterOn[i]);
        tbFilterOn[i].setColour (juce::ToggleButton::tickColourId, colours[i]);
        tbFilterOn[i].addListener (this);
        tbFilterOnAttachment[i].reset (new ButtonAttachment (valueTreeState,
                                                             "filterEnabled" + juce::String (i),
                                                             tbFilterOn[i]));

        const bool enabled = tbFilterOn[i].getToggleState();

        addAndMakeVisible (&cbFilterType[i]);

        if (i == 0)
        {
            cbFilterType[i].addItem ("HP (6dB/oct)", 1);
            cbFilterType[i].addItem ("HP (12dB/oct)", 2);
            cbFilterType[i].addItem ("HP (24db/oct)", 3);
            cbFilterType[i].addItem ("Low-shelf", 4);
        }
        else if (i == numFilterBands - 1)
        {
            cbFilterType[i].addItem ("LP (6dB/oct)", 1);
            cbFilterType[i].addItem ("LP (12dB/oct)", 2);
            cbFilterType[i].addItem ("LP (24dB/oct)", 3);
            cbFilterType[i].addItem ("High-shelf", 4);
        }
        else
        {
            cbFilterType[i].addItem ("Low-shelf", 1);
            cbFilterType[i].addItem ("Peak", 2);
            cbFilterType[i].addItem ("High-shelf", 3);
        }

        cbFilterType[i].setJustificationType (juce::Justification::centred);
        cbFilterTypeAttachment[i].reset (new ComboBoxAttachment (valueTreeState,
                                                                 "filterType" + juce::String (i),
                                                                 cbFilterType[i]));

        addAndMakeVisible (&slFilterFrequency[i]);
        slFilterFrequency[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slFilterFrequency[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
        slFilterFrequency[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterFrequencyAttachment[i].reset (
            new SliderAttachment (valueTreeState,
                                  "filterFrequency" + juce::String (i),
                                  slFilterFrequency[i]));

        addAndMakeVisible (&slFilterQ[i]);
        slFilterQ[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slFilterQ[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
        slFilterQ[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterQAttachment[i].reset (
            new SliderAttachment (valueTreeState, "filterQ" + juce::String (i), slFilterQ[i]));

        addAndMakeVisible (&slFilterGain[i]);
        slFilterGain[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slFilterGain[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
        slFilterGain[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterGainAttachment[i].reset (new SliderAttachment (valueTreeState,
                                                               "filterGain" + juce::String (i),
                                                               slFilterGain[i]));

        updateEnablement (i, enabled);
    }

    cbFilterType[0].addListener (this);
    cbFilterType[numFilterBands - 1].addListener (this);

    updateFilterVisualizer();

    // start timer after everything is set up properly
    startTimer (20);
}

MultiEQAudioProcessorEditor::~MultiEQAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MultiEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiEQAudioProcessorEditor::resized()
{
    DBG ("GUI resized to " << getLocalBounds().getWidth() << "x" << getLocalBounds().getHeight());
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);
    // =========== END: header and footer =================

    // try to not use explicit coordinates to position your GUI components
    // the removeFrom...() methods are quite handy to create scalable areas
    // best practice would be the use of flexBoxes...
    // the following is medium level practice ;-)
    juce::Rectangle<int> filterArea = area;
    { // upper row

        juce::Rectangle<int> cbArea (filterArea.removeFromBottom (50));
        for (int i = 0; i < numFilterBands; ++i)
        {
            slFilterFrequency[i].setBounds (cbArea.removeFromLeft (45));
            slFilterGain[i].setBounds (cbArea.removeFromLeft (40));
            slFilterQ[i].setBounds (cbArea.removeFromLeft (35));
            cbArea.removeFromLeft (20);
        }

        cbArea = filterArea.removeFromBottom (21);
        cbArea.removeFromLeft (3);
        for (int i = 0; i < numFilterBands; ++i)
        {
            tbFilterOn[i].setBounds (cbArea.removeFromLeft (18));
            cbArea.removeFromLeft (5);
            cbFilterType[i].setBounds (cbArea.removeFromLeft (92).reduced (0, 3));
            cbArea.removeFromLeft (25);
        }

        fv.setBounds (filterArea);
    }
}

void MultiEQAudioProcessorEditor::updateFilterVisualizer()
{
    processor.updateGuiCoefficients();
    fv.setSampleRate (processor.getSampleRate());
    for (int f = 0; f < numFilterBands; ++f)
        fv.replaceCoefficients (f, processor.getCoefficientsForGui (f));
}

void MultiEQAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.repaintFV.get())
    {
        processor.repaintFV = false;
        updateFilterVisualizer();
    }
}

void MultiEQAudioProcessorEditor::updateEnablement (const int idx, const bool shouldBeEnabled)
{
    slFilterFrequency[idx].setEnabled (shouldBeEnabled);
    slFilterGain[idx].setEnabled (shouldBeEnabled && gainEnabled[idx]);
    slFilterQ[idx].setEnabled (shouldBeEnabled && qEnabled[idx]);
    cbFilterType[idx].setEnabled (shouldBeEnabled);
    fv.enableFilter (idx, shouldBeEnabled);
}

void MultiEQAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    for (int f = 0; f < numFilterBands; ++f)
    {
        if (button == &tbFilterOn[f])
        {
            const bool state = button->getToggleState();
            updateEnablement (f, state);
        }
    }
}

void MultiEQAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    int idx = -1;
    if (comboBoxThatHasChanged == &cbFilterType[0])
        idx = 0;
    else if (comboBoxThatHasChanged == &cbFilterType[numFilterBands - 1])
        idx = numFilterBands - 1;
    else
        return;

    const auto id = comboBoxThatHasChanged->getSelectedItemIndex();
    if (id == 0 || id == 2)
    {
        qEnabled[idx] = false;
        gainEnabled[idx] = false;
    }
    else if (id == 1)
    {
        qEnabled[idx] = true;
        gainEnabled[idx] = false;
    }
    else
    {
        qEnabled[idx] = true;
        gainEnabled[idx] = true;
    }

    updateEnablement (idx, tbFilterOn[idx].getToggleState());
}
