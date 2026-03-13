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

#pragma once
#include "ReverseSlider.h"

//==============================================================================
/*
*/
class DoubleSlider : public juce::Component, public juce::Slider::Listener
{
public:
    DoubleSlider()
    {
        leftSlider.reset (new ReverseSlider ("left"));
        middleSlider.reset (new ReverseSlider ("middle"));
        rightSlider.reset (new ReverseSlider ("right"));

        addAndMakeVisible (leftSlider.get());
        addAndMakeVisible (middleSlider.get());
        addAndMakeVisible (rightSlider.get());

        leftSlider->setSliderStyle (juce::Slider::IncDecButtons);
        leftSlider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 50);
        leftSlider->setRange (minRange, maxRange, 1);
        leftSlider->setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_AutoDirection);
        leftSlider->addListener (this);

        middleSlider->setSliderStyle (juce::Slider::TwoValueHorizontal);
        middleSlider->setTextBoxStyle (juce::Slider::NoTextBox, false, 50, 50);
        middleSlider->addListener (this);

        rightSlider->setSliderStyle (juce::Slider::IncDecButtons);
        rightSlider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 50);
        rightSlider->setRange (minRange, maxRange, 1);
        rightSlider->setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_AutoDirection);
        rightSlider->addListener (this);
    }

    ~DoubleSlider() {}

    ReverseSlider* getLeftSliderAddress() { return leftSlider.get(); }
    ReverseSlider* getMiddleSliderAddress() { return middleSlider.get(); }
    ReverseSlider* getRightSliderAddress() { return rightSlider.get(); }

    void setLeftRightSliderWidth (float width)
    {
        leftRightSliderWidth = width;
        resized();
    };

    void setColour (juce::Colour colour)
    {
        middleSlider->setColour (juce::Slider::rotarySliderOutlineColourId, colour);
    };

    void setRangeAndPosition (juce::NormalisableRange<float> leftRange,
                              juce::NormalisableRange<float> rightRange)
    {
        minRange = juce::jmin (leftRange.start, rightRange.start);
        maxRange = juce::jmax (leftRange.end, rightRange.end);
        middleSlider->setRange (minRange, maxRange);
        middleSlider->setSkewFactor (leftRange.skew);

        middleSlider->setMinAndMaxValues (leftSlider->getValue(), rightSlider->getValue());
    };

    void setSkew (double skew)
    {
        leftSlider->setSkewFactor (skew);
        middleSlider->setSkewFactor (skew);
        rightSlider->setSkewFactor (skew);
    }

    void mouseDown (const juce::MouseEvent& event) override {};
    void mouseUp (const juce::MouseEvent& event) override {};
    void sliderDragStarted (juce::Slider* slider) override {};
    void sliderDragEnded (juce::Slider* slider) override {};
    void sliderValueChanged (juce::Slider* slider) override
    {
        if (slider->getName().equalsIgnoreCase ("middle"))
        {
            leftSlider->setValue (slider->getMinValue());
            rightSlider->setValue (slider->getMaxValue());
        }
        else if (slider->getName().equalsIgnoreCase ("left"))
        {
            middleSlider->setMinValue (leftSlider->getValue(), juce::dontSendNotification, true);
        }
        else if (slider->getName().equalsIgnoreCase ("right"))
        {
            middleSlider->setMaxValue (rightSlider->getValue(), juce::dontSendNotification, true);
        }
    };

    void paint (juce::Graphics& g) override {}

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        juce::Rectangle<int> bounds = getLocalBounds();

        leftSlider->setBounds (bounds.removeFromLeft (leftRightSliderWidth + buttonsWidth));
        leftSlider->setTextBoxStyle (juce::Slider::TextBoxLeft,
                                     false,
                                     leftRightSliderWidth,
                                     bounds.getHeight());

        rightSlider->setBounds (bounds.removeFromRight (leftRightSliderWidth + buttonsWidth));
        rightSlider->setTextBoxStyle (juce::Slider::TextBoxRight,
                                      false,
                                      leftRightSliderWidth,
                                      bounds.getHeight());

        middleSlider->setBounds (bounds);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoubleSlider)
    std::unique_ptr<ReverseSlider> leftSlider, middleSlider, rightSlider;
    float leftRightSliderWidth = 50;
    float minRange = 0;
    float maxRange = 1;
    float buttonsWidth = 30;
};
