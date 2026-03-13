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
DirectionalCompressorAudioProcessor::DirectionalCompressorAudioProcessor() :
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
    //W (tDesignN),
    Y (tDesignN, 64),
    YH (64, tDesignN),
    tempMat (64, tDesignN),
    P1 (64, 64)
{
    parameters.addParameterListener ("azimuth", this);
    parameters.addParameterListener ("elevation", this);
    parameters.addParameterListener ("width", this);
    parameters.addParameterListener ("orderSetting", this);

    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    preGain = parameters.getRawParameterValue ("preGain");

    c1Enabled = parameters.getRawParameterValue ("c1Enabled");
    c1DrivingSignal = parameters.getRawParameterValue ("c1DrivingSignal");
    c1Apply = parameters.getRawParameterValue ("c1Apply");
    c1Threshold = parameters.getRawParameterValue ("c1Threshold");
    c1Knee = parameters.getRawParameterValue ("c1Knee");
    c1Attack = parameters.getRawParameterValue ("c1Attack");
    c1Release = parameters.getRawParameterValue ("c1Release");
    c1Ratio = parameters.getRawParameterValue ("c1Ratio");
    c1Makeup = parameters.getRawParameterValue ("c1Makeup");

    c2Enabled = parameters.getRawParameterValue ("c2Enabled");
    c2DrivingSignal = parameters.getRawParameterValue ("c2DrivingSignal");
    c2Apply = parameters.getRawParameterValue ("c2Apply");
    c2Threshold = parameters.getRawParameterValue ("c2Threshold");
    c2Knee = parameters.getRawParameterValue ("c2Knee");
    c2Attack = parameters.getRawParameterValue ("c2Attack");
    c2Release = parameters.getRawParameterValue ("c2Release");
    c2Ratio = parameters.getRawParameterValue ("c2Ratio");
    c2Makeup = parameters.getRawParameterValue ("c2Makeup");

    azimuth = parameters.getRawParameterValue ("azimuth");
    elevation = parameters.getRawParameterValue ("elevation");
    width = parameters.getRawParameterValue ("width");
    listen = parameters.getRawParameterValue ("listen");

    c1MaxGR = 0.0f;
    c2MaxGR = 0.0f;
    c1GR = 0.0f;
    c2GR = 0.0f;

    // calc Y
    for (int p = 0; p < tDesignN; ++p)
        SHEval (7, tDesignX[p], tDesignY[p], tDesignZ[p], Y.getRawDataPointer() + p * 64, false);

    Y *= std::sqrt (4 * juce::MathConstants<float>::pi / tDesignN)
         / decodeCorrection (7); // reverting 7th order correction

    for (int r = 0; r < 64; ++r)
        for (int c = 0; c < tDesignN; ++c)
            YH (r, c) = Y (c, r);
}

DirectionalCompressorAudioProcessor::~DirectionalCompressorAudioProcessor()
{
}

//==============================================================================
int DirectionalCompressorAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DirectionalCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DirectionalCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DirectionalCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void DirectionalCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void DirectionalCompressorAudioProcessor::parameterChanged (const juce::String& parameterID,
                                                            float newValue)
{
    if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "width")
    {
        updatedPositionData = true;
        paramChanged = true;
    }
    else if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
    }
}

//==============================================================================
void DirectionalCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, true);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;

    compressor1.prepare (spec);
    compressor2.prepare (spec);

    omniW.setSize (1, samplesPerBlock);
    c1Gains.resize (samplesPerBlock);
    c2Gains.resize (samplesPerBlock);

    calcParams();
}

void DirectionalCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void DirectionalCompressorAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                                        juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting);
    if (paramChanged)
        calcParams();

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int bufferSize = buffer.getNumSamples();

    const int numCh = juce::jmin (input.getNumberOfChannels(), buffer.getNumChannels());
    if (numCh == 0)
        return;

    // Compressor 1 settings
    if (*c1Ratio > 15.9f)
        compressor1.setRatio (INFINITY);
    else
        compressor1.setRatio (*c1Ratio);

    compressor1.setKnee (*c1Knee);
    compressor1.setAttackTime (*c1Attack / 1000.0f);
    compressor1.setReleaseTime (*c1Release / 1000.0f);
    compressor1.setThreshold (*c1Threshold);
    compressor1.setMakeUpGain (*c1Makeup);

    // Compressor 2 settings
    if (*c2Ratio > 15.9f)
        compressor2.setRatio (INFINITY);
    else
        compressor2.setRatio (*c2Ratio);

    compressor2.setKnee (*c2Knee);
    compressor2.setAttackTime (*c2Attack / 1000.0f);
    compressor2.setReleaseTime (*c2Release / 1000.0f);
    compressor2.setThreshold (*c2Threshold);
    compressor2.setMakeUpGain (*c2Makeup);

    drivingPointers[0] = maskBuffer.getReadPointer (0);
    drivingPointers[1] = buffer.getReadPointer (0);
    drivingPointers[2] = omniW.getReadPointer (0);

    // preGain - can be tweaked by adding gain to compressor gains
    float preGainLinear = juce::Decibels::decibelsToGain (preGain->load());

    if (*useSN3D >= 0.5f)
        for (int i = 0; i < numCh; ++i)
            buffer.applyGain (i, 0, bufferSize, sn3d2n3d[i] * preGainLinear);
    else
        buffer.applyGain (juce::Decibels::decibelsToGain (preGain->load()));

    // --------- make copies of buffer
    omniW.copyFrom (0, 0, buffer, 0, 0, bufferSize);

    maskBuffer.clear();
    maskBuffer.setSample (0, 0, 0.0f);
    for (int chIn = 0; chIn < numCh; ++chIn)
    {
        const float* readPtr = buffer.getReadPointer (chIn);
        for (int chOut = 0; chOut < numCh; ++chOut)
        {
            maskBuffer.addFrom (chOut, 0, readPtr, bufferSize, P1 (chOut, chIn));
        }
    }
    /* This makes the buffer containing the negative mask */
    for (int chIn = 0; chIn < numCh; ++chIn)
        juce::FloatVectorOperations::subtract (buffer.getWritePointer (chIn),
                                               maskBuffer.getReadPointer (chIn),
                                               bufferSize);

    // ------- clear not needed channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // =============== COMPRESSOR 1 ====================
    {
        // set compressor driving signal
        const float* drivingSignalPtr = [this]() -> const float*
        {
            if (*c1DrivingSignal >= 0.5f && *c1DrivingSignal < 1.5f)
                return drivingPointers[0];
            else if (*c1DrivingSignal >= 1.5f)
                return drivingPointers[1];
            else
                return drivingPointers[2];
        }();

        compressor1.getGainFromSidechainSignal (drivingSignalPtr,
                                                c1Gains.getRawDataPointer(),
                                                bufferSize);
        c1MaxRMS = compressor1.getMaxLevelInDecibels();
        c1MaxGR =
            juce::Decibels::gainToDecibels (
                juce::FloatVectorOperations::findMinimum (c1Gains.getRawDataPointer(), bufferSize))
            - *c1Makeup;
    }

    // =============== COMPRESSOR 2 ====================
    {
        // set compressor driving signal
        const float* drivingSignalPtr = [this]() -> const float*
        {
            if (*c2DrivingSignal >= 0.5f && *c2DrivingSignal < 1.5f)
                return drivingPointers[0];
            else if (*c2DrivingSignal >= 1.5f)
                return drivingPointers[1];
            else
                return drivingPointers[2];
        }();

        compressor2.getGainFromSidechainSignal (drivingSignalPtr,
                                                c2Gains.getRawDataPointer(),
                                                bufferSize);
        c2MaxRMS = compressor2.getMaxLevelInDecibels();
        c2MaxGR =
            juce::Decibels::gainToDecibels (
                juce::FloatVectorOperations::findMinimum (c2Gains.getRawDataPointer(), bufferSize))
            - *c2Makeup;
    }

    // =============== OUTPUT CALCULATIONS ====================
    // REMEMBER: buffer contains negative mask content

    // apply gains from compressor 1
    if (*c1Enabled >= 0.5f)
    {
        if (*c1Apply >= 0.5f && *c1Apply < 1.5f) //mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c1Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
        else if (*c1Apply > 1.5f) //negative mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = buffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c1Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
        else
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c1Gains.getRawDataPointer(),
                                                       bufferSize);
                writeData = buffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c1Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
    }

    // apply gains from compressor 2
    if (*c2Enabled >= 0.5f)
    {
        if (*c2Apply >= 0.5f && *c2Apply < 1.5f) //mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c2Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
        else if (*c2Apply > 1.5f) //negative mask
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = buffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c2Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
        else
        {
            for (int channel = 0; channel < numCh; ++channel)
            {
                float* writeData = maskBuffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c2Gains.getRawDataPointer(),
                                                       bufferSize);
                writeData = buffer.getWritePointer (channel);
                juce::FloatVectorOperations::multiply (writeData,
                                                       c2Gains.getRawDataPointer(),
                                                       bufferSize);
            }
        }
    }

    //add channels or replace
    if (*listen >= 0.5f && *listen < 1.5f) //mask
    {
        juce::FloatVectorOperations::copy (buffer.getWritePointer (0),
                                           maskBuffer.getReadPointer (0),
                                           numCh * bufferSize);
    }
    else if (*listen < 0.5f)
    {
        for (int chIn = 0; chIn < numCh; ++chIn)
            juce::FloatVectorOperations::add (buffer.getWritePointer (chIn),
                                              maskBuffer.getReadPointer (chIn),
                                              bufferSize);
    }

    if (*useSN3D >= 0.5f)
        for (int i = 0; i < numCh; ++i)
            buffer.applyGain (i, 0, bufferSize, n3d2sn3d[i]);
}

