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

//==============================================================================
/*
*/
class LevelMeter : public juce::Component
{
    class Overlay : public juce::Component
    {
    public:
        Overlay() { setBufferedToImage (true); }
        ~Overlay() override = default;

        const float decibelsToY (const float dB)
        {
            return offset - scale * std::tanh (dB / minLevel * -2.0f);
        }

        void setMinLevel (float newMinLevel)
        {
            minLevel = newMinLevel;
            repaint();
        }

        const float getOffset() { return offset; }

        const float getMeterHeight() { return getHeight() - 2; }

        const juce::Rectangle<int> getMeterArea() { return meterArea; }

    private:
        void paint (juce::Graphics& g) override
        {
            juce::Path bg;
            juce::Rectangle<int> meterArea (0, 0, getWidth(), getHeight());
            meterArea.reduce (2, 2);
            int width = meterArea.getWidth();
            int xPos = meterArea.getX();
            bg.addRoundedRectangle (meterArea, 2);

            g.setColour (juce::Colour (0xFF212121));
            g.strokePath (bg, juce::PathStrokeType (2.f));

            g.setColour (juce::Colours::white);
            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
            g.setFont (9.0f);

            int lastTextDrawPos = -1;
            drawLevelMark (g, xPos, width, 0, "0");
            drawLevelMark (g, xPos, width, -3, "3");
            drawLevelMark (g, xPos, width, -6, "6");

            for (float dB = -10.0f; dB >= minLevel; dB -= 5.0f)
                lastTextDrawPos = drawLevelMark (g,
                                                 xPos,
                                                 width,
                                                 dB,
                                                 juce::String (juce::roundToInt (-dB)),
                                                 lastTextDrawPos);
        }

        void resized() override
        {
            offset = 0.1 * getHeight();
            scale = getHeight() - offset;

            meterArea = juce::Rectangle<int> (0, 0, getWidth(), getHeight()).reduced (2, 2);
        }

        const int inline drawLevelMark (juce::Graphics& g,
                                        int x,
                                        int width,
                                        const int level,
                                        const juce::String& label,
                                        int lastTextDrawPos = -1)
        {
            float yPos = decibelsToY (level);
            x = x + 1.0f;
            width = width - 2.0f;

            g.drawLine (x, yPos, x + 2, yPos);
            g.drawLine (x + width - 2, yPos, x + width, yPos);

            if (yPos - 4 > lastTextDrawPos)
            {
                g.drawText (label,
                            x + 2,
                            yPos - 4,
                            width - 4,
                            9,
                            juce::Justification::centred,
                            false);
                return yPos + 5;
            }
            return lastTextDrawPos;
        }

        juce::Rectangle<int> meterArea;
        float minLevel = -60.0f;
        float scale = 0.0f;
        float offset = 0.0f;
    };

public:
    LevelMeter() { addAndMakeVisible (overlay); }

    ~LevelMeter() {}

    void setGainReductionMeter (bool isGainReductionMeter)
    {
        isGRmeter = isGainReductionMeter;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const int height = overlay.getMeterHeight();

        auto meterArea = overlay.getMeterArea();

        g.setColour (juce::Colours::black);
        g.fillRect (meterArea);

        juce::Rectangle<int> lvlRect;
        if (isGRmeter)
            lvlRect = juce::Rectangle<int> (
                juce::Point<int> (meterArea.getX(), overlay.getOffset()),
                juce::Point<int> (meterArea.getRight(), overlay.decibelsToY (level)));
        else
            lvlRect = juce::Rectangle<int> (
                juce::Point<int> (meterArea.getX(), height),
                juce::Point<int> (meterArea.getRight(), overlay.decibelsToY (level)));

        g.setColour (levelColour);
        g.fillRect (lvlRect);
    }

    void setColour (juce::Colour newColour)
    {
        levelColour = newColour;
        repaint();
    }

    void setLevel (const float newLevel)
    {
        if (level != newLevel)
        {
            level = newLevel;
            repaint();
        }
    }

    void setMinLevel (float newMinLevel)
    {
        overlay.setMinLevel (newMinLevel);
        repaint();
    }

    void resized() override { overlay.setBounds (getLocalBounds()); }

private:
    Overlay overlay;

    juce::Colour levelColour = juce::Colour (juce::Colours::green);

    bool isGRmeter = false;
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
