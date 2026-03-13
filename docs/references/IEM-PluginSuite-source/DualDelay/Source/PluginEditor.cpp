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
DualDelayAudioProcessorEditor::DualDelayAudioProcessorEditor (
    DualDelayAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface())
{
    setLookAndFeel (&globalLaF);

    addAndMakeVisible (&title);
    title.setTitle (juce::String ("Dual"), juce::String ("Delay"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);

    cbNormalizationAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "useSN3D",
                                *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachement.reset (
        new ComboBoxAttachment (valueTreeState,
                                "orderSetting",
                                *title.getInputWidgetPtr()->getOrderCbPointer()));

    addAndMakeVisible (&SlDryGain);
    SlDryGainAttachment.reset (new SliderAttachment (valueTreeState, "dryGain", SlDryGain));
    SlDryGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlDryGain.setTextValueSuffix (" dB");
    SlDryGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlDryGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    // =========================== LEFT SIDE ==============================================================

    addAndMakeVisible (&SlLeftRot);
    SlLeftRotAttachment.reset (new SliderAttachment (valueTreeState, "rotationL", SlLeftRot));
    SlLeftRot.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftRot.setReverse (true);
    SlLeftRot.setTextValueSuffix (" deg");
    SlLeftRot.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftRot.setRotaryParameters (juce::MathConstants<float>::pi,
                                   3 * juce::MathConstants<float>::pi,
                                   false);
    SlLeftRot.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&SlLeftDelay);
    SlLeftDelayAttachment.reset (new SliderAttachment (valueTreeState, "delayTimeL", SlLeftDelay));
    SlLeftDelay.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftDelay.setTextValueSuffix (" ms");
    SlLeftDelay.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftDelay.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&SlLeftLfoRate);
    SlLeftLfoRateAttachment.reset (
        new SliderAttachment (valueTreeState, "lfoRateL", SlLeftLfoRate));
    SlLeftLfoRate.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftLfoRate.setTextValueSuffix (" Hz");
    SlLeftLfoRate.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftLfoRate.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (&SlLeftLfoDepth);
    SlLeftLfoDepthAttachment.reset (
        new SliderAttachment (valueTreeState, "lfoDepthL", SlLeftLfoDepth));
    SlLeftLfoDepth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftLfoDepth.setTextValueSuffix (" ms");
    SlLeftLfoDepth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftLfoDepth.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (&dblSlLeftFilter);
    dblSlLeftFilterHpAttachment.reset (
        new SliderAttachment (valueTreeState,
                              "HPcutOffL",
                              *dblSlLeftFilter.getLeftSliderAddress()));
    dblSlLeftFilterLpAttachment.reset (
        new SliderAttachment (valueTreeState,
                              "LPcutOffL",
                              *dblSlLeftFilter.getRightSliderAddress()));
    dblSlLeftFilter.setRangeAndPosition (valueTreeState.getParameterRange ("HPcutOffL"),
                                         valueTreeState.getParameterRange ("LPcutOffL"));
    dblSlLeftFilter.setColour (globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&SlLeftFb);
    SlLeftFbAttachment.reset (new SliderAttachment (valueTreeState, "feedbackL", SlLeftFb));
    SlLeftFb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftFb.setTextValueSuffix (" dB");
    SlLeftFb.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftFb.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);

    addAndMakeVisible (&SlLeftCrossFb);
    SlLeftCrossFbAttachment.reset (
        new SliderAttachment (valueTreeState, "xfeedbackL", SlLeftCrossFb));
    SlLeftCrossFb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftCrossFb.setTextValueSuffix (" dB");
    SlLeftCrossFb.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftCrossFb.setColour (juce::Slider::rotarySliderOutlineColourId,
                             globalLaF.ClWidgetColours[3]);

    addAndMakeVisible (&SlLeftGain);
    SlLeftGainAttachment.reset (new SliderAttachment (valueTreeState, "wetGainL", SlLeftGain));
    SlLeftGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlLeftGain.setTextValueSuffix (" dB");
    SlLeftGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlLeftGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    // =========================== RIGHT SIDE ================================================================

    addAndMakeVisible (&SlRightRot);
    SlRightRotAttachment.reset (new SliderAttachment (valueTreeState, "rotationR", SlRightRot));
    SlRightRot.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightRot.setReverse (true);
    SlRightRot.setTextValueSuffix (" deg");
    SlRightRot.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightRot.setRotaryParameters (juce::MathConstants<float>::pi,
                                    3 * juce::MathConstants<float>::pi,
                                    false);
    SlRightRot.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    addAndMakeVisible (&SlRightDelay);
    SlRightDelayAttachment.reset (
        new SliderAttachment (valueTreeState, "delayTimeR", SlRightDelay));
    SlRightDelay.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightDelay.setTextValueSuffix (" ms");
    SlRightDelay.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightDelay.setColour (juce::Slider::rotarySliderOutlineColourId,
                            globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&SlRightLfoRate);
    SlRightLfoRateAttachment.reset (
        new SliderAttachment (valueTreeState, "lfoRateR", SlRightLfoRate));
    SlRightLfoRate.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightLfoRate.setTextValueSuffix (" Hz");
    SlRightLfoRate.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightLfoRate.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (&SlRightLfoDepth);
    SlRightLfoDepthAttachment.reset (
        new SliderAttachment (valueTreeState, "lfoDepthR", SlRightLfoDepth));
    SlRightLfoDepth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightLfoDepth.setTextValueSuffix (" ms");
    SlRightLfoDepth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightLfoDepth.setColour (juce::Slider::rotarySliderOutlineColourId,
                               globalLaF.ClWidgetColours[2]);

    addAndMakeVisible (&dblSlRightFilter);
    dblSlRightFilterHpAttachment.reset (
        new SliderAttachment (valueTreeState,
                              "HPcutOffR",
                              *dblSlRightFilter.getLeftSliderAddress()));
    dblSlRightFilterLpAttachment.reset (
        new SliderAttachment (valueTreeState,
                              "LPcutOffR",
                              *dblSlRightFilter.getRightSliderAddress()));

    dblSlRightFilter.setRangeAndPosition (valueTreeState.getParameterRange ("HPcutOffR"),
                                          valueTreeState.getParameterRange ("LPcutOffR"));
    dblSlRightFilter.getLeftSliderAddress()->setTextValueSuffix (" Hz");
    dblSlRightFilter.getRightSliderAddress()->setTextValueSuffix (" Hz");
    dblSlRightFilter.setColour (globalLaF.ClWidgetColours[1]);

    addAndMakeVisible (&SlRightFb);
    SlRightFbAttachment.reset (new SliderAttachment (valueTreeState, "feedbackR", SlRightFb));
    SlRightFb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightFb.setTextValueSuffix (" dB");
    SlRightFb.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightFb.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[3]);

    addAndMakeVisible (&SlRightCrossFb);
    SlRightCrossFbAttachment.reset (
        new SliderAttachment (valueTreeState, "xfeedbackR", SlRightCrossFb));
    SlRightCrossFb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightCrossFb.setTextValueSuffix (" dB");
    SlRightCrossFb.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightCrossFb.setColour (juce::Slider::rotarySliderOutlineColourId,
                              globalLaF.ClWidgetColours[3]);

    addAndMakeVisible (&SlRightGain);
    SlRightGainAttachment.reset (new SliderAttachment (valueTreeState, "wetGainR", SlRightGain));
    SlRightGain.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    SlRightGain.setTextValueSuffix (" dB");
    SlRightGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    SlRightGain.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);

    // ============ GROUPCOMPONENTS =========
    addAndMakeVisible (&gcRotDelL);
    gcRotDelL.setText ("Rotation & Delay");
    gcRotDelL.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcRotDelR);
    gcRotDelR.setText ("Rotation & Delay");
    gcRotDelR.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcFiltL);
    gcFiltL.setText ("Spectral Filter");
    gcFiltL.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcFiltR);
    gcFiltR.setText ("Spectral Filter");
    gcFiltR.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcFbL);
    gcFbL.setText ("Feedback");
    gcFbL.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcFbR);
    gcFbR.setText ("Feedback");
    gcFbR.setTextLabelPosition (juce::Justification::centred);

    addAndMakeVisible (&gcOutput);
    gcOutput.setText ("Output Mix");
    gcOutput.setTextLabelPosition (juce::Justification::centred);

    // ============ LABELS =========
    addAndMakeVisible (&lbRotL);
    lbRotL.setText ("Yaw");

    addAndMakeVisible (&lbDelL);
    lbDelL.setText ("Delay");

    addAndMakeVisible (&lbFbL);
    lbFbL.setText ("Self");

    addAndMakeVisible (&lbXFbL);
    lbXFbL.setText ("Cross");

    addAndMakeVisible (&lbRotR);
    lbRotR.setText ("Yaw");

    addAndMakeVisible (&lbDelR);
    lbDelR.setText ("Delay");

    addAndMakeVisible (&lbFbR);
    lbFbR.setText ("Self");

    addAndMakeVisible (&lbXFbR);
    lbXFbR.setText ("Cross");

    addAndMakeVisible (&lbGainL);
    lbGainL.setText ("Delay I");

    addAndMakeVisible (&lbGainR);
    lbGainR.setText ("Delay II");

    addAndMakeVisible (&lbGainDry);
    lbGainDry.setText ("Dry");

    addAndMakeVisible (&lbLfoL);
    lbLfoL.setText ("Rate", "LFO", "Depth", false, true, false);

    addAndMakeVisible (&lbLfoR);
    lbLfoR.setText ("Rate", "LFO", "Depth", false, true, false);

    addAndMakeVisible (&lbFilterL);
    lbFilterL.setText ("HighPass", "Cutoff Frequency", "LowPass", false, true, false);

    addAndMakeVisible (&lbFilterR);
    lbFilterR.setText ("HighPass", "Cutoff Frequency", "LowPass", false, true, false);

    setSize (600, 500);

    startTimer (20);
}

