/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
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

//==============================================================================
FdnReverbAudioProcessor::FdnReverbAudioProcessor() :
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
    parameters.addParameterListener ("delayLength", this);
    parameters.addParameterListener ("revTime", this);
    parameters.addParameterListener ("fadeInTime", this);
    parameters.addParameterListener ("highCutoff", this);
    parameters.addParameterListener ("highQ", this);
    parameters.addParameterListener ("highGain", this);
    parameters.addParameterListener ("lowCutoff", this);
    parameters.addParameterListener ("lowQ", this);
    parameters.addParameterListener ("lowGain", this);
    parameters.addParameterListener ("dryWet", this);
    parameters.addParameterListener ("fdnSize", this);

    delayLength = parameters.getRawParameterValue ("delayLength");
    revTime = parameters.getRawParameterValue ("revTime");
    fadeInTime = parameters.getRawParameterValue ("fadeInTime");
    highCutoff = parameters.getRawParameterValue ("highCutoff");
    highQ = parameters.getRawParameterValue ("highQ");
    highGain = parameters.getRawParameterValue ("highGain");
    lowCutoff = parameters.getRawParameterValue ("lowCutoff");
    lowQ = parameters.getRawParameterValue ("lowQ");
    lowGain = parameters.getRawParameterValue ("lowGain");
    wet = parameters.getRawParameterValue ("dryWet");

    fdn.setFdnSize (FeedbackDelayNetwork::big);
    fdnFade.setFdnSize (FeedbackDelayNetwork::big);
    fdnFade.setDryWet (1.0f);
}

FdnReverbAudioProcessor::~FdnReverbAudioProcessor()
{
}

//==============================================================================
int FdnReverbAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int FdnReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FdnReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FdnReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FdnReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void FdnReverbAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == "delayLength")
    {
        fdn.setDelayLength (*delayLength);
        fdnFade.setDelayLength (*delayLength);
    }
    else if (parameterID == "revTime")
        fdn.setT60InSeconds (*revTime);
    else if (parameterID == "fadeInTime")
        fdnFade.setT60InSeconds (*fadeInTime);
    else if (parameterID == "dryWet")
        fdn.setDryWet (*wet);
    else if (parameterID == "fdnSize")
    {
        FeedbackDelayNetwork::FdnSize size { FeedbackDelayNetwork::FdnSize::big };
        if (newValue == 0.0f)
            size = FeedbackDelayNetwork::FdnSize::tiny;
        else if (newValue == 1.0f)
            size = FeedbackDelayNetwork::FdnSize::small;

        fdn.setFdnSize (size);
        fdnFade.setFdnSize (size);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = getSampleRate();
        spec.maximumBlockSize = getBlockSize();
        spec.numChannels = 64;
        fdn.prepare (spec);
        fdnFade.prepare (spec);
    }
    else
    {
        updateFilterParameters();
    }
}

void FdnReverbAudioProcessor::updateFilterParameters()
{
    FeedbackDelayNetwork::FilterParameter lowShelf;
    FeedbackDelayNetwork::FilterParameter highShelf;

    lowShelf.frequency = *lowCutoff;
    lowShelf.q = *lowQ;
    lowShelf.linearGain = juce::Decibels::decibelsToGain (lowGain->load());

    highShelf.frequency = *highCutoff;
    highShelf.q = *highQ;
    highShelf.linearGain = juce::Decibels::decibelsToGain (highGain->load());

    fdn.setFilterParameter (lowShelf, highShelf);
    fdnFade.setFilterParameter (lowShelf, highShelf);
}
//==============================================================================
void FdnReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    updateFilterParameters();

    copyBuffer.setSize (64, samplesPerBlock);
    copyBuffer.clear();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 64;
    fdn.prepare (spec);
    fdnFade.prepare (spec);

    maxPossibleChannels = getTotalNumInputChannels();
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::reset()
{
    fdn.reset();
    fdnFade.reset();
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    const int nChannels = buffer.getNumChannels();
    const int nSamples = buffer.getNumSamples();

    // make copy of input data
    if (*fadeInTime != 0.0f)
    {
        for (int i = 0; i < nChannels; i++)
        {
            copyBuffer.copyFrom (i, 0, buffer, i, 0, nSamples);
        }

        juce::dsp::AudioBlock<float> blockFade (copyBuffer.getArrayOfWritePointers(),
                                                nChannels,
                                                nSamples);
        fdnFade.process (juce::dsp::ProcessContextReplacing<float> (blockFade));
    }
    juce::dsp::AudioBlock<float> block (buffer);
    fdn.process (juce::dsp::ProcessContextReplacing<float> (block));

    if (*fadeInTime != 0.0f)
    {
        for (int i = 0; i < nChannels; i++)
        {
            buffer.addFrom (i, 0, copyBuffer, i, 0, nSamples, -*wet);
        }
    }

    auto fdnSize = fdn.getFdnSize();
    if (fdnSize < nChannels)
    {
        for (int ch = fdnSize; ch < nChannels; ++ch)
            buffer.clear (ch, 0, nSamples);
    }
}

//------------------------------------------------------------------------------
void FdnReverbAudioProcessor::setFreezeMode (bool freezeState)
{
    fdn.setFreeze (freezeState);
}

void FdnReverbAudioProcessor::getT60ForFrequencyArray (double* frequencies,
                                                       double* t60Data,
                                                       size_t numSamples)
{
    fdn.getT60ForFrequencyArray (frequencies, t60Data, numSamples);
}

//==============================================================================
bool FdnReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FdnReverbAudioProcessor::createEditor()
{
    return new FdnReverbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void FdnReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FdnReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    FdnReverbAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "delayLength",
        "Room Size",
        "",
        juce::NormalisableRange<float> (1.0f, 30.0f, 1.0f),
        20.0f,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "revTime",
        "Reverberation Time",
        "s",
        juce::NormalisableRange<float> (0.1f, 9.0f, 0.1f),
        5.f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "lowCutoff",
        "Lows Cutoff Frequency",
        "Hz",
        juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.2f),
        100.f,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "lowQ",
        "Lows Q Factor",
        "",
        juce::NormalisableRange<float> (0.01f, 0.9f, 0.01f),
        0.5f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "lowGain",
        "Lows Gain",
        "dB/s",
        juce::NormalisableRange<float> (-80.0f, 6.0, 0.1f),
        1.f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "highCutoff",
        "Highs Cutoff Frequency",
        "Hz",
        juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.2f),
        2000.f,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "highQ",
        "Highs Q Factor",
        "",
        juce::NormalisableRange<float> (0.01f, 0.9f, 0.01f),
        0.5f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "highGain",
        "Highs Gain",
        "dB/s",
        juce::NormalisableRange<float> (-80.0f, 4.0f, 0.1f),
        -10.f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "dryWet",
        "Dry/Wet",
        "",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f),
        0.5f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "fadeInTime",
        "Fade-in Time",
        "s",
        juce::NormalisableRange<float> (0.0f, 9.0f, 0.01f),
        0.f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "fdnSize",
        "Fdn Size (internal)",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        2.0f,
        [] (float value)
        {
            if (value == 0.0f)
                return "16";
            else if (value == 1.0f)
                return "32";
            else
                return "64";
        },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FdnReverbAudioProcessor();
}
