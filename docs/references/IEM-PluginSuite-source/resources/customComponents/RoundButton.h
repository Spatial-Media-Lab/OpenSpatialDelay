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
class RoundButton : public juce::ToggleButton
{
public:
    RoundButton() {}

    ~RoundButton() {}

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        juce::Rectangle<float> buttonArea;
        if (isCircularShape)
        {
            const float boxSize = bounds.getWidth() >= bounds.getHeight()
                                      ? bounds.getHeight() * 0.8f
                                      : bounds.getWidth() * 0.8f;
            buttonArea = juce::Rectangle<float> ((bounds.getWidth() - boxSize) * 0.5f,
                                                 (bounds.getHeight() - boxSize) * 0.5f,
                                                 boxSize,
                                                 boxSize);
        }
        else
        {
            buttonArea = juce::Rectangle<float> (bounds.getX(),
                                                 bounds.getY(),
                                                 bounds.getWidth(),
                                                 bounds.getHeight());
            buttonArea.reduce (0.5f, 0.4f);
        }

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

        isCircularShape == true ? g.drawEllipse (buttonArea, 1.0f)
                                : g.drawRoundedRectangle (buttonArea, 10.0f, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);
        g.setColour (findColour (juce::ToggleButton::tickColourId)
                         .withMultipliedAlpha (ticked              ? 1.0f
                                               : isMouseOverButton ? 0.5f
                                                                   : 0.2f));

        isCircularShape == true ? g.fillEllipse (buttonArea)
                                : g.fillRoundedRectangle (buttonArea, 10.0f);

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (13.0f, 1)));
        g.setFont (buttonArea.getHeight() * scaleFontSize);
        g.setColour (findColour (getToggleState() ? juce::TextButton::textColourOnId
                                                  : juce::TextButton::textColourOffId)
                         .withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f));

        g.setColour (ticked ? findColour (juce::ResizableWindow::backgroundColourId)
                            : findColour (juce::ToggleButton::tickColourId)
                                  .withMultipliedAlpha (isMouseOverButton ? 0.7f : 0.5f));

        g.drawText (getButtonText(), getLocalBounds(), juce::Justification::centred);
    }

    void resized() override {}

    void setScaleFontSize (const float newScale) { scaleFontSize = newScale; }

    void setCircularShape (bool shouldBeCircularShape) { isCircularShape = shouldBeCircularShape; }

private:
    bool isCircularShape = true;
    float scaleFontSize = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundButton)
};
