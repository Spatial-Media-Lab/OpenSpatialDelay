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

/* Parts of this code originate from Yair Chuchem's juce::AudioProcessorParameterSlider class:
 https://gist.github.com/yairchu */

#pragma once

#include <JuceHeader.h>

#define RS_FLT_EPSILON 1.19209290E-07F
class ReverseSlider : public juce::Slider
{
public:
    ReverseSlider() :
        juce::Slider(),
        lastDistanceFromDragStart (0),
        reversed (false),
        isDual (false),
        scrollWheelEnabled (true)
    {
    }

    ReverseSlider (const juce::String& componentName) :
        juce::Slider (componentName),
        lastDistanceFromDragStart (0),
        reversed (false),
        isDual (false),
        scrollWheelEnabled (true)
    {
    }

public:
    class SliderAttachment : public juce::AudioProcessorValueTreeState::SliderAttachment
    {
    public:
        SliderAttachment (juce::AudioProcessorValueTreeState& stateToControl,
                          const juce::String& parameterID,
                          ReverseSlider& sliderToControl) :
            juce::AudioProcessorValueTreeState::SliderAttachment (stateToControl,
                                                                  parameterID,
                                                                  sliderToControl)
        {
            sliderToControl.setParameter (stateToControl.getParameter (parameterID));
        }

        SliderAttachment (juce::AudioProcessorValueTreeState& stateToControl,
                          const juce::String& parameterID,
                          juce::Slider& sliderToControl) :
            juce::AudioProcessorValueTreeState::SliderAttachment (stateToControl,
                                                                  parameterID,
                                                                  sliderToControl)
        {
        }

        virtual ~SliderAttachment() = default;
    };

    void setReverse (bool shouldBeReversed)
    {
        if (reversed != shouldBeReversed)
        {
            reversed = shouldBeReversed;
            repaint();
        }
    }
    void setDual (bool shouldBeDual)
    {
        if (isDual != shouldBeDual)
        {
            isDual = shouldBeDual;
            repaint();
        }
    }

    void setParameter (const juce::AudioProcessorParameter* p)
    {
        if (parameter == p)
            return;
        parameter = p;
        updateText();
        repaint();
    }

    juce::String getTextFromValue (double value) override
    {
        if (parameter == nullptr)
            return juce::Slider::getTextFromValue (value);

        const juce::NormalisableRange<double> range (getMinimum(),
                                                     getMaximum(),
                                                     getInterval(),
                                                     getSkewFactor());
        const float normalizedVal = (float) range.convertTo0to1 (value);

        juce::String result = parameter->getText (normalizedVal, getNumDecimalPlacesToDisplay())
                              + " " + parameter->getLabel();
        return result;
    }

    double getValueFromText (const juce::String& text) override
    {
        if (parameter == nullptr)
            return juce::Slider::getValueFromText (text);
        const juce::NormalisableRange<double> range (getMinimum(),
                                                     getMaximum(),
                                                     getInterval(),
                                                     getSkewFactor());
        return range.convertFrom0to1 (parameter->getValueForText (text));
    }

    double proportionOfLengthToValue (double proportion) override
    {
        double ret = 0;
        if (reversed)
            ret =
                getMaximum() + getMinimum() - juce::Slider::proportionOfLengthToValue (proportion);
        else
            ret = juce::Slider::proportionOfLengthToValue (proportion);
        return ret;
    }

    double valueToProportionOfLength (double value) override
    {
        double ret = 0;
        if (reversed)
            ret = juce::jlimit (0., 1., 1.0 - juce::Slider::valueToProportionOfLength (value));
        else
            ret = juce::Slider::valueToProportionOfLength (value);
        return ret;
    }

    void increment() { setValue (getValue() + getInterval()); }

    void decrement() { setValue (getValue() - getInterval()); }

    void setScrollWheelEnabled (bool enabled)
    {
        scrollWheelEnabled = enabled;
        juce::Slider::setScrollWheelEnabled (enabled);
    }
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (isRotary() && ! getRotaryParameters().stopAtEnd && scrollWheelEnabled)
        {
            const double delta =
                (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY)
                * (wheel.isReversed ? -1.0f : 1.0f) * (reversed ? -1.0f : 1.0f);
            bool positiveDelta = delta >= 0.0;

            if (std::abs (getValue() - getMaximum()) < getInterval()
                || std::abs (getValue() - getMaximum()) < RS_FLT_EPSILON)
            {
                if (positiveDelta)
                    setValue (getMinimum());
            }
            else if (std::abs (getValue() - getMinimum()) < getInterval()
                     || std::abs (getValue() - getMinimum()) < RS_FLT_EPSILON)
            {
                if (! positiveDelta)
                    setValue (getMaximum());
            }
        }
        juce::Slider::mouseWheelMove (e, wheel);
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        lastDistanceFromDragStart = 0;
        juce::Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (isRotary() && ! getRotaryParameters().stopAtEnd && scrollWheelEnabled)
        {
            int delta = 0;
            switch (getSliderStyle())
            {
                case RotaryVerticalDrag:
                    delta = -e.getDistanceFromDragStartY() - lastDistanceFromDragStart;
                    break;
                case RotaryHorizontalDrag:
                    delta = e.getDistanceFromDragStartX() - lastDistanceFromDragStart;
                    break;
                case RotaryHorizontalVerticalDrag:
                    delta = e.getDistanceFromDragStartX() - e.getDistanceFromDragStartY()
                            - lastDistanceFromDragStart;
                    break;
                default:
                    break;
            }
            delta = delta * (reversed ? -1 : 1);

            if (std::abs (getValue() - getMaximum()) < getInterval()
                || std::abs (getValue() - getMaximum()) < RS_FLT_EPSILON)
            {
                if (delta > 0)
                {
                    setValue (getMinimum());
                    juce::Slider::mouseDown (e); //hack
                }
            }
            else if (std::abs (getValue() - getMinimum()) < getInterval()
                     || std::abs (getValue() - getMinimum()) < RS_FLT_EPSILON)
            {
                if (delta < 0)
                {
                    setValue (getMaximum());
                    juce::Slider::mouseDown (e); //hack
                }
            }
        }

        switch (getSliderStyle())
        {
            case RotaryVerticalDrag:
                lastDistanceFromDragStart = -e.getDistanceFromDragStartY();
                break;
            case RotaryHorizontalDrag:
                lastDistanceFromDragStart = e.getDistanceFromDragStartX();
                break;
            case RotaryHorizontalVerticalDrag:
                lastDistanceFromDragStart =
                    e.getDistanceFromDragStartX() - e.getDistanceFromDragStartY();
                break;
            default:
                break;
        }

        juce::Slider::mouseDrag (e);
    }

private:
    int lastDistanceFromDragStart;
    bool reversed;
    bool isDual;
    bool scrollWheelEnabled;
    const juce::AudioProcessorParameter* parameter { nullptr };
};
