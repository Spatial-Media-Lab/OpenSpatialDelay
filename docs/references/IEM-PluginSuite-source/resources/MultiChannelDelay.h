/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
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
#include "../JuceLibraryCode/JuceHeader.h"

using namespace juce::dsp;

template <typename FloatType>
class MultiChannelDelay : private ProcessorBase
{
public:
    MultiChannelDelay() {}
    ~MultiChannelDelay() {}

    void setDelayTime (const int channel, const float delayTimeInSeconds)
    {
        jassert (delayTimeInSeconds <= maxDelay);

        if (channel < numChannels)
        {
            if (delayTimeInSeconds <= 0.0f)
            {
                delayInSeconds.setUnchecked (channel, 0.0f);
            }
            else
            {
                delayInSeconds.setUnchecked (channel, delayTimeInSeconds);
            }
            delayInSamples.setUnchecked (channel,
                                         delayInSeconds.getUnchecked (channel) * spec.sampleRate);
        }
    }

    const int getDelayInSamples (const int channel)
    {
        jassert (channel < numChannels);
        if (channel < numChannels)
            return delayInSamples[channel];
        else
            return 0;
    }

    void setMaxDelayTime (const int maxDelayTimeInSeconds) { maxDelay = maxDelayTimeInSeconds; }

    void prepare (const juce::dsp::ProcessSpec& specs) override
    {
        spec = specs;

        const int maxDelayInSamples = specs.sampleRate * maxDelay;
        buffer.setSize (specs.numChannels, specs.maximumBlockSize + maxDelayInSamples);
        buffer.clear();
        writePosition = 0;
        numChannels = specs.numChannels;
        delayInSeconds.resize (numChannels);
        delayInSamples.resize (numChannels);
    }

    void process (const juce::dsp::ProcessContextReplacing<FloatType>& context) override
    {
        juce::ScopedNoDenormals noDenormals;

        auto abIn = context.getInputBlock();
        auto abOut = context.getOutputBlock();
        auto L = abIn.getNumSamples();
        auto nCh = abIn.getNumChannels();

        // write in delay line
        int startIndex, blockSize1, blockSize2;
        getWritePositions ((int) L, startIndex, blockSize1, blockSize2);

        for (int ch = 0; ch < nCh; ch++)
            buffer.copyFrom (ch, startIndex, abIn.getChannelPointer (ch), blockSize1);

        if (blockSize2 > 0)
            for (int ch = 0; ch < nCh; ch++)
                buffer.copyFrom (ch, 0, abIn.getChannelPointer (ch) + blockSize1, blockSize2);

        // read from delay line
        for (int ch = 0; ch < nCh; ch++)
        {
            int startIndex, blockSize1, blockSize2;
            getReadPositions (ch, (int) L, startIndex, blockSize1, blockSize2);

            juce::FloatVectorOperations::copy (abOut.getChannelPointer (ch),
                                               buffer.getReadPointer (ch) + startIndex,
                                               blockSize1);

            if (blockSize2 > 0)
                juce::FloatVectorOperations::copy (abOut.getChannelPointer (ch) + blockSize1,
                                                   buffer.getReadPointer (ch),
                                                   blockSize2);
        }

        writePosition += L;
        writePosition = writePosition % buffer.getNumSamples();
    }

    void reset() override {}

    void getWritePositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
    {
        const int L = buffer.getNumSamples();
        int pos = writePosition;
        if (pos < 0)
            pos = pos + L;
        pos = pos % L;

        jassert (pos >= 0 && pos < L);

        if (numSamples <= 0)
        {
            startIndex = 0;
            blockSize1 = 0;
            blockSize2 = 0;
        }
        else
        {
            startIndex = pos;
            blockSize1 = juce::jmin (L - pos, numSamples);
            numSamples -= blockSize1;
            blockSize2 = numSamples <= 0 ? 0 : numSamples;
        }
    }

    void getReadPositions (const int channel,
                           int numSamples,
                           int& startIndex,
                           int& blockSize1,
                           int& blockSize2)
    {
        jassert (channel < delayInSamples.size());
        const int L = buffer.getNumSamples();
        int pos = writePosition - delayInSamples.getUnchecked (channel);

        if (pos < 0)
            pos = pos + L;
        pos = pos % L;

        jassert (pos >= 0 && pos < L);

        if (numSamples <= 0)
        {
            startIndex = 0;
            blockSize1 = 0;
            blockSize2 = 0;
        }
        else
        {
            startIndex = pos;
            blockSize1 = juce::jmin (L - pos, numSamples);
            numSamples -= blockSize1;
            blockSize2 = numSamples <= 0 ? 0 : numSamples;
        }
    }

private:
    //==============================================================================
    juce::dsp::ProcessSpec spec = { -1, 0, 0 };

    juce::Array<float> delayInSeconds;
    juce::Array<int> delayInSamples;

    float maxDelay = 1.0f;
    int numChannels = 0;

    int writePosition = 0;
    juce::AudioBuffer<FloatType> buffer;
};
