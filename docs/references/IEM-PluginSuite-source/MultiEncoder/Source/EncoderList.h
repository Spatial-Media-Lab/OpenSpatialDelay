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
#include "PluginProcessor.h"
#include <JuceHeader.h>

typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

class ColourChangeButton : public juce::TextButton, public juce::ChangeListener
{
public:
    ColourChangeButton (MultiEncoderAudioProcessor& p,
                        SpherePanner& sphere,
                        SpherePanner::Element* elem,
                        int i) :
        juce::TextButton (juce::String (i)),
        processor (p),
        id (i),
        spherePanner (sphere),
        element (elem)
    {
        setSize (10, 24);
        changeWidthToFitText();
    }

    void clicked() override
    {
        auto colourSelector = std::make_unique<juce::ColourSelector>();
        colourSelector->setName ("background");
        colourSelector->setCurrentColour (findColour (juce::TextButton::buttonColourId));
        colourSelector->addChangeListener (this);
        colourSelector->setColour (juce::ColourSelector::backgroundColourId,
                                   juce::Colours::transparentBlack);
        colourSelector->setSize (300, 400);

        juce::CallOutBox::launchAsynchronously (std::move (colourSelector),
                                                getScreenBounds(),
                                                nullptr);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (juce::ColourSelector* cs = dynamic_cast<juce::ColourSelector*> (source))
        {
            processor.elementColours[id - 1] = cs->getCurrentColour();
            setColour (juce::TextButton::buttonColourId, cs->getCurrentColour());
            element->setColour (cs->getCurrentColour());
            setColour (juce::TextButton::textColourOffId,
                       juce::Colours::white.overlaidWith (cs->getCurrentColour()).contrasting());
            element->setTextColour (
                juce::Colours::white.overlaidWith (cs->getCurrentColour()).contrasting());
            spherePanner.repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto& lf = getLookAndFeel();

        juce::Rectangle<float> buttonArea (0.0f, 0.0f, getWidth(), getHeight());
        buttonArea.reduce (1.0f, 1.0f);

        const float width = getWidth() - 2;
        const float height = getHeight() - 2;

        if (width > 0 && height > 0)
        {
            const float cornerSize = juce::jmin (15.0f, juce::jmin (width, height) * 0.45f);
            juce::Path outline;
            outline.addRoundedRectangle (getX(),
                                         buttonArea.getY(),
                                         buttonArea.getWidth(),
                                         buttonArea.getHeight(),
                                         cornerSize,
                                         cornerSize);
            g.setColour (findColour (getToggleState() ? buttonOnColourId : buttonColourId));
            g.fillPath (outline);
        }

        lf.drawButtonText (g, *this, isMouseOver(), isMouseButtonDown());
    }

private:
    MultiEncoderAudioProcessor& processor;
    int id;
    SpherePanner& spherePanner;
    SpherePanner::Element* element;
};

class EncoderList : public juce::Component
{
public:
    EncoderList (MultiEncoderAudioProcessor& p,
                 SpherePanner& sphere,
                 juce::AudioProcessorValueTreeState* vts) :
        juce::Component(), processor (p), spherePanner (sphere), pVts (vts), nChannels (2)
    {
        setNumberOfChannels (nChannels);
    };
    ~EncoderList() {};

