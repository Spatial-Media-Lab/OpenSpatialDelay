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
#include "../../resources/MatrixMultiplication.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS 1
#include "../../resources/ConfigurationHelper.h"

#define ProcessorClass MatrixMultiplierAudioProcessor

//==============================================================================
class MatrixMultiplierAudioProcessor
    : public AudioProcessorBase<IOTypes::AudioChannels<64>, IOTypes::AudioChannels<64>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    MatrixMultiplierAudioProcessor();
    ~MatrixMultiplierAudioProcessor();

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

    //==============================================================================
    const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message) override;

    //==============================================================================
    void setMatrix (ReferenceCountedMatrix::Ptr newMatrixToUse)
    {
        matTrans.setMatrix (newMatrixToUse);
    }

    juce::File getLastDir() { return lastDir; }
    void setLastDir (juce::File newLastDir);
    void loadConfiguration (const juce::File& configurationFile);

    bool messageChanged { true };
    juce::String getMessageForEditor() { return messageForEditor; }

    ReferenceCountedMatrix::Ptr getCurrentMatrix() { return currentMatrix; }

private:
    //==============================================================================
    MatrixMultiplication matTrans;
    ReferenceCountedMatrix::Ptr currentMatrix { nullptr };

    juce::File lastDir;
    juce::File lastFile;
    std::unique_ptr<juce::PropertiesFile> properties;

    juce::String messageForEditor { "Please load a configuration." };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatrixMultiplierAudioProcessor)
};
