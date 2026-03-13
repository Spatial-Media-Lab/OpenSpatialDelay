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

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
ProbeDecoderAudioProcessorEditor::ProbeDecoderAudioProcessorEditor (
    ProbeDecoderAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    footer (p.getOSCParameterInterface()),
    processor (p),
    valueTreeState (vts),
    probe (*valueTreeState.getParameter ("azimuth"),
           valueTreeState.getParameterRange ("azimuth"),
           *valueTreeState.getParameter ("elevation"),
           valueTreeState.getParameterRange ("elevation"))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 325);
    setLookAndFeel (&globalLaF);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible (&sphere);
    //sphere.addListener(this);

    probe.setColour (juce::Colours::aqua);
    sphere.addElement (&probe);

    // ======================================

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Probe"), juce::String ("Decoder"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);

    addAndMakeVisible (&footer);

    toolTipWin.setLookAndFeel (&globalLaF);
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    toolTipWin.setOpaque (false);

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));

    // ======================== YAW PITCH ROLL GROUP
    ypGroup.setText ("Azimuth & Elevation");
    ypGroup.setTextLabelPosition (juce::Justification::centredLeft);
    ypGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible (&ypGroup);
    ypGroup.setVisible (true);

    addAndMakeVisible (&slAzimuth);
    slAzimuthAttachment.reset (new SliderAttachment (valueTreeState, "azimuth", slAzimuth));
    slAzimuth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slAzimuth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slAzimuth.setReverse (true);
    slAzimuth.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slAzimuth.setRotaryParameters (juce::MathConstants<float>::pi,
                                   3 * juce::MathConstants<float>::pi,
                                   false);
    slAzimuth.setTooltip ("Azimuth angle");
    slAzimuth.setTextValueSuffix (juce::CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible (&slElevation);
    slElevationAttachment.reset (new SliderAttachment (valueTreeState, "elevation", slElevation));
    slElevation.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slElevation.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slElevation.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slElevation.setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                     2.5 * juce::MathConstants<float>::pi,
                                     false);
    slElevation.setTooltip ("Elevation angle");

    // ================ LABELS ===================
    addAndMakeVisible (&lbAzimuth);
    lbAzimuth.setText ("Azimuth");

    addAndMakeVisible (&lbElevation);
    lbElevation.setText ("Elevation");

    startTimer (20);
}

ProbeDecoderAudioProcessorEditor::~ProbeDecoderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void ProbeDecoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void ProbeDecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        sphere.repaint();
    }
}

void ProbeDecoderAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);

    juce::Rectangle<int> sliderRow;

    // ============== SIDEBAR RIGHT ====================
    // =================================================
    juce::Rectangle<int> sideBarArea (area.removeFromRight (190));

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;

    const int rotSliderWidth = 40;
    const int labelHeight = 15;

    // -------------- Yaw Pitch Roll ------------------
    juce::Rectangle<int> yprArea (sideBarArea.removeFromTop (25 + rotSliderHeight + labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop (25); //for box headline

    sliderRow = (yprArea.removeFromTop (rotSliderHeight));
    slAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    lbAzimuth.setBounds (yprArea.removeFromLeft (rotSliderWidth));
    yprArea.removeFromLeft (rotSliderSpacing - 5);
    lbElevation.setBounds (yprArea.removeFromLeft (rotSliderWidth + 10));

    sideBarArea.removeFromTop (20);

    // ============== SIDEBAR LEFT ====================

    area.removeFromRight (10); // spacing
    sphere.setBounds (area.getX(), area.getY(), area.getWidth() - 20, area.getWidth() - 20);
}
