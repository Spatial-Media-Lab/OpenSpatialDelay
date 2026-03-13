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

#include "../../resources/AudioProcessorBase.h"
#include "../JuceLibraryCode/JuceHeader.h"

#include "../../resources/Quaternion.h"
#include "../../resources/Weights.h"
#include "../../resources/efficientSHvanilla.h"

#define ProcessorClass DirectivityShaperAudioProcessor

#define numberOfBands 4
using namespace juce::dsp;

//==============================================================================
/**
*/
class DirectivityShaperAudioProcessor
    : public AudioProcessorBase<IOTypes::AudioChannels<1>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 1;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DirectivityShaperAudioProcessor();
    ~DirectivityShaperAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();
    //==============================================================================

    float weights[numberOfBands][8];

    inline juce::dsp::IIR::Coefficients<float>::Ptr
        createFilterCoefficients (int type, double sampleRate, double frequency, double Q);

    IIR::Filter<float> filter[numberOfBands];

    float probeGains[numberOfBands];

    juce::Atomic<bool> repaintDV = true;
    juce::Atomic<bool> repaintXY = true;
    juce::Atomic<bool> repaintFV = true;
    juce::Atomic<bool> repaintSphere = true;

    void updateBuffers() override { repaintXY = true; };

private:
    //==============================================================================
    juce::AudioSampleBuffer filteredBuffer;

    iem::Quaternion<float> quats[numberOfBands];

    bool changeWeights = true;
    bool probeChanged = true;

    bool toggled = false;
    bool moving = false;

    float shOld[numberOfBands][64];

    // parameters
    std::atomic<float>* orderSetting;
    std::atomic<float>* useSN3D;
    std::atomic<float>* filterType[numberOfBands];
    std::atomic<float>* filterFrequency[numberOfBands];
    std::atomic<float>* filterQ[numberOfBands];
    std::atomic<float>* filterGain[numberOfBands];
    std::atomic<float>* probeAzimuth;
    std::atomic<float>* probeElevation;
    std::atomic<float>* probeRoll;
    std::atomic<float>* probeLock;
    std::atomic<float>* normalization;
    std::atomic<float>* order[numberOfBands];
    std::atomic<float>* shape[numberOfBands];
    std::atomic<float>* azimuth[numberOfBands];
    std::atomic<float>* elevation[numberOfBands];
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityShaperAudioProcessor)
};
