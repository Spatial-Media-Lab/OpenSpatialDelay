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

#include "../Compressor.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class CompressorVisualizer : public juce::Component
{
    class GridAndLabels : public juce::Component
    {
    public:
        GridAndLabels (float minDB) : minDecibels (minDB)
        {
            setBufferedToImage (true);
            createGrid();
        }

        ~GridAndLabels() {}

        void createGrid()
        {
            const float step = 10.0f;

            grid.clear();

            for (int val = -step; val > minDecibels; val -= step)
            {
                // add horizontal line
                grid.startNewSubPath (minDecibels, val);
                grid.lineTo (0.0f, val);

                // add vertical line
                grid.startNewSubPath (val, minDecibels);
                grid.lineTo (val, 0.0f);
            }
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
            g.strokePath (grid, juce::PathStrokeType (0.5f), contentTransform);

            g.setColour (juce::Colours::white.withMultipliedAlpha (0.5f));
            g.drawRect (contentBounds, 1.0f);

            juce::Line<float> unity (minDecibels + 0.5f, minDecibels + 0.5f, -0.5f, -0.5f);
            unity.applyTransform (contentTransform);
            float dashLengths[2];
            dashLengths[0] = 2.0f;
            dashLengths[1] = 2.0f;
            g.drawDashedLine (unity, dashLengths, 2, 0.5f);

            g.setColour (juce::Colours::white);
            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
            g.setFont (12.0f);

            const float step = 10.0f;
            float xPos = 0.0f;
            float yPos = 0.0f;
            contentTransform.transformPoint (xPos, yPos);

            g.drawText ("0 dB", xPos + 1, yPos - 12, 18, 12.0f, juce::Justification::left, false);

            for (int val = -step; val >= minDecibels; val -= step)
            {
                // vertical labels
                float xPos = 0.0f;
                float yPos = val;
                contentTransform.transformPoint (xPos, yPos);
                g.drawText (juce::String (val),
                            xPos + 4,
                            yPos - 6,
                            18,
                            12.0f,
                            juce::Justification::left,
                            false);

                // horizontal labels
                xPos = val;
                yPos = 0.0f;
                contentTransform.transformPoint (xPos, yPos);
                g.drawText (juce::String (val),
                            xPos - 9,
                            yPos - 12,
                            18,
                            12.0f,
                            juce::Justification::centred,
                            false);
            }
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            bounds.removeFromTop (12);
            bounds.removeFromRight (22);
            bounds.removeFromLeft (10);
            bounds.removeFromBottom (8);
            contentBounds = bounds;

            contentTransform = juce::AffineTransform::fromTargetPoints (
                juce::Point<int> (minDecibels, minDecibels),
                contentBounds.getBottomLeft(),
                juce::Point<int> (0, 0),
                contentBounds.getTopRight(),
                juce::Point<int> (0, minDecibels),
                contentBounds.getBottomRight());
        }

        juce::Rectangle<int> getBoundsForContent() { return contentBounds; }

        juce::AffineTransform getTransformForContent() { return contentTransform; }

    private:
        const float minDecibels;
        juce::Path grid;

        juce::Rectangle<int> contentBounds { 0, 0, 1, 1 };
        juce::AffineTransform contentTransform;
    };

    class Characteristic : public juce::Component
    {
    public:
        Characteristic (iem::Compressor* compressorToGetCharacteristicFrom, float minDB) :
            compressor (compressorToGetCharacteristicFrom), minDecibels (minDB)
        {
            setBufferedToImage (true);
        }

        ~Characteristic() {}

        void updateCharacteristic()
        {
            const float threshold = compressor->getTreshold();
            const float knee = compressor->getKnee();

            const float kneeStart = threshold - knee / 2.0f;
            const float kneeEnd = threshold + knee / 2.0f;

            characteristic.clear();
            characteristic.startNewSubPath (minDecibels - 1.0f, minDecibels - 1.0f);
            characteristic.lineTo (minDecibels, compressor->getCharacteristicSample (minDecibels));

            characteristic.lineTo (kneeStart, compressor->getCharacteristicSample (kneeStart));

            const int kneeSamples = juce::jmax (1, static_cast<int> (knee));
            float val = kneeStart;
            float step = knee / kneeSamples;
            for (int i = 0; i < kneeSamples; ++i)
            {
                val += step;
                characteristic.lineTo (val, compressor->getCharacteristicSample (val));
            }
            characteristic.lineTo (kneeEnd, compressor->getCharacteristicSample (kneeEnd));
            characteristic.lineTo (0.0f, compressor->getCharacteristicSample (0.0f));
            characteristic.lineTo (1.0f, compressor->getCharacteristicSample (0.0f));
            characteristic.lineTo (1.0f, minDecibels - 1.0f);
            characteristic.closeSubPath();

            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colours::white);
            g.strokePath (characteristic, juce::PathStrokeType (2.0f), transform);

            g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.3f));
            g.fillPath (characteristic, transform);
        }

        void resized() override {}

        void setTransformForContent (juce::AffineTransform newTransform)
        {
            transform = newTransform;
        }

        void setCompressorToVisualize (iem::Compressor* compressorToVisualize)
        {
            compressor = compressorToVisualize;
        }

    private:
        iem::Compressor* compressor;
        const float minDecibels;

        juce::Path characteristic;

        juce::AffineTransform transform;
    };

