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

#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr int filterTypePresets[] = { 1, 1, 1, 1, 1, 3 };
static constexpr float filterFrequencyPresets[] = { 20.0f,   120.0f,  500.0f,
                                                    2200.0f, 8000.0f, 16000.0f };

//==============================================================================
MultiEQAudioProcessor::MultiEQAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::discreteChannels (64), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::discreteChannels (64), true)
    #endif
            ,
#endif
        createParameterLayout())
{
    // get pointers to the parameters
    inputChannelsSetting = parameters.getRawParameterValue ("inputChannelsSetting");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputChannelsSetting", this);

    for (int i = 0; i < numFilterBands; ++i)
    {
        filterEnabled[i] = parameters.getRawParameterValue ("filterEnabled" + juce::String (i));
        filterType[i] = parameters.getRawParameterValue ("filterType" + juce::String (i));
        filterFrequency[i] = parameters.getRawParameterValue ("filterFrequency" + juce::String (i));
        filterQ[i] = parameters.getRawParameterValue ("filterQ" + juce::String (i));
        filterGain[i] = parameters.getRawParameterValue ("filterGain" + juce::String (i));

        parameters.addParameterListener ("filterType" + juce::String (i), this);
        parameters.addParameterListener ("filterFrequency" + juce::String (i), this);
        parameters.addParameterListener ("filterQ" + juce::String (i), this);
        parameters.addParameterListener ("filterGain" + juce::String (i), this);
    }

    additionalTempCoefficients[0] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    additionalTempCoefficients[1] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);

    for (int i = 0; i < numFilterBands; ++i)
    {
        createFilterCoefficients (i, 48000.0);
    }

    for (int i = 0; i < numFilterBands; ++i)
    {
        processorCoefficients[i] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    }

    additionalProcessorCoefficients[0] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);
    additionalProcessorCoefficients[1] = IIR::Coefficients<float>::makeAllPass (48000.0, 20.0f);

    copyFilterCoefficientsToProcessor();

    for (int i = 0; i < numFilterBands; ++i)
    {
        filterArrays[i].clear();
        for (int ch = 0; ch < ceil (64 / IIRfloat_elements); ++ch)
            filterArrays[i].add (new IIR::Filter<IIRfloat> (processorCoefficients[i]));
    }

    additionalFilterArrays[0].clear();
    for (int ch = 0; ch < ceil (64 / IIRfloat_elements); ++ch)
        additionalFilterArrays[0].add (
            new IIR::Filter<IIRfloat> (additionalProcessorCoefficients[0]));

    additionalFilterArrays[1].clear();
    for (int ch = 0; ch < ceil (64 / IIRfloat_elements); ++ch)
        additionalFilterArrays[1].add (
            new IIR::Filter<IIRfloat> (additionalProcessorCoefficients[1]));
}

MultiEQAudioProcessor::~MultiEQAudioProcessor()
{
}

