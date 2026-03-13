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

#include "OSCStatus.h"

OSCDialogWindow::OSCDialogWindow (OSCParameterInterface& oscInterface,
                                  OSCReceiverPlus& oscReceiver,
                                  OSCSenderPlus& oscSender) :
    interface (oscInterface), receiver (oscReceiver), sender (oscSender)
{
    //==== Receiver =====================================
    isReceiverConnected = receiver.isConnected();

    addAndMakeVisible (receiverGroup);
    receiverGroup.setText ("OSC Receiver");

    addAndMakeVisible (slRecPort);
    slRecPort.setText ("Listen to port", false, juce::Justification::centred);

    addAndMakeVisible (lbRPort);
    const int receiverPort = receiver.getPortNumber();
    lbRPort.setText (receiverPort == -1 ? "none" : juce::String (receiverPort),
                     juce::NotificationType::dontSendNotification);
    lbRPort.setEditable (true);
    lbRPort.setJustificationType (juce::Justification::centred);
    lbRPort.addListener (this);

    addAndMakeVisible (tbReceiverOpen);
    tbReceiverOpen.setButtonText (isReceiverConnected ? "CLOSE" : "OPEN");
    tbReceiverOpen.setColour (juce::TextButton::buttonColourId,
                              isReceiverConnected ? juce::Colours::orangered
                                                  : juce::Colours::limegreen);
    tbReceiverOpen.onClick = [this]() { checkPortAndConnectReceiver(); };

    //==== Receiver =====================================
    isSenderConnected = sender.isConnected();

    addAndMakeVisible (senderGroup);
    senderGroup.setText ("OSC Sender");

    addAndMakeVisible (lbSPort);
    const int senderPort = sender.getPortNumber();
    lbSPort.setText (senderPort == -1 ? "none" : juce::String (senderPort),
                     juce::NotificationType::dontSendNotification);
    lbSPort.setEditable (true);
    lbSPort.setJustificationType (juce::Justification::centred);
    lbSPort.addListener (this);

    addAndMakeVisible (lbSHostname);
    const auto senderHostName = sender.getHostName();
    lbSHostname.setText (senderHostName, juce::NotificationType::dontSendNotification);
    lbSHostname.setEditable (true);
    lbSHostname.setJustificationType (juce::Justification::centred);
    lbSHostname.addListener (this);

    addAndMakeVisible (lbSOSCAddress);
    const auto senderOSCAddress = interface.getOSCAddress();
    lbSOSCAddress.setText (senderOSCAddress, juce::NotificationType::dontSendNotification);
    lbSOSCAddress.setEditable (true);
    lbSOSCAddress.setJustificationType (juce::Justification::centred);
    lbSOSCAddress.onTextChange = [this]() { updateOSCAddress(); };

    addAndMakeVisible (tbSenderOpen);
    tbSenderOpen.setButtonText (isSenderConnected ? "DISCONNECT" : "CONNECT");
    tbSenderOpen.setColour (juce::TextButton::buttonColourId,
                            isSenderConnected ? juce::Colours::orangered
                                              : juce::Colours::limegreen);
    tbSenderOpen.onClick = [this]() { checkPortAndConnectSender(); };

    addAndMakeVisible (slSendIP);
    slSendIP.setText ("IP", false, juce::Justification::centred);

    addAndMakeVisible (slSendPort);
    slSendPort.setText ("Port", false, juce::Justification::centred);

    addAndMakeVisible (slSendName);
    slSendName.setText ("OSC Addr.", false, juce::Justification::centred);

    addAndMakeVisible (tbFlush);
    tbFlush.setButtonText ("Flush Params");
    tbFlush.setColour (juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
    tbFlush.onClick = [this]() { interface.sendParameterChanges (true); };

    addAndMakeVisible (intervalSlider);
    intervalSlider.setRange (1, 1000, 1);
    intervalSlider.setValue (interface.getInterval());
    intervalSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    intervalSlider.setColour (juce::Slider::rotarySliderOutlineColourId,
                              juce::Colours::cornflowerblue);
    intervalSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 14);
    intervalSlider.setTextValueSuffix (" ms");
    intervalSlider.onValueChange = [&]() { interface.setInterval (intervalSlider.getValue()); };

    addAndMakeVisible (slInterval);
    slInterval.setText ("Interval");

    startTimer (500);
}

void OSCDialogWindow::timerCallback()
{
    bool shouldReceiverBeConnected = receiver.isConnected();
    if (isReceiverConnected != shouldReceiverBeConnected)
    {
        isReceiverConnected = shouldReceiverBeConnected;
        tbReceiverOpen.setButtonText (isReceiverConnected ? "CLOSE" : "OPEN");
        tbReceiverOpen.setColour (juce::TextButton::buttonColourId,
                                  isReceiverConnected ? juce::Colours::orangered
                                                      : juce::Colours::limegreen);
        repaint();
    }

    bool shouldSenderBeConnected = sender.isConnected();
    if (isSenderConnected != shouldSenderBeConnected)
    {
        isSenderConnected = shouldSenderBeConnected;
        tbSenderOpen.setButtonText (isSenderConnected ? "DISCONNECT" : "CONNECT");
        tbSenderOpen.setColour (juce::TextButton::buttonColourId,
                                isSenderConnected ? juce::Colours::orangered
                                                  : juce::Colours::limegreen);
        repaint();
    }
}

