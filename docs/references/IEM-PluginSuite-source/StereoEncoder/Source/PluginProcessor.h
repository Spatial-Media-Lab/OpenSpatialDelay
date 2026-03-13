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
#include "../../resources/ambisonicTools.h"
#include "../../resources/efficientSHvanilla.h"

#include "../../resources/Conversions.h"

#define ProcessorClass StereoEncoderAudioProcessor

//==============================================================================
/**
*/
class StereoEncoderAudioProcessor
    : public AudioProcessorBase<IOTypes::AudioChannels<2>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 2;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    StereoEncoderAudioProcessor();
    ~StereoEncoderAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    // ====== OSC ==================================================================
    const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message) override;
    // =================

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();
    //==============================================================================

    inline void updateQuaternions();
    inline void updateEuler();

    juce::Vector3D<float> posC, posL, posR;

    juce::Atomic<bool> updatedPositionData;

    std::atomic<float>* orderSetting;
    std::atomic<float>* useSN3D;
    std::atomic<float>* qw;
    std::atomic<float>* qx;
    std::atomic<float>* qy;
    std::atomic<float>* qz;
    std::atomic<float>* azimuth;
    std::atomic<float>* elevation;
    std::atomic<float>* roll;
    std::atomic<float>* width;
    std::atomic<float>* highQuality;

    // --------------------

    bool sphericalInput;

    double phi, theta;

private:
    //==============================================================================
    bool processorUpdatingParams;

    float SHL[64];
    float SHR[64];
    float _SHL[64];
    float _SHR[64];

    juce::Atomic<bool> positionHasChanged = true;

    iem::Quaternion<float> quaternionDirection;

    juce::AudioBuffer<float> bufferCopy;

    juce::LinearSmoothedValue<float> smoothAzimuthL, smoothElevationL;
    juce::LinearSmoothedValue<float> smoothAzimuthR, smoothElevationR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoEncoderAudioProcessor)
};