    void setNumberOfChannels (int nCh)
    {
        const int nElements = sphereElementArray.size();
        const int nExcessive = nElements - nCh;

        if (nExcessive > 0) // we have to delete some
        {
            for (int i = nCh; i < nElements; ++i)
            {
                spherePanner.removeElement (sphereElementArray[i]);
            }
            slAzimuthSliderAttachment.removeLast (nExcessive);
            slElevationAttachmentArray.removeLast (nExcessive);
            slGainAttachmentArray.removeLast (nExcessive);
            muteButtonAttachmentArray.removeLast (nExcessive);
            soloButtonAttachmentArray.removeLast (nExcessive);
            sphereElementArray.removeLast (nExcessive);
            colourChooserArray.removeLast (nExcessive);
            slAzimuthArray.removeLast (nExcessive);
            slElevationArray.removeLast (nExcessive);
            slGainArray.removeLast (nExcessive);
            muteButtonArray.removeLast (nExcessive);
            soloButtonArray.removeLast (nExcessive);
        }
        else // we have to add some
        {
            for (int i = nElements; i < nCh; ++i)
            {
                sphereElementArray.add (new SpherePanner::AzimuthElevationParameterElement (
                    *pVts->getParameter ("azimuth" + juce::String (i)),
                    pVts->getParameterRange ("azimuth" + juce::String (i)),
                    *pVts->getParameter ("elevation" + juce::String (i)),
                    pVts->getParameterRange ("elevation" + juce::String (i))));
                colourChooserArray.add (new ColourChangeButton (processor,
                                                                spherePanner,
                                                                sphereElementArray.getLast(),
                                                                i + 1));
                slAzimuthArray.add (new ReverseSlider());
                slElevationArray.add (new ReverseSlider());
                slGainArray.add (new ReverseSlider());
                muteButtonArray.add (new MuteSoloButton());
                soloButtonArray.add (new MuteSoloButton());

                auto* sphereElement = sphereElementArray.getLast();
                auto* colourChooser = colourChooserArray.getLast();
                auto* azimuthSlider = slAzimuthArray.getLast();
                auto* elevationSlider = slElevationArray.getLast();
                auto* gainSlider = slGainArray.getLast();
                auto* muteButton = muteButtonArray.getLast();
                auto* soloButton = soloButtonArray.getLast();

                sphereElement->setLabel (juce::String (i + 1));
                sphereElement->setColour (processor.elementColours[i]);
                sphereElement->setTextColour (
                    juce::Colours::white.overlaidWith (processor.elementColours[i]).contrasting());

                spherePanner.addElement (sphereElement);

                addAndMakeVisible (colourChooser);
                colourChooser->setColour (juce::TextButton::buttonColourId,
                                          processor.elementColours[i]);
                colourChooser->setColour (
                    juce::TextButton::textColourOffId,
                    juce::Colours::white.overlaidWith (processor.elementColours[i]).contrasting());

                addAndMakeVisible (azimuthSlider);
                slAzimuthSliderAttachment.add (
                    new SliderAttachment (*pVts, "azimuth" + juce::String (i), *azimuthSlider));
                azimuthSlider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                azimuthSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
                azimuthSlider->setReverse (true);
                azimuthSlider->setColour (juce::Slider::rotarySliderOutlineColourId,
                                          juce::Colour (0xFF00CAFF));
                azimuthSlider->setRotaryParameters (juce::MathConstants<float>::pi,
                                                    3 * juce::MathConstants<float>::pi,
                                                    false);
                azimuthSlider->setTooltip ("Azimuth angle");

                addAndMakeVisible (elevationSlider);
                slElevationAttachmentArray.add (
                    new SliderAttachment (*pVts, "elevation" + juce::String (i), *elevationSlider));
                elevationSlider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                elevationSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
                elevationSlider->setColour (juce::Slider::rotarySliderOutlineColourId,
                                            juce::Colour (0xFF4FFF00));
                elevationSlider->setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                                      2.5 * juce::MathConstants<float>::pi,
                                                      false);
                elevationSlider->setTooltip ("Elevation angle");

                addAndMakeVisible (gainSlider);
                slGainAttachmentArray.add (
                    new SliderAttachment (*pVts, "gain" + juce::String (i), *gainSlider));
                gainSlider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                gainSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
                gainSlider->setColour (juce::Slider::rotarySliderOutlineColourId,
                                       juce::Colour (0xFFD0011B));
                gainSlider->setReverse (false);
                gainSlider->setTooltip ("Gain");
                gainSlider->setTextValueSuffix ("dB");

                addAndMakeVisible (muteButton);
                muteButtonAttachmentArray.add (
                    new ButtonAttachment (*pVts, "mute" + juce::String (i), *muteButton));
                muteButton->setType (MuteSoloButton::Type::mute);

                addAndMakeVisible (soloButton);
                soloButtonAttachmentArray.add (
                    new ButtonAttachment (*pVts, "solo" + juce::String (i), *soloButton));
                soloButton->setType (MuteSoloButton::Type::solo);
            }
        }

