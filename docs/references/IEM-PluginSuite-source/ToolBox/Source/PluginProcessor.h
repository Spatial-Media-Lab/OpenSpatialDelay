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
#include "../../resources/MaxRE.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/inPhase.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define ProcessorClass ToolBoxAudioProcessor

//==============================================================================
class ToolBoxAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<7>, IOTypes::Ambisonics<7>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    ToolBoxAudioProcessor();
    ~ToolBoxAudioProcessor() override;

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
    void updateBuffers() override; // use this to implement a buffer update method

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();

private:
    //==============================================================================
    // list of used audio parameters
    std::atomic<float>* inputOrderSetting;
    std::atomic<float>* outputOrderSetting;
    std::atomic<float>* useSn3dInput;
    std::atomic<float>* useSn3dOutput;
    std::atomic<float>* flipX;
    std::atomic<float>* flipY;
    std::atomic<float>* flipZ;
    std::atomic<float>* loaWeights;
    std::atomic<float>* gain;

    float previousWeights[64];

    void calculateWeights (float* weights, const int nChannelsIn, const int nChannelsOut);

    bool doFlipX, doFlipY, doFlipZ;
    // flips
    juce::BigInteger flipXMask, flipYMask, flipZMask;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolBoxAudioProcessor)
};