void DirectionalCompressorAudioProcessor::calcParams()
{
    paramChanged = false;

    // convert azimuth and elevation to cartesian coordinates
    auto pos = Conversions<float>::sphericalToCartesian (
        Conversions<float>::degreesToRadians (*azimuth),
        Conversions<float>::degreesToRadians (*elevation));
    pos = pos.normalised();

    for (int point = 0; point < tDesignN; ++point)
    {
        dist[point] = pos.x * tDesignX[point] + pos.y * tDesignY[point] + pos.z * tDesignZ[point];
        dist[point] /= std::sqrt (juce::square (tDesignX[point]) + juce::square (tDesignY[point])
                                  + juce::square (tDesignZ[point]));
        dist[point] = std::acos (dist[point]);
    }

    float widthHalf = Conversions<float>::degreesToRadians (*width)
                      * 0.25f; // it's actually width fourth (symmetric mask)
    widthHalf = juce::jmax (widthHalf, juce::FloatVectorOperations::findMinimum (dist, tDesignN));

    juce::FloatVectorOperations::clip (dist, dist, widthHalf, 3 * widthHalf, tDesignN);
    juce::FloatVectorOperations::add (dist, -widthHalf, tDesignN);
    juce::FloatVectorOperations::multiply (dist,
                                           0.25f * juce::MathConstants<float>::pi / widthHalf,
                                           tDesignN);

    for (int p = 0; p < tDesignN; ++p)
    {
        const float g = std::cos (dist[p]);
        for (int r = 0; r < 64; ++r)
            tempMat (r, p) = g * Y (p, r);
    }

    for (int r = 0; r < 64; ++r)
        for (int c = r; c < 64; ++c)
        {
            float sum = 0.0f;
            for (int i = 0; i < tDesignN; ++i)
                sum += tempMat (r, i) * YH (c, i);
            P1 (r, c) = sum;
            P1 (c, r) = sum;
        }
}

//==============================================================================
bool DirectionalCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DirectionalCompressorAudioProcessor::createEditor()
{
    return new DirectionalCompressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void DirectionalCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DirectionalCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

void DirectionalCompressorAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
    const int nChannels = input.getNumberOfChannels();
    maskBuffer.setSize (nChannels, getBlockSize());
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    DirectionalCompressorAudioProcessor::createParameterLayout()
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
        "preGain",
        "Input Gain ",
        "dB",
        juce::NormalisableRange<float> (-10.0f, 10.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    // compressor 1
    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Enabled",
        "Enable Compressor 1",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f)
                return "ON";
            else
                return "OFF";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1DrivingSignal",
        "Compressor 1 Driving Signal",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "Masked";
            else if (value >= 1.5f)
                return "Unmasked";
            else
                return "Full";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Apply",
        "Apply compression 1 to",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "Masked";
            else if (value >= 1.5f)
                return "Unmasked";
            else
                return "Full";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Threshold",
        "Threshold 1",
        "dB",
        juce::NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
        -10.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Knee",
        "Knee 1",
        "dB",
        juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Attack",
        "Attack Time 1",
        "ms",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Release",
        "Release Time 1",
        "ms",
        juce::NormalisableRange<float> (0.0f, 500.0f, 0.1f),
        150.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c1Ratio",
        "Ratio 1",
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
        "c1Makeup",
        "MakeUp Gain 1",
        "dB",
        juce::NormalisableRange<float> (-10.0f, 20.0f, 0.10f),
        0.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    // compressor 2
    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Enabled",
        "Enable Compressor 2",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f)
                return "ON";
            else
                return "OFF";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2DrivingSignal",
        "Compressor 2 Driving Signal",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "Masked";
            else if (value >= 1.5f)
                return "Unmasked";
            else
                return "Full";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Apply",
        "Apply compression 2 to",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        1.0,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "Masked";
            else if (value >= 1.5f)
                return "Unmasked";
            else
                return "Full";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Threshold",
        "Threshold 2",
        "dB",
        juce::NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
        -10.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Knee",
        "Knee 2",
        "dB",
        juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f),
        0.0f,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Attack",
        "Attack Time 2",
        "ms",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Release",
        "Release Time 2",
        "ms",
        juce::NormalisableRange<float> (0.0f, 500.0f, 0.1f),
        150.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "c2Ratio",
        "Ratio 2",
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
        "c2Makeup",
        "MakeUp Gain 2",
        "dB",
        juce::NormalisableRange<float> (-10.0f, 20.0f, 0.10f),
        0.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "azimuth",
        "Azimuth of mask",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "elevation",
        "Elevation of mask",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "width",
        "Width of mask",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (10.0f, 180.0f, 0.01f),
        40.0f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "listen",
        "Listen to",
        "",
        juce::NormalisableRange<float> (0.0f, 2.0f, 1.0f),
        0.0,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "Masked";
            else if (value >= 1.5f)
                return "Unmasked";
            else
                return "Full";
        },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DirectionalCompressorAudioProcessor();
}
