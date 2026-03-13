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
StereoEncoderAudioProcessor::StereoEncoderAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::discreteChannels (64), true)
    #endif
            ,
#endif
        createParameterLayout()),
    posC (1.0f, 0.0f, 0.0f),
    posL (1.0f, 0.0f, 0.0f),
    posR (1.0f, 0.0f, 0.0f),
    updatedPositionData (true)
{
    parameters.addParameterListener ("qw", this);
    parameters.addParameterListener ("qx", this);
    parameters.addParameterListener ("qy", this);
    parameters.addParameterListener ("qz", this);
    parameters.addParameterListener ("azimuth", this);
    parameters.addParameterListener ("elevation", this);
    parameters.addParameterListener ("roll", this);
    parameters.addParameterListener ("width", this);
    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    qw = parameters.getRawParameterValue ("qw");
    qx = parameters.getRawParameterValue ("qx");
    qy = parameters.getRawParameterValue ("qy");
    qz = parameters.getRawParameterValue ("qz");
    azimuth = parameters.getRawParameterValue ("azimuth");
    elevation = parameters.getRawParameterValue ("elevation");
    roll = parameters.getRawParameterValue ("roll");
    width = parameters.getRawParameterValue ("width");
    highQuality = parameters.getRawParameterValue ("highQuality");

    processorUpdatingParams = false;

    sphericalInput = true; //input from ypr

    juce::FloatVectorOperations::clear (SHL, 64);
    juce::FloatVectorOperations::clear (SHR, 64);
}

StereoEncoderAudioProcessor::~StereoEncoderAudioProcessor() = default;

//==============================================================================

int StereoEncoderAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int StereoEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StereoEncoderAudioProcessor::getProgramName (int index)
{
    return juce::String();
}

void StereoEncoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StereoEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, 2, *orderSetting, true);

    bufferCopy.setSize (2, samplesPerBlock);

    smoothAzimuthL.setCurrentAndTargetValue (*azimuth / 180.0f * juce::MathConstants<float>::pi);
    smoothElevationL.setCurrentAndTargetValue (*elevation / 180.0f
                                               * juce::MathConstants<float>::pi);

    smoothAzimuthR.setCurrentAndTargetValue (*azimuth / 180.0f * juce::MathConstants<float>::pi);
    smoothElevationR.setCurrentAndTargetValue (*elevation / 180.0f
                                               * juce::MathConstants<float>::pi);

    smoothAzimuthL.reset (1, samplesPerBlock);
    smoothElevationL.reset (1, samplesPerBlock);
    smoothAzimuthR.reset (1, samplesPerBlock);
    smoothElevationR.reset (1, samplesPerBlock);

    const float widthInRadiansQuarter { Conversions<float>::degreesToRadians (*width) / 4.0f };
    const iem::Quaternion<float> quatLRot { iem::Quaternion<float> (cos (widthInRadiansQuarter),
                                                                    0.0f,
                                                                    0.0f,
                                                                    sin (widthInRadiansQuarter)) };
    const iem::Quaternion<float> quatL = quaternionDirection * quatLRot;
    const iem::Quaternion<float> quatR = quaternionDirection * quatLRot.getConjugate();

    const auto left = quatL.getCartesian();
    const auto right = quatR.getCartesian();

    SHEval (7, left, _SHL);
    SHEval (7, right, _SHR);

    positionHasChanged = true; // just to be sure
}

void StereoEncoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

inline void StereoEncoderAudioProcessor::updateQuaternions()
{
    float ypr[3];
    ypr[0] = Conversions<float>::degreesToRadians (*azimuth);
    ypr[1] = -Conversions<float>::degreesToRadians (*elevation); // pitch
    ypr[2] = Conversions<float>::degreesToRadians (*roll);

    //updating not active params
    quaternionDirection.fromYPR (ypr);
    processorUpdatingParams = true;
    parameters.getParameter ("qw")->setValueNotifyingHost (
        parameters.getParameterRange ("qw").convertTo0to1 (quaternionDirection.w));
    parameters.getParameter ("qx")->setValueNotifyingHost (
        parameters.getParameterRange ("qx").convertTo0to1 (quaternionDirection.x));
    parameters.getParameter ("qy")->setValueNotifyingHost (
        parameters.getParameterRange ("qy").convertTo0to1 (quaternionDirection.y));
    parameters.getParameter ("qz")->setValueNotifyingHost (
        parameters.getParameterRange ("qz").convertTo0to1 (quaternionDirection.z));
    processorUpdatingParams = false;
}

