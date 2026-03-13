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
#include "../../resources/FilterVisualizerHelper.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define numFilterBands 6
using namespace juce::dsp;

#if JUCE_USE_SIMD
using IIRfloat = juce::dsp::SIMDRegister<float>;
static constexpr int IIRfloat_elements = juce::dsp::SIMDRegister<float>::size();
#else /* !JUCE_USE_SIMD */
using IIRfloat = float;
static constexpr int IIRfloat_elements = 1;
#endif /* JUCE_USE_SIMD */

#define ProcessorClass MultiEQAudioProcessor

//==============================================================================
class MultiEQAudioProcessor
    : public AudioProcessorBase<IOTypes::AudioChannels<64>, IOTypes::AudioChannels<64>>
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    //==============================================================================
    MultiEQAudioProcessor();
    ~MultiEQAudioProcessor() override;

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

    void updateFilterCoefficients (double sampleRate);
    void copyFilterCoefficientsToProcessor();

    IIR::Coefficients<double>::Ptr getCoefficientsForGui (const int filterIndex)
    {
        return guiCoefficients[filterIndex];
    };
    void updateGuiCoefficients();

    // FV repaint flag
    juce::Atomic<bool> repaintFV = true;

private:
    enum class RegularFilterType
    {
        FirstOrderHighPass,
        SecondOrderHighPass,
        LowShelf,
        PeakFilter,
        HighShelf,
        FirstOrderLowPass,
        SecondOrderLowPass,
        NothingToDo
    };

    enum class SpecialFilterType
    {
        FirstOrderHighPass,
        SecondOrderHighPass,
        LinkwitzRileyHighPass,
        LowShelf,
        FirstOrderLowPass,
        SecondOrderLowPass,
        LinkwitzRileyLowPass,
        HighShelf
    };

    void createLinkwitzRileyFilter (const bool isUpperBand);
    void createFilterCoefficients (const int filterIndex, const double sampleRate);

    inline void clear (juce::dsp::AudioBlock<IIRfloat>& ab);

    inline juce::dsp::IIR::Coefficients<float>::Ptr
        createFilterCoefficients (const RegularFilterType type,
                                  const double sampleRate,
                                  const float frequency,
                                  const float Q,
                                  const float gain);

    inline juce::dsp::IIR::Coefficients<double>::Ptr
        createFilterCoefficientsForGui (const RegularFilterType type,
                                        const double sampleRate,
                                        const float frequency,
                                        const float Q,
                                        const float gain);

    // filter dummy for GUI
    IIR::Coefficients<double>::Ptr guiCoefficients[numFilterBands];

    IIR::Coefficients<float>::Ptr processorCoefficients[numFilterBands];
    IIR::Coefficients<float>::Ptr additionalProcessorCoefficients[2];

    IIR::Coefficients<float>::Ptr tempCoefficients[numFilterBands];
    IIR::Coefficients<float>::Ptr additionalTempCoefficients[2];

    // data for interleaving audio
    juce::HeapBlock<char> interleavedBlockData[16], zeroData; //todo: dynamically?
    juce::OwnedArray<juce::dsp::AudioBlock<IIRfloat>> interleavedData;
    juce::dsp::AudioBlock<float> zero;

    // list of used audio parameters
    std::atomic<float>* inputChannelsSetting;
    std::atomic<float>* filterEnabled[numFilterBands];
    std::atomic<float>* filterType[numFilterBands];
    std::atomic<float>* filterFrequency[numFilterBands];
    std::atomic<float>* filterQ[numFilterBands];
    std::atomic<float>* filterGain[numFilterBands];

    // filters for processing
    juce::OwnedArray<IIR::Filter<IIRfloat>> filterArrays[numFilterBands];
    juce::OwnedArray<IIR::Filter<IIRfloat>> additionalFilterArrays[2];

    juce::Atomic<bool> userHasChangedFilterSettings = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEQAudioProcessor)
};
