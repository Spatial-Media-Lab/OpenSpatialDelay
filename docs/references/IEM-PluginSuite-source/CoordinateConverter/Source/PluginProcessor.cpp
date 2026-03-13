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
CoordinateConverterAudioProcessor::CoordinateConverterAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::discreteChannels (10), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::discreteChannels (64), true)
    #endif
            ,
#endif
        createParameterLayout())
{
    // get pointers to the parameters
    azimuth = parameters.getRawParameterValue ("azimuth");
    elevation = parameters.getRawParameterValue ("elevation");
    radius = parameters.getRawParameterValue ("radius");
    xPos = parameters.getRawParameterValue ("xPos");
    yPos = parameters.getRawParameterValue ("yPos");
    zPos = parameters.getRawParameterValue ("zPos");
    xReference = parameters.getRawParameterValue ("xReference");
    yReference = parameters.getRawParameterValue ("yReference");
    zReference = parameters.getRawParameterValue ("zReference");
    radiusRange = parameters.getRawParameterValue ("radiusRange");
    xRange = parameters.getRawParameterValue ("xRange");
    yRange = parameters.getRawParameterValue ("yRange");
    zRange = parameters.getRawParameterValue ("zRange");
    azimuthFlip = parameters.getRawParameterValue ("azimuthFlip");
    elevationFlip = parameters.getRawParameterValue ("elevationFlip");
    radiusFlip = parameters.getRawParameterValue ("radiusFlip");
    xFlip = parameters.getRawParameterValue ("xFlip");
    yFlip = parameters.getRawParameterValue ("yFlip");
    zFlip = parameters.getRawParameterValue ("zFlip");

    // add listeners to parameter changes
    parameters.addParameterListener ("azimuth", this);
    parameters.addParameterListener ("elevation", this);
    parameters.addParameterListener ("radius", this);
    parameters.addParameterListener ("xPos", this);
    parameters.addParameterListener ("yPos", this);
    parameters.addParameterListener ("zPos", this);
    parameters.addParameterListener ("xReference", this);
    parameters.addParameterListener ("yReference", this);
    parameters.addParameterListener ("zReference", this);
    parameters.addParameterListener ("radiusRange", this);
    parameters.addParameterListener ("xRange", this);
    parameters.addParameterListener ("yRange", this);
    parameters.addParameterListener ("zRange", this);
    parameters.addParameterListener ("azimuthFlip", this);
    parameters.addParameterListener ("elevationFlip", this);
    parameters.addParameterListener ("radiusFlip", this);
    parameters.addParameterListener ("xFlip", this);
    parameters.addParameterListener ("yFlip", this);
    parameters.addParameterListener ("zFlip", this);
}

CoordinateConverterAudioProcessor::~CoordinateConverterAudioProcessor()
{
}

//==============================================================================
int CoordinateConverterAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int CoordinateConverterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CoordinateConverterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CoordinateConverterAudioProcessor::getProgramName (int index)
{
    return {};
}

void CoordinateConverterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CoordinateConverterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void CoordinateConverterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void CoordinateConverterAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    // nothing to do
}

//==============================================================================
bool CoordinateConverterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CoordinateConverterAudioProcessor::createEditor()
{
    return new CoordinateConverterAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void CoordinateConverterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CoordinateConverterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
void CoordinateConverterAudioProcessor::parameterChanged (const juce::String& parameterID,
                                                          float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "radius")
    {
        repaintSphere = true;

        if (! updatingParams.get())
            updateCartesianCoordinates();
        return;
    }

    else if (parameterID == "xPos" || parameterID == "yPos" || parameterID == "zPos")
    {
        repaintPositionPlanes = true;

        if (! updatingParams.get())
            updateSphericalCoordinates();
        return;
    }

    else if (parameterID == "xReference" || parameterID == "yReference"
             || parameterID == "zReference" || parameterID == "xRange" || parameterID == "yRange"
             || parameterID == "zRange" || parameterID == "radiusRange")
    {
        if (cartesianWasLastUpdated)
            updateCartesianCoordinates();
        else
            updateSphericalCoordinates();
        return;
    }

    else if (parameterID == "azimuthFlip")
        azimuthFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;
    else if (parameterID == "elevationFlip")
        elevationFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;
    else if (parameterID == "radiusFlip")
        radiusFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;
    else if (parameterID == "xFlip")
        xFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;
    else if (parameterID == "yFlip")
        yFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;
    else if (parameterID == "zFlip")
        zFlipFactor = newValue >= 0.5f ? -1.0f : 1.0f;

    if (cartesianWasLastUpdated)
        updateCartesianCoordinates();
    else
        updateSphericalCoordinates();
}

