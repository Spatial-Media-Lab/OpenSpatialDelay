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

#include <JuceHeader.h>

//==============================================================================
/*
*/

class DirectivityVisualizer : public juce::Component
{
    struct WeightsAndColour
    {
        float* weights;
        juce::Colour colour;
    };

    const float deg2rad = juce::MathConstants<float>::pi / 180.0f;
    const int degStep = 1;
    const int nLookUpSamples = 360;
    const int maxdB = 90;
    const float power = 3.0f;
    const int dBstep = 10;
    //#define scale 4
    const float scale = std::sqrt (4 * juce::MathConstants<float>::pi) * decodeCorrection (7);

public:
    DirectivityVisualizer()
    {
        // 0th
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi) { return scale * (0.25f / (float) juce::MathConstants<float>::pi); },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 1st
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            { return scale * (0.75f / juce::MathConstants<float>::pi) * std::cos (phi); },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 2nd
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * 2.0f * (5.0f / 16.0f / juce::MathConstants<float>::pi)
                       * (3 * std::cos (phi) * std::cos (phi) - 1.0f);
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 3rd
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * (7.0f / juce::MathConstants<float>::pi) / 8.0f
                       * (5 * pow (std::cos (phi), 3) - 3.0f * std::cos (phi));
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 4th
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * 9.0f / 32.0f / juce::MathConstants<float>::pi
                       * (35 * pow (std::cos (phi), 4) - 30 * pow (std::cos (phi), 2) + 3.0f);
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 5th
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * 8.0f / 256.0f * 11.0f / juce::MathConstants<float>::pi
                       * (63 * pow (std::cos (phi), 5) - 70 * pow (std::cos (phi), 3)
                          + 15.0f * std::cos (phi));
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 6th
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * 16.0f / 1024.0f * 13.0f / juce::MathConstants<float>::pi
                       * (231 * pow (std::cos (phi), 6) - 315 * pow (std::cos (phi), 4)
                          + 105 * pow (std::cos (phi), 2) - 5.0f);
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        // 7th
        lookUpTables.add (new juce::dsp::LookupTableTransform<float> (
            [this] (float phi)
            {
                return scale * 16.0f / 1024.0f * 15.0f / juce::MathConstants<float>::pi
                       * (429 * pow (std::cos (phi), 7) - 693 * pow (std::cos (phi), 5)
                          + 315 * pow (std::cos (phi), 3) - 35 * std::cos (phi));
            },
            -juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi,
            nLookUpSamples));

        for (int phi = -180; phi <= 180; phi += degStep)
        {
            pointsOnCircle.add (juce::Point<float> (cos (deg2rad * phi), sin (deg2rad * phi)));
        }

        juce::Path circle;
        circle.addEllipse (-1.0f, -1.0f, 2.0f, 2.0f);
        juce::Path line;
        line.startNewSubPath (0.0f, -1.0f);
        line.lineTo (0.0f, 1.0f);

        grid.clear();
        for (int dB = 0; dB < maxdB; dB += dBstep)
            grid.addPath (circle, juce::AffineTransform().scaled (dBToRadius (-dB)));

        subGrid.clear();
        for (int dB = dBstep / 2; dB < maxdB; dB += dBstep)
            subGrid.addPath (circle, juce::AffineTransform().scaled (dBToRadius (-dB)));

        subGrid.addPath (line);
        subGrid.addPath (line,
                         juce::AffineTransform().rotation (0.25f * juce::MathConstants<float>::pi));
        subGrid.addPath (line,
                         juce::AffineTransform().rotation (0.5f * juce::MathConstants<float>::pi));
        subGrid.addPath (line,
                         juce::AffineTransform().rotation (0.75f * juce::MathConstants<float>::pi));
    }

    ~DirectivityVisualizer() {}

    float dBToRadius (float dB)
    {
        if (dB > 0.0f)
            dB = 0.0f;
        //float radius = 1.0f - pow(abs(dB) / maxdB, power);
        float radius = (exp (power * dB / maxdB) - exp (-power)) / (1.0f - exp (-power));
        if (radius < 0.0f)
            radius = 0.0f;
        return radius;
    }

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        const int scale = plotArea.getWidth() / 2;
        //const int height = bounds.getHeight();

        int centreX = bounds.getCentreX();
        int centreY = bounds.getCentreY();

        juce::Path path;
        path = grid;
        path.applyTransform (transform);
        g.setColour (juce::Colours::skyblue.withMultipliedAlpha (0.1f));
        g.fillPath (path);
        g.setColour (juce::Colours::white);
        g.strokePath (path, juce::PathStrokeType (1.0f));

        path = subGrid;
        path.applyTransform (transform);
        g.setColour (juce::Colours::skyblue.withMultipliedAlpha (0.3f));
        g.strokePath (path, juce::PathStrokeType (0.5f));

        g.setColour (juce::Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (12.0f);
        g.drawText ("0 dB",
                    centreX - 10,
                    centreY + scale * dBToRadius (0.0f) - 12,
                    20,
                    12,
                    juce::Justification::centred);
        g.drawText ("-10",
                    centreX - 10,
                    centreY + scale * dBToRadius (-10.0f),
                    20,
                    12,
                    juce::Justification::centred);
        g.drawText ("-20",
                    centreX - 10,
                    centreY + scale * dBToRadius (-20.0f),
                    20,
                    12,
                    juce::Justification::centred);

        int size = elements.size();
        for (int i = 0; i < size; ++i)
        {
            WeightsAndColour& handle (elements.getReference (i));
            g.setColour (handle.colour);

            path.clear();

            int idx = 0;
            for (int phi = -180; phi <= 180; phi += degStep)
            {
                float gain = 0.0f;
                float phiInRad = (float) phi * deg2rad;
                for (int o = 0; o < 8; ++o)
                {
                    gain += handle.weights[o] * lookUpTables[o]->processSample (phiInRad);
                }
                juce::Point<float> point =
                    dBToRadius (juce::Decibels::gainToDecibels (std::abs (gain), -1.0f * maxdB))
                    * pointsOnCircle[idx];
                if (phi == -180)
                    path.startNewSubPath (point);
                else
                    path.lineTo (point);
                ++idx;
            }

            path.closeSubPath();
            path.applyTransform (transform);
            g.strokePath (path, juce::PathStrokeType (2.0f));
        }
    }

    void resized() override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        juce::Point<int> centre = bounds.getCentre();

        bounds.reduce (10, 10);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth (bounds.getHeight());
        else
            bounds.setHeight (bounds.getWidth());
        bounds.setCentre (centre);

        transform = juce::AffineTransform::fromTargetPoints ((float) centre.x,
                                                             (float) centre.y,
                                                             (float) centre.x,
                                                             bounds.getY(),
                                                             bounds.getX(),
                                                             centre.y);

        plotArea = bounds;
    }

    void addElement (float* weights, juce::Colour colour) { elements.add ({ weights, colour }); }

private:
    juce::OwnedArray<juce::dsp::LookupTableTransform<float>> lookUpTables;
    juce::Path grid;
    juce::Path subGrid;
    juce::AffineTransform transform;
    juce::Rectangle<int> plotArea;

    int maxOrder;

    juce::Array<WeightsAndColour> elements;

    juce::Array<juce::Point<float>> pointsOnCircle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityVisualizer)
};
