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

#pragma once

#include "../../resources/AudioProcessorBase.h"
#include "../JuceLibraryCode/JuceHeader.h"

#include "../../resources/Quaternion.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/efficientSHvanilla.h"

#include "../../resources/Conversions.h"
#include "Grain.h"
#include <random>

#define ProcessorClass GranularEncoderAudioProcessor
#define maxNumGrains 512
#define windowResolution 1024
#define CIRC_BUFFER_SECONDS 8.0f
#define MAX_GRAIN_LENGTH 2.0f
#define MIN_GRAIN_LENGTH 0.001f
#define MAX_DELTA_T 2.0f
#define MIN_DELTA_T 0.001f
#define GUI_SKEW 0.5f

//==============================================================================
/**
 */
class GranularEncoderAudioProcessor
    : public AudioProcessorBase<IOTypes::AudioChannels<2>, IOTypes::Ambisonics<>>
{
public:
    constexpr static int numberOfInputChannels = 2;
    constexpr static int numberOfOutputChannels = 64;

    //==============================================================================
    GranularEncoderAudioProcessor();
    ~GranularEncoderAudioProcessor();

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

    void resampleAudioBuffer (juce::AudioBuffer<float>& inAudioBuffer,
                              float inSampleRate,
                              juce::AudioBuffer<float>& outAudioBuffer,
                              float outSampleRate);

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // ====== OSC ==================================================================
    const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message) override;
    // =================

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();
    //==============================================================================

    inline void updateQuaternions();
    inline void updateEuler();

    juce::Vector3D<float> getRandomGrainDirection3D();
    juce::Vector3D<float> getRandomGrainDirection2D();
    juce::AudioBuffer<float> getWindowBuffer (float modWeight);
    int getStartPositionCircBuffer() const;
    std::pair<int, float> getGrainLengthAndPitchFactor() const;
    int getDeltaTimeSamples();
    bool getChannelToSeed();

    bool getFreezeGUIBool();
    void initializeModeTransition (bool freeze);
    void finishModeTransition();

    float getMeanWindowGain();

    // void writeCircularBufferToDisk(juce::String filename); // Debug function

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

    std::atomic<float>* shape; // distribution parameter (circular to peaky, uniform at 0.0)
    std::atomic<float>* size; // total opening angle of distribution cap

    std::atomic<float>* roll; // legacy parameter from StereoEncoder
    std::atomic<float>* width; // legacy parameter from StereoEncoder

    std::atomic<float>* deltaTime;
    std::atomic<float>* deltaTimeMod;

    std::atomic<float>* grainLength;
    std::atomic<float>* grainLengthMod;

    std::atomic<float>* pitch;
    std::atomic<float>* pitchMod;

    std::atomic<float>* position;
    std::atomic<float>* positionMod;

    std::atomic<float>* windowAttack;
    std::atomic<float>* windowAttackMod;

    std::atomic<float>* windowDecay;
    std::atomic<float>* windowDecayMod;

    std::atomic<float>* mix;
    std::atomic<float>* sourceProbability;

    std::atomic<float>* highQuality;

    std::atomic<float>* freeze;
    std::atomic<float>* spatialize2D;

    // --------------------

    bool sphericalInput;

    double phi, theta;

    enum class OperationMode
    {
        Realtime,
        ToFreeze,
        Freeze,
        ToRealtime
    };

private:
    //==============================================================================
    bool processorUpdatingParams;

    float SHC[64];
    float _SHC[64];

    juce::Atomic<bool> positionHasChanged = true;

    iem::Quaternion<float> quaternionDirection;

    juce::AudioBuffer<float> bufferCopy;
    juce::AudioBuffer<float> dryAmbiBuffer;
    juce::AudioBuffer<float> wetAmbiBuffer;

    juce::AudioBuffer<float> circularBuffer;
    int circularBufferWriteHead;
    int circularBufferLength;

    int deltaTimeSamples = 0;
    float lastSampleRate;
    float sampleRateAtSerialize;

    int grainTimeCounter = 0;

    Grain grains[maxNumGrains];

    float _grainSH[maxNumGrains][64];

    std::mt19937 rng;

    juce::SmoothedValue<float> writeGainCircBuffer = 1.0f;
    OperationMode mode = OperationMode::Realtime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularEncoderAudioProcessor)
};
