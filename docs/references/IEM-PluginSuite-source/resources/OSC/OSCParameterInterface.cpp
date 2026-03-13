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

#include "OSCParameterInterface.h"
#include "AudioProcessorBase.h"

OSCParameterInterface::OSCParameterInterface (OSCMessageInterceptor& i,
                                              juce::AudioProcessorValueTreeState& valueTreeState) :
    interceptor (i), parameters (valueTreeState)
{
#ifdef DEBUG_PARAMETERS_FOR_DOCUMENTATION
    auto& params = parameters.processor.getParameters();
    for (auto& item : params)
    {
        if (auto* ptr = dynamic_cast<juce::AudioProcessorParameterWithID*> (
                item)) // that's maybe not the best solution, but it does the job for now
        {
            auto parameterID = ptr->paramID;
            auto parameterName = ptr->name;
            auto range = parameters.getParameterRange (parameterID);
            DBG ("| " << parameterID << " | " << range.getRange().getStart() << " : "
                      << range.getRange().getEnd() << " | " << parameterName << " | |");
        }
    }
#endif

    lastSentValues.resize (parameters.processor.getParameters().size());
    lastSentValues.fill (-1);
    setOSCAddress (juce::String (JucePlugin_Name));

    oscReceiver.addListener (this);

    startTimer (100);
}

std::unique_ptr<juce::RangedAudioParameter> OSCParameterInterface::createParameterTheOldWay (
    const juce::String& parameterID,
    const juce::String& parameterName,
    const juce::String& labelText,
    juce::NormalisableRange<float> valueRange,
    float defaultValue,
    std::function<juce::String (float)> valueToTextFunction,
    std::function<float (const juce::String&)> textToValueFunction,
    bool isMetaParameter,
    bool isAutomatableParameter,
    bool isDiscrete,
    juce::AudioProcessorParameter::Category category,
    bool isBoolean)
{
    return std::make_unique<juce::AudioProcessorValueTreeState::Parameter> (parameterID,
                                                                            parameterName,
                                                                            labelText,
                                                                            valueRange,
                                                                            defaultValue,
                                                                            valueToTextFunction,
                                                                            textToValueFunction,
                                                                            isMetaParameter,
                                                                            isAutomatableParameter,
                                                                            isDiscrete,
                                                                            category,
                                                                            isBoolean);
}

const bool OSCParameterInterface::processOSCMessage (juce::OSCMessage oscMessage)
{
    auto pattern = oscMessage.getAddressPattern();
    if (pattern.containsWildcards())
    {
        auto& params = parameters.processor.getParameters();
        for (auto& item : params)
        {
            if (auto* ptr = dynamic_cast<juce::AudioProcessorParameterWithID*> (
                    item)) // that's maybe not the best solution, but it does the job for now
            {
                auto paramAddress = ptr->paramID;
                if (pattern.matches (juce::OSCAddress ("/" + paramAddress)))
                {
                    if (oscMessage.size() > 0)
                    {
                        auto arg = oscMessage[0];
                        float value = 0.0f;
                        if (arg.isInt32())
                            value = arg.getInt32();
                        else if (arg.isFloat32())
                            value = arg.getFloat32();
                        else
                            return true;

                        setValue (paramAddress, value);
                    }
                }
            }
        }
    }

    juce::String paramAddress =
        oscMessage.getAddressPattern().toString().substring (1); // trimming forward slash
    if (auto parameter = parameters.getParameter (paramAddress))
    {
        if (oscMessage.size() > 0)
        {
            auto arg = oscMessage[0];
            float value = 0.0f;
            if (arg.isInt32())
                value = arg.getInt32();
            else if (arg.isFloat32())
                value = arg.getFloat32();
            else
                return true;

            setValue (paramAddress, value);
        }
        return true;
    }
    else
        return false;
}

void OSCParameterInterface::setValue (juce::String paramID, float value)
{
    auto range (parameters.getParameterRange (paramID));
    parameters.getParameter (paramID)->setValueNotifyingHost (range.convertTo0to1 (value));
}

