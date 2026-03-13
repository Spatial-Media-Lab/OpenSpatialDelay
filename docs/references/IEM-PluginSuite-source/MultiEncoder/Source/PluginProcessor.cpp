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
MultiEncoderAudioProcessor::MultiEncoderAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::discreteChannels (maxNumberOfInputs), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::discreteChannels (64), true)
    #endif
            ,
#endif
        createParameterLayout()),
    rms (64)
{
    // global properties
    juce::PropertiesFile::Options options;
    options.applicationName = "MultiEncoder";
    options.filenameSuffix = "settings";
    options.folderName = "IEM";
    options.osxLibrarySubFolder = "Preferences";

    properties.reset (new juce::PropertiesFile (options));
    lastDir = juce::File (properties->getValue ("presetFolder"));

    parameters.addParameterListener ("masterAzimuth", this);
    parameters.addParameterListener ("masterElevation", this);
    parameters.addParameterListener ("masterRoll", this);
    parameters.addParameterListener ("lockedToMaster", this);

    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("inputSetting", this);

    muteMask.clear();
    soloMask.clear();

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        azimuth[i] = parameters.getRawParameterValue ("azimuth" + juce::String (i));
        elevation[i] = parameters.getRawParameterValue ("elevation" + juce::String (i));
        gain[i] = parameters.getRawParameterValue ("gain" + juce::String (i));
        mute[i] = parameters.getRawParameterValue ("mute" + juce::String (i));
        solo[i] = parameters.getRawParameterValue ("solo" + juce::String (i));

        if (*mute[i] >= 0.5f)
            muteMask.setBit (i);
        if (*solo[i] >= 0.5f)
            soloMask.setBit (i);

        parameters.addParameterListener ("azimuth" + juce::String (i), this);
        parameters.addParameterListener ("elevation" + juce::String (i), this);
        parameters.addParameterListener ("mute" + juce::String (i), this);
        parameters.addParameterListener ("solo" + juce::String (i), this);
    }

    masterAzimuth = parameters.getRawParameterValue ("masterAzimuth");
    masterElevation = parameters.getRawParameterValue ("masterElevation");
    masterRoll = parameters.getRawParameterValue ("masterRoll");
    lockedToMaster = parameters.getRawParameterValue ("locking");

    inputSetting = parameters.getRawParameterValue ("inputSetting");
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");

    analyzeRMS = parameters.getRawParameterValue ("analyzeRMS");
    dynamicRange = parameters.getRawParameterValue ("dynamicRange");
    processorUpdatingParams = false;

    yprInput = true; //input from ypr

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        juce::FloatVectorOperations::clear (SH[i], 64);
        _gain[i] = 0.0f;
        //elemActive[i] = *gain[i] >= -59.9f;
        elementColours[i] = juce::Colours::cyan;
    }

    updateQuaternions();

    std::fill (rms.begin(), rms.end(), 0.0f);
}

MultiEncoderAudioProcessor::~MultiEncoderAudioProcessor()
{
}

int MultiEncoderAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MultiEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiEncoderAudioProcessor::getProgramName (int index)
{
    return juce::String();
}

void MultiEncoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputSetting, *orderSetting, true);

    timeConstant = exp (-1.0 / (sampleRate * 0.1 / samplesPerBlock)); // 100ms RMS averaging
    std::fill (rms.begin(), rms.end(), 0.0f);
}

void MultiEncoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MultiEncoderAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    checkInputAndOutput (this, *inputSetting, *orderSetting);

    const int nChOut = juce::jmin (buffer.getNumChannels(), output.getNumberOfChannels());
    const int nChIn = juce::jmin (buffer.getNumChannels(), input.getSize());
    const int ambisonicOrder = output.getOrder();

    if (*analyzeRMS > 0.5f && ! isNonRealtime())
    {
        const float oneMinusTimeConstant = 1.0f - timeConstant;
        for (int ch = 0; ch < nChIn; ++ch)
            rms[ch] = timeConstant * rms[ch]
                      + oneMinusTimeConstant * buffer.getRMSLevel (ch, 0, buffer.getNumSamples());
    }

    for (int i = 0; i < nChIn; ++i)
        bufferCopy.copyFrom (i, 0, buffer.getReadPointer (i), buffer.getNumSamples());

    buffer.clear();

    for (int i = 0; i < nChIn; ++i)
    {
        juce::FloatVectorOperations::copy (_SH[i], SH[i], nChOut);

        float currGain = 0.0f;

        if (! soloMask.isZero())
        {
            if (soloMask[i])
                currGain = juce::Decibels::decibelsToGain (gain[i]->load());
        }
        else
        {
            if (! muteMask[i])
                currGain = juce::Decibels::decibelsToGain (gain[i]->load());
        }

        const float azimuthInRad = juce::degreesToRadians (azimuth[i]->load());
        const float elevationInRad = juce::degreesToRadians (elevation[i]->load());

        const juce::Vector3D<float> pos {
            Conversions<float>::sphericalToCartesian (azimuthInRad, elevationInRad)
        };

        SHEval (ambisonicOrder, pos.x, pos.y, pos.z, SH[i]);

        if (*useSN3D >= 0.5f)
            juce::FloatVectorOperations::multiply (SH[i], SH[i], n3d2sn3d, nChOut);

        const float* inpReadPtr = bufferCopy.getReadPointer (i);
        for (int ch = 0; ch < nChOut; ++ch)
            buffer.addFromWithRamp (ch,
                                    0,
                                    inpReadPtr,
                                    buffer.getNumSamples(),
                                    _SH[i][ch] * _gain[i],
                                    SH[i][ch] * currGain);

        _gain[i] = currGain;
    }
}

//==============================================================================
bool MultiEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiEncoderAudioProcessor::createEditor()
{
    return new MultiEncoderAudioProcessorEditor (*this, parameters);
}

void MultiEncoderAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    DBG (parameterID << ": " << newValue);
    if (parameterID == "inputSetting" || parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
    }
    else if (parameterID.startsWith ("solo"))
    {
        const int id = parameterID.substring (4).getIntValue();
        soloMask.setBit (id, newValue >= 0.5f);
        soloMuteChanged = true;
    }
    else if (parameterID.startsWith ("mute"))
    {
        const int id = parameterID.substring (4).getIntValue();
        muteMask.setBit (id, newValue >= 0.5f);
        soloMuteChanged = true;
    }
    else if (parameterID == "lockedToMaster")
    {
        if (newValue >= 0.5f && ! locked)
        {
            const int nChIn = input.getSize();
            float ypr[3];
            ypr[2] = 0.0f;
            for (int i = 0; i < nChIn; ++i)
            {
                iem::Quaternion<float> masterQuat;
                float masterypr[3];
                masterypr[0] = juce::degreesToRadians (masterAzimuth->load());
                masterypr[1] = juce::degreesToRadians (masterElevation->load());
                masterypr[2] = -juce::degreesToRadians (masterRoll->load());
                masterQuat.fromYPR (masterypr);
                masterQuat.conjugate();

                ypr[0] = juce::degreesToRadians (azimuth[i]->load());
                ypr[1] = juce::degreesToRadians (elevation[i]->load());
                quats[i].fromYPR (ypr);
                quats[i] = masterQuat * quats[i];
            }
            locked = true;
        }
        else if (newValue < 0.5f)
            locked = false;
    }
    else if (locked
             && ((parameterID == "masterAzimuth") || (parameterID == "masterElevation")
                 || (parameterID == "masterRoll")))
    {
        if (dontTriggerMasterUpdate)
            return;

        moving = true;
        iem::Quaternion<float> masterQuat;
        float ypr[3];
        ypr[0] = juce::degreesToRadians (masterAzimuth->load());
        ypr[1] = juce::degreesToRadians (masterElevation->load());
        ypr[2] = -juce::degreesToRadians (masterRoll->load());
        masterQuat.fromYPR (ypr);

        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> temp = masterQuat * quats[i];
            temp.toYPR (ypr);
            parameters.getParameter ("azimuth" + juce::String (i))
                ->setValueNotifyingHost (parameters.getParameterRange ("azimuth" + juce::String (i))
                                             .convertTo0to1 (juce::radiansToDegrees (ypr[0])));
            parameters.getParameter ("elevation" + juce::String (i))
                ->setValueNotifyingHost (
                    parameters.getParameterRange ("elevation" + juce::String (i))
                        .convertTo0to1 (juce::radiansToDegrees (ypr[1])));
        }
        moving = false;
        updateSphere = true;
    }
    else if (locked && ! moving
             && (parameterID.startsWith ("azimuth") || parameterID.startsWith ("elevation")))
    {
        float ypr[3];
        ypr[2] = 0.0f;
        const int nChIn = input.getSize();
        for (int i = 0; i < nChIn; ++i)
        {
            iem::Quaternion<float> masterQuat;
            float masterypr[3];
            masterypr[0] = juce::degreesToRadians (masterAzimuth->load());
            masterypr[1] = juce::degreesToRadians (masterElevation->load());
            masterypr[2] = -juce::degreesToRadians (masterRoll->load());
            masterQuat.fromYPR (masterypr);
            masterQuat.conjugate();

            ypr[0] = juce::degreesToRadians (azimuth[i]->load());
            ypr[1] = juce::degreesToRadians (elevation[i]->load());
            quats[i].fromYPR (ypr);
            quats[i] = masterQuat * quats[i];
        }
        updateSphere = true;
    }
    else if (parameterID.startsWith ("azimuth") || parameterID.startsWith ("elevation")
             || (parameterID == "masterAzimuth") || (parameterID == "masterElevation"))
    {
        updateSphere = true;
    }
}