void CoordinateConverterAudioProcessor::updateCartesianCoordinates()
{
    updatingParams = true;

    auto cartesian = Conversions<float>::sphericalToCartesian (
        juce::degreesToRadians (azimuth->load()) * azimuthFlipFactor,
        juce::degreesToRadians (elevation->load()) * elevationFlipFactor,
        (0.5f - radiusFlipFactor * (0.5f - *radius)) * *radiusRange);

    cartesian += { *xReference, *yReference, *zReference };
    cartesian.x /= *xRange * xFlipFactor;
    cartesian.y /= *yRange * yFlipFactor;
    cartesian.z /= *zRange * zFlipFactor;

    parameters.getParameter ("xPos")->setValueNotifyingHost (
        parameters.getParameterRange ("xPos").convertTo0to1 (cartesian.x));
    parameters.getParameter ("yPos")->setValueNotifyingHost (
        parameters.getParameterRange ("yPos").convertTo0to1 (cartesian.y));
    parameters.getParameter ("zPos")->setValueNotifyingHost (
        parameters.getParameterRange ("zPos").convertTo0to1 (cartesian.z));
    repaintPositionPlanes = true;

    cartesianWasLastUpdated = true;
    updatingParams = false;
}

void CoordinateConverterAudioProcessor::updateSphericalCoordinates()
{
    updatingParams = true;

    auto cartesian = juce::Vector3D<float> (*xPos * *xRange * xFlipFactor,
                                            *yPos * *yRange * yFlipFactor,
                                            *zPos * *zRange * zFlipFactor);

    cartesian -= { *xReference, *yReference, *zReference };
    auto spherical = Conversions<float>::cartesianToSpherical (cartesian);

    spherical.x /= *radiusRange; // radius component

    if (spherical.x >= 1.0f)
        spherical.x = 1.0f;

    spherical.x = 0.5f - radiusFlipFactor * (0.5 - spherical.x);
    spherical.y *= azimuthFlipFactor;
    spherical.z *= elevationFlipFactor;

    parameters.getParameter ("radius")->setValueNotifyingHost (
        parameters.getParameterRange ("radius").convertTo0to1 (spherical.x));
    parameters.getParameter ("azimuth")->setValueNotifyingHost (
        parameters.getParameterRange ("azimuth").convertTo0to1 (spherical.y));
    parameters.getParameter ("elevation")
        ->setValueNotifyingHost (
            parameters.getParameterRange ("elevation").convertTo0to1 (spherical.z));
    repaintPositionPlanes = true;

    cartesianWasLastUpdated = false;
    updatingParams = false;
}

void CoordinateConverterAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    CoordinateConverterAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "azimuth",
        "Azimuth Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "elevation",
        "Elevation Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "radius",
        "Radius",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        1.0,
        [] (float value) { return juce::String (value, 3); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "xPos",
        "X Coordinate",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f),
        1.0,
        [] (float value) { return juce::String (value, 4); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "yPos",
        "Y Coordinate",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f),
        0.0,
        [] (float value) { return juce::String (value, 4); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "zPos",
        "Z Coordinate",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f),
        0.0,
        [] (float value) { return juce::String (value, 4); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "xReference",
        "X Reference",
        "m",
        juce::NormalisableRange<float> (-50.0f, 50.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 3); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "yReference",
        "Y Reference",
        "m",
        juce::NormalisableRange<float> (-50.0f, 50.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 3); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "zReference",
        "Z Reference",
        "m",
        juce::NormalisableRange<float> (-50.0f, 50.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 3); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "radiusRange",
        "Radius juce::Range",
        "m",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f),
        1.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "xRange",
        "X juce::Range",
        "m",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f),
        1.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "yRange",
        "Y juce::Range",
        "m",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f),
        1.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "zRange",
        "Z juce::Range",
        "m",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f),
        1.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "azimuthFlip",
        "Invert Azimuth",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "elevationFlip",
        "Invert Elevation",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "radiusFlip",
        "Invert Radius Axis",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "xFlip",
        "Invert X Axis",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "yFlip",
        "Invert Y Axis",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "zFlip",
        "Invert Z Axis",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CoordinateConverterAudioProcessor();
}