void OSCParameterInterface::oscMessageReceived (const juce::OSCMessage& message)
{
    juce::OSCMessage messageCopy (message);
    if (! interceptor.interceptOSCMessage (messageCopy))
    {
        juce::String prefix ("/" + juce::String (JucePlugin_Name));
        if (message.getAddressPattern().toString().startsWith (prefix))
        {
            juce::OSCMessage msg (message);
            msg.setAddressPattern (message.getAddressPattern().toString().substring (
                juce::String (JucePlugin_Name).length() + 1));

            if (processOSCMessage (msg))
                return;
        }

        if (interceptor.processNotYetConsumedOSCMessage (message))
            return;

        // open/change osc port
        if (message.getAddressPattern().toString().equalsIgnoreCase ("/openOSCPort")
            && message.size() == 1)
        {
            int newPort = -1;

            if (message[0].isInt32())
                newPort = message[0].getInt32();
            else if (message[0].isFloat32())
                newPort = static_cast<int> (message[0].getFloat32());

            if (newPort > 0)
                juce::MessageManager::callAsync ([this, newPort]()
                                                 { oscReceiver.connect (newPort); });
        }

        if (message.getAddressPattern().toString().equalsIgnoreCase ("/flushParams"))
            juce::MessageManager::callAsync ([this]() { sendParameterChanges (true); });
    }
}

void OSCParameterInterface::oscBundleReceived (const juce::OSCBundle& bundle)
{
    for (int i = 0; i < bundle.size(); ++i)
    {
        auto elem = bundle[i];
        if (elem.isMessage())
            oscMessageReceived (elem.getMessage());
        else if (elem.isBundle())
            oscBundleReceived (elem.getBundle());
    }
}

void OSCParameterInterface::timerCallback()
{
    sendParameterChanges();
}

void OSCParameterInterface::sendParameterChanges (const bool forceSend)
{
    if (! oscSender.isConnected())
        return;

    auto& params = parameters.processor.getParameters();
    const int nParams = params.size();
    for (int i = 0; i < nParams; ++i)
    {
        auto item = params[i];
        if (auto* ptr = dynamic_cast<juce::AudioProcessorParameterWithID*> (
                item)) // that's maybe not the best solution, but it does the job for now
        {
            const auto normValue = ptr->getValue();

            if (forceSend || lastSentValues[i] != normValue)
            {
                lastSentValues.set (i, normValue);

                const auto paramID = ptr->paramID;
                auto range (parameters.getParameterRange (paramID));

                try
                {
                    juce::OSCMessage message (address + paramID, range.convertFrom0to1 (normValue));
                    oscSender.send (message);
                }
                catch (...)
                {
                };
            }
        }
    }

    interceptor.sendAdditionalOSCMessages (oscSender, address);
}

void OSCParameterInterface::setInterval (const int interValInMilliseconds)
{
    startTimer (juce::jlimit (1, 1000, interValInMilliseconds));
}

void OSCParameterInterface::setOSCAddress (juce::String newAddress)
{
    if (newAddress.isEmpty())
        address = "/";
    else
    {
        newAddress = newAddress.trimCharactersAtStart ("/");
        newAddress = newAddress.trimCharactersAtEnd ("/");
        newAddress = newAddress.removeCharacters (" öäü#*,?[]{}");

        if (newAddress.isEmpty())
            address = "/";
        else
            address = "/" + newAddress + "/";
    }
}

juce::ValueTree OSCParameterInterface::getConfig() const
{
    juce::ValueTree config ("OSCConfig");

    config.setProperty ("ReceiverPort", oscReceiver.getPortNumber(), nullptr);
    config.setProperty ("SenderIP", oscSender.getHostName(), nullptr);
    config.setProperty ("SenderPort", oscSender.getPortNumber(), nullptr);
    config.setProperty ("SenderOSCAddress", getOSCAddress(), nullptr);
    config.setProperty ("SenderInterval", getInterval(), nullptr);

    return config;
}

void OSCParameterInterface::setConfig (juce::ValueTree config)
{
    jassert (config.hasType ("OSCConfig"));

    oscReceiver.connect (config.getProperty ("ReceiverPort", -1));
    setOSCAddress (config.getProperty ("SenderOSCAddress", juce::String (JucePlugin_Name)));
    setInterval (config.getProperty ("SenderInterval", 100));
    oscSender.connect (config.getProperty ("SenderIP", ""), config.getProperty ("SenderPort", -1));
}
