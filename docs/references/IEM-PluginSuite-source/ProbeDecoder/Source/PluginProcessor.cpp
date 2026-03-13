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
ProbeDecoderAudioProcessor::ProbeDecoderAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::discreteChannels (64), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::mono(), true)
    #endif
            ,
#endif
        createParameterLayout())
{
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    azimuth = parameters.getRawParameterValue ("azimuth");
    elevation = parameters.getRawParameterValue ("elevation");

    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("azimuth", this);
    parameters.addParameterListener ("elevation", this);

    juce::FloatVectorOperations::clear (previousSH, 64);
}

ProbeDecoderAudioProcessor::~ProbeDecoderAudioProcessor() = default;

//==============================================================================

int ProbeDecoderAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ProbeDecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProbeDecoderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ProbeDecoderAudioProcessor::getProgramName (int index)
{
    return juce::String();
}

void ProbeDecoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ProbeDecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, 1, true);
}

void ProbeDecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ProbeDecoderAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *orderSetting, 1);
    const int ambisonicOrder = input.getOrder();
    const int nChannels = juce::jmin (buffer.getNumChannels(), input.getNumberOfChannels());

    juce::Vector3D<float> xyz =
        Conversions<float>::sphericalToCartesian (juce::degreesToRadians (azimuth->load()),
                                                  juce::degreesToRadians (elevation->load()));

    float sh[64];

    SHEval (ambisonicOrder, xyz, sh, false);

    const int nCh = juce::jmin (buffer.getNumChannels(), nChannels);
    const int numSamples = buffer.getNumSamples();

    if (*useSN3D >= 0.5f)
        juce::FloatVectorOperations::multiply (sh, sh, sn3d2n3d, nChannels);

    buffer.applyGainRamp (0, 0, numSamples, previousSH[0], sh[0]);

    for (int i = 1; i < nCh; i++)
    {
        buffer.addFromWithRamp (0, 0, buffer.getReadPointer (i), numSamples, previousSH[i], sh[i]);
        buffer.clear (i, 0, numSamples);
    }

    juce::FloatVectorOperations::copy (previousSH, sh, nChannels);
}

//==============================================================================
bool ProbeDecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ProbeDecoderAudioProcessor::createEditor()
{
    return new ProbeDecoderAudioProcessorEditor (*this, parameters);
}

void ProbeDecoderAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == "orderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "azimuth" || parameterID == "elevation")
    {
        updatedPositionData = true;
    }
}

//==============================================================================
void ProbeDecoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ProbeDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    ProbeDecoderAudioProcessor::createParameterLayout()
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
        "azimuth",
        "Azimuth angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "elevation",
        "Elevation angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProbeDecoderAudioProcessor();
}