void MultiEQAudioProcessor::updateGuiCoefficients()
{
    const double sampleRate = getSampleRate() == 0 ? 48000.0 : getSampleRate();

    // Low band
    const auto lowBandFrequency =
        juce::jmin (static_cast<float> (0.5 * sampleRate), filterFrequency[0]->load());
    const SpecialFilterType lowType = SpecialFilterType (static_cast<int> (*filterType[0]));

    switch (lowType)
    {
        case SpecialFilterType::LinkwitzRileyHighPass:
        {
            auto coeffs = IIR::Coefficients<double>::makeHighPass (sampleRate, lowBandFrequency);
            coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (
                coeffs->coefficients,
                coeffs->coefficients);
            guiCoefficients[0] = coeffs;
            break;
        }
        case SpecialFilterType::FirstOrderHighPass:
            guiCoefficients[0] =
                IIR::Coefficients<double>::makeFirstOrderHighPass (sampleRate, lowBandFrequency);
            break;
        case SpecialFilterType::SecondOrderHighPass:
            guiCoefficients[0] =
                IIR::Coefficients<double>::makeHighPass (sampleRate, lowBandFrequency, *filterQ[0]);
            break;
        case SpecialFilterType::LowShelf:
            guiCoefficients[0] = IIR::Coefficients<double>::makeLowShelf (
                sampleRate,
                lowBandFrequency,
                *filterQ[0],
                juce::Decibels::decibelsToGain (filterGain[0]->load()));
            break;
        default:
            break;
    }

    // High band
    const auto highBandFrequency = juce::jmin (static_cast<float> (0.5 * sampleRate),
                                               filterFrequency[numFilterBands - 1]->load());
    const SpecialFilterType highType =
        SpecialFilterType (4 + static_cast<int> (*filterType[numFilterBands - 1]));

    switch (highType)
    {
        case SpecialFilterType::LinkwitzRileyLowPass:
        {
            auto coeffs = IIR::Coefficients<double>::makeLowPass (sampleRate, highBandFrequency);
            coeffs->coefficients = FilterVisualizerHelper<double>::cascadeSecondOrderCoefficients (
                coeffs->coefficients,
                coeffs->coefficients);
            guiCoefficients[numFilterBands - 1] = coeffs;
            break;
        }
        case SpecialFilterType::FirstOrderLowPass:
            guiCoefficients[numFilterBands - 1] =
                IIR::Coefficients<double>::makeFirstOrderLowPass (sampleRate, highBandFrequency);
            break;
        case SpecialFilterType::SecondOrderLowPass:
            guiCoefficients[numFilterBands - 1] =
                IIR::Coefficients<double>::makeLowPass (sampleRate,
                                                        highBandFrequency,
                                                        *filterQ[numFilterBands - 1]);
            break;
        case SpecialFilterType::HighShelf:
            guiCoefficients[numFilterBands - 1] = IIR::Coefficients<double>::makeHighShelf (
                sampleRate,
                highBandFrequency,
                *filterQ[numFilterBands - 1],
                juce::Decibels::decibelsToGain (filterGain[numFilterBands - 1]->load()));
            break;
        default:
            break;
    }

    // regular bands

    for (int f = 1; f < numFilterBands - 1; ++f)
    {
        const auto frequency =
            juce::jmin (static_cast<float> (0.5 * sampleRate), filterFrequency[f]->load());
        const RegularFilterType type = RegularFilterType (2 + static_cast<int> (*filterType[f]));
        switch (type)
        {
            case RegularFilterType::LowShelf:
                guiCoefficients[f] = IIR::Coefficients<double>::makeLowShelf (
                    sampleRate,
                    frequency,
                    *filterQ[f],
                    juce::Decibels::decibelsToGain (filterGain[f]->load()));
                break;
            case RegularFilterType::PeakFilter:
                guiCoefficients[f] = IIR::Coefficients<double>::makePeakFilter (
                    sampleRate,
                    frequency,
                    *filterQ[f],
                    juce::Decibels::decibelsToGain (filterGain[f]->load()));
                break;
            case RegularFilterType::HighShelf:
                guiCoefficients[f] = IIR::Coefficients<double>::makeHighShelf (
                    sampleRate,
                    frequency,
                    *filterQ[f],
                    juce::Decibels::decibelsToGain (filterGain[f]->load()));
                break;
            default:
                break;
        }
    }
}

