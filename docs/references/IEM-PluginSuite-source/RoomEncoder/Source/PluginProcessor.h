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

#define JUCE_USE_VDSP_FRAMEWORK 1

#pragma once

#include "../../resources/AudioProcessorBase.h"
#include "../../resources/ambisonicTools.h"
#include "../../resources/customComponents/FilterVisualizer.h"
#include "../../resources/efficientSHvanilla.h"
#include "../../resources/interpLagrangeWeights.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "reflections.h"

#ifdef JUCE_MAC
    #define VIMAGE_H // avoid namespace clashes
    #include <Accelerate/Accelerate.h>
#endif

#if JUCE_USE_SIMD
using IIRfloat = juce::dsp::SIMDRegister<float>;
static constexpr int IIRfloat_elements = juce::dsp::SIMDRegister<float>::size();
#else /* !JUCE_USE_SIMD */
using IIRfloat = float;
static constexpr int IIRfloat_elements = 1;
#endif /* JUCE_USE_SIMD */

#define ProcessorClass RoomEncoderAudioProcessor

const int mSig[] = { 1, -1 };
using namespace juce::dsp;

struct RoomParams
{
    bool validRoomData = false;
    bool validListenerData = false;
    bool validReflectionData = false;
    float roomX, roomY, roomZ;
    float listenerX, listenerY, listenerZ;
    float reflCoeff, lowShelfFreq, lowShelfGain, highShelfFreq, highShelfGain, numRefl;
    float wallAttenuationFront, wallAttenuationBack, wallAttenuationLeft, wallAttenuationRight,
        wallAttenuationCeiling, wallAttenuationFloor;
};

struct SharedParams
{
    SharedParams()
    {
        rooms.add (RoomParams());
        rooms.add (RoomParams());
        rooms.add (RoomParams());
        rooms.add (RoomParams());
    }
    juce::Array<RoomParams> rooms;
};

struct ReflectionProperty
{
    const int x; // coordinate in image source space
    const int y; // coordinate in image source space
    const int z; // coordinate in image source space
    const int order; // image source order
    const int xPlusReflections; // number of reflections at front wall
    const int xMinusReflections; // number of reflections at back wall
    const int yPlusReflections; // number of reflections at left wall
    const int yMinusReflections; // number of reflections at right wall
    const int zPlusReflections; // number of reflections at ceiling
    const int zMinusReflections; // number of reflections at floor
};

//==============================================================================
/**
*/
class RoomEncoderAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<>, IOTypes::Ambisonics<>>,
      private juce::Timer
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    RoomEncoderAudioProcessor();
    ~RoomEncoderAudioProcessor();

    //==============================================================================

    void initializeReflectionList();

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
    double oldDelay[nImgSrc];
    float allGains[nImgSrc];

    //filter coefficients
    IIR::Coefficients<float>::Ptr lowShelfCoefficients;
    IIR::Coefficients<float>::Ptr highShelfCoefficients;

    bool userChangedFilterSettings = true;
    bool updateFv = false;

    void timerCallback() override;

    void updateFilterCoefficients (double sampleRate);
    void calculateImageSourcePositions (const float t, const float b, const float h);

    std::atomic<float>* numRefl;
    float mRadius[nImgSrc];

    void updateBuffers() override;

    juce::Atomic<bool> repaintPositionPlanes = true;

private:
    //==============================================================================
    inline void clear (juce::dsp::AudioBlock<IIRfloat>& ab);

    bool readingSharedParams = false;
    ;

    double phi;
    double theta;

    // Parameters
    std::atomic<float>* directivityOrderSetting;
    std::atomic<float>* inputIsSN3D;
    std::atomic<float>* orderSetting;
    std::atomic<float>* useSN3D;

    std::atomic<float>* roomX;
    std::atomic<float>* roomY;
    std::atomic<float>* roomZ;

    std::atomic<float>* sourceX;
    std::atomic<float>* sourceY;
    std::atomic<float>* sourceZ;

    std::atomic<float>* listenerX;
    std::atomic<float>* listenerY;
    std::atomic<float>* listenerZ;

    std::atomic<float>* reflCoeff;

    std::atomic<float>* lowShelfFreq;
    std::atomic<float>* lowShelfGain;
    std::atomic<float>* highShelfFreq;
    std::atomic<float>* highShelfGain;

    std::atomic<float>* syncChannel;
    std::atomic<float>* syncRoomSize;
    std::atomic<float>* syncReflection;
    std::atomic<float>* syncListener;

    std::atomic<float>* renderDirectPath;
    std::atomic<float>* directPathZeroDelay;
    std::atomic<float>* directPathUnityGain;

    std::atomic<float>* wallAttenuationFront;
    std::atomic<float>* wallAttenuationBack;
    std::atomic<float>* wallAttenuationLeft;
    std::atomic<float>* wallAttenuationRight;
    std::atomic<float>* wallAttenuationCeiling;
    std::atomic<float>* wallAttenuationFloor;

    int _numRefl;

    juce::SharedResourcePointer<SharedParams> sharedParams;

    //SIMD IIR Filter

    juce::OwnedArray<juce::OwnedArray<IIR::Filter<IIRfloat>>> lowShelfArray;
    juce::OwnedArray<juce::OwnedArray<IIR::Filter<IIRfloat>>> highShelfArray;
    juce::HeapBlock<char> interleavedBlockData[16], zeroData; //todo: dynamically?
    juce::OwnedArray<juce::dsp::AudioBlock<IIRfloat>> interleavedData;
    juce::dsp::AudioBlock<float> zero;

    juce::Array<int> filterPoints { 1, 7, 25, 61, 113, 169, 213 };

    juce::Vector3D<float> sourcePos, listenerPos;

    float mx[nImgSrc];
    float my[nImgSrc];
    float mz[nImgSrc];
    float smx[nImgSrc];
    float smy[nImgSrc];
    float smz[nImgSrc];

    int bufferSize;
    int bufferReadIdx;

    int readOffset;

    float powReflCoeff[maxOrderImgSrc + 1];
    double dist2smpls;

    float SHcoeffsOld[nImgSrc][64];
    IIRfloat SHsampleOld[nImgSrc][16]; //TODO: can be smaller: (N+1)^2/IIRfloat_elements()

    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> monoBuffer;

    juce::OwnedArray<ReflectionProperty> reflectionList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomEncoderAudioProcessor)
};