void OSCDialogWindow::updateOSCAddress()
{
    interface.setOSCAddress (lbSOSCAddress.getText());

    lbSOSCAddress.setText (interface.getOSCAddress(), juce::NotificationType::dontSendNotification);
}

void OSCDialogWindow::labelTextChanged (juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &lbRPort)
    {
        DBG ("Receiver label changed");
        auto val = lbRPort.getTextValue();
        const int v = val.getValue();

        if (receiver.isConnected())
        {
            if (v == -1 || (v > 1000 && v < 15000))
            {
                receiver.disconnect();
                checkPortAndConnectReceiver();
            }
        }
    }
    else if (labelThatHasChanged == &lbSPort || labelThatHasChanged == &lbSHostname)
    {
        DBG ("Sender label changed");
        if (sender.isConnected())
        {
            sender.disconnect();
            checkPortAndConnectSender();
        }
    }
}

void OSCDialogWindow::checkPortAndConnectSender()
{
    if (sender.isConnected())
    {
        sender.disconnect();
    }
    else
    {
        if (lbSPort.getText() == "none" || lbSPort.getText() == "off")
        {
            sender.connect ("", -1);
            lbSPort.setText ("none", juce::NotificationType::dontSendNotification);
        }

        auto val = lbSPort.getTextValue();
        const int v = val.getValue();

        const auto ip = lbSHostname.getText();

        if (v == -1 || (v > 1000 && v < 15000))
        {
            if (! sender.connect (ip, v))
            {
                juce::AlertWindow alert (
                    "Connection could not be established!",
                    "Make sure the desired port is available and not already occupied by other clients. Also make sure the IP has a correct format!",
                    juce::AlertWindow::NoIcon,
                    this);
                alert.setLookAndFeel (&getLookAndFeel());
                alert.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
                alert.runModalLoop();
            }
        }
    }
}

void OSCDialogWindow::checkPortAndConnectReceiver()
{
    if (receiver.isConnected())
    {
        receiver.disconnect();
    }
    else
    {
        if (lbRPort.getText() == "none" || lbRPort.getText() == "off")
        {
            receiver.connect (-1);
            lbRPort.setText ("none", juce::NotificationType::dontSendNotification);
        }

        auto val = lbRPort.getTextValue();
        int v = val.getValue();

        if (v == -1 || (v > 1000 && v < 15000))
        {
            if (! receiver.connect (v))
            {
                juce::AlertWindow alert (
                    "Connection could not be established!",
                    "Make sure the desired port is available and not already occupied by other clients.",
                    juce::AlertWindow::NoIcon,
                    this);
                alert.setLookAndFeel (&getLookAndFeel());
                alert.addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
                alert.runModalLoop();
            }
        }
    }
}

void OSCDialogWindow::resized()
{
    auto bounds = getLocalBounds();

    //==== Receiver =================
    receiverGroup.setBounds (bounds.removeFromTop (25));

    auto row = bounds.removeFromTop (20);
    slRecPort.setBounds (row.removeFromLeft (80));
    row.removeFromLeft (3);
    lbRPort.setBounds (row.removeFromLeft (50));
    row.removeFromLeft (8);
    tbReceiverOpen.setBounds (row);

    bounds.removeFromTop (10);

    //==== Sender =================
    senderGroup.setBounds (bounds.removeFromTop (25));

    row = bounds.removeFromTop (20);
    slSendIP.setBounds (row.removeFromLeft (40));
    row.removeFromLeft (3);
    lbSHostname.setBounds (row);

    bounds.removeFromTop (5);
    row = bounds.removeFromTop (20);

    slSendPort.setBounds (row.removeFromLeft (40));
    row.removeFromLeft (3);
    lbSPort.setBounds (row.removeFromLeft (50));
    row.removeFromLeft (8);
    tbSenderOpen.setBounds (row);

    bounds.removeFromTop (5);
    row = bounds.removeFromTop (20);

    slSendName.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (1);
    lbSOSCAddress.setBounds (row);

    bounds.removeFromTop (5);

    row = bounds.removeFromTop (50);
    slInterval.setBounds (row.removeFromLeft (40));
    row.removeFromLeft (3);
    intervalSlider.setBounds (row.removeFromLeft (60));

    row.reduce (0, 15);
    tbFlush.setBounds (row.removeFromRight (80));
}

//==============================================================================
/*
 */

