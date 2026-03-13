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
CoordinateConverterAudioProcessorEditor::CoordinateConverterAudioProcessorEditor (
    CoordinateConverterAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    panner (*valueTreeState.getParameter ("azimuth"),
            valueTreeState.getParameterRange ("azimuth"),
            *valueTreeState.getParameter ("elevation"),
            valueTreeState.getParameterRange ("elevation")),
    xyzPanner (*valueTreeState.getParameter ("xPos"),
               valueTreeState.getParameterRange ("xPos"),
               *valueTreeState.getParameter ("yPos"),
               valueTreeState.getParameterRange ("yPos"),
               *valueTreeState.getParameter ("zPos"),
               valueTreeState.getParameterRange ("zPos"))
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (470, 590, 800, 650); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Coordinate"), juce::String ("Converter"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    addAndMakeVisible (gcSpherical);
    gcSpherical.setText ("Spherical Coordinates");

    addAndMakeVisible (gcCartesian);
    gcCartesian.setText ("Cartesian Coordinates");

    addAndMakeVisible (gcRange);
    gcRange.setText ("Normalization / Range Settings");

    addAndMakeVisible (gcReference);
    gcReference.setText ("Reference Position");

    addAndMakeVisible (sphere);
    panner.setColour (juce::Colours::white);
    sphere.addElement (&panner);

    // ============== BEGIN: SPHERICAL COORDINATES ============

    addAndMakeVisible (slAzimuth);
    slAzimuthAttachment.reset (new SliderAttachment (valueTreeState, "azimuth", slAzimuth));
    slAzimuth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slAzimuth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slAzimuth.setReverse (true);
    slAzimuth.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slAzimuth.setRotaryParameters (juce::MathConstants<float>::pi,
                                   3 * juce::MathConstants<float>::pi,
                                   false);
    slAzimuth.setTooltip ("Azimuth angle");

    addAndMakeVisible (slElevation);
    slElevationAttachment.reset (new SliderAttachment (valueTreeState, "elevation", slElevation));
    slElevation.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slElevation.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slElevation.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slElevation.setRotaryParameters (0.5 * juce::MathConstants<float>::pi,
                                     2.5 * juce::MathConstants<float>::pi,
                                     false);
    slElevation.setTooltip ("Elevation angle");

    addAndMakeVisible (slRadius);
    slRadiusAttachment.reset (new SliderAttachment (valueTreeState, "radius", slRadius));
    slRadius.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slRadius.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slRadius.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slRadius.setReverse (false);
    slRadius.setTooltip ("Radius (normalized)");

    addAndMakeVisible (lbAzimuth);
    lbAzimuth.setText ("Azimuth");

    addAndMakeVisible (lbElevation);
    lbElevation.setText ("Elevation");

    addAndMakeVisible (lbRadius);
    lbRadius.setText ("Radius");

    addAndMakeVisible (tbAzimuthFlip);
    tbAzimuthFlipAttachment.reset (
        new ButtonAttachment (valueTreeState, "azimuthFlip", tbAzimuthFlip));
    tbAzimuthFlip.setColour (juce::ToggleButton::ColourIds::tickColourId,
                             globalLaF.ClWidgetColours[0]);
    tbAzimuthFlip.setButtonText ("Flip");

    addAndMakeVisible (tbElevationFlip);
    tbElevationFlipAttachment.reset (
        new ButtonAttachment (valueTreeState, "elevationFlip", tbElevationFlip));
    tbElevationFlip.setColour (juce::ToggleButton::ColourIds::tickColourId,
                               globalLaF.ClWidgetColours[1]);
    tbElevationFlip.setButtonText ("Flip");

    addAndMakeVisible (tbRadiusFlip);
    tbRadiusFlipAttachment.reset (
        new ButtonAttachment (valueTreeState, "radiusFlip", tbRadiusFlip));
    tbRadiusFlip.setColour (juce::ToggleButton::ColourIds::tickColourId,
                            globalLaF.ClWidgetColours[2]);
    tbRadiusFlip.setButtonText ("Flip");

    // ============== END: SPHERICAL COORDINATES ============

    // ============== BEGIN: CARTESIAN COORDINATES ============

    addAndMakeVisible (&xyPlane);
    xyPlane.addElement (&xyzPanner);
    xyPlane.useAutoScale (true);
    xyPlane.setDimensions ({ 2.0f, 2.0f, 2.0f });

    addAndMakeVisible (&zyPlane);
    zyPlane.setPlane (PositionPlane::Planes::zy);
    zyPlane.addElement (&xyzPanner);
    zyPlane.useAutoScale (true);
    zyPlane.setDimensions ({ 2.0f, 2.0f, 2.0f });

    addAndMakeVisible (slXPos);
    slXPosAttachment.reset (new SliderAttachment (valueTreeState, "xPos", slXPos));
    slXPos.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slXPos.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slXPos.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slXPos.setTooltip ("x coordinate (normalized)");

    addAndMakeVisible (slYPos);
    slYPosAttachment.reset (new SliderAttachment (valueTreeState, "yPos", slYPos));
    slYPos.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slYPos.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slYPos.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slYPos.setTooltip ("y coordinate (normalized)");

    addAndMakeVisible (slZPos);
    slZPosAttachment.reset (new SliderAttachment (valueTreeState, "zPos", slZPos));
    slZPos.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slZPos.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slZPos.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slZPos.setTooltip ("z coordinate (normalized)");

    addAndMakeVisible (lbXPos);
    lbXPos.setText ("X");

    addAndMakeVisible (lbYPos);
    lbYPos.setText ("Y");

    addAndMakeVisible (lbZPos);
    lbZPos.setText ("Z");

    addAndMakeVisible (tbXFlip);
    tbXFlipAttachment.reset (new ButtonAttachment (valueTreeState, "xFlip", tbXFlip));
    tbXFlip.setColour (juce::ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[0]);
    tbXFlip.setButtonText ("Flip");
    tbXFlip.addListener (this);

    addAndMakeVisible (tbYFlip);
    tbYFlipAttachment.reset (new ButtonAttachment (valueTreeState, "yFlip", tbYFlip));
    tbYFlip.setColour (juce::ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[1]);
    tbYFlip.setButtonText ("Flip");
    tbYFlip.addListener (this);

    addAndMakeVisible (tbZFlip);
    tbZFlipAttachment.reset (new ButtonAttachment (valueTreeState, "zFlip", tbZFlip));
    tbZFlip.setColour (juce::ToggleButton::ColourIds::tickColourId, globalLaF.ClWidgetColours[2]);
    tbZFlip.setButtonText ("Flip");
    tbZFlip.addListener (this);

    xyPlane.setXFlip (tbXFlip.getToggleState());
    xyPlane.setYFlip (tbYFlip.getToggleState());
    zyPlane.setYFlip (tbYFlip.getToggleState());
    zyPlane.setZFlip (tbZFlip.getToggleState());

    addAndMakeVisible (slXReference);
    slXReference.setJustificationType (juce::Justification::centred);
    slXReference.setEditable (true);
    slXReferenceAttachment.reset (new LabelAttachment (valueTreeState, "xReference", slXReference));

    addAndMakeVisible (slYReference);
    slYReference.setJustificationType (juce::Justification::centred);
    slYReference.setEditable (true);
    slYReferenceAttachment.reset (new LabelAttachment (valueTreeState, "yReference", slYReference));

    addAndMakeVisible (slZReference);
    slZReference.setJustificationType (juce::Justification::centred);
    slZReference.setEditable (true);
    slZReferenceAttachment.reset (new LabelAttachment (valueTreeState, "zReference", slZReference));

    addAndMakeVisible (lbXReference);
    lbXReference.setText ("x Ref");

    addAndMakeVisible (lbYReference);
    lbYReference.setText ("y Ref");

    addAndMakeVisible (lbZReference);
    lbZReference.setText ("z Ref");

    // ============== END: CARTESIAN COORDINATES ============

    // ============== BEGIN: RANGE SETTINGS ============

    addAndMakeVisible (slRadiusRange);
    slRadiusRange.setJustificationType (juce::Justification::centred);
    slRadiusRange.setEditable (true);
    slRadiusRangeAttachment.reset (
        new LabelAttachment (valueTreeState, "radiusRange", slRadiusRange));

    addAndMakeVisible (lbRadiusRange);
    lbRadiusRange.setText ("Radius");

    addAndMakeVisible (slXRange);
    slXRange.setJustificationType (juce::Justification::centred);
    slXRange.setEditable (true);
    slXRangeAttachment.reset (new LabelAttachment (valueTreeState, "xRange", slXRange));

    addAndMakeVisible (lbXRange);
    lbXRange.setText ("+/- X");

    addAndMakeVisible (slYRange);
    slYRange.setJustificationType (juce::Justification::centred);
    slYRange.setEditable (true);
    slYRangeAttachment.reset (new LabelAttachment (valueTreeState, "yRange", slYRange));

    addAndMakeVisible (lbYRange);
    lbYRange.setText ("+/- Y");

    addAndMakeVisible (slZRange);
    slZRange.setJustificationType (juce::Justification::centred);
    slZRange.setEditable (true);
    slZRangeAttachment.reset (new LabelAttachment (valueTreeState, "zRange", slZRange));

    addAndMakeVisible (lbZRange);
    lbZRange.setText ("+/- Z");

    // ============== END: RANGE SETTINGS ============

    // start timer after everything is set up properly
    startTimer (20);
}

