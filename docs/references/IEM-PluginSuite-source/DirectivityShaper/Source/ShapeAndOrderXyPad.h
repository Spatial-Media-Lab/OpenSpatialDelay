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

#include "../../resources/customComponents/XYPad.h"
#include "../JuceLibraryCode/JuceHeader.h"
//==============================================================================
/*
*/
class ShapeAndOrderXyPad : public XYPad
{
public:
    ShapeAndOrderXyPad() { maxOrder = 7; }

    ~ShapeAndOrderXyPad() {}

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        const int height = bounds.getHeight();

        int centreX = bounds.getCentreX();
        int centreY = bounds.getCentreY();

        g.setColour (juce::Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
        g.setFont (12.0f);
        g.drawText ("SHAPE", centreX - 15, height - 12, 30, 12, juce::Justification::centred);
        //g.drawMultiLineText("LEFT", 0, centreY-12, 10);
        g.drawFittedText ("O\nR\nD\nE\nR",
                          0,
                          centreY - 40,
                          10,
                          80,
                          juce::Justification::centred,
                          5);
        //g.drawFittedText("R\nI\nG\nH\nT", bounds.getWidth()-10, centreY-40, 10, 80, juce::Justification::centred, 5);

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.3f));
        g.fillRect (plotArea.reduced (2, 2));

        if (maxOrder < 7)
        {
            juce::Rectangle<int> restricted (plotArea);
            restricted.setHeight ((float) restricted.getHeight() / 7 * (7 - maxOrder));

            g.setColour (juce::Colours::red);
            g.drawRect (restricted, 1.0f);
            g.setColour (juce::Colours::red.withMultipliedAlpha (0.3f));
            g.fillRect (restricted.reduced (2, 2));
        }

        g.setColour (juce::Colours::black.withMultipliedAlpha (0.2f));
        for (int i = 1; i < 7; ++i)
        {
            float y = orderToY (i);
            g.drawLine (plotArea.getX(), y, plotArea.getRight(), y);
        }
        g.drawLine (centreX, plotArea.getY(), centreX, plotArea.getBottom());

        g.setColour (juce::Colours::white);
        for (int i = 0; i < 8; ++i)
        {
            float y = orderToY (i);
            g.drawFittedText (juce::String (i),
                              bounds.getWidth() - 9,
                              y - 6,
                              10,
                              12,
                              juce::Justification::centredLeft,
                              1);
        }
        g.drawFittedText ("basic", plotArea.getX(), 0, 40, 12, juce::Justification::left, 1);
        g.drawFittedText ("maxrE", centreX - 20, 0, 40, 12, juce::Justification::centred, 1);
        g.drawFittedText ("inphase",
                          plotArea.getRight() - 40,
                          0,
                          40,
                          12,
                          juce::Justification::right,
                          1);

        g.setColour (juce::Colours::white.withMultipliedAlpha (0.8));
        g.drawRect (plotArea, 1);

        XYPad::paint (g);
    }
    float orderToY (int order)
    {
        int height = plotArea.getHeight();
        return plotArea.getY() + (float) height / 7 * (7 - order);
    }
    void resized() override { XYPad::resized(); }
    void setMaxOrder (int order)
    {
        if (maxOrder != order)
        {
            maxOrder = order;
            repaint();
        }
    }

private:
    int maxOrder;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShapeAndOrderXyPad)
};