void StereoEncoderAudioProcessor::updateEuler()
{
    float ypr[3];
    quaternionDirection = iem::Quaternion<float> (*qw, *qx, *qy, *qz);
    quaternionDirection.normalize();
    quaternionDirection.toYPR (ypr);

    //updating not active params
    processorUpdatingParams = true;
    parameters.getParameter ("azimuth")->setValueNotifyingHost (
        parameters.getParameterRange ("azimuth").convertTo0to1 (
            Conversions<float>::radiansToDegrees (ypr[0])));
    parameters.getParameter ("elevation")
        ->setValueNotifyingHost (
            parameters.getParameterRange ("elevation")
                .convertTo0to1 (-Conversions<float>::radiansToDegrees (ypr[1])));
    parameters.getParameter ("roll")->setValueNotifyingHost (
        parameters.getParameterRange ("roll").convertTo0to1 (
            Conversions<float>::radiansToDegrees (ypr[2])));
    processorUpdatingParams = false;
}

void StereoEncoderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, 2, *orderSetting);

    const int L = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels() < 2 ? 1 : 2;

    const int ambisonicOrder =
        *orderSetting < 0.5f ? output.getOrder() : juce::roundToInt (orderSetting->load()) - 1;
    const int nChOut = juce::jmin (buffer.getNumChannels(), juce::square (ambisonicOrder + 1));

    for (int i = 0; i < totalNumInputChannels; ++i)
        bufferCopy.copyFrom (i, 0, buffer.getReadPointer (i), buffer.getNumSamples());
    buffer.clear();

    const float widthInRadiansQuarter { Conversions<float>::degreesToRadians (*width) / 4.0f };
    const iem::Quaternion<float> quatLRot { iem::Quaternion<float> (cos (widthInRadiansQuarter),
                                                                    0.0f,
                                                                    0.0f,
                                                                    sin (widthInRadiansQuarter)) };
    const iem::Quaternion<float> quatL = quaternionDirection * quatLRot;
    const iem::Quaternion<float> quatR = quaternionDirection * quatLRot.getConjugate();

    const auto left = quatL.getCartesian();
    const auto right = quatR.getCartesian();

    //conversion to spherical for high-quality mode
    float azimuthL, azimuthR, elevationL, elevationR;
    Conversions<float>::cartesianToSpherical (left, azimuthL, elevationL);
    Conversions<float>::cartesianToSpherical (right, azimuthR, elevationR);

    if (*highQuality < 0.5f) // no high-quality
    {
        if (positionHasChanged.compareAndSetBool (false, true))
        {
            smoothAzimuthL.setCurrentAndTargetValue (azimuthL);
            smoothElevationL.setCurrentAndTargetValue (elevationL);
            smoothAzimuthR.setCurrentAndTargetValue (azimuthR);
            smoothElevationR.setCurrentAndTargetValue (elevationR);

            SHEval (ambisonicOrder, left.x, left.y, left.z, SHL);
            SHEval (ambisonicOrder, right.x, right.y, right.z, SHR);

            if (*useSN3D > 0.5f)
            {
                juce::FloatVectorOperations::multiply (SHL, SHL, n3d2sn3d, nChOut);
                juce::FloatVectorOperations::multiply (SHR, SHR, n3d2sn3d, nChOut);
            }
        }
        const float* leftIn = bufferCopy.getReadPointer (0);
        const float* rightIn = bufferCopy.getReadPointer (1);
        for (int i = 0; i < nChOut; ++i)
        {
            buffer.copyFromWithRamp (i, 0, leftIn, buffer.getNumSamples(), _SHL[i], SHL[i]);
            buffer.addFromWithRamp (i, 0, rightIn, buffer.getNumSamples(), _SHR[i], SHR[i]);
        }
    }
    else // high-quality sampling
    {
        if (smoothAzimuthL.getTargetValue() - azimuthL > juce::MathConstants<float>::pi)
            smoothAzimuthL.setCurrentAndTargetValue (smoothAzimuthL.getTargetValue()
                                                     - 2.0f * juce::MathConstants<float>::pi);
        else if (azimuthL - smoothAzimuthL.getTargetValue() > juce::MathConstants<float>::pi)
            smoothAzimuthL.setCurrentAndTargetValue (smoothAzimuthL.getTargetValue()
                                                     + 2.0f * juce::MathConstants<float>::pi);

        if (smoothElevationL.getTargetValue() - elevationL > juce::MathConstants<float>::pi)
            smoothElevationL.setCurrentAndTargetValue (smoothElevationL.getTargetValue()
                                                       - 2.0f * juce::MathConstants<float>::pi);
        else if (elevationL - smoothElevationL.getTargetValue() > juce::MathConstants<float>::pi)
            smoothElevationL.setCurrentAndTargetValue (smoothElevationL.getTargetValue()
                                                       + 2.0f * juce::MathConstants<float>::pi);

        if (smoothAzimuthR.getTargetValue() - azimuthR > juce::MathConstants<float>::pi)
            smoothAzimuthR.setCurrentAndTargetValue (smoothAzimuthR.getTargetValue()
                                                     - 2.0f * juce::MathConstants<float>::pi);
        else if (azimuthR - smoothAzimuthR.getTargetValue() > juce::MathConstants<float>::pi)
            smoothAzimuthR.setCurrentAndTargetValue (smoothAzimuthR.getTargetValue()
                                                     + 2.0f * juce::MathConstants<float>::pi);

        if (smoothElevationR.getTargetValue() - elevationR > juce::MathConstants<float>::pi)
            smoothElevationR.setCurrentAndTargetValue (smoothElevationR.getTargetValue()
                                                       - 2.0f * juce::MathConstants<float>::pi);
        else if (elevationR - smoothElevationR.getTargetValue() > juce::MathConstants<float>::pi)
            smoothElevationR.setCurrentAndTargetValue (smoothElevationR.getTargetValue()
                                                       + 2.0f * juce::MathConstants<float>::pi);

        smoothAzimuthL.setTargetValue (azimuthL);
        smoothElevationL.setTargetValue (elevationL);
        smoothAzimuthR.setTargetValue (azimuthR);
        smoothElevationR.setTargetValue (elevationR);

        for (int i = 0; i < L; ++i) // left
        {
            const float azimuth = smoothAzimuthL.getNextValue();
            const float elevation = smoothElevationL.getNextValue();
            float sample = bufferCopy.getSample (0, i);

            const juce::Vector3D<float> pos =
                Conversions<float>::sphericalToCartesian (azimuth, elevation);
            SHEval (ambisonicOrder, pos.x, pos.y, pos.z, SHL);

            for (int ch = 0; ch < nChOut; ++ch)
                buffer.setSample (ch, i, sample * SHL[ch]);
        }

        for (int i = 0; i < L; ++i) // right
        {
            const float azimuth = smoothAzimuthR.getNextValue();
            const float elevation = smoothElevationR.getNextValue();
            float sample = bufferCopy.getSample (1, i);

            const juce::Vector3D<float> pos =
                Conversions<float>::sphericalToCartesian (azimuth, elevation);
            SHEval (ambisonicOrder, pos.x, pos.y, pos.z, SHR);

            for (int ch = 0; ch < nChOut; ++ch)
                buffer.addSample (ch, i, sample * SHR[ch]);
        }

        if (*useSN3D > 0.5f)
        {
            for (int ch = 0; ch < nChOut; ++ch)
            {
                buffer.applyGain (ch, 0, L, n3d2sn3d[ch]);
            }

            juce::FloatVectorOperations::multiply (SHL, SHL, n3d2sn3d, nChOut);
            juce::FloatVectorOperations::multiply (SHR, SHR, n3d2sn3d, nChOut);
        }
    }
    juce::FloatVectorOperations::copy (_SHL, SHL, nChOut);
    juce::FloatVectorOperations::copy (_SHR, SHR, nChOut);
}

