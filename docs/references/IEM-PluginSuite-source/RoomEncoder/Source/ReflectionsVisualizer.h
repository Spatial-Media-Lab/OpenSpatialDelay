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
class ReflectionsVisualizer : public juce::Component, private juce::Timer
{
    const float mL = 23.0f;
    const float mR = 10.0f;
    const float mT = 7.0f;
    const float mB = 15.0f;

public:
    ReflectionsVisualizer()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

        startTimer (10); //TODO avoid using timer
    }
    ~ReflectionsVisualizer() {}

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.01f));
        g.fillAll();

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.9f));
        g.strokePath (axes, juce::PathStrokeType (1.0f));
        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (dBGrid, juce::PathStrokeType (0.5f));

        g.setColour (juce::Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (12.0f);

        for (int dB = 0; dB >= -60; dB -= 10)
        {
            float yPos = dBToY ((float) dB);
            g.drawText (juce::String (dB),
                        0,
                        yPos - 6,
                        18,
                        12.0f,
                        juce::Justification::right,
                        false);
        }

        int msStep;
        if (xRangeInMs < 80)
            msStep = 5;
        else if (xRangeInMs < 200)
            msStep = 10;
        else
            msStep = 20;

        for (int timeInMs = 0; timeInMs <= xRangeInMs; timeInMs += msStep)
        {
            float xPos = timeToX ((float) timeInMs);
            g.drawText (juce::String (timeInMs),
                        xPos - 15.0f,
                        mT + plotHeight + 2.0f,
                        30,
                        12.0f,
                        juce::Justification::centred,
                        false);
        }

        const float xFactor = 1000.0f / 343.2f;

        if (radiusPtr != nullptr)
        {
            int numRef = juce::roundToInt (numReflPtr->load());

            float gainDb = juce::Decibels::gainToDecibels (gainPtr[0]);
            if (gainDb > -60.0f && gainDb <= 20.0f)
            {
                const float xPos = timeToX (zeroDelay ? 0.0f : radiusPtr[0] * xFactor);
                const float yPos = dBToY (gainDb);
                g.drawLine (xPos, yPos, xPos, mT + plotHeight, 2.0f);
            }

            g.setColour (juce::Colours::white.withMultipliedAlpha (0.5f));

            for (int i = 1; i <= numRef; ++i)
            {
                float gainDb = juce::Decibels::gainToDecibels (gainPtr[i]);
                if (gainDb > -60.0f && gainDb < 20.0f)
                {
                    const float radius = radiusPtr[i] - (zeroDelay ? radiusPtr[0] : 0.0f);
                    const float xPos = timeToX (radius * xFactor);
                    const float yPos = dBToY (gainDb);
                    g.drawLine (xPos, yPos, xPos, mT + plotHeight, 1.5f);
                }
            }
        }
    }

    inline float timeToX (float timeInMs) { return mL + timeInMs / xRangeInMs * plotWidth; }

    inline float dBToY (float gainInDB)
    {
        const float dynRange = -1.0f / 60.0f;
        return mT + dynRange * gainInDB * plotHeight;
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const double delta =
            100
            * (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY);
        //bool positiveDelta = delta >= 0.0;

        xRangeInMs += juce::roundToInt (delta);
        xRangeInMs = juce::jmin (xRangeInMs, 550);
        xRangeInMs = juce::jmax (xRangeInMs, 40);
    }
    void setDataPointers (float* Gain, float* Radius, std::atomic<float>* NumRefl)
    {
        gainPtr = Gain;
        numReflPtr = NumRefl;
        radiusPtr = Radius;
    }

    void setZeroDelay (const bool shouldBeZeroDelay)
    {
        if (zeroDelay != shouldBeZeroDelay)
        {
            zeroDelay = shouldBeZeroDelay;
            repaint();
        }
    }

    void resized() override
    {
        plotWidth = getLocalBounds().getWidth() - mL - mR;
        plotHeight = getLocalBounds().getHeight() - mT - mB;

        axes.clear();
        //horizontal
        axes.startNewSubPath (timeToX (0.0f) - 2, dBToY (-60.0f));
        axes.lineTo (timeToX (xRangeInMs), dBToY (-60.0f));

        //vertical
        axes.startNewSubPath (timeToX (0.0f), dBToY (-60.0f) + 2.0f);
        axes.lineTo (timeToX (0.0f), mT - 2.0f);

        dBGrid.clear();
        for (int dB = 0; dB >= -50; dB -= 10)
        {
            float yPos = dBToY ((float) dB);
            dBGrid.startNewSubPath (mL - 2.0f, yPos);
            dBGrid.lineTo (mL + plotWidth, yPos);
        }
    }
    void timerCallback() override { repaint(); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReflectionsVisualizer)
    juce::Path axes;
    juce::Path dBGrid;
    float plotWidth = 1.0f;
    float plotHeight = 1.0f;
    int xRangeInMs = 100;
    std::atomic<float>* numReflPtr = nullptr;
    float* gainPtr = nullptr;
    float* radiusPtr = nullptr;

    bool zeroDelay = false;
};
