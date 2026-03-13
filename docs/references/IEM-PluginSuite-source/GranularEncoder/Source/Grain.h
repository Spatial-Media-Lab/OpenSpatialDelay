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

#include "JuceHeader.h"

class Grain
{
public:
    struct GrainJobParameters
    {
        int startPositionCircBuffer = 0;
        int startOffsetInBlock = 0;
        int grainLengthSamples = 0;
        float pitchReadFactor = 1.0f;
        std::array<float, 64> channelWeights;
        float gainFactor = 1.0f;
        bool seedFromLeftCircBuffer = true;
        juce::AudioBuffer<float> windowBuffer;
    };

    Grain();

    void setBlockSize (int numSampOutBuffer);
    void startGrain (const GrainJobParameters& grainParameters);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& circularBuffer);

    bool isActive() const;

private:
    GrainJobParameters _params;
    int _currentIndex;
    bool _isActive;
    int _blockCounter;
    juce::AudioBuffer<float> _outputBuffer;
};