DualDelayAudioProcessorEditor::~DualDelayAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void DualDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void DualDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int textHeight = 14;
    const int sliderHeight = 70;

    juce::Rectangle<int> area (getLocalBounds());
    juce::Rectangle<int> groupArea;
    juce::Rectangle<int> sliderRow;

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);

    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);

    juce::Rectangle<int> tempArea;

    // ======== BEGIN: Rotations and Delays =========
    tempArea = area.removeFromTop (30 + sliderHeight + textHeight);

    // ------ left side ---------
    groupArea = tempArea.removeFromLeft (250);
    gcRotDelL.setBounds (groupArea);
    groupArea.removeFromTop (30);

    sliderRow = groupArea.removeFromTop (sliderHeight);
    SlLeftRot.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlLeftDelay.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlLeftLfoRate.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlLeftLfoDepth.setBounds (sliderRow.removeFromLeft (55));

    lbRotL.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbDelL.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbLfoL.setBounds (groupArea.reduced (9, 0));

    // ------ right side --------
    groupArea = tempArea.removeFromRight (250);
    gcRotDelR.setBounds (groupArea);
    groupArea.removeFromTop (30);

    sliderRow = groupArea.removeFromTop (sliderHeight);
    SlRightRot.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlRightDelay.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlRightLfoRate.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlRightLfoDepth.setBounds (sliderRow.removeFromLeft (55));

    lbRotR.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbDelR.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbLfoR.setBounds (groupArea.reduced (9, 0));

    // ======== END: Rotations and Delays =================

    area.removeFromTop (40); // spacing

    // ======== BEGIN: Filters =============
    tempArea = area.removeFromTop (45 + textHeight);

    // ----- left side ------
    groupArea = tempArea.removeFromLeft (250);
    gcFiltL.setBounds (groupArea);
    groupArea.removeFromTop (30);

    dblSlLeftFilter.setBounds (groupArea.removeFromTop (15));
    lbFilterL.setBounds (groupArea.reduced (15, 0));

    // ----- right side ------
    groupArea = tempArea.removeFromRight (250);
    gcFiltR.setBounds (groupArea);
    groupArea.removeFromTop (30);

    dblSlRightFilter.setBounds (groupArea.removeFromTop (15));
    lbFilterR.setBounds (groupArea.reduced (15, 0));
    // ======== END: Filters ===============

    area.removeFromTop (40); // spacing

    // ======== BEGIN: Feedback =============

    tempArea = area.removeFromTop (30 + sliderHeight + textHeight);

    // ------ left side -------------
    groupArea = tempArea.removeFromLeft (120);
    gcFbL.setBounds (groupArea);
    groupArea.removeFromTop (30);

    sliderRow = groupArea.removeFromTop (sliderHeight);

    SlLeftFb.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10);
    SlLeftCrossFb.setBounds (sliderRow.removeFromLeft (55));

    lbFbL.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbXFbL.setBounds (groupArea.removeFromLeft (55));

    // ------ right side -------------
    groupArea = tempArea.removeFromRight (120);
    gcFbR.setBounds (groupArea);
    groupArea.removeFromTop (30);

    sliderRow = groupArea.removeFromTop (sliderHeight);

    SlRightFb.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10);
    SlRightCrossFb.setBounds (sliderRow.removeFromLeft (55));

    lbFbR.setBounds (groupArea.removeFromLeft (55));
    groupArea.removeFromLeft (10);
    lbXFbR.setBounds (groupArea.removeFromLeft (55));

    // ======== END: Feedback ===============

    // ======== BEGIN: Output Mix ===========
    int actualWidth = tempArea.getWidth();
    int wantedWidth = 186;
    tempArea.removeFromLeft (juce::roundToInt ((actualWidth - wantedWidth) / 2));
    tempArea.setWidth (wantedWidth);

    gcOutput.setBounds (tempArea);
    tempArea.removeFromTop (30);

    sliderRow = tempArea.removeFromTop (sliderHeight);

    SlLeftGain.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlDryGain.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders
    SlRightGain.setBounds (sliderRow.removeFromLeft (55));
    sliderRow.removeFromLeft (10); // spacing between rotary sliders

    lbGainL.setBounds (tempArea.removeFromLeft (55));
    tempArea.removeFromLeft (10); // spacing between rotary sliders
    lbGainDry.setBounds (tempArea.removeFromLeft (55));
    tempArea.removeFromLeft (10); // spacing between rotary sliders
    lbGainR.setBounds (tempArea.removeFromLeft (55));

    // ======== END: Output Mix =============
}
void DualDelayAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    auto sizes = processor.getMaxSize();
    sizes.first = juce::jmin (sizes.first, sizes.second);
    sizes.second = sizes.first;
    title.setMaxSize (sizes);
    // ==========================================
}
