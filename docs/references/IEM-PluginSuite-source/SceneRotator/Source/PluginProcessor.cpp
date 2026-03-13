/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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

/*
 The computation of Ambisonic rotation matrices is done by the recursive method
 of Ivanic and Ruedenberg:

    Ivanic, J., Ruedenberg, K. (1996). Rotation Matrices for Real Spherical
    Harmonics. Direct Determination by Recursion. The Journal of Physical
    Chemistry, 100(15), 6342?6347.

 Including their corrections:

    Ivanic, J., Ruedenberg, K. (1998). Rotation Matrices for Real Spherical
    Harmonics. Direct Determination by Recursion Page: Additions and
    Corrections. Journal of Physical Chemistry A, 102(45), 9099?9100.

 It also follows the implementations of Archontis Politis (Spherical Harmonic
 Transform Toolbox) and Matthias Kronlachner (AmbiX Plug-in Suite).
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SceneRotatorAudioProcessor::SceneRotatorAudioProcessor() :
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
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    yaw = parameters.getRawParameterValue ("yaw");
    pitch = parameters.getRawParameterValue ("pitch");
    roll = parameters.getRawParameterValue ("roll");
    qw = parameters.getRawParameterValue ("qw");
    qx = parameters.getRawParameterValue ("qx");
    qy = parameters.getRawParameterValue ("qy");
    qz = parameters.getRawParameterValue ("qz");
    invertYaw = parameters.getRawParameterValue ("invertYaw");
    invertPitch = parameters.getRawParameterValue ("invertPitch");
    invertRoll = parameters.getRawParameterValue ("invertRoll");
    invertQuaternion = parameters.getRawParameterValue ("invertQuaternion");
    rotationSequence = parameters.getRawParameterValue ("rotationSequence");

    // add listeners to parameter changes
    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    parameters.addParameterListener ("yaw", this);
    parameters.addParameterListener ("pitch", this);
    parameters.addParameterListener ("roll", this);
    parameters.addParameterListener ("qw", this);
    parameters.addParameterListener ("qx", this);
    parameters.addParameterListener ("qy", this);
    parameters.addParameterListener ("qz", this);
    parameters.addParameterListener ("invertYaw", this);
    parameters.addParameterListener ("invertPitch", this);
    parameters.addParameterListener ("invertRoll", this);
    parameters.addParameterListener ("invertQuaternion", this);
    parameters.addParameterListener ("rotationSequence", this);

    orderMatrices.add (new juce::dsp::Matrix<float> (0, 0)); // 0th
    orderMatricesCopy.add (new juce::dsp::Matrix<float> (0, 0)); // 0th

    for (int l = 1; l <= 7; ++l)
    {
        const int nCh = (2 * l + 1);
        auto elem = orderMatrices.add (new juce::dsp::Matrix<float> (nCh, nCh));
        elem->clear();
        auto elemCopy = orderMatricesCopy.add (new juce::dsp::Matrix<float> (nCh, nCh));
        elemCopy->clear();
    }

    startTimer (500);
}

SceneRotatorAudioProcessor::~SceneRotatorAudioProcessor()
{
    closeMidiInput();
}

//==============================================================================
int SceneRotatorAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SceneRotatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SceneRotatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SceneRotatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SceneRotatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SceneRotatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, true);

    copyBuffer.setSize (copyBuffer.getNumChannels(), samplesPerBlock);

    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::MidiMessageCollector::reset (sampleRate);
    rotationParamsHaveChanged = true;
}

void SceneRotatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SceneRotatorAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *orderSetting, *orderSetting, false);
    juce::ScopedNoDenormals noDenormals;

    const int L = buffer.getNumSamples();

    const int inputOrder = input.getOrder();
    const int nChIn = juce::jmin (input.getNumberOfChannels(), buffer.getNumChannels());
    const int actualOrder = floor (sqrt (nChIn)) - 1;
    const int actualChannels = juce::square (actualOrder + 1);
    jassert (actualChannels <= nChIn);

    if (currentMidiScheme != MidiScheme::none)
    {
        removeNextBlockOfMessages (midiMessages, buffer.getNumSamples());

        for (const auto& msg : midiMessages)
        {
            const auto message = msg.getMessage();

            if (! message.isController())
                break;

            switch (currentMidiScheme)
            {
                case MidiScheme::mrHeadTrackerYprDir:
                case MidiScheme::mrHeadTrackerYprInv:
                    switch (message.getControllerNumber())
                    {
                        case 48:
                            yawLsb = message.getControllerValue();
                            break;
                        case 49:
                            pitchLsb = message.getControllerValue();
                            break;
                        case 50:
                            rollLsb = message.getControllerValue();
                            break;

                        case 16:
                        {
                            float yawVal =
                                (128 * message.getControllerValue() + yawLsb) * (1.0f / 16384);
                            parameters.getParameter ("yaw")->setValueNotifyingHost (yawVal);
                            break;
                        }

                        case 17:
                        {
                            float pitchVal =
                                (128 * message.getControllerValue() + pitchLsb) * (1.0f / 16384);
                            parameters.getParameter ("pitch")->setValueNotifyingHost (pitchVal);
                            break;
                        }

                        case 18:
                        {
                            float rollVal =
                                (128 * message.getControllerValue() + rollLsb) * (1.0f / 16384);
                            parameters.getParameter ("roll")->setValueNotifyingHost (rollVal);
                            break;
                        }
                    } // switch (message.getControllerNumber())
                    break;

                case MidiScheme::mrHeadTrackerQuaternions:
                    switch (message.getControllerNumber())
                    {
                        case 48:
                            qwLsb = message.getControllerValue();
                            break;
                        case 49:
                            qxLsb = message.getControllerValue();
                            break;
                        case 50:
                            qyLsb = message.getControllerValue();
                            break;
                        case 51:
                            qzLsb = message.getControllerValue();
                            break;

                        case 16:
                        {
                            float qwVal =
                                (128 * message.getControllerValue() + qwLsb) * (1.0f / 16384);
                            parameters.getParameter ("qw")->setValueNotifyingHost (qwVal);
                            break;
                        }

                        case 17:
                        {
                            float qxVal =
                                (128 * message.getControllerValue() + qxLsb) * (1.0f / 16384);
                            parameters.getParameter ("qx")->setValueNotifyingHost (qxVal);
                            break;
                        }

                        case 18:
                        {
                            float qyVal =
                                (128 * message.getControllerValue() + qyLsb) * (1.0f / 16384);
                            parameters.getParameter ("qy")->setValueNotifyingHost (qyVal);
                            break;
                        }

                        case 19:
                        {
                            float qzVal =
                                (128 * message.getControllerValue() + qzLsb) * (1.0f / 16384);
                            parameters.getParameter ("qz")->setValueNotifyingHost (qzVal);
                            break;
                        }
                    } // switch (message.getControllerNumber())
                    break;
                default:
                    break;
            } // switch (currentMidiScheme)
        } //while (i.getNextEvent (message, time))
    } //if (currentMidiScheme != MidiScheme::none)

    bool newRotationMatrix = false;
    if (rotationParamsHaveChanged.get())
    {
        newRotationMatrix = true;
        calcRotationMatrix (inputOrder);
    }

    // make copy of input
    for (int ch = 0; ch < actualChannels; ++ch)
        copyBuffer.copyFrom (ch, 0, buffer, ch, 0, L);

    // clear all channels except first
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, L);

    // rotate buffer
    for (int l = 1; l <= actualOrder; ++l)
    {
        const int offset = l * l;
        const int nCh = 2 * l + 1;
        auto R = orderMatrices[l];
        auto Rcopy = orderMatricesCopy[l];
        for (int o = 0; o < nCh; ++o)
        {
            const int chOut = offset + o;
            for (int p = 0; p < nCh; ++p)
            {
                buffer.addFromWithRamp (chOut,
                                        0,
                                        copyBuffer.getReadPointer (offset + p),
                                        L,
                                        Rcopy->operator() (o, p),
                                        R->operator() (o, p));
            }
        }
    }

    // make copies for fading between old and new matrices
    if (newRotationMatrix)
        for (int l = 1; l <= inputOrder; ++l)
            *orderMatricesCopy[l] = *orderMatrices[l];

    midiMessages.clear();
}

double SceneRotatorAudioProcessor::P (int i,
                                      int l,
                                      int a,
                                      int b,
                                      juce::dsp::Matrix<float>& R1,
                                      juce::dsp::Matrix<float>& Rlm1)
{
    double ri1 = R1 (i + 1, 2);
    double rim1 = R1 (i + 1, 0);
    double ri0 = R1 (i + 1, 1);

    if (b == -l)
        return ri1 * Rlm1 (a + l - 1, 0) + rim1 * Rlm1 (a + l - 1, 2 * l - 2);
    else if (b == l)
        return ri1 * Rlm1 (a + l - 1, 2 * l - 2) - rim1 * Rlm1 (a + l - 1, 0);
    else
        return ri0 * Rlm1 (a + l - 1, b + l - 1);
};

double SceneRotatorAudioProcessor::U (int l,
                                      int m,
                                      int n,
                                      juce::dsp::Matrix<float>& Rone,
                                      juce::dsp::Matrix<float>& Rlm1)
{
    return P (0, l, m, n, Rone, Rlm1);
}

double SceneRotatorAudioProcessor::V (int l,
                                      int m,
                                      int n,
                                      juce::dsp::Matrix<float>& Rone,
                                      juce::dsp::Matrix<float>& Rlm1)
{
    if (m == 0)
    {
        auto p0 = P (1, l, 1, n, Rone, Rlm1);
        auto p1 = P (-1, l, -1, n, Rone, Rlm1);
        return p0 + p1;
    }
    else if (m > 0)
    {
        auto p0 = P (1, l, m - 1, n, Rone, Rlm1);
        if (m == 1) // d = 1;
            return p0 * sqrt (2);
        else // d = 0;
            return p0 - P (-1, l, 1 - m, n, Rone, Rlm1);
    }
    else
    {
        auto p1 = P (-1, l, -m - 1, n, Rone, Rlm1);
        if (m == -1) // d = 1;
            return p1 * sqrt (2);
        else // d = 0;
            return p1 + P (1, l, m + 1, n, Rone, Rlm1);
    }
}

double SceneRotatorAudioProcessor::W (int l,
                                      int m,
                                      int n,
                                      juce::dsp::Matrix<float>& Rone,
                                      juce::dsp::Matrix<float>& Rlm1)
{
    if (m > 0)
    {
        auto p0 = P (1, l, m + 1, n, Rone, Rlm1);
        auto p1 = P (-1, l, -m - 1, n, Rone, Rlm1);
        return p0 + p1;
    }
    else if (m < 0)
    {
        auto p0 = P (1, l, m - 1, n, Rone, Rlm1);
        auto p1 = P (-1, l, 1 - m, n, Rone, Rlm1);
        return p0 - p1;
    }

    return 0.0;
}

void SceneRotatorAudioProcessor::calcRotationMatrix (const int order)
{
    const auto yawRadians =
        Conversions<float>::degreesToRadians (*yaw) * (*invertYaw > 0.5 ? -1 : 1);
    const auto pitchRadians =
        Conversions<float>::degreesToRadians (*pitch) * (*invertPitch > 0.5 ? -1 : 1);
    const auto rollRadians =
        Conversions<float>::degreesToRadians (*roll) * (*invertRoll > 0.5 ? -1 : 1);

    auto ca = std::cos (yawRadians);
    auto cb = std::cos (pitchRadians);
    auto cy = std::cos (rollRadians);

    auto sa = std::sin (yawRadians);
    auto sb = std::sin (pitchRadians);
    auto sy = std::sin (rollRadians);

    juce::dsp::Matrix<float> rotMat (3, 3);

    if (*rotationSequence >= 0.5f) // roll -> pitch -> yaw (extrinsic rotations)
    {
        rotMat (0, 0) = ca * cb;
        rotMat (1, 0) = sa * cb;
        rotMat (2, 0) = -sb;

        rotMat (0, 1) = ca * sb * sy - sa * cy;
        rotMat (1, 1) = sa * sb * sy + ca * cy;
        rotMat (2, 1) = cb * sy;

        rotMat (0, 2) = ca * sb * cy + sa * sy;
        rotMat (1, 2) = sa * sb * cy - ca * sy;
        rotMat (2, 2) = cb * cy;
    }
    else // yaw -> pitch -> roll (extrinsic rotations)
    {
        rotMat (0, 0) = ca * cb;
        rotMat (1, 0) = sa * cy + ca * sb * sy;
        rotMat (2, 0) = sa * sy - ca * sb * cy;

        rotMat (0, 1) = -sa * cb;
        rotMat (1, 1) = ca * cy - sa * sb * sy;
        rotMat (2, 1) = ca * sy + sa * sb * cy;

        rotMat (0, 2) = sb;
        rotMat (1, 2) = -cb * sy;
        rotMat (2, 2) = cb * cy;
    }

    auto Rl = orderMatrices[1];

    Rl->operator() (0, 0) = rotMat (1, 1);
    Rl->operator() (0, 1) = rotMat (1, 2);
    Rl->operator() (0, 2) = rotMat (1, 0);
    Rl->operator() (1, 0) = rotMat (2, 1);
    Rl->operator() (1, 1) = rotMat (2, 2);
    Rl->operator() (1, 2) = rotMat (2, 0);
    Rl->operator() (2, 0) = rotMat (0, 1);
    Rl->operator() (2, 1) = rotMat (0, 2);
    Rl->operator() (2, 2) = rotMat (0, 0);

    for (int l = 2; l <= order; ++l)
    {
        auto Rone = orderMatrices[1];
        auto Rlm1 = orderMatrices[l - 1];
        auto Rl = orderMatrices[l];
        for (int m = -l; m <= l; ++m)
        {
            for (int n = -l; n <= l; ++n)
            {
                const int d = (m == 0) ? 1 : 0;
                double denom;
                if (abs (n) == l)
                    denom = (2 * l) * (2 * l - 1);
                else
                    denom = l * l - n * n;

                double u = sqrt ((l * l - m * m) / denom);
                double v = sqrt ((1.0 + d) * (l + abs (m) - 1.0) * (l + abs (m)) / denom)
                           * (1.0 - 2.0 * d) * 0.5;
                double w = sqrt ((l - abs (m) - 1.0) * (l - abs (m)) / denom) * (1.0 - d) * (-0.5);

                if (u != 0.0)
                    u *= U (l, m, n, *Rone, *Rlm1);
                if (v != 0.0)
                    v *= V (l, m, n, *Rone, *Rlm1);
                if (w != 0.0)
                    w *= W (l, m, n, *Rone, *Rlm1);

                Rl->operator() (m + l, n + l) = u + v + w;
            }
        }
    }

    rotationParamsHaveChanged = false;
}

//==============================================================================
bool SceneRotatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SceneRotatorAudioProcessor::createEditor()
{
    return new SceneRotatorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void SceneRotatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    state.setProperty ("MidiDeviceName", juce::var (currentMidiDeviceInfo.name), nullptr);
    state.setProperty ("MidiDeviceScheme",
                       juce::var (static_cast<int> (currentMidiScheme)),
                       nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SceneRotatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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

            if (parameters.state.hasProperty ("MidiDeviceName"))
                openMidiInput (parameters.state.getProperty ("MidiDeviceName", juce::var ("")),
                               true);
            else
                closeMidiInput();

            if (parameters.state.hasProperty ("MidiDeviceScheme"))
                setMidiScheme (MidiScheme (static_cast<int> (
                    parameters.state.getProperty ("MidiDeviceScheme", juce::var (0)))));
        }

    usingYpr = true;
}

//==============================================================================
void SceneRotatorAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (! updatingParams.get())
    {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy"
            || parameterID == "qz")
        {
            usingYpr = false;
            updateEuler();
            rotationParamsHaveChanged = true;
        }
        else if (parameterID == "yaw" || parameterID == "pitch" || parameterID == "roll")
        {
            usingYpr = true;
            updateQuaternions();
            rotationParamsHaveChanged = true;
        }
    }

    if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
    }
    else if (parameterID == "invertYaw" || parameterID == "invertPitch"
             || parameterID == "invertRoll" || parameterID == "invertQuaternion")
    {
        if (usingYpr.get())
            updateQuaternions();
        else
            updateEuler();

        rotationParamsHaveChanged = true;
    }
    else if (parameterID == "rotationSequence")
    {
        if (usingYpr.get())
            updateQuaternions();
        else
            updateEuler();

        rotationParamsHaveChanged = true;
    }
}

inline void SceneRotatorAudioProcessor::updateQuaternions()
{
    const float wa = cos (Conversions<float>::degreesToRadians (*yaw) * 0.5f);
    const float za =
        sin (Conversions<float>::degreesToRadians (*yaw) * (*invertYaw >= 0.5 ? -0.5f : 0.5f));
    const float wb = cos (Conversions<float>::degreesToRadians (*pitch) * 0.5f);
    const float yb =
        sin (Conversions<float>::degreesToRadians (*pitch) * (*invertPitch >= 0.5 ? -0.5f : 0.5f));
    const float wc = cos (Conversions<float>::degreesToRadians (*roll) * 0.5f);
    const float xc =
        sin (Conversions<float>::degreesToRadians (*roll) * (*invertRoll >= 0.5 ? -0.5f : 0.5f));

    float qw, qx, qy, qz;

    if (*rotationSequence >= 0.5f) // roll -> pitch -> yaw (extrinsic rotations)
    {
        qw = wa * wc * wb + za * xc * yb;
        qx = wa * xc * wb - za * wc * yb;
        qy = wa * wc * yb + za * xc * wb;
        qz = za * wc * wb - wa * xc * yb;
    }
    else // yaw -> pitch -> roll (extrinsic rotations)
    {
        qw = wc * wb * wa - xc * yb * za;
        qx = wc * yb * za + xc * wb * wa;
        qy = wc * yb * wa - xc * wb * za;
        qz = wc * wb * za + xc * yb * wa;
    }

    if (*invertQuaternion >= 0.5f)
    {
        qx = -qx;
        qy = -qy;
        qz = -qz;
    }

    updatingParams = true;
    parameters.getParameter ("qw")->setValueNotifyingHost (
        parameters.getParameterRange ("qw").convertTo0to1 (qw));
    parameters.getParameter ("qx")->setValueNotifyingHost (
        parameters.getParameterRange ("qx").convertTo0to1 (qx));
    parameters.getParameter ("qy")->setValueNotifyingHost (
        parameters.getParameterRange ("qy").convertTo0to1 (qy));
    parameters.getParameter ("qz")->setValueNotifyingHost (
        parameters.getParameterRange ("qz").convertTo0to1 (qz));
    updatingParams = false;
}

void SceneRotatorAudioProcessor::updateEuler()
{
    float ypr[3];
    auto quaternionDirection = iem::Quaternion<float> (*qw, *qx, *qy, *qz);
    quaternionDirection.normalize();

    if (*invertQuaternion >= 0.5f)
        quaternionDirection = quaternionDirection.getConjugate();

    // Thanks to Amy de Buitléir for this great algorithm!

    const float p0 = quaternionDirection.w;
    const float p1 = quaternionDirection.z;
    const float p2 = quaternionDirection.y;
    const float p3 = quaternionDirection.x;

    float e;

    if (*rotationSequence >= 0.5f) // roll -> pitch -> yaw (extrinsic rotations)
        e = -1.0f;
    else // yaw -> pitch -> roll (extrinsic rotations)
        e = 1.0f;

    // pitch (y-axis rotation)
    float t0 = 2.0f * (p0 * p2 + e * p1 * p3);
    t0 = juce::jlimit (-1.0f, 1.0f, t0);
    ypr[1] = asin (t0);

    if (ypr[1] == juce::MathConstants<float>::pi || ypr[1] == -juce::MathConstants<float>::pi)
    {
        ypr[2] = 0.0f;
        ypr[0] = atan2 (p1, p0);
    }
    else
    {
        // yaw (z-axis rotation)
        t0 = 2.0f * (p0 * p1 - e * p2 * p3);
        float t1 = 1.0f - 2.0f * (p1 * p1 + p2 * p2);
        ypr[0] = atan2 (t0, t1);

        // roll (x-axis rotation)
        t0 = 2.0f * (p0 * p3 - e * p1 * p2);
        t1 = 1.0f - 2.0f * (p2 * p2 + p3 * p3);
        ypr[2] = atan2 (t0, t1);
    }

    if (*invertYaw >= 0.5)
        ypr[0] *= -1.0f;
    if (*invertPitch >= 0.5)
        ypr[1] *= -1.0f;
    if (*invertRoll >= 0.5)
        ypr[2] *= -1.0f;

    //updating not active params
    updatingParams = true;
    parameters.getParameter ("yaw")->setValueNotifyingHost (
        parameters.getParameterRange ("yaw").convertTo0to1 (
            Conversions<float>::radiansToDegrees (ypr[0])));
    parameters.getParameter ("pitch")->setValueNotifyingHost (
        parameters.getParameterRange ("pitch").convertTo0to1 (
            Conversions<float>::radiansToDegrees (ypr[1])));
    parameters.getParameter ("roll")->setValueNotifyingHost (
        parameters.getParameterRange ("roll").convertTo0to1 (
            Conversions<float>::radiansToDegrees (ypr[2])));
    updatingParams = false;
}

void SceneRotatorAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());

    copyBuffer.setSize (input.getNumberOfChannels(), copyBuffer.getNumSamples());
}

//==============================================================================
const bool SceneRotatorAudioProcessor::interceptOSCMessage (juce::OSCMessage& message)
{
    juce::String prefix ("/" + juce::String (JucePlugin_Name));
    if (message.getAddressPattern().toString().equalsIgnoreCase (
            "/" + juce::String (JucePlugin_Name) + "/quaternions")
        && message.size() == 4)
    {
        float qs[4];
        for (int i = 0; i < 4; ++i)
            if (message[i].isFloat32())
                qs[i] = message[i].getFloat32();
            else if (message[i].isInt32())
                qs[i] = message[i].getInt32();

        oscParameterInterface.setValue ("qw", qs[0]);
        oscParameterInterface.setValue ("qx", qs[1]);
        oscParameterInterface.setValue ("qy", qs[2]);
        oscParameterInterface.setValue ("qz", qs[3]);
        return true;
    }
    else if (message.getAddressPattern().toString().equalsIgnoreCase (
                 "/" + juce::String (JucePlugin_Name) + "/ypr")
             && message.size() == 3)
    {
        float ypr[3];
        for (int i = 0; i < 3; ++i)
            if (message[i].isFloat32())
                ypr[i] = message[i].getFloat32();
            else if (message[i].isInt32())
                ypr[i] = message[i].getInt32();

        oscParameterInterface.setValue ("yaw", ypr[0]);
        oscParameterInterface.setValue ("pitch", ypr[1]);
        oscParameterInterface.setValue ("roll", ypr[2]);
        return true;
    }

    return false;
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    SceneRotatorAudioProcessor::createParameterLayout()
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
        "yaw",
        "Yaw Angle",
        juce::CharPointer_UTF8 (R"(°)"),
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.01f),
        0.0,
        [] (float value) { return juce::String (value, 2); },
        nullptr,
        true));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "pitch",
        "Pitch Angle",
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
        "invertYaw",
        "Invert Yaw",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "invertPitch",
        "Invert Pitch",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "invertRoll",
        "Invert Roll",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "invertQuaternion",
        "Invert Quaternion",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        0.0,
        [] (float value) { return value >= 0.5f ? "ON" : "OFF"; },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "rotationSequence",
        "Sequence of Rotations",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0,
        [] (float value) { return value >= 0.5f ? "Roll->Pitch->Yaw" : "Yaw->Pitch->Roll"; },
        nullptr));

    return params;
}

//==============================================================================
void SceneRotatorAudioProcessor::timerCallback()
{
    // retrying to connect to a desired device which might not be physically connected
    if (currentMidiDeviceInfo.identifier != "" && midiInput == nullptr)
        openMidiInput (currentMidiDeviceInfo);
}

//==============================================================================
juce::MidiDeviceInfo SceneRotatorAudioProcessor::getCurrentMidiDeviceInfo()
{
    return currentMidiDeviceInfo;
}

/**
 Open Midi input with just a given device name as string. Overloaded method for compatibility reasons
 */