inline juce::dsp::IIR::Coefficients<float>::Ptr
    MultiEQAudioProcessor::createFilterCoefficients (const RegularFilterType type,
                                                     const double sampleRate,
                                                     const float frequency,
                                                     const float Q,
                                                     const float gain)
{
    const auto f = juce::jmin (static_cast<float> (0.5 * sampleRate), frequency);
    switch (type)
    {
        case RegularFilterType::FirstOrderHighPass:
            return IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, f);
            break;
        case RegularFilterType::SecondOrderHighPass:
            return IIR::Coefficients<float>::makeHighPass (sampleRate, f, Q);
            break;
        case RegularFilterType::LowShelf:
            return IIR::Coefficients<float>::makeLowShelf (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::PeakFilter:
            return IIR::Coefficients<float>::makePeakFilter (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::HighShelf:
            return IIR::Coefficients<float>::makeHighShelf (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::FirstOrderLowPass:
            return IIR::Coefficients<float>::makeFirstOrderLowPass (sampleRate, f);
            break;
        case RegularFilterType::SecondOrderLowPass:
            return IIR::Coefficients<float>::makeLowPass (sampleRate, f, Q);
            break;
        default:
            return IIR::Coefficients<float>::makeAllPass (sampleRate, f, Q);
            break;
    }
}

inline juce::dsp::IIR::Coefficients<double>::Ptr
    MultiEQAudioProcessor::createFilterCoefficientsForGui (const RegularFilterType type,
                                                           const double sampleRate,
                                                           const float frequency,
                                                           const float Q,
                                                           const float gain)
{
    const auto f = juce::jmin (static_cast<float> (0.5 * sampleRate), frequency);
    switch (type)
    {
        case RegularFilterType::FirstOrderHighPass:
            return IIR::Coefficients<double>::makeFirstOrderHighPass (sampleRate, f);
            break;
        case RegularFilterType::SecondOrderHighPass:
            return IIR::Coefficients<double>::makeHighPass (sampleRate, f, Q);
            break;
        case RegularFilterType::LowShelf:
            return IIR::Coefficients<double>::makeLowShelf (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::PeakFilter:
            return IIR::Coefficients<double>::makePeakFilter (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::HighShelf:
            return IIR::Coefficients<double>::makeHighShelf (sampleRate, f, Q, gain);
            break;
        case RegularFilterType::FirstOrderLowPass:
            return IIR::Coefficients<double>::makeFirstOrderLowPass (sampleRate, f);
            break;
        case RegularFilterType::SecondOrderLowPass:
            return IIR::Coefficients<double>::makeLowPass (sampleRate, f, Q);
            break;
        default:
            return IIR::Coefficients<double>::makeAllPass (sampleRate, f, Q);
            break;
    }
}

void MultiEQAudioProcessor::createLinkwitzRileyFilter (const bool isUpperBand)
{
    if (isUpperBand)
    {
        const auto frequency = juce::jmin (static_cast<float> (0.5 * getSampleRate()),
                                           filterFrequency[numFilterBands - 1]->load());
        tempCoefficients[numFilterBands - 1] =
            IIR::Coefficients<float>::makeLowPass (getSampleRate(),
                                                   frequency,
                                                   *filterQ[numFilterBands - 1]);
        additionalTempCoefficients[1] = processorCoefficients[numFilterBands - 1];
    }
    else
    {
        const auto frequency =
            juce::jmin (static_cast<float> (0.5 * getSampleRate()), filterFrequency[0]->load());
        tempCoefficients[0] =
            IIR::Coefficients<float>::makeHighPass (getSampleRate(), frequency, *filterQ[0]);
        additionalTempCoefficients[0] = processorCoefficients[0];
    }
}

void MultiEQAudioProcessor::createFilterCoefficients (const int filterIndex,
                                                      const double sampleRate)
{
    const int type = juce::roundToInt (filterType[filterIndex]->load());
    if (filterIndex == 0 && type == 2)
    {
        createLinkwitzRileyFilter (false);
    }
    else if (filterIndex == numFilterBands - 1 && type == 2)
    {
        createLinkwitzRileyFilter (true);
    }
    else
    {
        RegularFilterType filterType = RegularFilterType::NothingToDo;
        switch (filterIndex)
        {
            case 0:
                jassert (type != 2);
                if (type == 0)
                    filterType = RegularFilterType::FirstOrderHighPass;
                else if (type == 3)
                    filterType = RegularFilterType::LowShelf;
                else if (type == 1)
                    filterType = RegularFilterType::SecondOrderHighPass;
                break;

            case numFilterBands - 1:
                jassert (type != 2);
                if (type == 0)
                    filterType = RegularFilterType::FirstOrderLowPass;
                else if (type == 3)
                    filterType = RegularFilterType::HighShelf;
                else if (type == 1)
                    filterType = RegularFilterType::SecondOrderLowPass;
                break;

            default:
                jassert (type <= 2);
                switch (type)
                {
                    case 0:
                        filterType = RegularFilterType::LowShelf;
                        break;
                    case 1:
                        filterType = RegularFilterType::PeakFilter;
                        break;
                    case 2:
                        filterType = RegularFilterType::HighShelf;
                        break;
                    default:
                        break;
                }
                break;
        }
        tempCoefficients[filterIndex] = createFilterCoefficients (
            filterType,
            sampleRate,
            *filterFrequency[filterIndex],
            *filterQ[filterIndex],
            juce::Decibels::decibelsToGain (filterGain[filterIndex]->load()));
    }
}

void MultiEQAudioProcessor::copyFilterCoefficientsToProcessor()
{
    for (int b = 0; b < numFilterBands; ++b)
        *processorCoefficients[b] = *tempCoefficients[b];

    *additionalProcessorCoefficients[0] = *additionalTempCoefficients[0];
    *additionalProcessorCoefficients[1] = *additionalTempCoefficients[1];

    userHasChangedFilterSettings = false;
}

inline void MultiEQAudioProcessor::clear (juce::dsp::AudioBlock<IIRfloat>& ab)
{
    const int N = static_cast<int> (ab.getNumSamples()) * IIRfloat_elements;
    const int nCh = static_cast<int> (ab.getNumChannels());

    for (int ch = 0; ch < nCh; ++ch)
        juce::FloatVectorOperations::clear (reinterpret_cast<float*> (ab.getChannelPointer (ch)),
                                            N);
}

//==============================================================================
int MultiEQAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int MultiEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, true);

    for (int f = 0; f < numFilterBands; ++f)
    {
        createFilterCoefficients (f, sampleRate);
    }
    copyFilterCoefficientsToProcessor();

    interleavedData.clear();

    for (int i = 0; i < ceil (64 / IIRfloat_elements); ++i)
    {
        // reset filters
        for (int f = 0; f < numFilterBands; ++f)
        {
            filterArrays[f][i]->reset (IIRfloat (0.0f));
        }

        interleavedData.add (
            new juce::dsp::AudioBlock<IIRfloat> (interleavedBlockData[i], 1, samplesPerBlock));
        //interleavedData.getLast()->clear(); // this one's broken in JUCE 5.4.5
        clear (*interleavedData.getLast());
    }

    zero = juce::dsp::AudioBlock<float> (zeroData, IIRfloat_elements, samplesPerBlock);
    zero.clear();
}

void MultiEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MultiEQAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *inputChannelsSetting, *inputChannelsSetting, false);
    juce::ScopedNoDenormals noDenormals;

    const int L = buffer.getNumSamples();

    const int maxNChIn = juce::jmin (buffer.getNumChannels(), input.getSize());
    if (maxNChIn < 1)
        return;

    const int nSIMDFilters = 1 + (maxNChIn - 1) / IIRfloat_elements;

    // update iir filter coefficients
    if (userHasChangedFilterSettings.get())
        copyFilterCoefficientsToProcessor();

    using Format = juce::AudioData::Format<juce::AudioData::Float32, juce::AudioData::NativeEndian>;

    //interleave input data
    int partial = maxNChIn % IIRfloat_elements;
    if (partial == 0)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            juce::AudioData::interleaveSamples (
                juce::AudioData::NonInterleavedSource<Format> { buffer.getArrayOfReadPointers()
                                                                    + i * IIRfloat_elements,
                                                                IIRfloat_elements },
                juce::AudioData::InterleavedDest<Format> {
                    reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                    IIRfloat_elements },
                L);
        }
    }
    else
    {
        int i;
        for (i = 0; i < nSIMDFilters - 1; ++i)
        {
            juce::AudioData::interleaveSamples (
                juce::AudioData::NonInterleavedSource<Format> { buffer.getArrayOfReadPointers()
                                                                    + i * IIRfloat_elements,
                                                                IIRfloat_elements },
                juce::AudioData::InterleavedDest<Format> {
                    reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                    IIRfloat_elements },
                L);
        }

        const float* addr[IIRfloat_elements];
        int ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getReadPointer (i * IIRfloat_elements + ch);
        }
        for (; ch < IIRfloat_elements; ++ch)
        {
            addr[ch] = zero.getChannelPointer (ch);
        }
        juce::AudioData::interleaveSamples (
            juce::AudioData::NonInterleavedSource<Format> { addr, IIRfloat_elements },
            juce::AudioData::InterleavedDest<Format> {
                reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                IIRfloat_elements },
            L);
    }

    // apply filters
    for (int f = 0; f < numFilterBands; ++f)
    {
        if (*filterEnabled[f] > 0.5f)
        {
            for (int i = 0; i < nSIMDFilters; ++i)
            {
                const IIRfloat* chPtr[1] = { interleavedData[i]->getChannelPointer (0) };
                juce::dsp::AudioBlock<IIRfloat> ab (const_cast<IIRfloat**> (chPtr), 1, L);
                juce::dsp::ProcessContextReplacing<IIRfloat> context (ab);
                filterArrays[f][i]->process (context);
            }
        }
    }

    // check and apply additional filters (Linkwitz Riley -> two BiQuads)
    if (static_cast<int> (*filterType[0]) == 2 && *filterEnabled[0] > 0.5f)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            const IIRfloat* chPtr[1] = { chPtr[0] = interleavedData[i]->getChannelPointer (0) };
            juce::dsp::AudioBlock<IIRfloat> ab (const_cast<IIRfloat**> (chPtr), 1, L);
            juce::dsp::ProcessContextReplacing<IIRfloat> context (ab);
            additionalFilterArrays[0][i]->process (context);
        }
    }
    if (static_cast<int> (*filterType[numFilterBands - 1]) == 2
        && *filterEnabled[numFilterBands - 1] > 0.5f)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            const IIRfloat* chPtr[1] = { interleavedData[i]->getChannelPointer (0) };
            juce::dsp::AudioBlock<IIRfloat> ab (const_cast<IIRfloat**> (chPtr), 1, L);
            juce::dsp::ProcessContextReplacing<IIRfloat> context (ab);
            additionalFilterArrays[1][i]->process (context);
        }
    }

    // deinterleave
    if (partial == 0)
    {
        for (int i = 0; i < nSIMDFilters; ++i)
        {
            juce::AudioData::deinterleaveSamples (
                juce::AudioData::InterleavedSource<Format> {
                    reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                    IIRfloat_elements },
                juce::AudioData::NonInterleavedDest<Format> { buffer.getArrayOfWritePointers()
                                                                  + i * IIRfloat_elements,
                                                              IIRfloat_elements },
                L);
        }
    }
    else
    {
        int i;
        for (i = 0; i < nSIMDFilters - 1; ++i)
        {
            juce::AudioData::deinterleaveSamples (
                juce::AudioData::InterleavedSource<Format> {
                    reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                    IIRfloat_elements },
                juce::AudioData::NonInterleavedDest<Format> { buffer.getArrayOfWritePointers()
                                                                  + i * IIRfloat_elements,
                                                              IIRfloat_elements },
                L);
        }

        float* addr[IIRfloat_elements];
        int ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getWritePointer (i * IIRfloat_elements + ch);
        }
        for (; ch < IIRfloat_elements; ++ch)
        {
            addr[ch] = zero.getChannelPointer (ch);
        }
        juce::AudioData::deinterleaveSamples (
            juce::AudioData::InterleavedSource<Format> {
                reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)),
                IIRfloat_elements },
            juce::AudioData::NonInterleavedDest<Format> { addr, IIRfloat_elements },
            L);
        zero.clear();
    }
}

