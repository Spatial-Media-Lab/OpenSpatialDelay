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
#include <complex>
template <typename coefficientsType>
class FilterVisualizer : public juce::Component
{
    struct Settings
    {
        float fMin = 20.0f; // minimum displayed frequency
        float fMax = 20000.0f; // maximum displayed frequency
        float dbMin = -15.0f; // min displayed dB
        float dbMax = 15.0f; // max displayed dB
        float gridDiv = 5.0f; // how many dB per divisions (between grid lines)
        bool gainHandleLin = false; // are the filter gain sliders linear?
    };

    template <typename T>
    struct FilterWithSlidersAndColour
    {
        typename juce::dsp::IIR::Coefficients<T>::Ptr coefficients;
        juce::Colour colour;
        juce::Slider* frequencySlider = nullptr;
        juce::Slider* gainSlider = nullptr;
        juce::Slider* qSlider = nullptr;
        float* overrideGain = nullptr;
        bool enabled = true;
    };

    const float mL = 23.0f;
    const float mR = 10.0f;
    const float mT = 7.0f;
    const float mB = 15.0f;
    const float OH = 3.0f;

public:
    FilterVisualizer() : overallGainInDb (0.0f), sampleRate (48000.0) { init(); }

    FilterVisualizer (float fMin,
                      float fMax,
                      float dbMin,
                      float dbMax,
                      float gridDiv,
                      bool gainHandleLin = false) :
        overallGainInDb (0.0f), sampleRate (48000.0), s { fMin,  fMax,    dbMin,
                                                          dbMax, gridDiv, gainHandleLin }
    {
        init();
    }

    ~FilterVisualizer() override {}

