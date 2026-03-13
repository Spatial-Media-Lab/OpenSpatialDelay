/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Stefan Riedel
 Copyright (c) 2022 - Institute of Electronic Music and Acoustics (IEM)
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

#include "Grain.h"

Grain::Grain()
{
    _isActive = false;
    _currentIndex = 0;
    _blockCounter = 0;
}

void Grain::setBlockSize (int numSampOutBuffer)
{
    _outputBuffer.setSize (1, numSampOutBuffer);
    _outputBuffer.clear();
}

void Grain::startGrain (const GrainJobParameters& grainParameters) // Type envelopeType)
{
    _params = grainParameters;

    _isActive = true;
    _currentIndex = 0;
    _blockCounter = 0;

    _outputBuffer.clear();
}

void Grain::processBlock (juce::AudioBuffer<float>& buffer,
                          juce::AudioBuffer<float>& circularBuffer)
{
    if (! _isActive)
        return;

    int numSampCircBuffer = circularBuffer.getNumSamples();
    int numSampOutBuffer = buffer.getNumSamples();
    jassert (_outputBuffer.getNumSamples() == numSampOutBuffer);

    _outputBuffer.clear();
    int nChOut = buffer.getNumChannels();

    const float* circularLeftChannel = circularBuffer.getReadPointer (0);
    const float* circularRightChannel = circularBuffer.getReadPointer (1);

    const float* circularBuffToSeed;
    if (_params.seedFromLeftCircBuffer)
        circularBuffToSeed = circularLeftChannel;
    else
        circularBuffToSeed = circularRightChannel;

    const float* window_ptr = _params.windowBuffer.getReadPointer (0);
    const int windowNumSamples = _params.windowBuffer.getNumSamples();

    float* outputBufferWritePtr = _outputBuffer.getWritePointer (0);

    int outStart;
    if (_blockCounter == 0)
    {
        outStart = _params.startOffsetInBlock;
    }
    else
    {
        outStart = 0;
    }

    // Write samples to internal mono buffer
    for (int i = outStart; i < numSampOutBuffer; i++)
    {
        if (_currentIndex < _params.grainLengthSamples) // grain still needs samples
        {
            // Linear interpolation of buffer samples
            float readIndex =
                _params.startPositionCircBuffer + (_currentIndex * _params.pitchReadFactor);
            int readIndexInt = static_cast<int> (readIndex);
            int readIndexIntNext = readIndexInt + 1;
            float sampleFracWeight = readIndex - readIndexInt;
            if (readIndexInt >= numSampCircBuffer)
                readIndexInt = readIndexInt - numSampCircBuffer;
            if (readIndexIntNext >= numSampCircBuffer)
                readIndexIntNext = readIndexIntNext - numSampCircBuffer;
            float sampleIntPart = circularBuffToSeed[readIndexInt];
            float sampleFracPart = circularBuffToSeed[readIndexIntNext] - sampleIntPart;
            float sampleValue = sampleIntPart + sampleFracWeight * sampleFracPart;

            // Linear interpolation for grain window function
            float windowIndex = static_cast<float> (_currentIndex)
                                / static_cast<float> (_params.grainLengthSamples)
                                * (windowNumSamples - 1);
            int windowIndexInt = static_cast<int> (windowIndex);
            jassert (windowIndexInt >= 0 && windowIndexInt < (windowNumSamples - 1));

            int windowIndexIntNext = windowIndexInt + 1;
            float windowFracWeight = windowIndex - windowIndexInt;
            float windowIntPart = window_ptr[windowIndexInt];
            float windFracPart = window_ptr[windowIndexIntNext] - windowIntPart;
            float windowValue = windowIntPart + windowFracWeight * windFracPart;
            jassert (windowValue >= 0.0f && windowValue <= 1.0f);
            outputBufferWritePtr[i] = sampleValue * windowValue;

            _currentIndex++;
        }
        else
        {
            _isActive = false;
            break;
        }
    }

    const float* outputBufferReadPtr = _outputBuffer.getReadPointer (0);
    for (int ch = 0; ch < nChOut; ++ch)
    {
        buffer.addFrom (ch,
                        0,
                        outputBufferReadPtr,
                        buffer.getNumSamples(),
                        _params.channelWeights[ch] * _params.gainFactor);
    }

    _blockCounter++;
};

bool Grain::isActive() const
{
    return _isActive;
};
