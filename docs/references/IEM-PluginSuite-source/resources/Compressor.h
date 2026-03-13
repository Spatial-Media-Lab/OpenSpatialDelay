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
namespace iem
{

class Compressor
{
public:
    Compressor() {}
    ~Compressor() {}

    void prepare (const juce::dsp::ProcessSpec spec)
    {
        sampleRate = spec.sampleRate;

        alphaAttack = 1.0 - timeToGain (attackTime);
        alphaRelease = 1.0 - timeToGain (releaseTime);

        prepared = true;
    }

    void setAttackTime (float attackTimeInSeconds)
    {
        attackTime = attackTimeInSeconds;
        alphaAttack = 1.0 - timeToGain (attackTime);
    }

    void setReleaseTime (float releaseTimeInSeconds)
    {
        releaseTime = releaseTimeInSeconds;
        alphaRelease = 1.0 - timeToGain (releaseTime);
    }

    double timeToGain (float timeInSeconds) { return exp (-1.0 / (sampleRate * timeInSeconds)); }

    void setKnee (float kneeInDecibels)
    {
        knee = kneeInDecibels;
        kneeHalf = knee / 2.0f;
    }

    const float getKnee() { return knee; }

    void setThreshold (float thresholdInDecibels) { threshold = thresholdInDecibels; }

    const float getTreshold() { return threshold; }

    void setMakeUpGain (float makeUpGainInDecibels) { makeUpGain = makeUpGainInDecibels; }

    const float getMakeUpGain() { return makeUpGain; }

    void setRatio (float ratio) { slope = 1.0f / ratio - 1.0f; }

    const float getMaxLevelInDecibels() { return maxLevel; }

    void applyCharacteristicToOverShoot (float& overShoot)
    {
        if (overShoot <= -kneeHalf)
            overShoot = 0.0f; //y_G = levelInDecibels;
        else if (overShoot > -kneeHalf && overShoot <= kneeHalf)
            overShoot =
                0.5f * slope * juce::square (overShoot + kneeHalf)
                / knee; //y_G = levelInDecibels + 0.5f * slope * square(overShoot + kneeHalf) / knee;
        else
            overShoot = slope * overShoot;
    }

    void getGainFromSidechainSignal (const float* sideChainSignal,
                                     float* destination,
                                     const int numSamples)
    {
        maxLevel = -INFINITY;
        for (int i = 0; i < numSamples; ++i)
        {
            // convert sample to decibels
            float levelInDecibels = juce::Decibels::gainToDecibels (abs (sideChainSignal[i]));
            if (levelInDecibels > maxLevel)
                maxLevel = levelInDecibels;
            // calculate overshoot and apply knee and ratio
            float overShoot = levelInDecibels - threshold;
            applyCharacteristicToOverShoot (overShoot); //y_G = levelInDecibels + slope * overShoot;

            // ballistics
            const float diff = overShoot - state;
            if (diff < 0.0f)
                state += alphaAttack * diff;
            else
                state += alphaRelease * diff;

            destination[i] = juce::Decibels::decibelsToGain (state + makeUpGain);
        }
    }

    void getGainFromSidechainSignalInDecibelsWithoutMakeUpGain (const float* sideChainSignal,
                                                                float* destination,
                                                                const int numSamples)
    {
        maxLevel = -INFINITY;
        for (int i = 0; i < numSamples; ++i)
        {
            // convert sample to decibels
            float levelInDecibels = juce::Decibels::gainToDecibels (abs (sideChainSignal[i]));
            if (levelInDecibels > maxLevel)
                maxLevel = levelInDecibels;
            // calculate overshoot and apply knee and ratio
            float overShoot = levelInDecibels - threshold;
            applyCharacteristicToOverShoot (overShoot); //y_G = levelInDecibels + slope * overShoot;

            // ballistics
            const float diff = overShoot - state;
            if (diff < 0.0f)
                state += alphaAttack * diff;
            else
                state += alphaRelease * diff;

            destination[i] = state;
        }
    }

    void getCharacteristic (float* inputLevels, float* dest, const int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = getCharacteristicSample (inputLevels[i]);
        }
    }

    inline float getCharacteristicSample (float inputLevel)
    {
        float overShoot = inputLevel - threshold;
        applyCharacteristicToOverShoot (overShoot);
        return overShoot + inputLevel + makeUpGain;
    }

private:
    double sampleRate { 0.0 };
    bool prepared;

    float knee { 0.0f }, kneeHalf { 0.0f };
    float threshold { -10.0f };
    float attackTime { 0.01f };
    float releaseTime { 0.15f };
    float slope { 0.0f };
    float makeUpGain { 0.0f };

    float maxLevel { -INFINITY };

    //state juce::variable
    float state { 0.0f };

    double alphaAttack;
    double alphaRelease;
};

} // namespace iem
