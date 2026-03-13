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

#include "../../resources/customComponents/SpherePanner.h"

/**
 This is kind of hacky, I know!
 */
class EnergySpherePanner : public SpherePanner
{
public:
    EnergySpherePanner (const std::vector<float>& rmsArray) : rms (rmsArray) {}

    void paintOverChildren (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1)));

        const int size = elements.size();
        for (int i = 0; i < size; ++i)
        {
            SpherePanner::Element* handle = elements.getUnchecked (i);

            auto pos = handle->getCoordinates();
            const bool isUp = pos.z >= -0.0f;

            const float diam = 15.0f + 4.0f * pos.z;
            const juce::Colour colour =
                handle->isActive() ? handle->getColour() : juce::Colours::grey;
            g.setColour (colour);

            if (linearElevation)
            {
                const float r = sqrt (pos.y * pos.y + pos.x * pos.x);
                const float factor = std::asin (r) / r / juce::MathConstants<float>::halfPi;
                pos *= factor;
            }

            const juce::Rectangle<float> circleArea (centreX - pos.y * radius - diam / 2,
                                                     centreY - pos.x * radius - diam / 2,
                                                     diam,
                                                     diam);
            juce::Path panPos;

            panPos.addEllipse (circleArea);
            g.strokePath (panPos, juce::PathStrokeType (1.0f));

            if (visualize && i != 0) // assume and hope, the first element is the master control
            {
                const float level = juce::Decibels::gainToDecibels (rms[i - 1]);
                const float alpha =
                    juce::jlimit (0.0f, 1.0f, (level - peakLevel) / dynRange + 1.0f);
                g.setColour (colour.withAlpha (alpha));
                g.drawEllipse (circleArea.withSizeKeepingCentre (1.4f * diam, 1.4f * diam), 1.4f);
            }

            if (i == activeElem)
            {
                g.setColour (colour.withAlpha (0.8f));
                g.drawEllipse (circleArea.withSizeKeepingCentre (1.3f * diam, 1.3f * diam), 0.9f);
            }

            g.setColour (colour.withAlpha (isUp ? 1.0f : 0.3f));
            g.fillPath (panPos);
            g.setColour (isUp ? handle->getTextColour() : colour);

            g.setFont (isUp ? 15.0f : 10.0f);
            g.drawText (handle->getLabel(),
                        circleArea.toNearestInt(),
                        juce::Justification::centred,
                        false);
        }
    };

    void setPeakLevel (float peakLevelInDecibels) { peakLevel = peakLevelInDecibels; }

    void setDynamicRange (float dynamicRangeInDecibels) { dynRange = dynamicRangeInDecibels; }

    void visualizeRMS (float shouldVisualize)
    {
        visualize = shouldVisualize;
        repaint();
    }

private:
    bool visualize = false;
    float peakLevel = 0.0f;
    float dynRange = 60.0f;
    const std::vector<float>& rms;
};