//==============================================================================
void MultiEncoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    for (int i = 0; i < maxNumberOfInputs; ++i)
        state.setProperty ("colour" + juce::String (i), elementColours[i].toString(), nullptr);

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MultiEncoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            dontTriggerMasterUpdate = true;
            parameters.state = juce::ValueTree::fromXml (*xmlState);
            dontTriggerMasterUpdate = false;

            updateQuaternions();
            for (int i = 0; i < maxNumberOfInputs; ++i)
                if (parameters.state.getProperty ("colour" + juce::String (i)).toString() != "0")
                    elementColours[i] = juce::Colour::fromString (
                        parameters.state.getProperty ("colour" + juce::String (i)).toString());
                else
                    elementColours[i] = juce::Colours::cyan;
            updateColours = true;

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
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiEncoderAudioProcessor();
}

void MultiEncoderAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());

    const int nChIn = input.getSize();
    const int _nChIn = input.getPreviousSize();

    bufferCopy.setSize (nChIn, getBlockSize());

    // disable solo and mute for deleted input channels
    for (int i = nChIn; i < _nChIn; ++i)
    {
        parameters.getParameter ("mute" + juce::String (i))->setValueNotifyingHost (0.0f);
        parameters.getParameter ("solo" + juce::String (i))->setValueNotifyingHost (0.0f);
    }
};

void MultiEncoderAudioProcessor::updateQuaternions()
{
    float ypr[3];
    ypr[2] = 0.0f;

    iem::Quaternion<float> masterQuat;
    float masterypr[3];
    masterypr[0] = juce::degreesToRadians (masterAzimuth->load());
    masterypr[1] = juce::degreesToRadians (masterElevation->load());
    masterypr[2] = -juce::degreesToRadians (masterRoll->load());
    masterQuat.fromYPR (masterypr);
    masterQuat.conjugate();

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        ypr[0] = juce::degreesToRadians (azimuth[i]->load());
        ypr[1] = juce::degreesToRadians (elevation[i]->load());
        quats[i].fromYPR (ypr);
        quats[i] = masterQuat * quats[i];
    }
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    MultiEncoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "inputSetting",
        "Number of input channels ",
        "",
        juce::NormalisableRange<float> (0.0f, maxNumberOfInputs, 1.0f),
        startNnumberOfInputs,
        [] (float value) { return juce::String (value); },
        nullptr));

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
        "masterAzimuth",
        "Master azimuth angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "masterElevation",
        "Master elevation angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "masterRoll",
        "Master roll angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0f,
        [] (float value) { return juce::String (value, 2); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "lockedToMaster",
        "Lock Directions relative to Master",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0f,
        [] (float value) { return (value >= 0.5f) ? "locked" : "not locked"; },
        nullptr));

    for (int i = 0; i < maxNumberOfInputs; ++i)
    {
        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "azimuth" + juce::String (i),
            "Azimuth angle " + juce::String (i + 1),
            juce::CharPointer_UTF8 (R"(°)"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
            0.0,
            [] (float value) { return juce::String (value, 2); },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "elevation" + juce::String (i),
            "Elevation angle " + juce::String (i + 1),
            juce::CharPointer_UTF8 (R"(°)"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
            0.0,
            [] (float value) { return juce::String (value, 2); },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "gain" + juce::String (i),
            "Gain " + juce::String (i + 1),
            "dB",
            juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f),
            0.0f,
            [] (float value) { return (value >= -59.9f) ? juce::String (value, 1) : "-inf"; },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "mute" + juce::String (i),
            "Mute input " + juce::String (i + 1),
            "",
            juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
            0.0f,
            [] (float value) { return (value >= 0.5f) ? "muted" : "not muted"; },
            nullptr));

        params.push_back (OSCParameterInterface::createParameterTheOldWay (
            "solo" + juce::String (i),
            "Solo input " + juce::String (i + 1),
            "",
            juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
            0.0f,
            [] (float value) { return (value >= 0.5f) ? "soloed" : "not soloed"; },
            nullptr));
    }

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "analyzeRMS",
        "Analzes RMS",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0f,
        [] (float value) { return (value >= 0.5f) ? "on" : "off"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "peakLevel",
        "Peak level",
        "dB",
        juce::NormalisableRange<float> (-50.0f, 10.0f, 0.1f),
        0.0,
        [] (float value) { return juce::String (value, 1); },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "dynamicRange",
        "Dynamic juce::Range",
        "dB",
        juce::NormalisableRange<float> (10.0f, 60.0f, 1.f),
        35.0,
        [] (float value) { return juce::String (value, 0); },
        nullptr));

    return params;
}