//==============================================================================
bool StereoEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StereoEncoderAudioProcessor::createEditor()
{
    return new StereoEncoderAudioProcessorEditor (*this, parameters);
}

void StereoEncoderAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (! processorUpdatingParams)
    {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy"
            || parameterID == "qz")
        {
            sphericalInput = false;
            updateEuler();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "roll")
        {
            sphericalInput = true;
            updateQuaternions();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "width")
        {
            updatedPositionData = true;
            positionHasChanged = true;
        }
    }
    if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
        positionHasChanged = true;
    }
    else if (parameterID == "useSN3D")
    {
        positionHasChanged = true;
    }
}

//==============================================================================
void StereoEncoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void StereoEncoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

const bool
    StereoEncoderAudioProcessor::processNotYetConsumedOSCMessage (const juce::OSCMessage& message)
{
    juce::String prefix ("/" + juce::String (JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return false;

    juce::OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring (
        juce::String (JucePlugin_Name).length() + 1));

    if (msg.getAddressPattern().toString().equalsIgnoreCase ("/quaternions") && msg.size() == 4)
    {
        float qs[4];
        for (int i = 0; i < 4; ++i)
            if (msg[i].isFloat32())
                qs[i] = msg[i].getFloat32();
            else if (msg[i].isInt32())
                qs[i] = msg[i].getInt32();

        oscParameterInterface.setValue ("qw", qs[0]);
        oscParameterInterface.setValue ("qx", qs[1]);
        oscParameterInterface.setValue ("qy", qs[2]);
        oscParameterInterface.setValue ("qz", qs[3]);

        return true;
    }

    return false;
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    StereoEncoderAudioProcessor::createParameterLayout()
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
        "qw",
        "Quaternion W",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f),
        1.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "qx",
        "Quaternion X",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "qy",
        "Quaternion Y",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "qz",
        "Quaternion Z",
        "",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "azimuth",
        "Azimuth Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "elevation",
        "Elevation Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "roll",
        "Roll Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "width",
        "Stereo Width",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-360.0f, 360.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "highQuality",
        "Sample-wise Panning",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0f,
        [] (float value) { return value < 0.5f ? "OFF" : "ON"; },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoEncoderAudioProcessor();
}
