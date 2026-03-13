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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <EQData.h>
#include <IRData.h>

const juce::StringArray BinauralDecoderAudioProcessor::headphoneEQs =
    juce::StringArray ("AKG-K141MK2",
                       "AKG-K240DF",
                       "AKG-K240MK2",
                       "AKG-K271MK2",
                       "AKG-K271STUDIO",
                       "AKG-K601",
                       "AKG-K701",
                       "AKG-K702",
                       "AKG-K1000-Closed",
                       "AKG-K1000-Open",
                       "AudioTechnica-ATH-M50",
                       "Beyerdynamic-DT250",
                       "Beyerdynamic-DT770PRO-250Ohms",
                       "Beyerdynamic-DT880",
                       "Beyerdynamic-DT990PRO",
                       "Presonus-HD7",
                       "Sennheiser-HD430",
                       "Sennheiser-HD480",
                       "Sennheiser-HD560ovationII",
                       "Sennheiser-HD565ovation",
                       "Sennheiser-HD600",
                       "Sennheiser-HD650",
                       "SHURE-SRH940");

//==============================================================================
BinauralDecoderAudioProcessor::BinauralDecoderAudioProcessor() :
    AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
        BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput ("Input", juce::AudioChannelSet::discreteChannels (64), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::discreteChannels (64), true)
    #endif
            ,
#endif
        createParameterLayout())
{
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    applyHeadphoneEq = parameters.getRawParameterValue ("applyHeadphoneEq");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("applyHeadphoneEq", this);

    // load IRs
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    juce::WavAudioFormat wavFormat;

    juce::MemoryInputStream* mis[7];
    mis[0] = new juce::MemoryInputStream (IRData::irsOrd1_wav, IRData::irsOrd1_wavSize, false);
    mis[1] = new juce::MemoryInputStream (IRData::irsOrd2_wav, IRData::irsOrd2_wavSize, false);
    mis[2] = new juce::MemoryInputStream (IRData::irsOrd3_wav, IRData::irsOrd3_wavSize, false);
    mis[3] = new juce::MemoryInputStream (IRData::irsOrd4_wav, IRData::irsOrd4_wavSize, false);
    mis[4] = new juce::MemoryInputStream (IRData::irsOrd5_wav, IRData::irsOrd5_wavSize, false);
    mis[5] = new juce::MemoryInputStream (IRData::irsOrd6_wav, IRData::irsOrd6_wavSize, false);
    mis[6] = new juce::MemoryInputStream (IRData::irsOrd7_wav, IRData::irsOrd7_wavSize, false);

    for (int i = 0; i < 7; ++i)
    {
        irs[i].setSize (juce::square (i + 2), irLength);
        std::unique_ptr<juce::AudioFormatReader> reader (wavFormat.createReaderFor (mis[i], true));
        reader->read (&irs[i], 0, irLength, 0, true, false);
        irs[i].applyGain (0.3f);
    }
}

BinauralDecoderAudioProcessor::~BinauralDecoderAudioProcessor()
{
}

int BinauralDecoderAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int BinauralDecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BinauralDecoderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BinauralDecoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void BinauralDecoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BinauralDecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputOrderSetting, 0, true);

    juce::dsp::ProcessSpec convSpec;
    convSpec.sampleRate = sampleRate;
    convSpec.maximumBlockSize = samplesPerBlock;
    convSpec.numChannels =
        2; // convolve two channels (which actually point two one and the same input channel)

    EQ.prepare (convSpec);
}

void BinauralDecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void BinauralDecoderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                  juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *inputOrderSetting, 0, false);
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2)
    {
        buffer.clear();
        return;
    }

    const int nCh = juce::jmin (buffer.getNumChannels(), input.getNumberOfChannels());
    const int L = buffer.getNumSamples();
    const int ergL = overlapBuffer.getNumSamples();
    const int overlap = irLengthMinusOne;
    const int copyL = juce::jmin (L, overlap); // copy max L samples of the overlap data

    if (*useSN3D >= 0.5f)
        for (int ch = 1; ch < nCh; ++ch)
            buffer.applyGain (ch, 0, buffer.getNumSamples(), sn3d2n3d[ch]);

    // clear accumulation buffers
    juce::FloatVectorOperations::clear (reinterpret_cast<float*> (accumMid.data()), fftLength + 2);
    juce::FloatVectorOperations::clear (reinterpret_cast<float*> (accumSide.data()), fftLength + 2);

    const int nZeros = fftLength - L;

    //compute mid signal in frequency domain
    for (int midix = 0; midix < nMidCh; ++midix)
    {
        const int ch = mix2cix[midix];

        juce::FloatVectorOperations::copy (reinterpret_cast<float*> (fftBuffer.data()),
                                           buffer.getReadPointer (ch),
                                           L);
        juce::FloatVectorOperations::clear (reinterpret_cast<float*> (fftBuffer.data()) + L,
                                            nZeros);

        fft->performRealOnlyForwardTransform (reinterpret_cast<float*> (fftBuffer.data()));

        const auto tfMid =
            reinterpret_cast<const std::complex<float>*> (irsFrequencyDomain.getReadPointer (ch));
        for (int i = 0; i < fftLength / 2 + 1; ++i)
            accumMid[i] += fftBuffer[i] * tfMid[i];
    }

    //compute side signal in frequency domain
    for (int sidix = 0; sidix < nSideCh; ++sidix)
    {
        const int ch = six2cix[sidix];

        juce::FloatVectorOperations::copy (reinterpret_cast<float*> (fftBuffer.data()),
                                           buffer.getReadPointer (ch),
                                           L);
        juce::FloatVectorOperations::clear (reinterpret_cast<float*> (fftBuffer.data()) + L,
                                            nZeros);

        fft->performRealOnlyForwardTransform (reinterpret_cast<float*> (fftBuffer.data()));

        const auto tfSide =
            reinterpret_cast<const std::complex<float>*> (irsFrequencyDomain.getReadPointer (ch));
        for (int i = 0; i < fftLength / 2 + 1; ++i)
            accumSide[i] += fftBuffer[i] * tfSide[i];
    }

    fft->performRealOnlyInverseTransform (reinterpret_cast<float*> (accumMid.data()));
    fft->performRealOnlyInverseTransform (reinterpret_cast<float*> (accumSide.data()));

    ///* MS -> LR  */
    juce::FloatVectorOperations::copy (buffer.getWritePointer (0),
                                       reinterpret_cast<float*> (accumMid.data()),
                                       L);
    juce::FloatVectorOperations::copy (buffer.getWritePointer (1),
                                       reinterpret_cast<float*> (accumMid.data()),
                                       L);
    juce::FloatVectorOperations::add (buffer.getWritePointer (0),
                                      reinterpret_cast<float*> (accumSide.data()),
                                      L);
    juce::FloatVectorOperations::subtract (buffer.getWritePointer (1),
                                           reinterpret_cast<float*> (accumSide.data()),
                                           L);

    juce::FloatVectorOperations::add (buffer.getWritePointer (0),
                                      overlapBuffer.getWritePointer (0),
                                      copyL);
    juce::FloatVectorOperations::add (buffer.getWritePointer (1),
                                      overlapBuffer.getWritePointer (1),
                                      copyL);

    if (copyL < overlap) // there is some overlap left, want some?
    {
        const int howManyAreLeft = overlap - L;

        //shift the overlap buffer to the left
        juce::FloatVectorOperations::copy (overlapBuffer.getWritePointer (0),
                                           overlapBuffer.getReadPointer (0, L),
                                           howManyAreLeft);
        juce::FloatVectorOperations::copy (overlapBuffer.getWritePointer (1),
                                           overlapBuffer.getReadPointer (1, L),
                                           howManyAreLeft);

        //clear the tail
        juce::FloatVectorOperations::clear (overlapBuffer.getWritePointer (0, howManyAreLeft),
                                            ergL - howManyAreLeft);
        juce::FloatVectorOperations::clear (overlapBuffer.getWritePointer (1, howManyAreLeft),
                                            ergL - howManyAreLeft);

        /* MS -> LR  */
        juce::FloatVectorOperations::add (overlapBuffer.getWritePointer (0),
                                          reinterpret_cast<float*> (accumMid.data()) + L,
                                          irLengthMinusOne);
        juce::FloatVectorOperations::add (overlapBuffer.getWritePointer (1),
                                          reinterpret_cast<float*> (accumMid.data()) + L,
                                          irLengthMinusOne);
        juce::FloatVectorOperations::add (overlapBuffer.getWritePointer (0),
                                          reinterpret_cast<float*> (accumSide.data()) + L,
                                          irLengthMinusOne);
        juce::FloatVectorOperations::subtract (overlapBuffer.getWritePointer (1),
                                               reinterpret_cast<float*> (accumSide.data()) + L,
                                               irLengthMinusOne);
    }
    else
    {
        /* MS -> LR  */
        juce::FloatVectorOperations::copy (overlapBuffer.getWritePointer (0),
                                           reinterpret_cast<float*> (accumMid.data()) + L,
                                           irLengthMinusOne);
        juce::FloatVectorOperations::copy (overlapBuffer.getWritePointer (1),
                                           reinterpret_cast<float*> (accumMid.data()) + L,
                                           irLengthMinusOne);
        juce::FloatVectorOperations::add (overlapBuffer.getWritePointer (0),
                                          reinterpret_cast<float*> (accumSide.data()) + L,
                                          irLengthMinusOne);
        juce::FloatVectorOperations::subtract (overlapBuffer.getWritePointer (1),
                                               reinterpret_cast<float*> (accumSide.data()) + L,
                                               irLengthMinusOne);
    }

    if (*applyHeadphoneEq >= 0.5f)
    {
        float* channelData[2] = { buffer.getWritePointer (0), buffer.getWritePointer (1) };
        juce::dsp::AudioBlock<float> sumBlock (channelData, 2, L);
        juce::dsp::ProcessContextReplacing<float> eqContext (sumBlock);
        EQ.process (eqContext);
    }

    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool BinauralDecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BinauralDecoderAudioProcessor::createEditor()
{
    return new BinauralDecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void BinauralDecoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BinauralDecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect (
                    parameters.state.getProperty ("OSCPort", juce::var (-1)));
                parameters.state.removeProperty ("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName ("OSCConfig");
            if (oscConfig.isValid())
                oscParameterInterface.setConfig (oscConfig);
        }
}

