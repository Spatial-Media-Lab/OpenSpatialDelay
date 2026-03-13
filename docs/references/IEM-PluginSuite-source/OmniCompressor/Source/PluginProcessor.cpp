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

//==============================================================================
OmniCompressorAudioProcessor::OmniCompressorAudioProcessor() :
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
    parameters.addParameterListener ("orderSetting", this);

    orderSetting = parameters.getRawParameterValue ("orderSetting");
    threshold = parameters.getRawParameterValue ("threshold");
    knee = parameters.getRawParameterValue ("knee");
    outGain = parameters.getRawParameterValue ("outGain");
    ratio = parameters.getRawParameterValue ("ratio");
    attack = parameters.getRawParameterValue ("attack");
    release = parameters.getRawParameterValue ("release");
    lookAhead = parameters.getRawParameterValue ("lookAhead");
    reportLatency = parameters.getRawParameterValue ("reportLatency");
    GR = 0.0f;

    delay.setDelayTime (0.005f);
    grProcessing.setDelayTime (0.005f);
}

OmniCompressorAudioProcessor::~OmniCompressorAudioProcessor()
{
}

//==============================================================================
int OmniCompressorAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int OmniCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OmniCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OmniCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void OmniCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void OmniCompressorAudioProcessor::parameterChanged (const juce::String& parameterID,
                                                     float newValue)
{
    if (parameterID == "orderSetting")
        userChangedIOSettings = true;
}

//==============================================================================
void OmniCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, true);

    RMS.resize (samplesPerBlock);
    allGR.resize (samplesPerBlock);

    gains.setSize (1, samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;

    compressor.prepare (spec);
    grProcessing.prepare (spec);
    spec.numChannels = getTotalNumInputChannels();
    delay.prepare (spec);

    if (*reportLatency >= 0.5f && *lookAhead >= 0.5f)
        setLatencySamples (delay.getDelayInSamples());
    else
        setLatencySamples (0);
}

void OmniCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void OmniCompressorAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting);

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int bufferSize = buffer.getNumSamples();

    const int numCh = juce::jmin (buffer.getNumChannels(),
                                  input.getNumberOfChannels(),
                                  output.getNumberOfChannels());
    //const int ambisonicOrder = juce::jmin(input.getOrder(), output.getOrder());
    const float* bufferReadPtr = buffer.getReadPointer (0);

    const bool useLookAhead = *lookAhead >= 0.5f;

    if (*ratio > 15.9f)
        compressor.setRatio (INFINITY);
    else
        compressor.setRatio (*ratio);

    compressor.setKnee (*knee);

    compressor.setAttackTime (*attack * 0.001f);

    compressor.setReleaseTime (*release * 0.001f);
    compressor.setThreshold (*threshold);
    compressor.setMakeUpGain (*outGain);

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (useLookAhead)
    {
        compressor.getGainFromSidechainSignalInDecibelsWithoutMakeUpGain (bufferReadPtr,
                                                                          gains.getWritePointer (0),
                                                                          bufferSize);
        maxGR = juce::FloatVectorOperations::findMinimum (gains.getWritePointer (0), bufferSize);

        // delay input signal
        {
            juce::dsp::AudioBlock<float> ab (buffer);
            juce::dsp::ProcessContextReplacing<float> context (ab);
            delay.process (context);
        }

        grProcessing.pushSamples (gains.getReadPointer (0), bufferSize);
        grProcessing.process();
        grProcessing.readSamples (gains.getWritePointer (0), bufferSize);

        // convert from decibels to gain values
        for (int i = 0; i < bufferSize; ++i)
        {
            gains.setSample (0,
                             i,
                             juce::Decibels::decibelsToGain (gains.getSample (0, i) + *outGain));
        }
    }
    else
    {
        compressor.getGainFromSidechainSignal (bufferReadPtr,
                                               gains.getWritePointer (0),
                                               bufferSize);
        maxGR =
            juce::Decibels::gainToDecibels (
                juce::FloatVectorOperations::findMinimum (gains.getWritePointer (0), bufferSize))
            - *outGain;
    }

    maxRMS = compressor.getMaxLevelInDecibels();

    for (int channel = 0; channel < numCh; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        juce::FloatVectorOperations::multiply (channelData, gains.getWritePointer (0), bufferSize);
    }
}

//==============================================================================
bool OmniCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OmniCompressorAudioProcessor::createEditor()
{
    return new OmniCompressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void OmniCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OmniCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    OmniCompressorAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "orderSetting",
        "Ambisonics Order",
        "",
        juce::NormalisableRange<float> (0.0f, 8.0f, 1.0f),
        0.0f,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "0th";
            else if (value >= 1.5f && value < 2.5f)
                return "1st";
            else if (value >= 2.5f && value < 3.5f)
                return "2nd";
            else if (value >= 3.5f && value < 4.5f)
                return "3rd";
            else if (value >= 4.5f && value < 5.5f)
                return "4th";
            else if (value >= 5.5f && value < 6.5f)
                return "5th";
            else if (value >= 6.5f && value < 7.5f)
                return "6th";
            else if (value >= 7.5f)
                return "7th";
            else
                return "Auto";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "useSN3D",
        "Normalization",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "SN3D";
            else
                return "N3D";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "threshold",
        "Threshold",
        "dB",
        juce::NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
        -10.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "knee",
        "Knee",
        "dB",
        juce::NormalisableRange<float> (0.0f, 30.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "attack",
        "Attack Time",
        "ms",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "release",
        "Release Time",
        "ms",
        juce::NormalisableRange<float> (0.0f, 500.0f, 0.1f),
        150.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "ratio",
        "Ratio",
        " : 1",
        juce::NormalisableRange<float> (1.0f, 16.0f, .2f),
        4.0,
        [] (float value)
        {
            if (value > 15.9f)
                return juce::String ("inf");
            return juce::String (value, 1);
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "outGain",
        "MakeUp Gain",
        "dB",
        juce::NormalisableRange<float> (-10.0f, 20.0f, 0.1f),
        0.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "lookAhead",
        "LookAhead",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON (5ms)" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "reportLatency",
        "Report Latency to DAW",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "Yes";
            else
                return "No";
        },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmniCompressorAudioProcessor();
}
