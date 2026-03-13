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

#include "../../resources/heatmap.h"
#include "../../resources/viridis_cropped.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class VisualizerColormap : public juce::Component
{
public:
    VisualizerColormap()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
    }

    ~VisualizerColormap() {}

    void paint (juce::Graphics& g) override
    {
        juce::Colour colormapData[256];
        if (usePerceptualColormap)
            for (int i = 0; i < 256; ++i)
            {
                const float alpha = juce::jlimit (0.0f, 1.0f, (float) i / 50.0f);
                colormapData[i] = juce::Colour::fromFloatRGBA (viridis_cropped[i][0],
                                                               viridis_cropped[i][1],
                                                               viridis_cropped[i][2],
                                                               alpha);
            }
        else
            for (int i = 0; i < 256; ++i)
            {
                colormapData[i] = juce::Colour::fromFloatRGBA (heatmap[i][0],
                                                               heatmap[i][1],
                                                               heatmap[i][2],
                                                               heatmap[i][3]);
            }

        juce::Rectangle<int> colormapArea (getLocalBounds());
        colormapArea.removeFromTop (12);
        colormapArea.removeFromBottom (6);

        colormapArea.removeFromRight (25);
        juce::ColourGradient gradient;
        gradient.point1 = colormapArea.getTopLeft().toFloat();
        gradient.point2 = colormapArea.getBottomLeft().toFloat();

        for (int i = 0; i < 256; ++i)
            gradient.addColour (1.0f - i * 1.0f / 256, colormapData[i]);

        juce::Path path;
        path.addRectangle (colormapArea);
        g.setGradientFill (gradient);
        g.fillPath (path);

        g.setColour (juce::Colours::white);
        int width = colormapArea.getWidth();

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1)));
        g.drawText ("dB", 25, 0, width, 12, juce::Justification::centred);

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
        g.setFont (12.0f);

        const float yStep = (float) colormapArea.getHeight() / 7;
        g.drawText (juce::String (maxLevel, 1), 25, 12, width, 12, juce::Justification::centred);
        for (int i = 1; i < 8; ++i)
        {
            g.drawText (juce::String (maxLevel - range / 7.0 * i, 1),
                        25,
                        6 + yStep * i,
                        width,
                        12,
                        juce::Justification::centred);
        }
    }

    void setMaxLevel (const float newMaxLevel)
    {
        maxLevel = newMaxLevel;
        repaint();
    }

    void setRange (const float newRange)
    {
        range = newRange;
        repaint();
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        usePerceptualColormap = ! usePerceptualColormap;
        repaint();
    }

    void setColormap (bool shouldUsePerceptualColormap)
    {
        if (usePerceptualColormap != shouldUsePerceptualColormap)
        {
            usePerceptualColormap = shouldUsePerceptualColormap;
            repaint();
        }
    }

    bool getColormap() { return usePerceptualColormap; }

    void resized() override {}

private:
    bool usePerceptualColormap = true;
    float maxLevel = 0.0f;
    float range = 35.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizerColormap)
};
