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

#include "../../resources/AmbisonicDecoder.h"
#include "../../resources/AudioProcessorBase.h"
#include <JuceHeader.h>

#define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 1
#include "../../resources/ConfigurationHelper.h"

#include "../../resources/FilterVisualizerHelper.h"
#include "../../resources/ReferenceCountedDecoder.h"

#define ProcessorClass SimpleDecoderAudioProcessor

using namespace juce::dsp;
//==============================================================================
class SimpleDecoderAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::AudioChannels<>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    static const juce::StringArray weightsStrings;

    //==============================================================================
    SimpleDecoderAudioProcessor();
    ~SimpleDecoderAudioProcessor() override;

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

    //======= OSC ==================================================================
    inline const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message) override;

    //==============================================================================
    juce::File getLastDir() { return lastDir; }
    void setLastDir (juce::File newLastDir);
    void loadConfiguration (const juce::File& presetFile);

    juce::Atomic<bool> updateDecoderInfo = true;
    juce::Atomic<bool> messageChanged { true };
    juce::String getMessageForEditor() { return messageForEditor; }

    ReferenceCountedDecoder::Ptr getCurrentDecoderConfig() { return decoderConfig; }

    IIR::Coefficients<double>::Ptr cascadedHighPassCoeffs, cascadedLowPassCoeffs;
    juce::Atomic<bool> guiUpdateLowPassCoefficients = true;
    juce::Atomic<bool> guiUpdateHighPassCoefficients = true;
    juce::Atomic<bool> guiUpdateLowPassGain = true;
    juce::Atomic<bool> guiUpdateSampleRate = true;

private:
    //==============================================================================
    void updateLowPassCoefficients (double sampleRate, float frequency);
    void updateHighPassCoefficients (double sampleRate, float frequency);

    void loadConfigFromString (juce::String string);

    // list of used audio parameters
    std::atomic<float>* inputOrderSetting;
    std::atomic<float>* useSN3D;
    std::atomic<float>* lowPassFrequency;
    std::atomic<float>* lowPassGain;
    std::atomic<float>* highPassFrequency;

    std::atomic<float>* swMode;
    std::atomic<float>* swChannel;
    std::atomic<float>* weights;

    // =========================================

    float omniGain = 0.0f;

    juce::File lastDir;

    juce::String lastConfigString;

    std::unique_ptr<juce::PropertiesFile> properties;

    juce::AudioBuffer<float> swBuffer;

    // processors
    std::unique_ptr<IIR::Filter<float>> lowPass1;
    std::unique_ptr<IIR::Filter<float>> lowPass2;
    IIR::Coefficients<float>::Ptr highPassCoeffs;
    IIR::Coefficients<float>::Ptr lowPassCoeffs;

    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass1;
    ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> highPass2;

    juce::dsp::Gain<float> masterGain;

    juce::dsp::ProcessSpec highPassSpecs { 48000, 0, 0 };

    AmbisonicDecoder decoder;

    ReferenceCountedDecoder::Ptr decoderConfig { nullptr };
    juce::String messageForEditor { "" };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDecoderAudioProcessor)
};