//==============================================================================

juce::Result MultiEncoderAudioProcessor::loadConfiguration (const juce::File& configFile)
{
    juce::ValueTree newSources ("NewSources");

    juce::Result result =
        ConfigurationHelper::parseFileForLoudspeakerLayout (configFile, newSources, nullptr);

    if (result.wasOk())
    {
        int nSrc = 0;
        const auto nElements = newSources.getNumChildren();

        for (int i = 0; i < nElements; ++i)
        {
            auto src = newSources.getChild (i);
            const int ch = src.getProperty ("Channel");
            const bool isImaginary = src.getProperty ("Imaginary");
            if (! isImaginary && ch > nSrc)
                nSrc = ch;
        }

        DBG (nSrc << " Sources!");
        parameters.getParameterAsValue ("inputSetting").setValue (nSrc);

        for (int s = 0; s < nSrc; ++s)
            parameters.getParameterAsValue ("mute" + juce::String (s)).setValue (1);

        for (int e = 0; e < nElements; ++e)
        {
            const auto src = newSources.getChild (e);
            const int ch = static_cast<int> (src.getProperty ("Channel", 0)) - 1;
            const bool isImaginary = src.getProperty ("Imaginary");

            if (isImaginary || ch < 0 || ch >= 64)
                continue;

            parameters.getParameterAsValue ("mute" + juce::String (ch)).setValue (0);

            float azi = src.getProperty ("Azimuth", 0.0f);
            float ele = src.getProperty ("Elevation", 0.0f);

            if (azi > 180.0f || azi < -180.0f || ele > 90.0f || ele < -90.0f)
                wrapSphericalCoordinates (&azi, &ele);

            parameters.getParameterAsValue ("azimuth" + juce::String (ch)).setValue (azi);
            parameters.getParameterAsValue ("elevation" + juce::String (ch)).setValue (ele);
        }
    }

    return result;
}

void MultiEncoderAudioProcessor::wrapSphericalCoordinates (float* azi, float* ele)
{
    // Wrap elevation
    if (*ele > 180.0f || *ele < -180.0f)
    {
        float fullRotations = std::copysign (std::ceil (std::abs (*ele) / 360.0f), *ele);
        *ele -= fullRotations * 360.0f;
    }
    if (*ele > 90.0f || *ele < -90.0f)
    {
        *ele = std::copysign (180.0 - std::abs (*ele), *ele);
        *azi += 180.0f;
    }

    // Wrap azimuth
    if (*azi > 180.0f || *azi < -180.0f)
    {
        float fullRotations = std::copysign (std::ceil (std::abs (*azi) / 360.0f), *azi);
        *azi -= fullRotations * 360.0f;
    }
}

void MultiEncoderAudioProcessor::setLastDir (juce::File newLastDir)
{
    lastDir = newLastDir;
    const juce::var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}