void SceneRotatorAudioProcessor::openMidiInput (const juce::String midiDeviceName,
                                                bool forceUpdatingcurrentMidiDeviceInfo)
{
    const juce::ScopedLock scopedLock (changingMidiDevice);

    juce::Array<juce::MidiDeviceInfo> devices = juce::MidiInput::getAvailableDevices();
    juce::MidiDeviceInfo selectedMidiDevice;

    // Check which available device corresponds to the given device name
    for (int i = 0; i < devices.size(); ++i)
    {
        if (devices[i].name.compare (midiDeviceName) == 0)
        {
            selectedMidiDevice = devices[i];
            break;
        }
    }

    openMidiInput (selectedMidiDevice, forceUpdatingcurrentMidiDeviceInfo);
}

void SceneRotatorAudioProcessor::openMidiInput (juce::MidiDeviceInfo midiDevice,
                                                bool forceUpdatingcurrentMidiDeviceInfo)
{
    if (midiDevice.identifier.isEmpty())
        return closeMidiInput(); // <- not sure if that syntax is totally wrong or brilliant!

    const juce::ScopedLock scopedLock (changingMidiDevice);

    juce::Array<juce::MidiDeviceInfo> devices = juce::MidiInput::getAvailableDevices();

    const int index = devices.indexOf (midiDevice);
    //const auto deviceIdentifier 0;
    if (index != -1)
    {
        midiInput = juce::MidiInput::openDevice (devices[index].identifier, this);
        if (midiInput == nullptr)
        {
            deviceHasChanged = true;
            showMidiOpenError = true;
            return;
        }

        midiInput->start();

        DBG ("Opened MidiInput: " << midiInput->getName());

        currentMidiDeviceInfo = midiDevice;
        deviceHasChanged = true;

        return;
    }

    if (forceUpdatingcurrentMidiDeviceInfo)
    {
        currentMidiDeviceInfo = midiDevice;
        deviceHasChanged = true;
    }

    return;
}