public:
    CompressorVisualizer (iem::Compressor* compressorToVisualize) :
        compressor (compressorToVisualize),
        minDecibels (-60.0f),
        gridAndLabels (minDecibels),
        characteristic (compressor, minDecibels)
    {
        init();
    }

    CompressorVisualizer (iem::Compressor& compressorToVisualize) :
        compressor (&compressorToVisualize),
        minDecibels (-60.0f),
        gridAndLabels (minDecibels),
        characteristic (compressor, minDecibels)
    {
        init();
    }

    CompressorVisualizer (iem::Compressor* compressorToVisualize, const float rangeInDecibels) :
        compressor (compressorToVisualize),
        minDecibels (-1.0f * rangeInDecibels),
        gridAndLabels (minDecibels),
        characteristic (compressor, minDecibels)
    {
        init();
    }

    CompressorVisualizer (iem::Compressor& compressorToVisualize, const float rangeInDecibels) :
        compressor (&compressorToVisualize),
        minDecibels (-1.0f * rangeInDecibels),
        gridAndLabels (minDecibels),
        characteristic (compressor, minDecibels)
    {
        init();
    }

    ~CompressorVisualizer() {}

    void init()
    {
        addAndMakeVisible (gridAndLabels);
        addAndMakeVisible (characteristic);
        updateCharacteristic();
    }

    void updateCharacteristic() { characteristic.updateCharacteristic(); }

    void setMarkerLevels (const float inputLevel, const float gainReduction)
    {
        const float makeUpGain = compressor->getMakeUpGain();
        const auto tempOutLevel = inputLevel + gainReduction + makeUpGain;

        if (inLevel != inputLevel || outLevel != tempOutLevel)
        {
            inLevel = inputLevel;
            outLevel = tempOutLevel;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override {}

    void paintOverChildren (juce::Graphics& g) override
    {
        if (inLevel < minDecibels || outLevel < minDecibels)
            return;

        juce::Rectangle<float> circle (0.0f, 0.0f, 10.0f, 10.0f);

        float x = inLevel;
        float y = outLevel;
        transform.transformPoint (x, y);
        circle.setCentre (x, y);

        g.setColour (juce::Colours::cornflowerblue);
        g.fillRoundedRectangle (circle, 5.0f);
    }

    void resized() override
    {
        gridAndLabels.setBounds (getLocalBounds());
        const juce::Rectangle<int> contentBounds = gridAndLabels.getBoundsForContent();
        transform = gridAndLabels.getTransformForContent();

        characteristic.setTransformForContent (
            transform.translated (-contentBounds.getX(), -contentBounds.getY()));
        characteristic.setBounds (contentBounds);
    }

    void setCompressorToVisualize (iem::Compressor* compressorToVisualize)
    {
        compressor = compressorToVisualize;
        characteristic.setCompressorToVisualize (compressor);
    }

private:
    iem::Compressor* compressor;
    const float minDecibels;
    GridAndLabels gridAndLabels;
    Characteristic characteristic;
    juce::AffineTransform transform;

    float inLevel = 0.0f;
    float outLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorVisualizer)
};
