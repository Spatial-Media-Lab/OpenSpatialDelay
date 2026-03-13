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
#include "../../resources/Compressor.h"
#include "../../resources/Delay.h"
#include "../../resources/MaxRE.h"
#include "../../resources/ambisonicTools.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "LookAheadGainReduction.h"

#define ProcessorClass OmniCompressorAudioProcessor

//==============================================================================
/**
*/
class OmniCompressorAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    OmniCompressorAudioProcessor();
    ~OmniCompressorAudioProcessor() override;

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
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();

    //==============================================================================
    juce::Atomic<float> maxRMS;
    juce::Atomic<float> maxGR;
    iem::Compressor compressor;

private:
    //==============================================================================
    Delay delay;
    LookAheadGainReduction grProcessing;

    juce::Array<float> RMS, allGR;
    juce::AudioBuffer<float> gains;

    float GR;
    std::atomic<float>* orderSetting;
    std::atomic<float>* threshold;
    std::atomic<float>* outGain;
    std::atomic<float>* ratio;
    std::atomic<float>* attack;
    std::atomic<float>* release;
    std::atomic<float>* knee;
    std::atomic<float>* lookAhead;
    std::atomic<float>* reportLatency;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmniCompressorAudioProcessor)
};
