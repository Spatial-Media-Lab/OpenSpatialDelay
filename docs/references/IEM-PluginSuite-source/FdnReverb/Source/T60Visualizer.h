/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich, Sebastian Grill
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

class T60Visualizer : public juce::Component
{
    struct Settings
    {
        float fMin = 20.0f; // minimum displayed frequency
        float fMax = 20000.0f; // maximum displayed frequency
        float yMin = 0.1f; // min displayed seconds
        float yMax = 60.0f; // max displayed seconds
        float gridDiv = 5.0f; // how many seconds per divisions (between grid lines)
    };

    const float mL = 23.0f;
    const float mR = 10.0f;
    const float mT = 7.0f;
    const float mB = 15.0f;
    const float OH = 3.0f;

public:
    T60Visualizer() : juce::Component(), overallGainInDb (0.0f), sampleRate (48000) {};
    T60Visualizer (float fMin, float fMax, float yMin, float yMax, float gridDiv) :
        juce::Component(),
        overallGainInDb (0.0f),
        sampleRate (48000),
        s { fMin, fMax, yMin, yMax, gridDiv } {};
    ~T60Visualizer() {};

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.01f));
        g.fillAll();

        //int width = getWidth();

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (12.0f);

        // time labels
        g.setColour (juce::Colours::white);

        juce::String axislabel = juce::String (0.1f);
        g.drawText (axislabel, 0, t60ToY (0.1f) - 6, 18, 12.0f, juce::Justification::right, false);

        axislabel = juce::String (1.f);
        g.drawText (axislabel, 0, t60ToY (1.f) - 6, 18, 12.0f, juce::Justification::right, false);

        for (float t = s.yMin; t <= s.yMax; t += powf (10, floorf (log10 (t))))
        {
            int ypos = t60ToY (t);

            juce::String axislabel;
            bool drawText = false;

            if ((t == 1) || (t == 10) || (t == 60))
            {
                axislabel = juce::String ((int) t);
                drawText = true;
            }
            if (drawText)
            {
                g.drawText (axislabel, 0, ypos - 6, 18, 12.0f, juce::Justification::right, false);
            }
        }

        // frequency labels
        for (float f = s.fMin; f <= s.fMax; f += powf (10, floorf (log10 (f))))
        {
            int xpos = hzToX (f);

            juce::String axislabel;
            bool drawText = false;

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500))
            {
                axislabel = juce::String ((int) f);
                drawText = true;
            }
            else if ((f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                axislabel = juce::String ((int) f / 1000);
                axislabel << "k";
                drawText = true;
            }

            if (drawText)
            {
                g.drawText (axislabel,
                            xpos - 10,
                            t60ToY (s.yMin) + OH + 0.0f,
                            20,
                            12,
                            juce::Justification::centred,
                            false);
            }
        }

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (dbGridPath, juce::PathStrokeType (0.5f));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.9f));
        g.strokePath (dbGridPathBold, juce::PathStrokeType (1.0f));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.9f));
        g.strokePath (hzGridPathBold, juce::PathStrokeType (1.0f));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (hzGridPath, juce::PathStrokeType (0.5f));

        // draw filter magnitude responses
        juce::Path magnitude;
        allMagnitudesInDb.fill (overallGainInDb);

        int xMin = hzToX (s.fMin);
        int xMax = hzToX (s.fMax);
        int yMax = t60ToY (s.yMin);
        int yMin = t60ToY (s.yMax);

        g.excludeClipRegion (
            juce::Rectangle<int> (0.0f, yMax + OH, getWidth(), getHeight() - yMax - OH));

        for (int i = arrayOfCoefficients.size(); --i >= 0;)
        {
            //bool isActive = activeElem == i;

            juce::dsp::IIR::Coefficients<float>::Ptr handle =
                (juce::dsp::IIR::Coefficients<float>::Ptr) arrayOfCoefficients.getUnchecked (i);
            magnitude.clear();

            float db = juce::Decibels::gainToDecibels (
                handle->getMagnitudeForFrequency (xToHz (xMin), sampleRate));
            allMagnitudesInDb.setUnchecked (0, allMagnitudesInDb[0] + db);

            for (int x = xMin + 1; x <= xMax; ++x)
            {
                float db = juce::Decibels::gainToDecibels (
                    handle->getMagnitudeForFrequency (xToHz (x), sampleRate));
                allMagnitudesInDb.setUnchecked (x - xMin, allMagnitudesInDb[x - xMin] + db);
            }
        }

        //all magnitudes combined
        magnitude.clear();

        magnitude.startNewSubPath (
            xMin,
            juce::jlimit ((float) yMin,
                          (float) yMax + OH + 1,
                          t60ToYFloat (gainToT60Float (allMagnitudesInDb[0]))));

        for (int x = xMin + 1; x <= xMax; ++x)
        {
            magnitude.lineTo (
                x,
                juce::jlimit ((float) yMin,
                              (float) yMax + OH + 1,
                              t60ToYFloat (gainToT60Float (allMagnitudesInDb[x - xMin]))));
        }
        g.setColour (juce::Colours::white);
        g.strokePath (magnitude, juce::PathStrokeType (1.5f));

        g.setColour (juce::Colours::white.withMultipliedAlpha (0.3f));
        g.fillPath (tolerancePath,
                    juce::AffineTransform::translation (0.0f,
                                                        t60ToY (gainToT60Float (overallGainInDb))
                                                            - t60ToY (10.0f)));
    };

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const double delta =
            100
            * (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY);
        //bool positiveDelta = delta >= 0.0;

        float value = s.yMax + juce::roundToInt (delta);
        value = juce::jmin (value, 80.0f);
        value = juce::jmax (value, 5.0f);

        s.yMax = value;
        resized();
        repaint();
    }

    void createTolerancePath()
    {
        const float tRef = 10; //10 is our reference, can be anything
        tolerancePath.clear();

        tolerancePath.startNewSubPath (hzToX (s.fMin),
                                       t60ToYFloat (getToleranceT60 (s.fMin, tRef, true)));
        tolerancePath.lineTo (hzToX (250.0f), t60ToYFloat (1.2f * tRef));
        tolerancePath.lineTo (hzToX (s.fMax), t60ToYFloat (1.2f * tRef));
        tolerancePath.lineTo (hzToX (s.fMax), t60ToYFloat (getToleranceT60 (s.fMax, tRef, false)));
        tolerancePath.lineTo (hzToX (2000.0f), t60ToYFloat (0.8 * tRef));
        tolerancePath.lineTo (hzToX (250.0f), t60ToYFloat (0.8 * tRef));
        tolerancePath.lineTo (hzToX (s.fMin), t60ToYFloat (getToleranceT60 (s.fMin, tRef, false)));
        tolerancePath.closeSubPath();
    }

    float getToleranceT60 (float frequency, float tRef, bool upper)
    {
        if (frequency < 250)
        {
            if (upper)
                return tRef * (frequency * -0.002673797f + 1.868449198f);
            else
                return tRef * (frequency * -0.002139037f + 1.334759358f);
        }
        else if (frequency < 2000)
        {
            if (upper)
                return tRef * 1.2f;
            else
                return tRef * 0.8f;
        }
        else
        {
            if (upper)
                return tRef * 1.2f;
            else
                return tRef * juce::jlimit (0.1f, 1.f, frequency * -0.00005f + 0.9f);
        }
    }

    float gainToT60Float (float db) { return -3.f * 20.f / db; }

    float t60ToGain (float T60) { return -3.f * 20.f / T60; }

    int t60ToY (float db)
    {
        int ypos = t60ToYFloat (db);
        return ypos;
    }

    float t60ToYFloat (float db)
    {
        float height = (float) getHeight() - mB - mT;
        float ypos = mT + height * (1.f - (log (db / s.yMin) / log (s.yMax / s.yMin)));
        return ypos;
    }

    float yToT60 (float y)
    {
        float height = (float) getHeight() - mB - mT;
        float db = s.yMin * powf (s.yMax / s.yMin, 1.f - (y - mT) / height);
        return db;
    }

    int hzToX (float hz)
    {
        float width = (float) getWidth() - mL - mR;
        int xpos = mL + width * (log (hz / s.fMin) / log (s.fMax / s.fMin));
        return xpos;
    }

    float xToHz (int x)
    {
        float width = (float) getWidth() - mL - mR;
        return s.fMin * powf ((s.fMax / s.fMin), ((x - mL) / width));
    }

    void setSampleRate (int newSampleRate) { sampleRate = newSampleRate; }

    void setOverallGain (float newGain)
    {
        overallGainInDb = juce::Decibels::gainToDecibels (newGain, -500.0f);
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition();
        float frequency = xToHz (pos.x);
        float gain = yToT60 (pos.y);

        if (activeElem != -1)
        {
            juce::Slider* slHandle;

            slHandle = arrayOfFrequencySliders[activeElem];
            if (slHandle != nullptr)
                slHandle->setValue (frequency);

            slHandle = arrayOfGainSliders[activeElem];
            if (slHandle != nullptr)
                slHandle->setValue (gain);
        }
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition();
        int oldActiveElem = activeElem;
        activeElem = -1;
        for (int i = arrayOfCoefficients.size(); --i >= 0;)
        {
            juce::Point<int> filterPos (arrayOfFrequencySliders[i] == nullptr
                                            ? hzToX (0.0f)
                                            : hzToX (arrayOfFrequencySliders[i]->getValue()),
                                        arrayOfGainSliders[i] == nullptr
                                            ? t60ToY (0.0f)
                                            : t60ToY (arrayOfGainSliders[i]->getValue()));
            if (pos.getDistanceSquaredFrom (filterPos) < 45)
            {
                activeElem = i;
                break;
            }
        }

        if (oldActiveElem != activeElem)
            repaint();
    }

    void resized() override
    {
        int xMin = hzToX (s.fMin);
        int xMax = hzToX (s.fMax);
        allMagnitudesInDb.resize (xMax - xMin + 1);

        const float width = getWidth() - mL - mR;
        dbGridPath.clear();
        dbGridPathBold.clear();

        dbGridPathBold.startNewSubPath (mL - OH, t60ToY (0.1f));
        dbGridPathBold.lineTo (mL + width + OH, t60ToY (0.1f));

        dbGridPathBold.startNewSubPath (mL - OH, t60ToY (1.f));
        dbGridPathBold.lineTo (mL + width + OH, t60ToY (1.f));

        for (float t = s.yMin; t <= s.yMax; t += powf (10, floorf (log10 (t))))
        {
            int ypos = t60ToY (t);

            if ((t == 10) || (t == 60))
            {
                dbGridPathBold.startNewSubPath (mL - OH, ypos);
                dbGridPathBold.lineTo (mL + width + OH, ypos);
            }
            else
            {
                dbGridPath.startNewSubPath (mL - OH, ypos);
                dbGridPath.lineTo (mL + width + OH, ypos);
            }
        }

        hzGridPath.clear();
        hzGridPathBold.clear();

        for (float f = s.fMin; f <= s.fMax; f += powf (10, floorf (log10 (f))))
        {
            int xpos = hzToX (f);

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500) || (f == 1000) || (f == 5000)
                || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath (xpos, t60ToY (s.yMax) - OH);
                hzGridPathBold.lineTo (xpos, t60ToY (s.yMin) + OH);
            }
            else
            {
                hzGridPath.startNewSubPath (xpos, t60ToY (s.yMax) - OH);
                hzGridPath.lineTo (xpos, t60ToY (s.yMin) + OH);
            }
        }

        createTolerancePath();
    }

    void addCoefficients (juce::dsp::IIR::Coefficients<float>::Ptr newCoeffs,
                          juce::Colour newColourForCoeffs,
                          juce::Slider* frequencySlider = nullptr,
                          juce::Slider* gainSlider = nullptr)
    {
        arrayOfCoefficients.add (newCoeffs);
        arrayOfColours.add (newColourForCoeffs);
        arrayOfGainSliders.add (gainSlider);
        arrayOfFrequencySliders.add (frequencySlider);
    }

private:
    float overallGainInDb;

    int sampleRate;

    int activeElem = 0;

    Settings s;
    juce::Path dbGridPath;
    juce::Path dbGridPathBold;
    juce::Path hzGridPath;
    juce::Path hzGridPathBold;
    juce::Path tolerancePath;

    juce::Array<float> allMagnitudesInDb;
    juce::Array<juce::dsp::IIR::Coefficients<float>::Ptr> arrayOfCoefficients;
    juce::Array<juce::Slider*> arrayOfGainSliders, arrayOfFrequencySliders;

    juce::Array<juce::Colour> arrayOfColours;
};
