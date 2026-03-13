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

//==============================================================================
/*
*/
class XYPad : public juce::Component
{
    struct xySlidersAndColour
    {
        juce::Slider* xSlider = nullptr;
        juce::Slider* ySlider = nullptr;
        juce::Colour colour;
    };

public:
    XYPad() {}
    ~XYPad() {}

    void paint (juce::Graphics& g) override
    {
        int topLeftX = plotArea.getX();
        int bottomY = plotArea.getY() + plotArea.getHeight();

        int size = elements.size();
        for (int i = 0; i < size; ++i)
        {
            bool isActive = activeElem == i;

            xySlidersAndColour handle = elements.getReference (i);

            juce::Range<double> xRange = handle.xSlider->getRange();
            juce::Range<double> yRange = handle.ySlider->getRange();

            float xPos =
                topLeftX
                + (handle.xSlider->getValue() - xRange.getStart()) * width / xRange.getLength();
            float yPos =
                bottomY
                - (handle.ySlider->getValue() - yRange.getStart()) * height / yRange.getLength();

            juce::Path path;
            path.addEllipse (xPos - 8, yPos - 8, 16, 16);
            g.setColour (handle.colour);
            g.fillPath (path);

            if (isActive)
            {
                g.setColour (juce::Colours::white);
                g.strokePath (path, juce::PathStrokeType (2.0f));
            }
        }
    }

    void resized() override
    {
        juce::Rectangle<int> bounds = getLocalBounds();

        bounds.reduce (12, 12);

        plotArea = bounds;
        width = bounds.getWidth();
        height = bounds.getHeight();
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition();
        int oldActiveElem = activeElem;
        activeElem = -1;

        int topLeftX = plotArea.getX();
        int bottomY = plotArea.getY() + plotArea.getHeight();
        for (int i = elements.size(); --i >= 0;)
        {
            xySlidersAndColour handle = elements.getReference (i);

            juce::Range<double> xRange = handle.xSlider->getRange();
            juce::Range<double> yRange = handle.ySlider->getRange();

            float xPos =
                topLeftX
                + (handle.xSlider->getValue() - xRange.getStart()) * width / xRange.getLength();
            float yPos =
                bottomY
                - (handle.ySlider->getValue() - yRange.getStart()) * height / yRange.getLength();

            if (pos.getDistanceSquaredFrom (juce::Point<float> (xPos, yPos).toInt()) < 80)
            {
                activeElem = i;
                break;
            }
        }
        if (oldActiveElem != activeElem)
            repaint();
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition() - plotArea.getTopLeft();
        if (activeElem != -1 && elements.size() - 1 >= activeElem)
        {
            xySlidersAndColour handle = elements.getReference (activeElem);
            juce::Range<double> xRange = handle.xSlider->getRange();
            juce::Range<double> yRange = handle.ySlider->getRange();

            handle.xSlider->setValue (xRange.getLength() * pos.x / width + xRange.getStart());
            handle.ySlider->setValue (yRange.getLength() * (height - pos.y) / height
                                      + yRange.getStart());
            repaint();
        }
    }
    void mouseUp (const juce::MouseEvent& event) override
    {
        activeElem = -1;
        repaint();
    }

    void addElement (juce::Slider& newXSlider, juce::Slider& newYSlider, juce::Colour newColour)
    {
        elements.add ({ &newXSlider, &newYSlider, newColour });
    }

protected:
    juce::Array<xySlidersAndColour> elements;

    int activeElem = -1;

    juce::Rectangle<int> plotArea;
    float width;
    float height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYPad)
};
