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
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "OSCUtilities.h"
#include <sys/types.h>

#if defined(_MSC_VER)
    #include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

//#define DEBUG_PARAMETERS_FOR_DOCUMENTATION

/**
 This class can be used to add parameters to a AudioProcessorValueTree and make them controllable via OSC. The used parameterID will be saved in a juce::StringArray. If the OSCPattern of the forwarded OSCMessages matches one of the parameterIDs, that Parameter will be controlled.
 */

class OSCParameterInterface
    : public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
      private juce::Timer
{
public:
    OSCParameterInterface (OSCMessageInterceptor& interceptor,
                           juce::AudioProcessorValueTreeState& valueTreeState);

    static std::unique_ptr<juce::RangedAudioParameter> createParameterTheOldWay (
        const juce::String& parameterID,
        const juce::String& parameterName,
        const juce::String& labelText,
        juce::NormalisableRange<float> valueRange,
        float defaultValue,
        std::function<juce::String (float)> valueToTextFunction = nullptr,
        std::function<float (const juce::String&)> textToValueFunction = nullptr,
        bool isMetaParameter = false,
        bool isAutomatableParameter = true,
        bool isDiscrete = false,
        juce::AudioProcessorParameter::Category category =
            juce::AudioProcessorParameter::genericParameter,
        bool isBoolean = false);

    /**
     Checks whether the OSCAdressPattern of the OSCMessage matches one of the ParameterID's and changes the parameter on success. Returns true, if there is a match. Make sure the plugin-name-prefix was trimmed.
     */
    const bool processOSCMessage (juce::OSCMessage oscMessage);

    /**
     Sets the value of an audio-parameter with the specified parameter ID. The provided value will be mapped to a 0-to-1 range.
     */
    void setValue (juce::String paramID, float value);

    OSCReceiverPlus& getOSCReceiver() { return oscReceiver; }
    OSCSenderPlus& getOSCSender() { return oscSender; }

    void oscMessageReceived (const juce::OSCMessage& message) override;
    void oscBundleReceived (const juce::OSCBundle& bundle) override;

    void timerCallback() override;

    void sendParameterChanges (bool forceSend = false);
    void setOSCAddress (juce::String newAddress);

    juce::String getOSCAddress() const { return address; }

    void setInterval (int interValInMilliseconds);
    int getInterval() const { return getTimerInterval(); }

    juce::ValueTree getConfig() const;
    void setConfig (juce::ValueTree config);

private:
    OSCMessageInterceptor& interceptor;
    juce::AudioProcessorValueTreeState& parameters;

    OSCReceiverPlus oscReceiver;
    OSCSenderPlus oscSender;

    juce::String address;
    juce::Array<float> lastSentValues;
};
