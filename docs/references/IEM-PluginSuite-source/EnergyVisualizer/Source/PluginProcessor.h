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
#include "../../resources/efficientSHvanilla.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "../hammerAitovSample.h"

#define ProcessorClass EnergyVisualizerAudioProcessor

//==============================================================================
/**
*/
class EnergyVisualizerAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Nothing>,
      private juce::Timer
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    EnergyVisualizerAudioProcessor();
    ~EnergyVisualizerAudioProcessor();

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

    float getPeakLevelSetting() const { return peakLevel->load (std::memory_order_relaxed); }
    float getDynamicRange() const { return dynamicRange->load (std::memory_order_relaxed); }
    bool getHoldRMSSetting() const
    {
        if (holdMax->load (std::memory_order_relaxed) >= 0.5f)
            return true;
        else
            return false;
    }

    std::vector<float> rms;
    juce::Atomic<juce::Time> lastEditorTime;

private:
    //==============================================================================
    // parameters
    std::atomic<float>* orderSetting;
    std::atomic<float>* useSN3D;
    std::atomic<float>* peakLevel;
    std::atomic<float>* dynamicRange;
    std::atomic<float>* holdMax;
    std::atomic<float>* RMStimeConstant;

    float timeConstant;

    juce::Atomic<bool> doProcessing = true;

    juce::dsp::Matrix<float> decoderMatrix;
    std::vector<float> weights;
    std::vector<float> sampledSignal;

    void timerCallback() override;
    void sendAdditionalOSCMessages (juce::OSCSender& oscSender,
                                    const juce::OSCAddressPattern& address) override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnergyVisualizerAudioProcessor)
};
