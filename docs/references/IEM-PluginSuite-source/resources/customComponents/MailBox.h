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

#include "../JuceLibraryCode/JuceHeader.h"
#pragma once

// For now, this class defines messages, which can be displayed by the Display component. Future might bring more features like switching between several active messages... we will see ;-)

namespace MailBox
{
struct Message
{ // could become a reference counted object and Display should check regularly if its the only one who holds this reference -> delete
    juce::String headline = "No Message available";
    juce::String text = "";
    juce::Colour messageColour = juce::Colours::lightgrey;
};

class Display : public juce::Component
{
public:
    Display() : juce::Component() {}

    ~Display() override {}

    void paint (juce::Graphics& g) override
    {
        juce::Colour messageColour = message.messageColour;

        juce::Colour textColour = juce::Colours::white;

        juce::Rectangle<int> background (getLocalBounds());

        g.setColour (messageColour);
        g.drawRect (background);
        g.setColour (messageColour.withMultipliedAlpha (0.1f));
        g.fillRect (background);

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
        g.setFont (17.0f);

        juce::Rectangle<int> textArea = background.reduced (4, 2);
        g.setColour (textColour);
        g.drawText (message.headline, textArea.removeFromTop (20), juce::Justification::topLeft);

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (14.0f);

        g.drawFittedText (message.text,
                          textArea,
                          juce::Justification::topLeft,
                          juce::roundToInt (textArea.getHeight() / 13.0f));
    }

    void resized() override {}

    void setMessage (Message messageToDisplay)
    {
        message = messageToDisplay;
        repaint();
    }

private:
    Message message;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Display)
};
} // namespace MailBox
