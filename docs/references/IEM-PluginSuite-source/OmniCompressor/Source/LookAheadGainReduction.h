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
class LookAheadGainReduction
{
public:
    LookAheadGainReduction() {}
    ~LookAheadGainReduction() {}

    void setDelayTime (float delayTimeInSeconds)
    {
        if (delayTimeInSeconds <= 0.0f)
        {
            delay = 0.0f;
        }
        else
        {
            delay = delayTimeInSeconds;
        }

        prepare (spec);
    }

    const int getDelayInSamples() { return delayInSamples; }

    void prepare (const juce::dsp::ProcessSpec& specs)
    {
        spec = specs;

        delayInSamples = delay * specs.sampleRate;

        buffer.setSize (specs.numChannels, specs.maximumBlockSize + delayInSamples);
        buffer.clear();
        writePosition = 0;
    }

    void pushSamples (const float* src, int numSamples)
    {
        int startIndex, blockSize1, blockSize2;

        // write in delay line
        getWritePositions (numSamples, startIndex, blockSize1, blockSize2);

        buffer.copyFrom (0, startIndex, src, blockSize1);

        if (blockSize2 > 0)
            buffer.copyFrom (0, 0, src + blockSize1, blockSize2);

        writePosition += numSamples;
        writePosition = writePosition % buffer.getNumSamples();

        lastPushedSamples = numSamples;
    }

    void readSamples (float* dest, int numSamples)
    {
        jassert (numSamples == lastPushedSamples);

        int startIndex, blockSize1, blockSize2;

        // read from delay line
        getReadPositions (numSamples, startIndex, blockSize1, blockSize2);
        juce::FloatVectorOperations::copy (dest,
                                           buffer.getReadPointer (0) + startIndex,
                                           blockSize1);

        if (blockSize2 > 0)
            juce::FloatVectorOperations::copy (dest + blockSize1,
                                               buffer.getReadPointer (0),
                                               blockSize2);
    }

    void process()
    {
        jassert (delayInSamples > 0);

        juce::ScopedNoDenormals noDenormals;

        float nextGainReductionValue = 0.0f;
        float step = 0.0f;

        // last pushed samples
        int startIndex = writePosition - 1;
        if (startIndex < 0)
            startIndex += buffer.getNumSamples();

        int size1, size2;

        getProcessPositions (startIndex, lastPushedSamples, size1, size2);
        for (int i = 0; i < size1; ++i)
        {
            float smpl = buffer.getSample (0, startIndex);
            if (smpl > nextGainReductionValue)
            {
                buffer.setSample (0, startIndex, nextGainReductionValue);
                nextGainReductionValue += step;
            }
            else
            {
                step = -smpl / delayInSamples;
                nextGainReductionValue = smpl + step;
            }
            --startIndex;
        }
        if (size2 > 0)
        {
            startIndex = buffer.getNumSamples() - 1;
            for (int i = 0; i < size2; ++i)
            {
                float smpl = buffer.getSample (0, startIndex);
                if (smpl > nextGainReductionValue)
                {
                    buffer.setSample (0, startIndex, nextGainReductionValue);
                    nextGainReductionValue += step;
                }
                else
                {
                    step = -smpl / delayInSamples;
                    nextGainReductionValue = smpl + step;
                }
                --startIndex;
            }
        }

        if (startIndex < 0)
            startIndex = buffer.getNumSamples() - 1;

        getProcessPositions (startIndex, delayInSamples, size1, size2);
        bool breakWasUsed = false;

        for (int i = 0; i < size1; ++i)
        {
            float smpl = buffer.getSample (0, startIndex);
            if (smpl > nextGainReductionValue)
            {
                buffer.setSample (0, startIndex, nextGainReductionValue);
                nextGainReductionValue += step;
            }
            else
            {
                breakWasUsed = true;
                break;
            }
            --startIndex;
        }
        if (! breakWasUsed && size2 > 0)
        {
            startIndex = buffer.getNumSamples() - 1;
            for (int i = 0; i < size2; ++i)
            {
                float smpl = buffer.getSample (0, startIndex);
                if (smpl > nextGainReductionValue)
                {
                    buffer.setSample (0, startIndex, nextGainReductionValue);
                    nextGainReductionValue += step;
                }
                else
                {
                    break;
                }
                --startIndex;
            }
        }
    }

private:
    inline void
        getProcessPositions (int startIndex, int numSamples, int& blockSize1, int& blockSize2)
    {
        jassert (startIndex >= 0 && startIndex < buffer.getNumSamples());

        if (numSamples <= 0)
        {
            blockSize1 = 0;
            blockSize2 = 0;
        }
        else
        {
            blockSize1 = juce::jmin (startIndex + 1, numSamples);
            numSamples -= blockSize1;
            blockSize2 = numSamples <= 0 ? 0 : numSamples;
        }
    }

    inline void
        getWritePositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
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

    inline void getReadPositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
    {
        const int L = buffer.getNumSamples();
        int pos = writePosition - lastPushedSamples - delayInSamples;

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
    float delay;
    int delayInSamples = 0;
    int writePosition = 0;
    int lastPushedSamples = 0;
    juce::AudioBuffer<float> buffer;
};