void SceneRotatorAudioProcessor::closeMidiInput()
{
    const juce::ScopedLock scopedLock (changingMidiDevice);
    if (midiInput != nullptr)
    {
        midiInput->stop();
        midiInput.reset();
        DBG ("Closed MidiInput");
    }

    // It ain't beautiful, but it's honest work
    currentMidiDeviceInfo.name = "";
    currentMidiDeviceInfo.identifier = "";
    deviceHasChanged = true;

    return;
}

void SceneRotatorAudioProcessor::setMidiScheme (MidiScheme newMidiScheme)
{
    currentMidiScheme = newMidiScheme;
    DBG ("Scheme set to " << midiSchemeNames[static_cast<int> (newMidiScheme)]);

    switch (newMidiScheme)
    {
        case MidiScheme::none:
            break;

        case MidiScheme::mrHeadTrackerYprDir:
            parameters.getParameter ("rotationSequence")
                ->setValueNotifyingHost (1.0f); // roll->pitch->yaw
            break;

        case MidiScheme::mrHeadTrackerYprInv:
            parameters.getParameter ("rotationSequence")
                ->setValueNotifyingHost (1.0f); // roll->pitch->yaw
            break;

        case MidiScheme::mrHeadTrackerQuaternions:
            break;

        default:
            DBG ("Not supported MidiScheme - I guess the casting from int failed hard!");
            jassertfalse;
            break;
    }

    schemeHasChanged = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SceneRotatorAudioProcessor();
}
