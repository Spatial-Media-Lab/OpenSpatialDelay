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
#include "../../resources/ambisonicTools.h"
#include "../../resources/interpLagrangeWeights.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define ProcessorClass DualDelayAudioProcessor

//==============================================================================
/**
*/
class DualDelayAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>, true>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    DualDelayAudioProcessor();
    ~DualDelayAudioProcessor();

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
    void updateBuffers() override;

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();
    //==============================================================================

private:
    //==============================================================================
    // parameters
    std::atomic<float>* dryGain;
    std::atomic<float>* wetGainL;
    std::atomic<float>* wetGainR;
    std::atomic<float>* delayTimeL;
    std::atomic<float>* delayTimeR;
    std::atomic<float>* rotationL;
    std::atomic<float>* rotationR;
    std::atomic<float>* LPcutOffL;
    std::atomic<float>* LPcutOffR;
    std::atomic<float>* HPcutOffL;
    std::atomic<float>* HPcutOffR;
    std::atomic<float>* feedbackL;
    std::atomic<float>* feedbackR;
    std::atomic<float>* xfeedbackL;
    std::atomic<float>* xfeedbackR;
    std::atomic<float>* lfoRateL;
    std::atomic<float>* lfoRateR;
    std::atomic<float>* lfoDepthL;
    std::atomic<float>* lfoDepthR;
    std::atomic<float>* orderSetting;

    float _delayL, _delayR;

    juce::AudioBuffer<float> AudioIN;

    juce::AudioBuffer<float> delayBufferLeft;
    juce::AudioBuffer<float> delayBufferRight;
    juce::AudioBuffer<float> delayOutLeft;
    juce::AudioBuffer<float> delayOutRight;
    juce::AudioBuffer<float> delayInLeft;
    juce::AudioBuffer<float> delayInRight;

    juce::AudioBuffer<float> delayTempBuffer;

    juce::Array<float> delay;
    juce::Array<int> interpCoeffIdx;
    juce::Array<int> idx;

    juce::dsp::Oscillator<float> LFOLeft, LFORight;

    int writeOffsetLeft;
    int writeOffsetRight;
    int readOffsetLeft;
    int readOffsetRight;

    float* readPointer;
    juce::Array<float> sin_z;
    juce::Array<float> cos_z;

    void calcParams (float phi);
    void rotateBuffer (juce::AudioBuffer<float>* bufferToRotate,
                       const int nChannels,
                       const int samples);
    float feedback = 0.8f;

    juce::OwnedArray<juce::IIRFilter> lowPassFiltersLeft;
    juce::OwnedArray<juce::IIRFilter> lowPassFiltersRight;
    juce::OwnedArray<juce::IIRFilter> highPassFiltersLeft;
    juce::OwnedArray<juce::IIRFilter> highPassFiltersRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessor)
};
