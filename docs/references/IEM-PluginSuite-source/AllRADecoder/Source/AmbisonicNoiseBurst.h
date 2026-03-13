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
#include "../../resources/Conversions.h"

class AmbisonicNoiseBurst
{
public:
    AmbisonicNoiseBurst()
    {
        originalNoise.setSize (1, L);

        // create noise
        juce::Random random;
        for (int i = 0; i < L; ++i)
            originalNoise.setSample (0, i, random.nextFloat() * 2.0f - 1.0f);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = 44100.0f;
        spec.maximumBlockSize = L;
        spec.numChannels = 1;

        juce::dsp::IIR::Filter<float> filter;
        filter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighPass (spec.sampleRate, 200.0f);
        juce::dsp::AudioBlock<float> ab (originalNoise);
        juce::dsp::ProcessContextReplacing<float> context (ab);

        filter.prepare (spec);
        filter.process (context);

        filter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (spec.sampleRate, 200.0f);
        filter.prepare (spec);
        filter.reset();
        filter.process (context);

        // fade-in/-out
        originalNoise.applyGainRamp (0, 0, 1000, 0.0f, 1.0f);
        originalNoise.applyGainRamp (0, L - 10000, 10000, 1.0f, 0.0f);

        // level
        originalNoise.applyGain (juce::Decibels::decibelsToGain (-10.0f));

        // copy buffer
        resampledNoise = originalNoise;
        resampledL = L;
    }

    ~AmbisonicNoiseBurst() {}

    void prepare (const juce::dsp::ProcessSpec spec)
    {
        if (sampleRate != spec.sampleRate)
            resampleNoise (spec.sampleRate);
        sampleRate = spec.sampleRate;

        active = false;
    }

    void resampleNoise (double newSampleRate)
    {
        double factorReading = 44100.0 / newSampleRate;
        resampledL = juce::roundToInt (L / factorReading + 0.49);

        juce::MemoryAudioSource memorySource (originalNoise, false, false);
        juce::ResamplingAudioSource resamplingSource (&memorySource, false, 1);

        resamplingSource.setResamplingRatio (factorReading);
        resamplingSource.prepareToPlay (L, 44100.0);

        resampledNoise.setSize (1, resampledL);
        juce::AudioSourceChannelInfo info;
        info.startSample = 0;
        info.numSamples = resampledL;
        info.buffer = &resampledNoise;

        resamplingSource.getNextAudioBlock (info);
    }

    void setOrder (int order) { ambisonicOrder = juce::jmin (order, 7); }

    void setNormalization (bool useSN3D) { this->useSN3D = useSN3D; }

    void play (float azimuthInDegrees, float elevationInDegrees)
    {
        if (! active.get())
        {
            Conversions<float>::sphericalToCartesian (juce::degreesToRadians (azimuthInDegrees),
                                                      juce::degreesToRadians (elevationInDegrees),
                                                      1.0f,
                                                      x,
                                                      y,
                                                      z);
            currentPosition = 0;
            active = true;
        }
    }

    const bool isActive() { return active.get(); }

    void processBuffer (juce::AudioBuffer<float> buffer)
    {
        if (active.get())
        {
            const int bufferSize = buffer.getNumSamples();
            const int copyL = juce::jmin (bufferSize, resampledL - currentPosition);
            const int nCh = juce::jmin (buffer.getNumChannels(), juce::square (ambisonicOrder + 1));

            float SH[64];
            SHEval (ambisonicOrder, x, y, z, SH);

            if (useSN3D)
                juce::FloatVectorOperations::multiply (SH, n3d2sn3d, nCh);

            for (int ch = 0; ch < nCh; ++ch)
                buffer.addFrom (ch, 0, resampledNoise, 0, currentPosition, copyL, SH[ch]);

            currentPosition += copyL;
            if (currentPosition >= resampledL)
            {
                active = false;
            }
        }
    }

private:
    double sampleRate;

    const int L = 22050;
    int currentPosition;

    int resampledL;
    juce::Atomic<bool> active;

    int ambisonicOrder = 0;
    float x = 1.0f;
    float y = 0.0f;
    float z = 0.0f;

    bool useSN3D = true;

    juce::AudioBuffer<float> originalNoise;
    juce::AudioBuffer<float> resampledNoise;
};
