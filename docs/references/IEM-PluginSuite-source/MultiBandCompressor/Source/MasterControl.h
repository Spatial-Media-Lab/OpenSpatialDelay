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

//==============================================================================
/**
    This is a simple wrapper for a juce::Slider that controls other components,
    e.g. other juce::Sliders, by forwarding mouse events. It's intended to hold
    pointers to the sliders it is controlling.
*/
class MasterControl : public juce::Component
{
public:
    MasterControl() {}

    ~MasterControl() {}

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

        auto drag = e.getOffsetFromDragStart();
        DBG (drag.getX() << " - " << drag.getY());

        auto dist = drag.getX() - drag.getY();

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

    void addSlave (juce::Component& newComponentToControl)
    {
        elements.add (&newComponentToControl);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::white);

        auto thickness = (! isDragging && isMouseOver()) ? 1.0f : 0.5f;

        const bool isDragginUp = dragDirection == 1;
        const bool isDragginDown = dragDirection == -1;

        auto upThickness = isDragginUp ? 1.0f : thickness;
        auto downThickness = isDragginDown ? 1.0f : thickness;

        g.strokePath (triangleUp, juce::PathStrokeType (upThickness));
        g.strokePath (triangleDown, juce::PathStrokeType (downThickness));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (isDragginUp ? 0.9f : 0.3f));
        g.fillPath (triangleUp);

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (isDragginDown ? 0.9f : 0.3f));
        g.fillPath (triangleDown);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        auto centre = bounds.getCentre();

        bounds.setHeight (25);
        bounds.setWidth (20);
        bounds.setCentre (centre);

        auto height = bounds.getHeight();

        auto upperHalf = bounds.removeFromTop (height / 2);
        auto lowerHalf = bounds;

        upperHalf.removeFromBottom (2);
        triangleUp.clear();
        triangleUp.addTriangle (upperHalf.getBottomLeft(),
                                upperHalf.getBottomRight(),
                                { upperHalf.getCentreX(), upperHalf.getY() });

        lowerHalf.removeFromTop (2);
        triangleDown.clear();
        triangleDown.addTriangle (lowerHalf.getTopLeft(),
                                  lowerHalf.getTopRight(),
                                  { lowerHalf.getCentreX(), lowerHalf.getBottom() });
    }

private:
    juce::Array<Component*> elements;

    juce::Path triangleUp, triangleDown;

    bool isDragging = false;
    int dragDirection = 0;
    int isOverTriangle = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterControl)
};
