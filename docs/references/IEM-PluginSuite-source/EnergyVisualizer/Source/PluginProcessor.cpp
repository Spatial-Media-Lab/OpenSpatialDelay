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
EnergyVisualizerAudioProcessor::EnergyVisualizerAudioProcessor() :
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
        createParameterLayout()),
    decoderMatrix (nSamplePoints, 64)
{
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    peakLevel = parameters.getRawParameterValue ("peakLevel");
    dynamicRange = parameters.getRawParameterValue ("dynamicRange");
    holdMax = parameters.getRawParameterValue ("holdMax");
    RMStimeConstant = parameters.getRawParameterValue ("RMStimeConstant");

    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("RMStimeConstant", this);

    for (int point = 0; point < nSamplePoints; ++point)
    {
        auto* matrixRowPtr = decoderMatrix.getRawDataPointer() + point * 64;
        SHEval (7,
                hammerAitovSampleX[point],
                hammerAitovSampleY[point],
                hammerAitovSampleZ[point],
                matrixRowPtr,
                false);
        juce::FloatVectorOperations::multiply (
            matrixRowPtr,
            matrixRowPtr,
            sn3d2n3d,
            64); //expecting sn3d normalization -> converting it to handle n3d
    }
    decoderMatrix *= 1.0f / decodeCorrection (7); // revert 7th order correction

    rms.resize (nSamplePoints);
    std::fill (rms.begin(), rms.end(), 0.0f);

    weights.resize (64);

    startTimer (200);
}

EnergyVisualizerAudioProcessor::~EnergyVisualizerAudioProcessor()
{
}

//==============================================================================
int EnergyVisualizerAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int EnergyVisualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnergyVisualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EnergyVisualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnergyVisualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EnergyVisualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, 0, true);

    timeConstant = exp (-1.0 / (sampleRate * (*RMStimeConstant / 1000) / samplesPerBlock));

    sampledSignal.resize (samplesPerBlock);
    std::fill (rms.begin(), rms.end(), 0.0f);
}

void EnergyVisualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void EnergyVisualizerAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                                   juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    checkInputAndOutput (this, *orderSetting, 0);

    if (! doProcessing.get() && ! oscParameterInterface.getOSCSender().isConnected())
        return;

    //const int nCh = buffer.getNumChannels();
    const int L = buffer.getNumSamples();
    const int workingOrder = juce::jmin (isqrt (buffer.getNumChannels()) - 1, input.getOrder());

    const int nCh = squares[workingOrder + 1];

    copyMaxRE (workingOrder, weights.data());
    juce::FloatVectorOperations::multiply (weights.data(),
                                           maxRECorrection[workingOrder]
                                               * decodeCorrection (workingOrder),
                                           nCh);

    if (*useSN3D < 0.5f)
        juce::FloatVectorOperations::multiply (weights.data(), n3d2sn3d, nCh);

    const float oneMinusTimeConstant = 1.0f - timeConstant;
    for (int i = 0; i < nSamplePoints; ++i)
    {
        juce::FloatVectorOperations::copyWithMultiply (sampledSignal.data(),
                                                       buffer.getReadPointer (0),
                                                       decoderMatrix (i, 0) * weights[0],
                                                       buffer.getNumSamples());
        for (int ch = 1; ch < nCh; ++ch)
            juce::FloatVectorOperations::addWithMultiply (sampledSignal.data(),
                                                          buffer.getReadPointer (ch),
                                                          decoderMatrix (i, ch) * weights[ch],
                                                          L);

        // calculate rms
        float sum = 0.0f;
        for (int i = 0; i < L; ++i)
        {
            const auto sample = sampledSignal[i];
            sum += sample * sample;
        }

        rms[i] = timeConstant * rms[i] + oneMinusTimeConstant * std::sqrt (sum / L);
    }
}

//==============================================================================
bool EnergyVisualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EnergyVisualizerAudioProcessor::createEditor()
{
    return new EnergyVisualizerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void EnergyVisualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EnergyVisualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void EnergyVisualizerAudioProcessor::parameterChanged (const juce::String& parameterID,
                                                       float newValue)
{
    if (parameterID == "orderSetting")
        userChangedIOSettings = true;
    if (parameterID == "RMStimeConstant")
        timeConstant = exp (-1.0 / (getSampleRate() * (*RMStimeConstant / 1000) / getBlockSize()));
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    EnergyVisualizerAudioProcessor::createParameterLayout()
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
        "peakLevel",
        "Peak level",
        "dB",
        juce::NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "dynamicRange",
        "Dynamic Range",
        "dB",
        juce::NormalisableRange<float> (10.0f, 60.0f, 1.f),
        35.0f,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "holdMax",
        "Hold maximal RMS value",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "ON";
            else
                return "OFF";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "RMStimeConstant",
        "RMS time constant",
        "ms",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f, 0.4f),
        100.0f,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    return params;
}

//==============================================================================
void EnergyVisualizerAudioProcessor::timerCallback()
{
    juce::RelativeTime timeDifference = juce::Time::getCurrentTime() - lastEditorTime.get();
    if (timeDifference.inMilliseconds() > 800)
        doProcessing = false;
    else
        doProcessing = true;
}

//==============================================================================
void EnergyVisualizerAudioProcessor::sendAdditionalOSCMessages (
    juce::OSCSender& oscSender,
    const juce::OSCAddressPattern& address)
{
    juce::OSCMessage message (address.toString() + "/RMS");
    for (int i = 0; i < nSamplePoints; ++i)
        message.addFloat32 (rms[i]);
    oscSender.send (message);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnergyVisualizerAudioProcessor();
}
