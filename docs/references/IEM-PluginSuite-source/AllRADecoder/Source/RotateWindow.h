/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich, Franz Zotter
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

#include "../../resources/customComponents/SimpleLabel.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class RotateWindow : public juce::Component
{
public:
    RotateWindow (AllRADecoderAudioProcessor& p) : processor (p)
    {
        addAndMakeVisible (headline);
        headline.setText ("Add to Azimuth angles", true, juce::Justification::centred);

        addAndMakeVisible (lbValue);
        lbValue.setText ("0", juce::dontSendNotification);
        lbValue.setEditable (true);
        lbValue.setJustificationType (juce::Justification::centred);

        addAndMakeVisible (tbRotate);
        tbRotate.setButtonText ("ROTATE");
        tbRotate.setColour (juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
        tbRotate.onClick = [this]() { checkAndTriggerRotation(); };
    }

    ~RotateWindow() {}

    void paint (juce::Graphics& g) override {}

    void checkAndTriggerRotation()
    {
        auto val = lbValue.getTextValue();
        float v = val.getValue();
        if (v <= 360.0f && v >= -360.0f)
            processor.rotate (v);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        headline.setBounds (bounds.removeFromTop (12));

        bounds.removeFromTop (2);

        auto row = bounds.removeFromTop (20);

        tbRotate.setBounds (row.removeFromRight (60));

        bounds.removeFromRight (5);
        lbValue.setBounds (row);
    }

private:
    AllRADecoderAudioProcessor& processor;
    SimpleLabel headline;
    juce::Label lbValue;
    juce::TextButton tbRotate;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotateWindow)
};