        nChannels = nCh;

        setBounds (0,
                   0,
                   200,
                   nChannels * (63)); // const int rotSliderHeight = 55; const int labelHeight = 15;
    }

    void updateColours()
    {
        for (int i = 0; i < nChannels; ++i)
        {
            colourChooserArray[i]->setColour (juce::TextButton::buttonColourId,
                                              processor.elementColours[i]);
            colourChooserArray[i]->setColour (
                juce::TextButton::textColourOffId,
                juce::Colours::white.overlaidWith (processor.elementColours[i]).contrasting());
            sphereElementArray[i]->setColour (processor.elementColours[i]);
            sphereElementArray[i]->setTextColour (
                juce::Colours::white.overlaidWith (processor.elementColours[i]).contrasting());
        }
        repaint();
    }

    void paint (juce::Graphics& g) override {};

    void resized() override
    {
        juce::Rectangle<int> bounds = getBounds();
        juce::Rectangle<int> sliderRow;

        const int rotSliderSpacing = 10;
        const int rotSliderHeight = 55;
        const int rotSliderWidth = 40;

        for (int i = 0; i < nChannels; ++i)
        {
            sliderRow = bounds.removeFromTop (rotSliderHeight);
            colourChooserArray[i]->setBounds (sliderRow.removeFromLeft (22).reduced (0, 18));
            sliderRow.removeFromLeft (5);
            slAzimuthArray[i]->setBounds (sliderRow.removeFromLeft (rotSliderWidth));
            sliderRow.removeFromLeft (rotSliderSpacing);
            slElevationArray[i]->setBounds (sliderRow.removeFromLeft (rotSliderWidth));
            sliderRow.removeFromLeft (rotSliderSpacing);
            slGainArray[i]->setBounds (sliderRow.removeFromLeft (rotSliderWidth));
            sliderRow.removeFromLeft (rotSliderSpacing);

            sliderRow.reduce (0, 6);
            sliderRow.setWidth (18);
            soloButtonArray[i]->setBounds (sliderRow.removeFromTop (18));
            sliderRow.removeFromTop (5);
            muteButtonArray[i]->setBounds (sliderRow.removeFromTop (18));
            bounds.removeFromTop (8); //spacing
        }

        repaint();
    }
    juce::OwnedArray<SpherePanner::AzimuthElevationParameterElement> sphereElementArray;

    juce::OwnedArray<ReverseSlider>& getAzimuthArray() { return slAzimuthArray; }
    juce::OwnedArray<ReverseSlider>& getElevationArray() { return slElevationArray; }
    juce::OwnedArray<ReverseSlider>& getGainArray() { return slGainArray; }

private:
    MultiEncoderAudioProcessor& processor;
    SpherePanner& spherePanner;
    juce::AudioProcessorValueTreeState* pVts;
    int nChannels;

    juce::OwnedArray<ReverseSlider> slAzimuthArray, slElevationArray, slGainArray;
    juce::OwnedArray<MuteSoloButton> muteButtonArray, soloButtonArray;
    juce::OwnedArray<ColourChangeButton> colourChooserArray;
    juce::OwnedArray<SliderAttachment> slAzimuthSliderAttachment;
    juce::OwnedArray<SliderAttachment> slElevationAttachmentArray;
    juce::OwnedArray<SliderAttachment> slGainAttachmentArray;
    juce::OwnedArray<ButtonAttachment> muteButtonAttachmentArray;
    juce::OwnedArray<ButtonAttachment> soloButtonAttachmentArray;
};
