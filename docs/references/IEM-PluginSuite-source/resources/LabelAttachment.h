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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

// This class is copied from juce::AudioProcessorValueTreeState.cpp to make it accessible.
struct AttachedControlBase : public juce::AudioProcessorValueTreeState::Listener,
                             public juce::AsyncUpdater
{
    AttachedControlBase (juce::AudioProcessorValueTreeState& s, const juce::String& p) :
        state (s), paramID (p), lastValue (0)
    {
        state.addParameterListener (paramID, this);
    }

    void removeListener() { state.removeParameterListener (paramID, this); }

    void setNewUnnormalisedValue (float newUnnormalisedValue)
    {
        if (juce::AudioProcessorParameter* p = state.getParameter (paramID))
        {
            const float newValue =
                state.getParameterRange (paramID).convertTo0to1 (newUnnormalisedValue);

            if (p->getValue() != newValue)
                p->setValueNotifyingHost (newValue);
        }
    }

    void setNewNormalisedValue (float newNormalisedValue)
    {
        if (juce::AudioProcessorParameter* p = state.getParameter (paramID))
        {
            if (p->getValue() != newNormalisedValue)
                p->setValueNotifyingHost (newNormalisedValue);
        }
    }

    void sendInitialUpdate()
    {
        if (std::atomic<float>* v = state.getRawParameterValue (paramID))
            parameterChanged (paramID, *v);
    }

    void parameterChanged (const juce::String&, float newValue) override
    {
        lastValue = newValue;

        if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();
            setValue (newValue);
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void beginParameterChange()
    {
        if (juce::AudioProcessorParameter* p = state.getParameter (paramID))
        {
            if (state.undoManager != nullptr)
                state.undoManager->beginNewTransaction();

            p->beginChangeGesture();
        }
    }

    void endParameterChange()
    {
        if (juce::AudioProcessorParameter* p = state.getParameter (paramID))
            p->endChangeGesture();
    }

    void handleAsyncUpdate() override { setValue (lastValue); }

    virtual void setValue (float) = 0;

    juce::AudioProcessorValueTreeState& state;
    juce::String paramID;
    float lastValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttachedControlBase)
};

// This one I wrote myself ;-)
class LabelAttachment : private AttachedControlBase, private juce::Label::Listener
{
public:
    LabelAttachment (juce::AudioProcessorValueTreeState& stateToControl,
                     const juce::String& parameterID,
                     juce::Label& labelToControl) :
        AttachedControlBase (stateToControl, parameterID),
        label (labelToControl),
        ignoreCallbacks (false)
    {
        parameter = state.getParameter (paramID);
        sendInitialUpdate();
        label.addListener (this);
    }

    ~LabelAttachment()
    {
        label.removeListener (this);
        removeListener();
    }

    void labelTextChanged (juce::Label* labelThatHasChanged) override
    {
        auto newValue = getNormalizedValueFromText (label.getText());
        const juce::ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            beginParameterChange();
            setNewUnnormalisedValue (newValue);
            endParameterChange();
        }

        updateText();
    }

    float getNormalizedValueFromText (const juce::String& text)
    {
        float value = text.getFloatValue();
        return value;
    }

    void setValue (float newValue) override
    {
        const juce::ScopedLock selfCallbackLock (selfCallbackMutex);

        {
            juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            updateText();
        }
    }

    void updateText()
    {
        juce::String text = parameter->getText (parameter->getValue(), 2) + " " + parameter->label;
        label.setText (text, juce::NotificationType::dontSendNotification);
    }

private:
    juce::Label& label;
    bool ignoreCallbacks;
    juce::CriticalSection selfCallbackMutex;

    const juce::AudioProcessorParameterWithID* parameter { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabelAttachment)
};
