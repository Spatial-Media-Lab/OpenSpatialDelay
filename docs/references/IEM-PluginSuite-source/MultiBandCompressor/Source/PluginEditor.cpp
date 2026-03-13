/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Markus Huber
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

//==============================================================================
MultiBandCompressorAudioProcessorEditor::MultiBandCompressorAudioProcessorEditor (
    MultiBandCompressorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p),
    processor (p),
    valueTreeState (vts),
    footer (p.getOSCParameterInterface()),
    filterBankVisualizer (20.0f, 20000.0f, -15.0f, 20.0f, 5.0f, p.getSampleRate(), numFilterBands)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits (980, 980 * 0.6, 1600, 1600 * 0.6); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (juce::String ("MultiBand"), juce::String ("Compressor"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================

    cbNormalizationAtachement =
        std::make_unique<ComboBoxAttachment> (valueTreeState,
                                              "useSN3D",
                                              *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement =
        std::make_unique<ComboBoxAttachment> (valueTreeState,
                                              "orderSetting",
                                              *title.getInputWidgetPtr()->getOrderCbPointer());

    tooltips.setMillisecondsBeforeTipAppears (800);
    tooltips.setOpaque (false);

    const juce::Colour colours[numFilterBands] = { juce::Colours::cornflowerblue,
                                                   juce::Colours::greenyellow,
                                                   juce::Colours::yellow,
                                                   juce::Colours::orangered };

    for (int i = 0; i < numFilterBands; ++i)
    {
        // ==== COMPRESSOR VISUALIZATION ====
        compressorVisualizers.add (new CompressorVisualizer (p.getCompressor (i)));
        addAndMakeVisible (compressorVisualizers[i]);

        // ===== GR METERS =====
        addAndMakeVisible (&GRmeter[i]);
        GRmeter[i].setMinLevel (-25.0f);
        GRmeter[i].setColour (juce::Colours::red.withMultipliedAlpha (0.8f));
        GRmeter[i].setGainReductionMeter (true);

        // ==== SLIDERS ====
        addAndMakeVisible (&slThreshold[i]);
        slThresholdAttachment[i] =
            std::make_unique<SliderAttachment> (valueTreeState,
                                                "threshold" + juce::String (i),
                                                slThreshold[i]);
        slThreshold[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slThreshold[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slThreshold[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slThreshold[i].setTextValueSuffix (" dB");

        addAndMakeVisible (&slKnee[i]);
        slKneeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState,
                                                                  "knee" + juce::String (i),
                                                                  slKnee[i]);
        slKnee[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slKnee[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slKnee[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slKnee[i].setTextValueSuffix (" dB");

        addAndMakeVisible (&slRatio[i]);
        slRatioAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState,
                                                                   "ratio" + juce::String (i),
                                                                   slRatio[i]);
        slRatio[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slRatio[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slRatio[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);

        addAndMakeVisible (&slAttackTime[i]);
        slAttackTimeAttachment[i] = std::make_unique<SliderAttachment> (valueTreeState,
                                                                        "attack" + juce::String (i),
                                                                        slAttackTime[i]);
        slAttackTime[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slAttackTime[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slAttackTime[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slRatio[i].setTextValueSuffix (" ms");

        addAndMakeVisible (&slReleaseTime[i]);
        slReleaseTimeAttachment[i] =
            std::make_unique<SliderAttachment> (valueTreeState,
                                                "release" + juce::String (i),
                                                slReleaseTime[i]);
        slReleaseTime[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slReleaseTime[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slReleaseTime[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slReleaseTime[i].setTextValueSuffix (" ms");

        addAndMakeVisible (&slMakeUpGain[i]);
        slMakeUpGainAttachment[i] =
            std::make_unique<SliderAttachment> (valueTreeState,
                                                "makeUpGain" + juce::String (i),
                                                slMakeUpGain[i]);
        slMakeUpGain[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slMakeUpGain[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slMakeUpGain[i].setColour (juce::Slider::rotarySliderOutlineColourId, colours[i]);
        slMakeUpGain[i].setTextValueSuffix (" dB");
        slMakeUpGain[i].setName (juce::String ("MakeUpGain" + juce::String (i)));
        slMakeUpGain[i].addListener (this);

        // ===== LABELS =====
        addAndMakeVisible (&lbKnee[i]);
        lbKnee[i].setText ("Knee");
        lbKnee[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible (&lbThreshold[i]);
        lbThreshold[i].setText ("Threshold");
        lbThreshold[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible (&lbMakeUpGain[i]);
        lbMakeUpGain[i].setText ("Makeup");
        lbMakeUpGain[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible (&lbRatio[i]);
        lbRatio[i].setText ("Ratio");
        lbRatio[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible (&lbAttack[i]);
        lbAttack[i].setText ("Attack");
        lbAttack[i].setTextColour (globalLaF.ClFace);

        addAndMakeVisible (&lbRelease[i]);
        lbRelease[i].setText ("Release");
        lbRelease[i].setTextColour (globalLaF.ClFace);

        // ==== BUTTONS ====
        tbSolo[i].setColour (juce::ToggleButton::tickColourId,
                             juce::Colour (0xFFFFFF66).withMultipliedAlpha (0.85f));
        tbSolo[i].setScaleFontSize (0.75f);
        tbSolo[i].setButtonText ("S");
        tbSolo[i].setName ("solo" + juce::String (i));
        tbSolo[i].setTooltip ("Solo band #" + juce::String (i));
        soloAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            valueTreeState,
            "solo" + juce::String (i),
            tbSolo[i]);
        tbSolo[i].setClickingTogglesState (true);
        tbSolo[i].addListener (this);
        addAndMakeVisible (&tbSolo[i]);

        tbBypass[i].setColour (juce::ToggleButton::tickColourId,
                               juce::Colour (0xFF5bAE87).withMultipliedAlpha (0.85f));
        tbBypass[i].setScaleFontSize (0.75f);
        tbBypass[i].setButtonText ("B");
        tbBypass[i].setName ("bypass" + juce::String (i));
        tbBypass[i].setTooltip ("Bypass band #" + juce::String (i));
        bypassAttachment[i] =
            std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
                valueTreeState,
                "bypass" + juce::String (i),
                tbBypass[i]);
        tbBypass[i].setClickingTogglesState (true);
        tbBypass[i].addListener (this);
        addAndMakeVisible (&tbBypass[i]);
    }

    // ==== FILTER VISUALIZATION ====
    juce::dsp::IIR::Coefficients<double>::Ptr coeffs1;
    juce::dsp::IIR::Coefficients<double>::Ptr coeffs2;
    for (int i = 0; i < numFilterBands; ++i)
    {
        switch (i)
        {
            case (int) MultiBandCompressorAudioProcessor::FrequencyBands::Low:
                coeffs1 = processor.lowPassLRCoeffs[1];
                coeffs2 = processor.lowPassLRCoeffs[0];
                break;
            case (int) MultiBandCompressorAudioProcessor::FrequencyBands::MidLow:
                coeffs1 = processor.lowPassLRCoeffs[1];
                coeffs2 = processor.highPassLRCoeffs[0];
                break;
            case (int) MultiBandCompressorAudioProcessor::FrequencyBands::MidHigh:
                coeffs1 = processor.highPassLRCoeffs[1];
                coeffs2 = processor.lowPassLRCoeffs[2];
                break;
            case (int) MultiBandCompressorAudioProcessor::FrequencyBands::High:
                coeffs1 = processor.highPassLRCoeffs[1];
                coeffs2 = processor.highPassLRCoeffs[2];
                break;
        }

        filterBankVisualizer.setFrequencyBand (i, coeffs1, coeffs2, colours[i]);
        filterBankVisualizer.setBypassed (i, tbBypass[i].getToggleState());
        filterBankVisualizer.setSolo (i, tbSolo[i].getToggleState());
        filterBankVisualizer.updateMakeUpGain (i, slMakeUpGain[i].getValue());
    }

    addAndMakeVisible (&filterBankVisualizer);

    // SHOW OVERALL MAGNITUDE BUTTON
    tbOverallMagnitude.setColour (juce::ToggleButton::tickColourId, juce::Colours::white);
    tbOverallMagnitude.setButtonText ("show total magnitude");
    tbOverallMagnitude.setName ("overallMagnitude");
    tbOverallMagnitude.setClickingTogglesState (true);
    tbOverallMagnitude.addListener (this);
    addAndMakeVisible (&tbOverallMagnitude);

    // ==== CROSSOVER SLIDERS ====
    for (int i = 0; i < numFilterBands - 1; ++i)
    {
        slCrossoverAttachment[i] =
            std::make_unique<SliderAttachment> (valueTreeState,
                                                "crossover" + juce::String (i),
                                                slCrossover[i]);
        addAndMakeVisible (&slCrossover[i]);
        slCrossover[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slCrossover[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 12);
        slCrossover[i].setColour (juce::Slider::rotarySliderOutlineColourId,
                                  globalLaF.ClRotSliderArrow);
        slCrossover[i].setTooltip ("Crossover Frequency " + juce::String (i + 1));
        slCrossover[i].setName ("Crossover" + juce::String (i));
        slCrossover[i].addListener (this);

        // add crossover to visualizer
        filterBankVisualizer.addCrossover (&slCrossover[i]);
    }

    // ==== METERS - INPUT/OUTPUT ====
    addAndMakeVisible (&omniInputMeter);
    omniInputMeter.setMinLevel (-60.0f);
    omniInputMeter.setColour (juce::Colours::green.withMultipliedAlpha (0.8f));
    omniInputMeter.setGainReductionMeter (false);
    addAndMakeVisible (&lbInput);
    lbInput.setText ("Input");
    lbInput.setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&omniOutputMeter);
    omniOutputMeter.setMinLevel (-60.0f);
    omniOutputMeter.setColour (juce::Colours::green.withMultipliedAlpha (0.8f));
    omniOutputMeter.setGainReductionMeter (false);
    addAndMakeVisible (&lbOutput);
    lbOutput.setText ("Output");
    lbOutput.setTextColour (globalLaF.ClFace);

    // ==== MASTER CONTROLS ====
    addAndMakeVisible (&slMasterThreshold);
    slMasterThreshold.setName ("MasterThreshold");
    addAndMakeVisible (&lbThreshold[numFilterBands]);
    lbThreshold[numFilterBands].setText ("Thresh.");
    lbThreshold[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterKnee);
    slMasterKnee.setName ("MasterKnee");
    addAndMakeVisible (&lbKnee[numFilterBands]);
    lbKnee[numFilterBands].setText ("Knee");
    lbKnee[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterMakeUpGain);
    slMasterMakeUpGain.setName ("MasterMakeUpGain");
    addAndMakeVisible (&lbMakeUpGain[numFilterBands]);
    lbMakeUpGain[numFilterBands].setText ("Gain");
    lbMakeUpGain[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterRatio);
    slMasterRatio.setName ("MasterMakeUpGain");
    addAndMakeVisible (&lbRatio[numFilterBands]);
    lbRatio[numFilterBands].setText ("Ratio");
    lbRatio[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterAttackTime);
    slMasterAttackTime.setName ("MasterAttackTime");
    addAndMakeVisible (&lbAttack[numFilterBands]);
    lbAttack[numFilterBands].setText ("Attack");
    lbAttack[numFilterBands].setTextColour (globalLaF.ClFace);

    addAndMakeVisible (&slMasterReleaseTime);
    slMasterReleaseTime.setName ("MasterReleaseTime");
    addAndMakeVisible (&lbRelease[numFilterBands]);
    lbRelease[numFilterBands].setText ("Rel.");
    lbRelease[numFilterBands].setTextColour (globalLaF.ClFace);

    gcMasterControls.setText ("Master controls");
    addAndMakeVisible (&gcMasterControls);

    // add sliders to master controls
    for (int i = 0; i < numFilterBands; ++i)
    {
        slMasterThreshold.addSlave (slThreshold[i]);
        slMasterKnee.addSlave (slKnee[i]);
        slMasterMakeUpGain.addSlave (slMakeUpGain[i]);
        slMasterRatio.addSlave (slRatio[i]);
        slMasterAttackTime.addSlave (slAttackTime[i]);
        slMasterReleaseTime.addSlave (slReleaseTime[i]);
    }

    /* resized () is called here, because otherwise the compressorVisualizers won't be drawn to the GUI until one manually resizes the window.
    It seems resized() somehow gets called *before* the constructor and therefore juce::OwnedArray<CompressorVisualizers> is still empty on the first resized call... */
    resized();

    // start timer after everything is set up properly
    startTimer (50);
}

MultiBandCompressorAudioProcessorEditor::~MultiBandCompressorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MultiBandCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiBandCompressorAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
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
    area.removeFromBottom (5);
    // =========== END: header and footer =================

    // ==== SPLIT INTO 4 BASIC SECTIONS ====
    //    const float leftToRightRatio = 0.87;
    const int leftToRightGap = 6;
    const float filterBankToLowerRatio = 0.34f;
    const float crossoverAndButtonToCompressorsRatio = 0.1645f;
    const int filterToCrossoverAndButtonGap = 2;
    const int compressorToCrossoverAndButtonGap = 2;

    // split
    //    juce::Rectangle<int> leftArea = area.removeFromLeft (area.proportionOfWidth (leftToRightRatio));
    //    juce::Rectangle<int> rightArea (area);

    juce::Rectangle<int> rightArea = area.removeFromRight (130);
    area.removeFromRight (leftToRightGap);
    juce::Rectangle<int> leftArea (area);

    //    leftArea.removeFromRight (leftToRightGap / 2);
    //    rightArea.removeFromLeft (leftToRightGap / 2);
    juce::Rectangle<int> filterBankArea =
        leftArea.removeFromTop (leftArea.proportionOfHeight (filterBankToLowerRatio));
    juce::Rectangle<int> compressorArea = leftArea;
    juce::Rectangle<int> crossoverAndButtonArea = compressorArea.removeFromTop (
        compressorArea.proportionOfHeight (crossoverAndButtonToCompressorsRatio));

    // safeguard against haphephobia
    filterBankArea.removeFromBottom (filterToCrossoverAndButtonGap / 2);
    crossoverAndButtonArea.removeFromTop (filterToCrossoverAndButtonGap / 2);
    crossoverAndButtonArea.removeFromBottom (compressorToCrossoverAndButtonGap / 2);
    compressorArea.removeFromTop (compressorToCrossoverAndButtonGap / 2);

    // ==== FILTER VISUALIZATION ====
    filterBankVisualizer.setBounds (filterBankArea);

    // ==== BUTTONS AND CROSSOVER SLIDERS ====
    const int crossoverToButtonGap = 32;
    const int buttonToButtonGap = 0;
    const float crossoverToButtonsRatio = 0.85f;
    const float trimButtonsHeight = 0.17f;
    const float trimButtonsWidth = 0.17f;
    juce::Rectangle<int> soloButtonArea;
    juce::Rectangle<int> bypassButtonArea;
    juce::Rectangle<int> crossoverArea;

    const int buttonsWidth = crossoverAndButtonArea.getWidth()
                             / (numFilterBands + (numFilterBands - 1) * crossoverToButtonsRatio);
    const int crossoverSliderWidth = buttonsWidth * crossoverToButtonsRatio;

    for (int i = 0; i < numFilterBands; ++i)
    {
        // juce::Buttons
        bypassButtonArea = crossoverAndButtonArea.removeFromLeft (buttonsWidth);
        bypassButtonArea.reduce (crossoverToButtonGap / 2, 0);
        soloButtonArea = bypassButtonArea.removeFromLeft (bypassButtonArea.proportionOfWidth (0.5));
        soloButtonArea.removeFromRight (buttonToButtonGap / 2);
        bypassButtonArea.removeFromLeft (buttonToButtonGap / 2);
        tbSolo[i].setBounds (
            soloButtonArea.reduced (soloButtonArea.proportionOfWidth (trimButtonsWidth),
                                    soloButtonArea.proportionOfHeight (trimButtonsHeight)));
        tbBypass[i].setBounds (
            bypassButtonArea.reduced (bypassButtonArea.proportionOfWidth (trimButtonsWidth),
                                      bypassButtonArea.proportionOfHeight (trimButtonsHeight)));

        // juce::Sliders
        if (i < numFilterBands - 1)
        {
            crossoverArea = crossoverAndButtonArea.removeFromLeft (crossoverSliderWidth);
            slCrossover[i].setBounds (crossoverArea.reduced (crossoverToButtonGap / 2, 0));
        }
    }

    // ==== COMPRESSOR VISUALIZATION ====
    const float paramToCharacteristiscRatio = 0.47f;
    const float meterToCharacteristicRatio = 0.175f;
    const float labelToParamRatio = 0.17f;
    const int paramRowToRowGap = 2;
    const int paramToCharacteristicGap = 2;
    const int bandToBandGap = 6;
    const int meterToCharacteristicGap = 6;
    const float trimMeterHeightRatio = 0.02f;

    compressorArea.reduce (
        ((compressorArea.getWidth() - (numFilterBands - 1) * bandToBandGap) % numFilterBands) / 2,
        0);
    const int widthPerBand =
        ((compressorArea.getWidth() - (numFilterBands - 1) * bandToBandGap) / numFilterBands);
    juce::Rectangle<int> characteristicArea, paramArea, paramRow1, paramRow2, labelRow1, labelRow2,
        grMeterArea;

    for (int i = 0; i < numFilterBands; ++i)
    {
        characteristicArea = compressorArea.removeFromLeft (widthPerBand);

        // Compressor parameters
        paramArea = characteristicArea.removeFromBottom (
            characteristicArea.proportionOfHeight (paramToCharacteristiscRatio));
        paramArea.removeFromTop (paramToCharacteristicGap / 2);
        characteristicArea.removeFromBottom (paramToCharacteristicGap / 2);

        paramArea.reduce ((paramArea.getWidth() % 3) / 2, 0);
        const int sliderWidth = paramArea.getWidth() / 3;

        paramRow1 = paramArea.removeFromTop (paramArea.proportionOfHeight (0.5f));
        paramRow2 = paramArea;
        paramRow1.removeFromBottom (paramRowToRowGap / 2);
        paramRow2.removeFromTop (paramRowToRowGap / 2);
        labelRow1 = paramRow1.removeFromBottom (paramRow1.proportionOfHeight (labelToParamRatio));
        labelRow2 = paramRow2.removeFromBottom (paramRow2.proportionOfHeight (labelToParamRatio));

        lbThreshold[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbKnee[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        lbMakeUpGain[i].setBounds (labelRow1.removeFromLeft (sliderWidth));
        slThreshold[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slKnee[i].setBounds (paramRow1.removeFromLeft (sliderWidth));
        slMakeUpGain[i].setBounds (paramRow1.removeFromLeft (sliderWidth));

        lbRatio[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbAttack[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        lbRelease[i].setBounds (labelRow2.removeFromLeft (sliderWidth));
        slRatio[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slAttackTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));
        slReleaseTime[i].setBounds (paramRow2.removeFromLeft (sliderWidth));

        // Gain-Reduction meter
        grMeterArea = characteristicArea.removeFromRight (
            characteristicArea.proportionOfWidth (meterToCharacteristicRatio));
        grMeterArea.removeFromLeft (meterToCharacteristicGap / 2);
        characteristicArea.removeFromRight (meterToCharacteristicGap / 2);
        GRmeter[i].setBounds (
            grMeterArea.reduced (0, grMeterArea.proportionOfHeight (trimMeterHeightRatio)));

        // Compressor characteristic
        if (! (compressorVisualizers.isEmpty()))
            compressorVisualizers[i]->setBounds (characteristicArea);

        if (i < numFilterBands - 1)
            compressorArea.removeFromLeft (bandToBandGap);
    }

    // ==== INPUT & OUTPUT METER ====
    const float labelToMeterRatio = 0.1f;
    const int meterToMeterGap = 10;

    juce::Rectangle<int> meterArea =
        rightArea.removeFromTop (rightArea.proportionOfHeight (filterBankToLowerRatio));
    meterArea.reduce (meterArea.proportionOfWidth (0.18f), 0);
    juce::Rectangle<int> inputMeterArea =
        meterArea.removeFromLeft (meterArea.proportionOfWidth (0.5f));
    juce::Rectangle<int> outputMeterArea = meterArea;
    inputMeterArea.removeFromRight (meterToMeterGap / 2);
    outputMeterArea.removeFromLeft (meterToMeterGap / 2);
    juce::Rectangle<int> inputMeterLabelArea =
        inputMeterArea.removeFromBottom (inputMeterArea.proportionOfHeight (labelToMeterRatio));
    juce::Rectangle<int> outputMeterLabelArea =
        outputMeterArea.removeFromBottom (outputMeterArea.proportionOfHeight (labelToMeterRatio));

    omniInputMeter.setBounds (inputMeterArea);
    omniOutputMeter.setBounds (outputMeterArea);
    lbInput.setBounds (inputMeterLabelArea);
    lbOutput.setBounds (outputMeterLabelArea);

    // ==== MASTER SLIDERS + LABELS ====
    const float masterToUpperArea = 0.5;
    const float labelToSliderRatio = 0.24f;
    const int trimFromGroupComponentHeader = 25;
    const float trimSliderHeight = 0.125f;
    const float trimSliderWidth = 0.00f;
    const int masterToCompressorSectionGap = 18;

    juce::Rectangle<int> masterArea =
        rightArea.removeFromBottom (rightArea.proportionOfHeight (masterToUpperArea));
    masterArea.removeFromLeft (masterToCompressorSectionGap);
    gcMasterControls.setBounds (masterArea);
    masterArea.removeFromTop (trimFromGroupComponentHeader);

    juce::Rectangle<int> sliderRow =
        masterArea.removeFromTop (masterArea.proportionOfHeight (0.5f));
    //    sliderRow.reduce (sliderRow.proportionOfWidth (trimSliderWidth), sliderRow.proportionOfHeight (trimSliderHeight));
    juce::Rectangle<int> labelRow =
        sliderRow.removeFromBottom (sliderRow.proportionOfHeight (labelToSliderRatio));

    const int masterSliderWidth = 35;
    DBG (sliderRow.getWidth());

    slMasterThreshold.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    slMasterKnee.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    slMasterMakeUpGain.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    lbThreshold[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));
    lbKnee[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));
    lbMakeUpGain[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));

    sliderRow = masterArea;
    sliderRow.reduce (sliderRow.proportionOfWidth (trimSliderWidth),
                      sliderRow.proportionOfHeight (trimSliderHeight));
    labelRow = sliderRow.removeFromBottom (sliderRow.proportionOfHeight (labelToSliderRatio));

    slMasterRatio.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    slMasterAttackTime.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    slMasterReleaseTime.setBounds (sliderRow.removeFromLeft (masterSliderWidth));
    lbRatio[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));
    lbAttack[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));
    lbRelease[numFilterBands].setBounds (labelRow.removeFromLeft (masterSliderWidth));

    // ==== FILTERBANKVISUALIZER SETTINGS ====
    const float trimHeight = 0.4f;
    const int trimFromLeft = 5;

    rightArea.removeFromLeft (trimFromLeft);
    rightArea.reduce (0, rightArea.proportionOfHeight (trimHeight));
    juce::Rectangle<int> totalMagnitudeButtonArea =
        rightArea.removeFromTop (rightArea.proportionOfHeight (0.5));
    tbOverallMagnitude.setBounds (totalMagnitudeButtonArea);
}

void MultiBandCompressorAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider->getName().startsWith ("MakeUpGain"))
    {
        filterBankVisualizer.updateMakeUpGain (
            slider->getName().getLastCharacters (1).getIntValue(),
            slider->getValue());
        return;
    }
}

void MultiBandCompressorAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button->getName().startsWith ("bypass"))
    {
        int i = button->getName().getLastCharacters (1).getIntValue();
        filterBankVisualizer.setBypassed (i, button->getToggleState());
    }
    else if (button->getName().startsWith ("solo"))
    {
        int i = button->getName().getLastCharacters (1).getIntValue();
        filterBankVisualizer.setSolo (i, button->getToggleState());
    }
    else // overall magnitude button
    {
        displayOverallMagnitude = button->getToggleState();
        if (displayOverallMagnitude)
            filterBankVisualizer.activateOverallMagnitude();
        else
            filterBankVisualizer.deactivateOverallMagnitude();
    }
}

void MultiBandCompressorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.repaintFilterVisualization.get())
    {
        processor.repaintFilterVisualization = false;
        filterBankVisualizer.updateFreqBandResponses();
    }

    omniInputMeter.setLevel (processor.inputPeak.get());
    omniOutputMeter.setLevel (processor.outputPeak.get());

    for (int i = 0; i < numFilterBands; ++i)
    {
        const auto gainReduction = processor.maxGR[i].get();

        filterBankVisualizer.updateGainReduction (i, gainReduction);
        compressorVisualizers[i]->setMarkerLevels (processor.maxPeak[i].get(), gainReduction);

        if (processor.characteristicHasChanged[i].get())
        {
            compressorVisualizers[i]->updateCharacteristic();
            processor.characteristicHasChanged[i] = false;
        }

        GRmeter[i].setLevel (gainReduction);
    }

    if (displayOverallMagnitude)
        filterBankVisualizer.updateOverallMagnitude();
}
