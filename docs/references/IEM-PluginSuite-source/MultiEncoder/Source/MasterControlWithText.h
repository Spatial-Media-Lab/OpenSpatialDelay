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
#include "../../resources/customComponents/ReverseSlider.h"
#include "../JuceLibraryCode/JuceHeader.h"

class MasterControlWithText : public juce::Component
{
public:
    MasterControlWithText (juce::OwnedArray<ReverseSlider>& sliderArray) : elements (sliderArray) {}

    void setText (juce::String newText)
    {
        text = newText;
        repaint();
    }

    void mouseEnter (const juce::MouseEvent& e) override { repaint(); }

    void mouseExit (const juce::MouseEvent& e) override { repaint(); }

    void mouseMove (const juce::MouseEvent& e) override
    {
        if (triangleUp.contains (e.position))
            isOverTriangle = 1;
        else if (triangleDown.contains (e.position))
            isOverTriangle = -1;
        else
            isOverTriangle = 0;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        isDragging = true;

        const auto drag = e.getOffsetFromDragStart();
        const auto dist = drag.getX() - drag.getY();

        if (dist > 0)
            dragDirection = 1;
        else if (dist < 0)
            dragDirection = -1;
        else
            dragDirection = 0;

        repaint();

        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseDrag (e);
        }
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        isDragging = true;

        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseWheelMove (e, wheel);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] == nullptr)
                continue;

            if (auto* slider = dynamic_cast<ReverseSlider*> (elements[i]))
            {
                if (isOverTriangle == 1)
                {
                    slider->increment();
                }
                else if (isOverTriangle == -1)
                {
                    slider->decrement();
                }
            }
            elements[i]->mouseDown (e);
        }
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        isDragging = false;
        dragDirection = 0;

        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseUp (e);
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        const bool filled = (! isDragging && isMouseOver());

        const bool isDragginUp = dragDirection == 1;
        const bool isDragginDown = dragDirection == -1;

        g.setColour (juce::Colours::white.withAlpha ((filled || isDragginUp) ? 1.0f : 0.5f));
        g.fillPath (triangleUp);

        g.setColour (juce::Colours::white.withAlpha ((filled || isDragginDown) ? 1.0f : 0.5f));
        g.fillPath (triangleDown);

        bounds.removeFromLeft (bounds.getHeight() - 5);

        g.setColour (juce::Colours::white);
        g.setFont (bounds.getHeight());
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (bounds.getHeight(), 0)));
        g.drawText (text, bounds, juce::Justification::left, true);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        bounds.reduce (0, 3);

        bounds.setWidth (bounds.getHeight());

        auto height = bounds.getHeight();

        auto upperHalf = bounds.removeFromTop (height / 2);
        auto lowerHalf = bounds;

        upperHalf.removeFromBottom (1);
        triangleUp.clear();
        triangleUp.addTriangle (upperHalf.getBottomLeft(),
                                upperHalf.getBottomRight(),
                                { upperHalf.getCentreX(), upperHalf.getY() });

        lowerHalf.removeFromTop (1);
        triangleDown.clear();
        triangleDown.addTriangle (lowerHalf.getTopLeft(),
                                  lowerHalf.getTopRight(),
                                  { lowerHalf.getCentreX(), lowerHalf.getBottom() });
    }

private:
    juce::OwnedArray<ReverseSlider>& elements;
    juce::String text;

    juce::Path triangleUp, triangleDown;

    bool isDragging = false;
    int dragDirection = 0;
    int isOverTriangle = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterControlWithText)
};