//==============================================================================
void BinauralDecoderAudioProcessor::parameterChanged (const juce::String& parameterID,
                                                      float newValue)
{
    if (parameterID == "inputOrderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "applyHeadphoneEq")
    {
        const int sel (juce::roundToInt (newValue));
        if (sel > 0)
        {
            int sourceDataSize;
            juce::String name = headphoneEQs[sel - 1].replace ("-", "") + "_wav";
            auto* sourceData = EQData::getNamedResource (name.toUTF8(), sourceDataSize);
            if (sourceData == nullptr)
                DBG ("error");
            EQ.loadImpulseResponse (sourceData,
                                    sourceDataSize,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::no,
                                    2048,
                                    juce::dsp::Convolution::Normalise::no);
        }
    }
}

void BinauralDecoderAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());

    const double sampleRate = getSampleRate();
    const int blockSize = getBlockSize();

    int order = juce::jmax (input.getOrder(), 1);
    const int nCh =
        input
            .getNumberOfChannels(); // why not juce::jmin(buffer.....)? Is updateBuffers called before the first processBlock?
    DBG ("order: " << order);
    DBG ("nCh: " << nCh);

    int tmpOrder = sqrt (nCh) - 1;
    if (tmpOrder < order)
    {
        order = tmpOrder;
    }

    //get number of mid- and side-channels
    nSideCh = order * (order + 1) / 2;
    nMidCh = juce::square (order + 1)
             - nSideCh; //nMidCh = nCh - nSideCh; //nCh should be equalt to (order+1)^2

    if (order < 1)
        order = 1; // just use first order filters

    juce::AudioBuffer<float> resampledIRs;
    bool useResampled = false;
    irLength = 236;

    if (sampleRate != irsSampleRate && order != 0) // do resampling!
    {
        useResampled = true;
        double factorReading = irsSampleRate / sampleRate;
        irLength = juce::roundToInt (irLength / factorReading + 0.49);

        juce::MemoryAudioSource memorySource (irs[order - 1], false);
        juce::ResamplingAudioSource resamplingSource (&memorySource, false, nCh);

        resamplingSource.setResamplingRatio (factorReading);
        resamplingSource.prepareToPlay (irLength, sampleRate);

        resampledIRs.setSize (nCh, irLength);
        juce::AudioSourceChannelInfo info;
        info.startSample = 0;
        info.numSamples = irLength;
        info.buffer = &resampledIRs;

        resamplingSource.getNextAudioBlock (info);

        // compensate for more (correlated) samples contributing to output signal
        resampledIRs.applyGain (irsSampleRate / sampleRate);
    }

    irLengthMinusOne = irLength - 1;

    const int prevFftLength = fftLength;

    const int ergL = blockSize + irLength - 1; //max number of nonzero output samples
    fftLength = juce::nextPowerOfTwo (ergL); //fftLength >= ergL

    overlapBuffer.setSize (2, irLengthMinusOne);
    overlapBuffer.clear();

    if (prevFftLength != fftLength)
    {
        const int fftOrder = std::log2 (fftLength);

        fft = std::make_unique<juce::dsp::FFT> (fftOrder);

        fftBuffer.resize (fftLength);

        accumMid.resize (fftLength);
        accumSide.resize (fftLength);
    }

    irsFrequencyDomain.setSize (nCh, 2 * (fftLength / 2 + 1));
    irsFrequencyDomain.clear();

    for (int i = 0; i < nCh; ++i)
    {
        float* inOut = reinterpret_cast<float*> (fftBuffer.data());
        const float* src =
            useResampled ? resampledIRs.getReadPointer (i) : irs[order - 1].getReadPointer (i);
        juce::FloatVectorOperations::copy (inOut, src, irLength);
        juce::FloatVectorOperations::clear (inOut + irLength, fftLength - irLength); // zero padding
        fft->performRealOnlyForwardTransform (inOut);
        juce::FloatVectorOperations::copy (irsFrequencyDomain.getWritePointer (i),
                                           inOut,
                                           2 * (fftLength / 2 + 1));
    }
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    BinauralDecoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "inputOrderSetting",
        "Input Ambisonic Order",
        "",
        juce::NormalisableRange<float> (0.0f, 8.0f, 1.0f),
        0.0f,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "0th";
            else if (value >= 1.5f && value < 2.5f)
                return "1st";
            else if (value >= 2.5f && value < 3.5f)
                return "2nd";
            else if (value >= 3.5f && value < 4.5f)
                return "3rd";
            else if (value >= 4.5f && value < 5.5f)
                return "4th";
            else if (value >= 5.5f && value < 6.5f)
                return "5th";
            else if (value >= 6.5f && value < 7.5f)
                return "6th";
            else if (value >= 7.5f)
                return "7th";
            else
                return "Auto";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "useSN3D",
        "Input Normalization",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "SN3D";
            else
                return "N3D";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "applyHeadphoneEq",
        "Headphone Equalization",
        "",
        juce::NormalisableRange<float> (0.0f, float (headphoneEQs.size()), 1.0f),
        0.0f,
        [this] (float value)
        {
            if (value < 0.5f)
                return juce::String ("OFF");
            else
                return juce::String (this->headphoneEQs[juce::roundToInt (value) - 1]);
        },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauralDecoderAudioProcessor();
}
