/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2019 - Institute of Electronic Music and Acoustics (IEM)
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

#pragma once

#include <JuceHeader.h>

#include "IOHelper.h"
#include "OSC/OSCInputStream.h"
#include "OSC/OSCParameterInterface.h"

typedef std::vector<std::unique_ptr<juce::RangedAudioParameter>> ParameterList;

template <class inputType, class outputType, bool combined = false>

class AudioProcessorBase : public juce::AudioProcessor,
                           public OSCMessageInterceptor,
                           public juce::VSTCallbackHandler,
                           public IOHelper<inputType, outputType, combined>,
                           public juce::AudioProcessorValueTreeState::Listener
{
public:
    AudioProcessorBase() :
        AudioProcessor(),
        oscParameterInterface (*this, parameters),
        parameters (*this, nullptr, juce::String (JucePlugin_Name), {})
    {
    }

    AudioProcessorBase (ParameterList parameterLayout) :
        AudioProcessor(),
        parameters (*this,
                    nullptr,
                    juce::String (JucePlugin_Name),
                    { parameterLayout.begin(), parameterLayout.end() }),
        oscParameterInterface (*this, parameters)
    {
    }

    AudioProcessorBase (const BusesProperties& ioLayouts, ParameterList parameterLayout) :
        AudioProcessor (ioLayouts),
        parameters (*this,
                    nullptr,
                    juce::String (JucePlugin_Name),
                    { parameterLayout.begin(), parameterLayout.end() }),
        oscParameterInterface (*this, parameters)
    {
    }

    ~AudioProcessorBase() override {}

    //======== AudioProcessor stuff  =======================================================

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        ignoreUnused (layouts);
        return true;
    }
#endif

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool producesMidi() const override
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    double getTailLengthSeconds() const override { return 0.0; }

    //======== VSTCallbackHandler =======================================================
    juce::pointer_sized_int handleVstManufacturerSpecific (juce::int32 index,
                                                           juce::pointer_sized_int value,
                                                           void* ptr,
                                                           float opt) override
    {
        juce::ignoreUnused (opt);

        //0x69656D is hex code for `iem` in ASCII
        if (index == 0x0069656D) // prefix 00 chooses OSC message
        {
            try
            {
                size_t size = static_cast<size_t> (value); // let's make this the data size

                MyOSCInputStream inputStream (ptr, size);
                auto inMessage = inputStream.readMessage();

                oscParameterInterface.oscMessageReceived (inMessage);
                return 1;
            }
            catch (const juce::OSCFormatError&)
            {
                return -1;
            }
        }

        return 0;
    }

    juce::pointer_sized_int handleVstPluginCanDo (juce::int32 index,
                                                  juce::pointer_sized_int value,
                                                  void* ptr,
                                                  float opt) override
    {
        juce::ignoreUnused (index, value, opt);

        auto text = (const char*) ptr;
        auto matches = [=] (const char* s) { return strcmp (text, s) == 0; };

        if (matches ("wantsChannelCountNotifications"))
            return 1;

        if (matches ("hasIEMExtensions"))
            return 1;

        return 0;
    }
    //==============================================================================

    OSCParameterInterface& getOSCParameterInterface() { return oscParameterInterface; }

    //==============================================================================

    juce::AudioProcessorValueTreeState parameters;
    OSCParameterInterface oscParameterInterface;

private:
    bool shouldOpenNewPort = false;
    int newPortNumber = -1;
};