OSCStatus::OSCStatus (OSCParameterInterface& oscInterface) :
    oscParameterInterface (oscInterface),
    oscReceiver (oscInterface.getOSCReceiver()),
    oscSender (oscInterface.getOSCSender())
{
    isReceiverOpen = oscReceiver.isConnected();
    startTimer (500);
}

void OSCStatus::timerCallback()
{
    const int receiverPort = oscReceiver.getPortNumber();
    const int senderPort = oscSender.getPortNumber();
    const juce::String senderHostName = oscSender.getHostName();

    bool shouldReceiverBeConnected = oscReceiver.isConnected();
    bool shouldSenderBeConnected = oscSender.isConnected();

    if (isReceiverOpen != shouldReceiverBeConnected || lastReceiverPort != receiverPort)
    {
        lastReceiverPort = receiverPort;
        isReceiverOpen = shouldReceiverBeConnected;
        repaint();
    }

    if (isSenderOpen != shouldSenderBeConnected || lastSenderPort != senderPort
        || lastSenderHostName != senderHostName)
    {
        lastSenderPort = senderPort;
        lastSenderHostName = senderHostName;
        isSenderOpen = shouldSenderBeConnected;
        repaint();
    }
}

void OSCStatus::mouseMove (const juce::MouseEvent& event)
{
    if (! mouseOver && bounds.contains (event.getPosition()))
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        mouseOver = true;
        repaint();
    }
    else if (mouseOver && ! bounds.contains (event.getPosition()))
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
        mouseOver = false;
        repaint();
    }
}

void OSCStatus::mouseExit (const juce::MouseEvent& event)
{
    ignoreUnused (event);
    mouseOver = false;
    repaint();
}

void OSCStatus::mouseUp (const juce::MouseEvent& event)
{
    if (bounds.contains (event.getPosition()))
    {
        auto dialogWindow =
            std::make_unique<OSCDialogWindow> (oscParameterInterface, oscReceiver, oscSender);
        dialogWindow->setSize (211, 210);

        juce::CallOutBox& myBox = juce::CallOutBox::launchAsynchronously (
            std::move (dialogWindow),
            getScreenBounds().removeFromLeft (bounds.getWidth()),
            nullptr);
        myBox.setLookAndFeel (&getLookAndFeel());
    }
}

void OSCStatus::paint (juce::Graphics& g)
{
    juce::Colour receiveStatusColor =
        oscReceiver.getPortNumber() == -1 ? juce::Colours::white.withAlpha (0.1f)
        : oscReceiver.isConnected()       ? juce::Colours::limegreen
                                          : juce::Colours::red.withAlpha (0.5f);
    juce::Colour sendStatusColor = oscSender.getPortNumber() == -1
                                       ? juce::Colours::white.withAlpha (0.1f)
                                   : oscSender.isConnected() ? juce::Colours::limegreen
                                                             : juce::Colours::red.withAlpha (0.5f);

    const float alpha = mouseOver ? 1.0f : 0.5f;

    auto area = getLocalBounds();
    area = area.removeFromBottom (12);

    auto circleArea = area.removeFromLeft (12).toFloat().reduced (2.0f);
    circleArea.setY (circleArea.getY() - 1.0f);
    g.setColour (receiveStatusColor.withAlpha (alpha));
    g.drawRoundedRectangle (circleArea, 6, 1.0f);
    g.setColour (receiveStatusColor);
    g.fillRoundedRectangle (circleArea.removeFromLeft (14).reduced (2.0f), 6);

    area.removeFromLeft (2);

    circleArea = area.removeFromLeft (12).toFloat().reduced (2.0f);
    circleArea.setY (circleArea.getY() - 1.0f);
    g.setColour (sendStatusColor.withAlpha (alpha));
    g.drawRoundedRectangle (circleArea, 6, 1.0f);
    g.setColour (sendStatusColor);
    g.fillRoundedRectangle (circleArea.removeFromLeft (14).reduced (2.0f), 6);

    area.removeFromLeft (2);

    g.setColour (juce::Colours::white.withAlpha (mouseOver ? 1.0f : 0.5f));
    g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
    g.setFont (14.0f);

    juce::String text = "OSC";
    if (oscReceiver.isConnected() || oscSender.isConnected())
    {
        text += " (";
        if (oscReceiver.isConnected())
            text += "IN: " + juce::String (oscReceiver.getPortNumber());

        if (oscReceiver.isConnected() && oscSender.isConnected())
            text += " - ";

        if (oscSender.isConnected())
            text +=
                "OUT: " + oscSender.getHostName() + ":" + juce::String (oscSender.getPortNumber());

        text += ")";
    }

    auto textWidth = juce::roundToInt (g.getCurrentFont().getStringWidthFloat (text));

    const int targetSize = 12 + 2 + textWidth + 2 + 12;

    bounds = getLocalBounds().removeFromLeft (targetSize);

    g.drawText (text, area.withWidth (textWidth), juce::Justification::bottomLeft, true);
}
