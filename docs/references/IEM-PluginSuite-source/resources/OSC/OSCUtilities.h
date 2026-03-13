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

/**
 An extension to JUCE's OSCReceiver class with some useful methods.
 */

class OSCReceiverPlus : public juce::OSCReceiver
{
public:
    OSCReceiverPlus() { connected = false; }

    bool connect (int portNumber)
    {
        port = portNumber;
        if (portNumber == -1)
        {
            disconnect();
            connected = false;
            return true;
        }

        if (OSCReceiver::connect (port))
        {
            connected = true;
            return true;
        }
        else
            return false;
    }

    bool disconnect()
    {
        if (OSCReceiver::disconnect())
        {
            connected = false;
            return true;
        }
        else
            return false;
    }

    int getPortNumber() const { return port; }

    bool isConnected() const { return connected.get(); }

private:
    int port = -1;
    juce::Atomic<bool> connected;
};

class OSCSenderPlus : public juce::OSCSender
{
public:
    OSCSenderPlus() { connected = false; }

    bool connect (const juce::String& targetHostName, int portNumber)
    {
        hostName = targetHostName;
        port = portNumber;

        if (portNumber == -1 || targetHostName.isEmpty())
        {
            disconnect();
            connected = false;
            return true;
        }

        if (juce::OSCSender::connect (targetHostName, port))
        {
            connected = true;
            return true;
        }
        else
            return false;
    }

    bool disconnect()
    {
        if (OSCSender::disconnect())
        {
            connected = false;
            return true;
        }
        else
            return false;
    }

    int getPortNumber() const { return port; }

    juce::String getHostName() const { return hostName; }

    bool isConnected() const { return connected.get(); }

private:
    juce::String hostName;
    int port = -1;
    juce::Atomic<bool> connected;
};

class OSCMessageInterceptor
{
public:
    virtual ~OSCMessageInterceptor() = default;

    /**
     This method is expected to return true, if the juce::OSCMessage is considered to have been consumed, and should not be passed on.
     */
    virtual inline const bool interceptOSCMessage (juce::OSCMessage& message)
    {
        ignoreUnused (message);
        return false; // not consumed
    }

    /**
     This method will be called if the OSC message wasn't consumed by both 'interceptOscMessage(...)' and the oscParameterInterface.processOSCmessage(...)' method.
     The method is expected to return true, if the SOCMessage is considered to have been consumed, and should not be passed on.
     */
    virtual inline const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message)
    {
        ignoreUnused (message);
        return false;
    }

    /**
     Use this method to send additional juce::OSCMessages during the OSCSender's send routine.
     */
    virtual void sendAdditionalOSCMessages (juce::OSCSender&, const juce::OSCAddressPattern&) {}
};