CoordinateConverterAudioProcessorEditor::~CoordinateConverterAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void CoordinateConverterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void CoordinateConverterAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 17;
    const int rotSliderWidth = 40;

    const int sphericalWidth = 240;

    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);

    // =========== END: header and footer =================

    // ===== RANGE SETTINGS
    auto settingsArea = area.removeFromBottom (60);
    auto rangeArea = settingsArea.removeFromLeft (sphericalWidth);
    gcRange.setBounds (rangeArea);
    rangeArea.removeFromTop (25);

    {
        auto sliderRow = rangeArea.removeFromTop (18);

        slRadiusRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (20);

        slXRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (5);
        slYRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (5);
        slZRange.setBounds (sliderRow.removeFromLeft (50));

        sliderRow = rangeArea.removeFromTop (18);

        lbRadiusRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (20);

        lbXRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (5);
        lbYRange.setBounds (sliderRow.removeFromLeft (50));
        sliderRow.removeFromLeft (5);
        lbZRange.setBounds (sliderRow.removeFromLeft (50));
    }

    settingsArea.removeFromLeft (20);

    auto referenceArea = settingsArea;
    {
        gcReference.setBounds (referenceArea);
        referenceArea.removeFromTop (25);

        auto sliderRow = referenceArea.removeFromTop (18);

        slXReference.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (5);
        slYReference.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (5);
        slZReference.setBounds (sliderRow.removeFromLeft (45));

        sliderRow = referenceArea.removeFromTop (12);

        lbXReference.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (5);
        lbYReference.setBounds (sliderRow.removeFromLeft (45));
        sliderRow.removeFromLeft (5);
        lbZReference.setBounds (sliderRow.removeFromLeft (45));
    }

    area.removeFromBottom (10);

    // ===== SPHERICAL

    auto sphericalArea = area.removeFromLeft (sphericalWidth);
    gcSpherical.setBounds (sphericalArea);
    sphericalArea.removeFromTop (25);

    auto sliderArea =
        sphericalArea.removeFromBottom (rotSliderHeight + 12 + 5 + 20); // slider + spacing + label
    const int w = sphericalArea.getWidth();
    const int leftMargin = 0.5f * (w - 3 * rotSliderWidth - 2 * rotSliderSpacing);
    auto sliderRow = sliderArea.removeFromTop (rotSliderHeight);
    sliderRow.removeFromLeft (leftMargin);
    slAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slRadius.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderArea.removeFromBottom (5);

    sliderRow = sliderArea.removeFromTop (12);
    sliderRow.removeFromLeft (leftMargin);
    lbAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing - 5);
    lbElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 10));
    sliderRow.removeFromLeft (rotSliderSpacing - 5);
    lbRadius.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderRow = sliderArea.removeFromTop (20);
    sliderRow.removeFromLeft (leftMargin);
    tbAzimuthFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbElevationFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    tbRadiusFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sphere.setBounds (sphericalArea);

    area.removeFromLeft (20);

    // ==== CARTESIAN

    auto cartesianArea = area;
    gcCartesian.setBounds (cartesianArea);
    cartesianArea.removeFromTop (25);

    {
        auto sliderArea = cartesianArea.removeFromBottom (rotSliderHeight + 12 + 5
                                                          + 20); // slider + spacing + label
        const int w = cartesianArea.getWidth();
        const int leftMargin = 0.5f * (w - 3 * rotSliderWidth - 2 * rotSliderSpacing);

        sliderArea.removeFromLeft (leftMargin);

        auto sliderRow = sliderArea.removeFromTop (rotSliderHeight);
        slXPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing - 2);
        slYPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing - 2);
        slZPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = sliderArea.removeFromTop (12);
        lbXPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing - 2);
        lbYPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing - 2);
        lbZPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = sliderArea.removeFromTop (20);
        sliderRow.removeFromLeft (leftMargin);
        tbXFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        tbYFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        tbZFlip.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        const int sphereHeight = sphere.getHeight();
        const int planeHeight = (sphereHeight - 11) / 2;

        auto planeArea = cartesianArea.removeFromLeft (planeHeight);
        xyPlane.setBounds (planeArea.removeFromTop (planeHeight));
        planeArea.removeFromTop (6); // 5 left
        zyPlane.setBounds (planeArea.removeFromTop (planeHeight));
    }
}

void CoordinateConverterAudioProcessorEditor::timerCallback()
{
    if (processor.repaintPositionPlanes.get())
    {
        processor.repaintPositionPlanes = false;
        xyPlane.repaint();
        zyPlane.repaint();
    }

    if (processor.repaintSphere.get())
    {
        processor.repaintSphere = false;
        sphere.repaint();
    }
}

void CoordinateConverterAudioProcessorEditor::buttonStateChanged (juce::Button* button)
{
    if (button == &tbXFlip)
    {
        xyPlane.setXFlip (tbXFlip.getToggleState());
    }
    else if (button == &tbYFlip)
    {
        xyPlane.setYFlip (tbYFlip.getToggleState());
        zyPlane.setYFlip (tbYFlip.getToggleState());
    }
    else if (button == &tbZFlip)
    {
        zyPlane.setZFlip (tbZFlip.getToggleState());
    }
}
