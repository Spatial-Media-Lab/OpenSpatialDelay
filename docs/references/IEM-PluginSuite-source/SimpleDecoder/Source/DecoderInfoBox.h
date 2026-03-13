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
#include "ReferenceCountedDecoder.h"
#include "ambisonicTools.h"

//==============================================================================
/*
 */
class DecoderInfoBox : public juce::Component
{
    static constexpr int attributeHeight = 12;
    static constexpr int valueHeight = 17;
    static constexpr int spacing = 5;

public:
    DecoderInfoBox (juce::AudioProcessorValueTreeState& parameters)
    {
        addAndMakeVisible (cbWeights);
        cbWeights.setJustificationType (juce::Justification::centred);
        cbWeights.addItemList ({ "basic", "maxrE", "inPhase" }, 1);
        cbWeightsAttachment.reset (
            new juce::AudioProcessorValueTreeState::ComboBoxAttachment (parameters,
                                                                        "weights",
                                                                        cbWeights));
    }

    ~DecoderInfoBox() {}

    void setDecoderConfig (ReferenceCountedDecoder::Ptr newDecoderConfig)
    {
        decoder = newDecoderConfig;
        if (decoder == nullptr)
            cbWeights.setVisible (false);
        else
            cbWeights.setVisible (true);

        resized();
        repaint();
    }

    const int getMaxAttributeWidth()
    {
        auto font = juce::Font (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        font.setHeight (attributeHeight);
        return font.getStringWidth ("LOUDSPEAKERS:");
    }

    void resized() override
    {
        auto retainedDecoder = decoder;
        juce::Rectangle<int> bounds = getLocalBounds();
        arr.clear();

        if (retainedDecoder != nullptr)
        {
            const auto maxAttWidth = getMaxAttributeWidth();
            const int valueStart = maxAttWidth + spacing;
            const int valueWidth = juce::jmax (bounds.getWidth() - valueStart, 0);

            auto font = juce::Font (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
            font.setHeight (valueHeight);

            arr.addFittedText (font,
                               retainedDecoder->getDescription(),
                               valueStart,
                               valueHeight + 1,
                               valueWidth,
                               3 * valueHeight,
                               juce::Justification::topLeft,
                               4,
                               0.8f);

            const int descriptionEnd =
                arr.getBoundingBox (juce::jmax (0, arr.getNumGlyphs() - 1), 1, true).getBottom();

            cbWeights.setBounds (valueStart, descriptionEnd + 2 * valueHeight + 2, 80, valueHeight);
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto retainedDecoder = decoder;
        juce::Rectangle<int> bounds = getLocalBounds();
        const int width = bounds.getWidth();

        g.setColour (juce::Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2))); // regular font

        if (retainedDecoder == nullptr)
        {
            g.setFont (valueHeight);
            g.drawText ("No configuration loaded.",
                        20,
                        1,
                        width,
                        valueHeight,
                        juce::Justification::bottomLeft);
            g.drawMultiLineText (errorText, 20, 30, width - 20);
        }
        else
        {
            g.setFont (attributeHeight);

            const int maxAttributeWidth = getMaxAttributeWidth();
            const int resStart = maxAttributeWidth + spacing;
            const int resWidth = juce::jmax (width - resStart, 0);

            g.drawText ("NAME:",
                        0,
                        0,
                        maxAttributeWidth,
                        valueHeight,
                        juce::Justification::bottomRight);
            g.drawText ("DESCRIPTION:",
                        0,
                        valueHeight,
                        maxAttributeWidth,
                        valueHeight,
                        juce::Justification::bottomRight);

            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1))); // bold font
            g.setFont (valueHeight);
            g.drawText (retainedDecoder->getName(),
                        resStart,
                        1,
                        resWidth,
                        valueHeight,
                        juce::Justification::bottomLeft);

            juce::String descriptionText = retainedDecoder->getDescription();

            arr.draw (g);
            const int descEnd =
                arr.getBoundingBox (juce::jmax (0, arr.getNumGlyphs() - 1), 1, true).getBottom();

            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2))); // regular font
            g.setFont (attributeHeight);

            g.drawText ("ORDER:",
                        0,
                        descEnd,
                        maxAttributeWidth,
                        valueHeight,
                        juce::Justification::bottomRight);
            g.drawText ("LOUDSPEAKERS:",
                        0,
                        descEnd + valueHeight,
                        maxAttributeWidth,
                        valueHeight,
                        juce::Justification::bottomRight);
            g.drawText ("WEIGHTS:",
                        0,
                        descEnd + 2 * valueHeight,
                        maxAttributeWidth,
                        valueHeight,
                        juce::Justification::bottomRight);

            g.setFont (valueHeight);
            g.drawText (getOrderString (retainedDecoder->getOrder()),
                        resStart,
                        descEnd + 1,
                        resWidth,
                        valueHeight,
                        juce::Justification::bottomLeft);
            g.drawText (juce::String (retainedDecoder->getMatrix().getNumRows()),
                        resStart,
                        descEnd + valueHeight + 1,
                        resWidth,
                        valueHeight,
                        juce::Justification::bottomLeft);
        }
    }

    void setErrorMessage (juce::String errorMessage)
    {
        errorText = errorMessage;
        repaint();
    }

private:
    juce::ComboBox cbWeights;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cbWeightsAttachment;

    juce::String errorText { "" };
    ReferenceCountedDecoder::Ptr decoder { nullptr };

    juce::GlyphArrangement arr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecoderInfoBox)
};