    void init()
    {
        dynamic = s.dbMax - s.dbMin;
        zero = 2.0f * s.dbMax / dynamic;
        scale = 1.0f / (zero + std::tanh (s.dbMin / dynamic * -2.0f));
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.01f));
        g.fillAll();

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (12.0f);

        // db labels
        float dyn = s.dbMax - s.dbMin;
        int numgridlines = juce::roundToInt (dyn / s.gridDiv + 1);

        //g.setFont(juce::Font(getLookAndFeel().getTypefaceForFont (juce::Font(10.0f, 1)));
        g.setColour (juce::Colours::white);
        int lastTextDrawPos = -1;
        for (int i = 0; i < numgridlines; ++i)
        {
            const auto db_val = s.dbMax - i * s.gridDiv;
            lastTextDrawPos = drawLevelMark (g,
                                             0,
                                             20,
                                             juce::roundToInt (db_val),
                                             juce::String (db_val, 0),
                                             lastTextDrawPos);
        }

        // frequency labels
        for (float f = s.fMin; f <= s.fMax; f += pow (10.0f, floor (log10 (f))))
        {
            int xpos = hzToX (f);

            juce::String axislabel;
            bool drawText = false;

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500))
            {
                axislabel = juce::String (f, 0);
                drawText = true;
            }
            else if ((f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                axislabel = juce::String (f / 1000, 0);
                axislabel << "k";
                drawText = true;
            }
            else
                continue;

            g.drawText (axislabel,
                        xpos - 10,
                        dbToY (s.dbMin) + OH + 0.0f,
                        20,
                        12,
                        juce::Justification::centred,
                        false);
        }

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (dbGridPath, juce::PathStrokeType (0.5f));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.9f));
        g.strokePath (hzGridPathBold, juce::PathStrokeType (1.0f));

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (hzGridPath, juce::PathStrokeType (0.5f));

        // draw filter magnitude responses
        juce::Path magnitude;
        allMagnitudesInDb.fill (overallGainInDb);

        const int xMin = hzToX (s.fMin);
        const int xMax = hzToX (s.fMax);
        const int yMin = juce::jmax (dbToY (s.dbMax), 0);
        const int yMax = juce::jmax (dbToY (s.dbMin), yMin);

        const int yZero = filtersAreParallel ? yMax + 10 : dbToY (0.0f);

        g.excludeClipRegion (
            juce::Rectangle<int> (0.0f, yMax + OH, getWidth(), getHeight() - yMax - OH));

        if (filtersAreParallel)
            complexMagnitudes.fill (std::complex<double>());

        for (int b = elements.size(); --b >= 0;)
        {
            bool isActive = activeElem == b;
            magnitude.clear();

            auto handle (elements[b]);
            const bool isEnabled = handle->enabled;
            typename juce::dsp::IIR::Coefficients<coefficientsType>::Ptr coeffs =
                handle->coefficients;
            //calculate magnitude response
            if (coeffs != nullptr)
                coeffs->getMagnitudeForFrequencyArray (frequencies.getRawDataPointer(),
                                                       magnitudes.getRawDataPointer(),
                                                       numPixels,
                                                       sampleRate);
            float additiveDB = 0.0f;
            if (filtersAreParallel && handle->gainSlider != nullptr)
                additiveDB = handle->gainSlider->getValue();

            //calculate phase response if needed
            if (filtersAreParallel && coeffs != nullptr)
                coeffs->getPhaseForFrequencyArray (frequencies.getRawDataPointer(),
                                                   phases.getRawDataPointer(),
                                                   numPixels,
                                                   sampleRate);

            float db = juce::Decibels::gainToDecibels (magnitudes[0]) + additiveDB;
            magnitude.startNewSubPath (xMin,
                                       juce::jlimit (static_cast<float> (yMin),
                                                     static_cast<float> (yMax) + OH + 1.0f,
                                                     dbToYFloat (db)));

            for (int i = 1; i < numPixels; ++i)
            {
                const auto decibels = juce::Decibels::gainToDecibels (magnitudes[i]) + additiveDB;
                float y = juce::jlimit (static_cast<float> (yMin),
                                        static_cast<float> (yMax) + OH + 1.0f,
                                        dbToYFloat (decibels));
                float x = xMin + i;
                magnitude.lineTo (x, y);
            }

            g.setColour (handle->colour.withMultipliedAlpha (isEnabled ? 0.7f : 0.1f));
            g.strokePath (magnitude, juce::PathStrokeType (isActive ? 2.5f : 1.0f));

            magnitude.lineTo (xMax, yZero);
            magnitude.lineTo (xMin, yZero);
            magnitude.closeSubPath();

            g.setColour (handle->colour.withMultipliedAlpha (isEnabled ? 0.3f : 0.1f));
            g.fillPath (magnitude);

            float multGain = (handle->overrideGain != nullptr)
                                 ? *handle->overrideGain
                                 : juce::Decibels::decibelsToGain (additiveDB);

            //overall magnitude update
            if (isEnabled)
            {
                if (filtersAreParallel)
                {
                    for (int i = 0; i < numPixels; ++i)
                    {
                        complexMagnitudes.setUnchecked (
                            i,
                            complexMagnitudes[i]
                                + std::polar (magnitudes[i] * multGain, phases[i])); //*addGain
                    }
                }
                else
                {
                    for (int i = 0; i < numPixels; ++i)
                    {
                        const float dB = juce::Decibels::gainToDecibels (magnitudes[i] * multGain);
                        allMagnitudesInDb.setUnchecked (i, allMagnitudesInDb[i] + dB);
                    }
                }
            }
        }

        if (filtersAreParallel)
            for (int i = 0; i < numPixels; ++i)
            {
                const float dB = juce::Decibels::gainToDecibels (std::abs (complexMagnitudes[i]));
                allMagnitudesInDb.setUnchecked (i, allMagnitudesInDb[i] + dB);
            }

        //all magnitudes combined
        magnitude.clear();

        magnitude.startNewSubPath (xMin,
                                   juce::jlimit (static_cast<float> (yMin),
                                                 static_cast<float> (yMax) + OH + 1.0f,
                                                 dbToYFloat (allMagnitudesInDb[0])));

        for (int x = xMin + 1; x <= xMax; ++x)
        {
            magnitude.lineTo (x,
                              juce::jlimit (static_cast<float> (yMin),
                                            static_cast<float> (yMax) + OH + 1.0f,
                                            dbToYFloat (allMagnitudesInDb[x - xMin])));
        }

        g.setColour (juce::Colours::white);
        g.strokePath (magnitude, juce::PathStrokeType (1.5f));

        magnitude.lineTo (xMax, yZero);
        magnitude.lineTo (xMin, yZero);
        magnitude.closeSubPath();
        g.setColour (juce::Colours::white.withMultipliedAlpha (0.1f));
        g.fillPath (magnitude);

        const int size = elements.size();
        for (int i = 0; i < size; ++i)
        {
            auto handle (elements[i]);
            float circX = handle->frequencySlider == nullptr
                              ? hzToX (s.fMin)
                              : hzToX (handle->frequencySlider->getValue());
            float circY;
            if (! s.gainHandleLin)
                circY = handle->gainSlider == nullptr ? dbToY (0.0f)
                                                      : dbToY (handle->gainSlider->getValue());
            else
                circY =
                    handle->gainSlider == nullptr
                        ? dbToY (0.0f)
                        : dbToY (juce::Decibels::gainToDecibels (handle->gainSlider->getValue()));

            g.setColour (juce::Colour (0xFF191919));
            g.drawEllipse (circX - 5.0f, circY - 5.0f, 10.0f, 10.0f, 3.0f);

            g.setColour (handle->colour);
            g.drawEllipse (circX - 5.0f, circY - 5.0f, 10.0f, 10.0f, 2.0f);
            g.setColour (activeElem == i ? handle->colour : handle->colour.withSaturation (0.2));
            g.fillEllipse (circX - 5.0f, circY - 5.0f, 10.0f, 10.0f);
        }
    }

    int inline drawLevelMark (juce::Graphics& g,
                              int x,
                              int width,
                              const int level,
                              const juce::String& label,
                              int lastTextDrawPos = -1)
    {
        float yPos = dbToYFloat (level);
        x = x + 1;
        width = width - 2;

        if (yPos - 4 > lastTextDrawPos)
        {
            g.drawText (label, x + 2, yPos - 4, width - 4, 9, juce::Justification::right, false);
            return yPos + 5;
        }
        return lastTextDrawPos;
    }

    int dbToY (float dB)
    {
        int ypos = juce::roundToInt (dbToYFloat (dB));
        return ypos;
    }

    float dbToYFloat (float dB)
    {
        const float height = static_cast<float> (getHeight()) - mB - mT;
        if (height <= 0.0f)
            return 0.0f;
        float temp;
        if (dB < 0.0f)
            temp = zero + std::tanh (dB / dynamic * -2.0f);
        else
            temp = zero - 2.0f * dB / dynamic;

        return mT + scale * height * temp;
    }

    float yToDb (const float y)
    {
        float height = static_cast<float> (getHeight()) - mB - mT;

        float temp = (y - mT) / height / scale - zero;
        float dB;
        if (temp > 0.0f)
            dB = std::atanh (temp) * dynamic * -0.5f;
        else
            dB = -0.5f * temp * dynamic;
        return std::isnan (dB) ? s.dbMin : dB;
    }

    int hzToX (float hz)
    {
        float width = static_cast<float> (getWidth()) - mL - mR;
        int xpos = mL + width * (log (hz / s.fMin) / log (s.fMax / s.fMin));
        return xpos;
    }

    float xToHz (int x)
    {
        float width = static_cast<float> (getWidth()) - mL - mR;
        return s.fMin * pow ((s.fMax / s.fMin), ((x - mL) / width));
    }

    void setSampleRate (const double newSampleRate)
    {
        if (newSampleRate == 0)
            sampleRate = 48000.0;
        else
            sampleRate = newSampleRate;

        repaint();
    }

    void setOverallGain (float newGain)
    {
        float gainInDb = juce::Decibels::gainToDecibels (newGain);
        if (overallGainInDb != gainInDb)
        {
            overallGainInDb = gainInDb;
            repaint();
        }
    }

    void setOverallGainInDecibels (float newGainInDecibels)
    {
        if (overallGainInDb != newGainInDecibels)
        {
            overallGainInDb = newGainInDecibels;
            repaint();
        }
    }

    void mouseWheelMove (const juce::MouseEvent& event,
                         const juce::MouseWheelDetails& wheel) override
    {
        if (activeElem != -1)
        {
            juce::Slider* slHandle = elements[activeElem]->qSlider;
            if (slHandle != nullptr)
                slHandle->mouseWheelMove (event, wheel);
        }
    }
    void mouseDrag (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition();
        float frequency = xToHz (pos.x);
        float gain;
        if (! s.gainHandleLin)
            gain = yToDb (pos.y);
        else
            gain = juce::Decibels::decibelsToGain (yToDb (pos.y));

        if (activeElem != -1)
        {
            auto handle (elements[activeElem]);
            if (handle->frequencySlider != nullptr)
                handle->frequencySlider->setValue (frequency);
            if (handle->gainSlider != nullptr)
                handle->gainSlider->setValue (gain);
        }
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        juce::Point<int> pos = event.getPosition();
        int oldActiveElem = activeElem;
        activeElem = -1;
        for (int i = elements.size(); --i >= 0;)
        {
            auto handle (elements[i]);

            float gain;
            if (handle->gainSlider == nullptr)
                gain = 0.0f;
            else
            {
                if (! s.gainHandleLin)
                    gain = handle->gainSlider->getValue();
                else
                    gain = juce::Decibels::gainToDecibels (handle->gainSlider->getValue());
            }

            juce::Point<int> filterPos (
                handle->frequencySlider == nullptr ? hzToX (0.0f)
                                                   : hzToX (handle->frequencySlider->getValue()),
                handle->gainSlider == nullptr ? dbToY (0.0f) : dbToY (gain));
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
        numPixels = xMax - xMin + 1;

        frequencies.resize (numPixels);
        for (int i = 0; i < numPixels; ++i)
            frequencies.set (i, xToHz (xMin + i));

        allMagnitudesInDb.resize (numPixels);
        magnitudes.resize (numPixels);
        magnitudes.fill (1.0f);
        phases.resize (numPixels);
        complexMagnitudes.resize (numPixels);

        const float width = getWidth() - mL - mR;
        dbGridPath.clear();

        float dyn = s.dbMax - s.dbMin;
        int numgridlines = dyn / s.gridDiv + 1;

        for (int i = 0; i < numgridlines; ++i)
        {
            float db_val = s.dbMax - i * s.gridDiv;

            int ypos = dbToY (db_val);

            dbGridPath.startNewSubPath (mL - OH, ypos);
            dbGridPath.lineTo (mL + width + OH, ypos);
        }

        hzGridPath.clear();
        hzGridPathBold.clear();

        for (float f = s.fMin; f <= s.fMax; f += powf (10, floor (log10 (f))))
        {
            int xpos = hzToX (f);

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500) || (f == 1000) || (f == 5000)
                || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath (xpos, dbToY (s.dbMax) - OH);
                hzGridPathBold.lineTo (xpos, dbToY (s.dbMin) + OH);
            }
            else
            {
                hzGridPath.startNewSubPath (xpos, dbToY (s.dbMax) - OH);
                hzGridPath.lineTo (xpos, dbToY (s.dbMin) + OH);
            }
        }
    }

    void enableFilter (const int filterIdx, const bool shouldBeEnabled)
    {
        if (filterIdx < elements.size())
        {
            auto element = elements[filterIdx];
            element->enabled = shouldBeEnabled;
            repaint();
        }
    }

    void replaceCoefficients (
        const int filterIdx,
        typename juce::dsp::IIR::Coefficients<coefficientsType>::Ptr newCoefficients)
    {
        if (filterIdx < elements.size())
        {
            auto element = elements[filterIdx];
            element->coefficients = newCoefficients;
            repaint();
        }
    }

    void addCoefficients (typename juce::dsp::IIR::Coefficients<coefficientsType>::Ptr newCoeffs,
                          juce::Colour newColourForCoeffs,
                          juce::Slider* frequencySlider = nullptr,
                          juce::Slider* gainSlider = nullptr,
                          juce::Slider* qSlider = nullptr,
                          float* overrideLinearGain = nullptr)
    {
        elements.add (new FilterWithSlidersAndColour<coefficientsType> { newCoeffs,
                                                                         newColourForCoeffs,
                                                                         frequencySlider,
                                                                         gainSlider,
                                                                         qSlider,
                                                                         overrideLinearGain });
    }

    void setParallel (bool shouldBeParallel) { filtersAreParallel = shouldBeParallel; }

private:
    bool filtersAreParallel = false;
    float overallGainInDb { 0.0 };

    double sampleRate;

    int activeElem = -1;

    float dynamic, zero, scale;

    Settings s;
    juce::Path dbGridPath;
    juce::Path hzGridPath;
    juce::Path hzGridPathBold;

    juce::Array<double> frequencies;
    juce::Array<double> magnitudes;
    juce::Array<double> phases;
    int numPixels;

    juce::Array<std::complex<double>> complexMagnitudes;
    juce::Array<float> allMagnitudesInDb;

    juce::OwnedArray<FilterWithSlidersAndColour<coefficientsType>> elements;
};