//==============================================================================
bool MultiEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiEQAudioProcessor::createEditor()
{
    return new MultiEQAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MultiEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MultiEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (
                    parameters.state.getProperty ("OSCPort", juce::var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
        }
}

//==============================================================================
void MultiEQAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputChannelsSetting")
        userChangedIOSettings = true;
    else if (parameterID.startsWith ("filter"))
    {
        const int i = parameterID.getLastCharacters (1).getIntValue();

        createFilterCoefficients (i, getSampleRate());

        repaintFV = true;
        userHasChangedFilterSettings = true;
    }
}

void MultiEQAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    MultiEQAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "inputChannelsSetting",
        "Number of input channels ",
        "",
        juce::NormalisableRange<float> (0.0f, 64.0f, 1.0f),
        0.0f,
        [] (float value) { return value < 0.5f ? "Auto" : juce::String (value); },
        nullptr));

    int i = 0;
    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterEnabled" + juce::String (i),
        "Filter Enablement " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value) { return value < 0.5 ? juce::String ("OFF") : juce::String ("ON"); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterType" + juce::String (i),
        "Filter Type " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.0f, 3.0f, 1.0f),
        filterTypePresets[i],
        [] (float value)
        {
            if (value < 0.5f)
                return "HP (6dB/oct)";
            else if (value >= 0.5f && value < 1.5f)
                return "HP (12dB/oct)";
            else if (value >= 1.5f && value < 2.5f)
                return "HP (24dB/oct)";
            else
                return "Low-shelf";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterFrequency" + juce::String (i),
        "Filter Frequency " + juce::String (i + 1),
        "Hz",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f),
        filterFrequencyPresets[i],
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterQ" + juce::String (i),
        "Filter Q " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.05f, 8.0f, 0.05f),
        0.7f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterGain" + juce::String (i),
        "Filter Gain " + juce::String (i + 1),
        "dB",
        juce::NormalisableRange<float> (-60.0f, 15.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    for (int i = 1; i < numFilterBands - 1; ++i)
    {
        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "filterEnabled" + juce::String (i),
            "Filter Enablement " + juce::String (i + 1),
            "",
            juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
            1.0f,
            [] (float value) { return value < 0.5 ? juce::String ("OFF") : juce::String ("ON"); },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "filterType" + juce::String (i),
            "Filter Type " + juce::String (i + 1),
            "",
            juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
            filterTypePresets[i],
            [] (float value)
            {
                if (value < 0.5f)
                    return "Low-shelf";
                else if (value >= 0.5f && value < 1.5f)
                    return "Peak";
                else
                    return "High-shelf";
            },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "filterFrequency" + juce::String (i),
            "Filter Frequency " + juce::String (i + 1),
            "Hz",
            juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f),
            filterFrequencyPresets[i],
            [] (float value) { return juce::String (value, 0); },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "filterQ" + juce::String (i),
            "Filter Q " + juce::String (i + 1),
            "",
            juce::NormalisableRange<float> (0.05f, 8.0f, 0.05f),
            0.7f,
            [] (float value) { return juce::String (value, 2); },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "filterGain" + juce::String (i),
            "Filter Gain " + juce::String (i + 1),
            "dB",
            juce::NormalisableRange<float> (-60.0f, 15.0f, 0.1f),
            0.0f,
            [] (float value) { return juce::String (value, 1); },
            nullptr));
    }

    i = numFilterBands - 1;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterEnabled" + juce::String (i),
        "Filter Enablement " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value) { return value < 0.5 ? juce::String ("OFF") : juce::String ("ON"); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterType" + juce::String (i),
        "Filter Type " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.0f, 3.0f, 1.0f),
        filterTypePresets[i],
        [] (float value)
        {
            if (value < 0.5f)
                return "LP (6dB/Oct)";
            else if (value >= 0.5f && value < 1.5f)
                return "LP (12dB/oct)";
            else if (value >= 1.5f && value < 2.5f)
                return "LP (24dB/oct)";
            else
                return "High-shelf";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterFrequency" + juce::String (i),
        "Filter Frequency " + juce::String (i + 1),
        "Hz",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.4f),
        filterFrequencyPresets[i],
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterQ" + juce::String (i),
        "Filter Q " + juce::String (i + 1),
        "",
        juce::NormalisableRange<float> (0.05f, 8.0f, 0.05f),
        0.7f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "filterGain" + juce::String (i),
        "Filter Gain " + juce::String (i + 1),
        "dB",
        juce::NormalisableRange<float> (-60.0f, 15.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiEQAudioProcessor();
}
