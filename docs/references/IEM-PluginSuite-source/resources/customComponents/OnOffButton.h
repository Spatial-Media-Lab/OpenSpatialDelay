/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
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

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
static const unsigned char powerButtonData[] = {
    110, 109, 43,  135, 207, 65,  0,   0,   0,   0,   98,  47,  221, 189, 65,  0,   0,   0,   0,
    68,  139, 175, 65,  47,  221, 228, 63,  68,  139, 175, 65,  20,  174, 127, 64,  108, 68,  139,
    175, 65,  51,  51,  204, 65,  98,  68,  139, 175, 65,  35,  219, 221, 65,  47,  221, 189, 65,
    246, 40,  236, 65,  43,  135, 207, 65,  246, 40,  236, 65,  98,  39,  49,  225, 65,  246, 40,
    236, 65,  18,  131, 239, 65,  35,  219, 221, 65,  18,  131, 239, 65,  51,  51,  204, 65,  108,
    18,  131, 239, 65,  20,  174, 127, 64,  98,  18,  131, 239, 65,  47,  221, 228, 63,  39,  49,
    225, 65,  0,   0,   0,   0,   43,  135, 207, 65,  0,   0,   0,   0,   99,  109, 70,  182, 31,
    65,  133, 235, 13,  65,  98,  66,  96,  15,  65,  162, 69,  14,  65,  4,   86,  254, 64,  229,
    208, 20,  65,  35,  219, 229, 64,  178, 157, 33,  65,  98,  12,  2,   35,  64,  2,   43,  111,
    65,  0,   0,   0,   0,   31,  133, 170, 65,  0,   0,   0,   0,   221, 36,  224, 65,  98,  0,
    0,   0,   0,   76,  55,  41,  66,  98,  16,  58,  65,  125, 191, 87,  66,  43,  135, 207, 65,
    125, 191, 87,  66,  98,  12,  2,   33,  66,  125, 191, 87,  66,  98,  144, 79,  66,  76,  55,
    41,  66,  37,  134, 79,  66,  221, 36,  224, 65,  98,  12,  130, 79,  66,  31,  133, 170, 65,
    10,  87,  69,  66,  2,   43,  111, 65,  199, 203, 50,  66,  178, 157, 33,  65,  98,  94,  186,
    44,  66,  113, 61,  8,   65,  117, 147, 34,  66,  2,   43,  7,   65,  20,  46,  28,  66,  227,
    165, 31,  65,  98,  4,   214, 21,  66,  133, 235, 55,  65,  129, 149, 21,  66,  201, 118, 96,
    65,  57,  180, 27,  66,  74,  12,  122, 65,  98,  86,  142, 40,  66,  72,  225, 151, 65,  141,
    151, 47,  66,  231, 251, 186, 65,  141, 151, 47,  66,  221, 36,  224, 65,  98,  141, 151, 47,
    66,  172, 156, 23,  66,  186, 73,  15,  66,  186, 201, 55,  66,  139, 108, 207, 65,  186, 201,
    55,  66,  98,  174, 71,  128, 65,  186, 201, 55,  66,  188, 116, 255, 64,  172, 156, 23,  66,
    188, 116, 255, 64,  221, 36,  224, 65,  98,  188, 116, 255, 64,  231, 251, 186, 65,  133, 235,
    27,  65,  180, 200, 151, 65,  199, 75,  79,  65,  74,  12,  122, 65,  98,  217, 206, 103, 65,
    250, 126, 96,  65,  219, 249, 102, 65,  221, 36,  56,  65,  90,  100, 77,  65,  227, 165, 31,
    65,  98,  141, 151, 64,  65,  115, 104, 19,  65,  98,  16,  48,  65,  104, 145, 13,  65,  70,
    182, 31,  65,  133, 235, 13,  65,  99,  109, 70,  182, 31,  65,  133, 235, 13,  65,  99,  101,
    0,   0
};

class OnOffButton : public juce::ToggleButton
{
public:
    OnOffButton() { powerButton.loadPathFromData (powerButtonData, sizeof (powerButtonData)); }

    ~OnOffButton() override = default;

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        const float boxSize = bounds.getWidth() * 0.8f;
        juce::Rectangle<float> buttonArea (0.5f * (bounds.getWidth() - boxSize),
                                           0.5f * (bounds.getHeight() - boxSize),
                                           boxSize,
                                           boxSize);

        const bool isButtonDown = isMouseButtonDown();
        const bool isMouseOverButton = isMouseOver();

        const bool ticked = getToggleState();

        if (isButtonDown)
            buttonArea.reduce (0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce (0.4f, 0.4f);

        g.setColour (findColour (juce::ToggleButton::tickColourId)
                         .withMultipliedAlpha (ticked              ? 1.0f
                                               : isMouseOverButton ? 0.7f
                                                                   : 0.5f));

        g.drawEllipse (buttonArea, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);

        g.setColour (findColour (juce::ToggleButton::tickColourId)
                         .withMultipliedAlpha (ticked              ? 1.0f
                                               : isMouseOverButton ? 0.5f
                                                                   : 0.2f));

        g.fillEllipse (buttonArea);

        g.setColour (ticked ? findColour (juce::ResizableWindow::backgroundColourId)
                            : findColour (juce::ToggleButton::tickColourId)
                                  .withMultipliedAlpha (isMouseOverButton ? 0.7f : 0.5f));

        g.fillPath (powerButton,
                    powerButton.getTransformToScaleToFit (buttonArea.reduced (2.0f), true));
    }

    void resized() override {}

private:
    juce::Path powerButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OnOffButton)
};
