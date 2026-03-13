/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich, Franz Zotter
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

#include <cfloat>

const juce::StringArray AllRADecoderAudioProcessor::weightsStrings =
    juce::StringArray ("basic", "maxrE", "inphase");

//==============================================================================
AllRADecoderAudioProcessor::AllRADecoderAudioProcessor() :
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
        createParameterLayout()),
    energyDistribution (juce::Image::PixelFormat::ARGB, 200, 100, true),
    rEVector (juce::Image::PixelFormat::ARGB, 200, 100, true)
{
    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue ("inputOrderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    decoderOrder = parameters.getRawParameterValue ("decoderOrder");
    exportDecoder = parameters.getRawParameterValue ("exportDecoder");
    exportLayout = parameters.getRawParameterValue ("exportLayout");
    weights = parameters.getRawParameterValue ("weights");

    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("useSN3D", this);

    // global properties taken from resources/Standalone/StandaloneApp.cpp
    juce::PropertiesFile::Options options;

    options.applicationName = "AllRADecoder";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support/IEMAudioPlugins";
#if JUCE_LINUX || JUCE_BSD
    options.folderName = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getFullPathName() + juce::File::getSeparatorString() + "IEMAudioPlugins";
#else
    options.folderName = "IEMAudioPlugins";
#endif

    properties.reset (new juce::PropertiesFile (options));
    lastDir = juce::File (properties->getValue ("presetFolder"));

    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, 0.0f), 1),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 45.0f, 0.0f), 2, true),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 90.0f, 0.0f), 3),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 135.0f, 0.0f), 4),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 180.0f, 0.0f), 5),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -135.0f, 0.0f), 6),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -90.0f, 0.0f), 7),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -45.0f, 0.0f), 8),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 22.5f, 40.0f), 9),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 142.5f, 40.0f), 10),
        &undoManager);
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -97.5f, 40.0f), 11),
        &undoManager);

    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, -90.0f), 12),
        &undoManager);

    loudspeakers.addListener (this);
    prepareLayout();
}

AllRADecoderAudioProcessor::~AllRADecoderAudioProcessor()
{
}

int AllRADecoderAudioProcessor::getNumPrograms()
{
    return 2; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AllRADecoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AllRADecoderAudioProcessor::setCurrentProgram (int index)
{
    if (index == 1)
    {
        loudspeakers.removeListener (this);

        undoManager.beginNewTransaction();
        loudspeakers.removeAllChildren (&undoManager);

        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, 0.0f), 1),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -24.0f, 0.0f), 2),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -48.0f, 0.0f), 3),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -72.0f, 0.0f), 4),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -102.0f, 0.0f), 5),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -138.0f, 0.0f), 6),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -178.0f, 0.0f), 7),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 142.0f, 0.0f), 8),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 102.0f, 0.0f), 9),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 70.0f, 0.0f), 10),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 45.0f, 0.0f), 11),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 23.0f, 0.0f), 12),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -24.0f, 28.0f), 13),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -69.0f, 28.0f), 14),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -115.0f, 27.0f), 15),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -159.0f, 27.0f), 16),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 155.0f, 28.0f), 17),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 112.0f, 28.0f), 18),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 66.0f, 29.0f), 19),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 22.0f, 28.0f), 20),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, 60.0f), 21),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -91.0f, 59.0f), 22),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, -178.0f, 59.0f), 23),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 86.0f, 60.0f), 24),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, 90.0f), 25),
            &undoManager);
        loudspeakers.appendChild (
            createLoudspeakerFromSpherical (juce::Vector3D<float> (1.0f, 0.0f, -90.0f),
                                            25,
                                            true,
                                            0.0f),
            &undoManager);

        loudspeakers.addListener (this);
        prepareLayout();
        updateTable = true;
    }
}

const juce::String AllRADecoderAudioProcessor::getProgramName (int index)
{
    if (index == 1)
        return "IEM CUBE";
    else
        return "default";
}

void AllRADecoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AllRADecoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput (this, *inputOrderSetting, 64, true);

    juce::dsp::ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 64;

    decoder.prepare (specs);
    noiseBurst.prepare (specs);
    ambisonicNoiseBurst.prepare (specs);
}

void AllRADecoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void AllRADecoderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    checkInputAndOutput (this, *inputOrderSetting, 64, false);
    juce::ScopedNoDenormals noDenormals;

    // ====== is a decoder loaded? stop processing if not ===========
    decoder.checkIfNewDecoderAvailable();
    if (decoder.getCurrentDecoder() == nullptr)
    {
        buffer.clear();
    }
    else
    {
        ambisonicNoiseBurst.processBuffer (buffer);

        const int nChIn = juce::jmin (decoder.getCurrentDecoder()->getNumInputChannels(),
                                      buffer.getNumChannels(),
                                      input.getNumberOfChannels());
        const int nChOut = juce::jmin (decoder.getCurrentDecoder()->getNumOutputChannels(),
                                       buffer.getNumChannels());

        for (int ch = juce::jmax (nChIn, nChOut); ch < buffer.getNumChannels();
             ++ch) // clear all not needed channels
            buffer.clear (ch, 0, buffer.getNumSamples());

        decoder.setInputNormalization (*useSN3D >= 0.5f
                                           ? ReferenceCountedDecoder::Normalization::sn3d
                                           : ReferenceCountedDecoder::Normalization::n3d);

        const int L = buffer.getNumSamples();
        auto inputAudioBlock =
            juce::dsp::AudioBlock<float> (buffer.getArrayOfWritePointers(), nChIn, L);
        auto outputAudioBlock =
            juce::dsp::AudioBlock<float> (buffer.getArrayOfWritePointers(), nChOut, L);

        decoder.process (inputAudioBlock, outputAudioBlock);

        for (int ch = nChOut; ch < buffer.getNumChannels(); ++ch) // clear all not needed channels
            buffer.clear (ch, 0, buffer.getNumSamples());
    }
    noiseBurst.processBuffer (buffer);
}

//==============================================================================
bool AllRADecoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AllRADecoderAudioProcessor::createEditor()
{
    return new AllRADecoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AllRADecoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (parameters.state.getChildWithName ("Loudspeakers").isValid()
        && parameters.state.getChildWithName ("Loudspeakers") != loudspeakers)
    {
        parameters.state.removeChild (parameters.state.getChildWithName ("Loudspeakers"), nullptr);
    }
    parameters.state.appendChild (loudspeakers, nullptr);

    auto state = parameters.copyState();
    auto oscConfig = state.getOrCreateChildWithName ("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom (oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AllRADecoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
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

        juce::XmlElement* lsps (xmlState->getChildByName ("Loudspeakers"));
        if (lsps != nullptr)
        {
            loudspeakers.removeListener (this);
            loudspeakers.removeAllChildren (nullptr);
            const int nChilds = lsps->getNumChildElements();
            for (int i = 0; i < nChilds; ++i)
            {
                juce::XmlElement* lsp (lsps->getChildElement (i));
                if (lsp->getTagName() == "Element" || lsp->getTagName() == "Loudspeaker")
                    loudspeakers.appendChild (
                        createLoudspeakerFromSpherical (
                            juce::Vector3D<float> (lsp->getDoubleAttribute ("Radius", 1.0),
                                                   lsp->getDoubleAttribute ("Azimuth"),
                                                   lsp->getDoubleAttribute ("Elevation")),
                            lsp->getIntAttribute ("Channel", -1),
                            lsp->getBoolAttribute ("Imaginary", false),
                            lsp->getDoubleAttribute ("Gain", 1.0)),
                        &undoManager);
            }
            undoManager.clearUndoHistory();
            loudspeakers.addListener (this);
            prepareLayout();
            updateTable = true;
            calculateDecoder();
        }
    }
}

//==============================================================================
void AllRADecoderAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    DBG ("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputOrderSetting" || parameterID == "outputOrderSetting")
        userChangedIOSettings = true;
    else if (parameterID == "useSN3D")
        decoder.setInputNormalization (*useSN3D >= 0.5f
                                           ? ReferenceCountedDecoder::Normalization::sn3d
                                           : ReferenceCountedDecoder::Normalization::n3d);
}

void AllRADecoderAudioProcessor::updateBuffers()
{
    DBG ("IOHelper:  input size: " << input.getSize());
    DBG ("IOHelper: output size: " << output.getSize());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AllRADecoderAudioProcessor();
}

juce::Result AllRADecoderAudioProcessor::verifyLoudspeakers()
{
    const int nLsps = loudspeakers.getNumChildren();
    if (nLsps == 0)
        return juce::Result::fail ("There are no loudspeakers.");

    for (int i = 0; i < nLsps; ++i)
    {
        juce::ValueTree lsp = loudspeakers.getChild (i);
        if (! lsp.isValid())
            return juce::Result::fail ("Something went wrong. Try again!");

        if (lsp.hasProperty ("Azimuth"))
        {
            const auto azimuth = lsp.getProperty ("Azimuth", juce::var());
            if (! azimuth.isDouble() && ! azimuth.isInt())
                return juce::Result::fail (
                    "Loudspeaker #" + juce::String (i + 1)
                    + ": 'Azimuth' value is neither a double nor an integer.");
        }
        else
            return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                       + ": Missing 'Azumith' attribute.");

        if (lsp.hasProperty ("Elevation")) //mandatory
        {
            const auto elevation = lsp.getProperty ("Elevation", juce::var());
            if (! elevation.isDouble() && ! elevation.isInt())
                return juce::Result::fail (
                    "Loudspeaker #" + juce::String (i + 1)
                    + ": 'Elevation' value is neither a double nor an integer.");
        }
        else
            return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                       + ": Missing 'Elevation' attribute.");

        if (lsp.hasProperty ("Radius")) //optional
        {
            const auto radius = lsp.getProperty ("Radius", juce::var());
            if (! radius.isDouble() && ! radius.isInt())
                return juce::Result::fail (
                    "Loudspeaker #" + juce::String (i + 1)
                    + ": 'Radius' value is neither a double nor an integer.");
            if ((float) radius < FLT_EPSILON)
                return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                           + ": Radius has to be greater than zero.");
        }

        bool isImaginary = false;
        if (lsp.hasProperty ("IsImaginary")) //optional
        {
            const auto imaginary = lsp.getProperty ("IsImaginary", juce::var());
            if (! imaginary.isBool())
                return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                           + ": 'IsImaginary' value is not a bool.");
            isImaginary = imaginary;
        }

        if (! isImaginary)
        {
            if (lsp.hasProperty ("Channel")) //mandatory
            {
                const auto channel = lsp.getProperty ("Channel", juce::var());
                if (! channel.isInt())
                    return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                               + ": 'Channel' value is not an integer.");
            }
            else
                return juce::Result::fail ("Loudspeaker #" + juce::String (i + 1)
                                           + ": Missing 'Channel' attribute.");
        }
    }

    return juce::Result::ok();
}

juce::Result AllRADecoderAudioProcessor::calculateTris()
{
    return juce::Result::ok();
}

juce::ValueTree AllRADecoderAudioProcessor::createLoudspeakerFromCartesian (
    juce::Vector3D<float> cartCoordinates,
    int channel,
    bool isImaginary,
    float gain)
{
    juce::Vector3D<float> sphericalCoordinates = cartesianToSpherical (cartCoordinates);
    return createLoudspeakerFromSpherical (sphericalCoordinates, channel, isImaginary, gain);
}

juce::ValueTree AllRADecoderAudioProcessor::createLoudspeakerFromSpherical (
    juce::Vector3D<float> sphericalCoordinates,
    int channel,
    bool isImaginary,
    float gain)
{
    return ConfigurationHelper::createElement (sphericalCoordinates.y,
                                               sphericalCoordinates.z,
                                               sphericalCoordinates.x,
                                               channel,
                                               isImaginary,
                                               gain);
}

juce::Vector3D<float>
    AllRADecoderAudioProcessor::cartesianToSpherical (juce::Vector3D<float> cartvect)
{
    const float r = cartvect.length();
    return juce::Vector3D<float> (
        r, // radius
        juce::radiansToDegrees (std::atan2 (cartvect.y, cartvect.x)), // azimuth
        juce::radiansToDegrees (
            std::atan2 (cartvect.z,
                        std::sqrt (cartvect.x * cartvect.x + cartvect.y * cartvect.y))) // elevation
    );
}

juce::Vector3D<float>
    AllRADecoderAudioProcessor::sphericalToCartesian (juce::Vector3D<float> sphervect)
{
    return juce::Vector3D<float> (sphervect.x * std::cos (juce::degreesToRadians (sphervect.z))
                                      * std::cos (juce::degreesToRadians (sphervect.y)),
                                  sphervect.x * std::cos (juce::degreesToRadians (sphervect.z))
                                      * std::sin (juce::degreesToRadians (sphervect.y)),
                                  sphervect.x * std::sin (juce::degreesToRadians (sphervect.z)));
}

juce::Vector3D<float>
    AllRADecoderAudioProcessor::sphericalInRadiansToCartesian (juce::Vector3D<float> sphervect)
{
    return juce::Vector3D<float> (sphervect.x * std::cos (sphervect.z) * cos (sphervect.y),
                                  sphervect.x * std::cos (sphervect.z) * sin (sphervect.y),
                                  sphervect.x * std::sin (sphervect.z));
}

void AllRADecoderAudioProcessor::playNoiseBurst (const int channel)
{
    noiseBurst.setChannel (channel);
}

void AllRADecoderAudioProcessor::playAmbisonicNoiseBurst (const float azimuthInDegrees,
                                                          const float elevationInDegrees)
{
    auto dec = decoder.getCurrentDecoder();
    if (dec != nullptr)
    {
        ambisonicNoiseBurst.setOrder (decoder.getCurrentDecoder()->getOrder());
        ambisonicNoiseBurst.setNormalization (*useSN3D >= 0.5f);
        ambisonicNoiseBurst.play (azimuthInDegrees, elevationInDegrees);
    }
}

void AllRADecoderAudioProcessor::addImaginaryLoudspeakerBelow()
{
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (
        createLoudspeakerFromCartesian (juce::Vector3D<float> (0.0f, 0.0f, -1.0f),
                                        highestChannelNumber + 1,
                                        true,
                                        0.0f),
        &undoManager);
}

void AllRADecoderAudioProcessor::addRandomPoint()
{
    undoManager.beginNewTransaction();
    loudspeakers.appendChild (createLoudspeakerFromSpherical (
                                  juce::Vector3D<float> (1.0f,
                                                         (rand() * 360.0f) / RAND_MAX,
                                                         (rand() * 180.0f) / RAND_MAX - 90.0f),
                                  highestChannelNumber + 1),
                              &undoManager);
}

void AllRADecoderAudioProcessor::convertLoudspeakersToArray()
{
    imaginaryFlags.clear();
    highestChannelNumber = 0;
    int i = 0;
    int imaginaryCount = 0;
    for (juce::ValueTree::Iterator it = loudspeakers.begin(); it != loudspeakers.end(); ++it)
    {
        const bool isImaginary = (*it).getProperty ("Imaginary");
        juce::Vector3D<float> spherical;
        spherical.x = isImaginary ? (float) (*it).getProperty ("Radius") : 1.0f;
        spherical.y = (*it).getProperty ("Azimuth");
        spherical.z = (*it).getProperty ("Elevation");

        juce::Vector3D<float> cart = sphericalToCartesian (spherical);

        R3 newPoint { cart.x, cart.y, cart.z };
        newPoint.lspNum = i;

        if (isImaginary)
        {
            imaginaryFlags.setBit (i);
            ++imaginaryCount;
            newPoint.realLspNum = -1;
        }
        else
            newPoint.realLspNum = i - imaginaryCount;

        newPoint.azimuth = (*it).getProperty ("Azimuth");
        newPoint.elevation = (*it).getProperty ("Elevation");
        newPoint.radius = (*it).getProperty ("Radius");

        newPoint.isImaginary = isImaginary;
        newPoint.gain = (*it).getProperty ("Gain");
        newPoint.channel = (*it).getProperty ("Channel");

        if (newPoint.channel > highestChannelNumber)
            highestChannelNumber = newPoint.channel;

        DBG (newPoint.lspNum << " \t " << newPoint.realLspNum << " \t " << newPoint.gain << " \t "
                             << newPoint.x << " \t " << newPoint.y << " \t " << newPoint.z);
        points.push_back (newPoint);
        ++i;
    }
}

void AllRADecoderAudioProcessor::prepareLayout()
{
    isLayoutReady = false;

    wrapSphericalCoordinates();
    juce::Result res = checkLayout();
    if (res.failed())
    {
        DBG (res.getErrorMessage());
        MailBox::Message newMessage;
        newMessage.messageColour = juce::Colours::red;
        newMessage.headline = "Improper layout";
        newMessage.text = res.getErrorMessage();
        messageToEditor = newMessage;
        updateMessage = true;
    }
    else
    {
        DBG ("Layout is ready for creating a decoder!");
        MailBox::Message newMessage;
        newMessage.messageColour = juce::Colours::cornflowerblue;
        newMessage.headline = "Suitable layout";
        newMessage.text = "The layout is ready to calculate a decoder.";
        messageToEditor = newMessage;
        updateMessage = true;

        isLayoutReady = true;
    }
}

juce::Result AllRADecoderAudioProcessor::checkLayout()
{
    points.clear();
    triangles.clear();
    normals.clear();

    // Check if loudspeakers are stored properly
    juce::Result res = verifyLoudspeakers();
    if (res.failed())
    {
        updateLoudspeakerVisualization = true;
        return juce::Result::fail (res.getErrorMessage());
    }

    convertLoudspeakersToArray();

    // Check for duplicates
    std::vector<int> outx;
    std::vector<R3> noDuplicates;
    const int nDuplicates = de_duplicateR3 (points, outx, noDuplicates);

    if (nDuplicates > 0)
    {
        updateLoudspeakerVisualization = true;
        return juce::Result::fail ("ERROR 1: There are duplicate loudspeakers.");
    }

    const int nLsps = loudspeakers.getNumChildren();
    if (nLsps < 4)
    {
        updateLoudspeakerVisualization = true;
        return juce::Result::fail ("ERROR 2: There are less than 4 loudspeakers! Add some more!");
    }

    // calculate convex hull
    const int result = NewtonApple_hull_3D (points, triangles);
    if (result != 1)
    {
        return juce::Result::fail (
            "ERROR: An error occurred! The layout might be broken somehow. Try adding additional loudspeakers (e.g. imaginary ones) or make small changes to the coordinates.");
    }

    // normalise normal vectors
    for (int i = 0; i < triangles.size(); ++i)
    {
        const Tri tri = triangles[i];
        normals.push_back (juce::Vector3D<float> (tri.er, tri.ec, tri.ez).normalised());
    }

    updateLoudspeakerVisualization = true;

    juce::Array<int> usedIndices;
    // calculate centroid
    juce::Vector3D<float> centroid { 0.0f, 0.0f, 0.0f };
    for (int i = 0; i < nLsps; ++i)
    {
        R3 lsp = points[i];
        centroid += juce::Vector3D<float> (lsp.x, lsp.y, lsp.z);
        usedIndices.add (i);
    }
    centroid /= nLsps;

    DBG ("centroid: x: " << centroid.x << " y: " << centroid.y << " z: " << centroid.z);
    for (int i = 0; i < triangles.size(); ++i)
    {
        Tri tri = triangles[i];
        juce::Vector3D<float> a { points[tri.a].x, points[tri.a].y, points[tri.a].z };

        usedIndices.removeFirstMatchingValue (tri.a);
        usedIndices.removeFirstMatchingValue (tri.b);
        usedIndices.removeFirstMatchingValue (tri.c);

        const float dist = normals[i] * (a - centroid);
        if (dist < 0.001f) // too flat!
        {
            return juce::Result::fail (
                "ERROR 3: Convex hull is too flat. Check coordinates and/or try adding imaginary loudspeakers.");
        }

        if (normals[i] * a < 0.001f) // origin is not within hull
        {
            return juce::Result::fail (
                "ERROR 4: Point of origin is not within the convex hull. Try adding imaginary loudspeakers.");
        }

        const int numberOfImaginaryLspsInTriangle = (int) imaginaryFlags[points[tri.a].lspNum]
                                                    + (int) imaginaryFlags[points[tri.b].lspNum]
                                                    + (int) imaginaryFlags[points[tri.c].lspNum];
        if (numberOfImaginaryLspsInTriangle > 1)
        {
            return juce::Result::fail (
                "ERROR 5: There is a triangle with more than one imaginary loudspeaker.");
        }
    }

    if (usedIndices.size() > 0)
        return juce::Result::fail (
            "ERROR 6: There is at least one loudspeaker which is not part of the convex hull.");

    if (imaginaryFlags.countNumberOfSetBits() == nLsps)
        return juce::Result::fail ("ERROR 7: There are only imaginary loudspeakers.");

    juce::Array<int> routing;
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
        {
            const int channel = points[i].channel;
            if (channel < 1)
                return juce::Result::fail ("ERROR 8: A channel number is smaller than 1.");

            if (routing.contains (channel))
                return juce::Result::fail (
                    "ERROR 9: Channel number duplicates: a channel number may occur only once.");
            else
                routing.add (channel);
        }
    }

    return juce::Result::ok();
}

juce::Result AllRADecoderAudioProcessor::calculateDecoder()
{
    if (! isLayoutReady)
        return juce::Result::fail ("Layout not ready!");

    const int N = juce::roundToInt (decoderOrder->load()) + 1;
    const auto ambisonicWeights =
        ReferenceCountedDecoder::Weights (juce::roundToInt (weights->load()));
    const int nCoeffs = juce::square (N + 1);
    const int nLsps = (int) points.size();
    const int nRealLsps = nLsps - imaginaryFlags.countNumberOfSetBits();
    DBG ("Number of loudspeakers: " << nLsps << ". Number of real loudspeakers: " << nRealLsps);
    juce::dsp::Matrix<float> decoderMatrix (nRealLsps, nCoeffs);

    juce::Array<juce::dsp::Matrix<float>> inverseArray;
    for (int t = 0; t < triangles.size(); ++t) //iterate over each triangle
    {
        Tri tri = triangles[t];
        juce::Array<int> triangleIndices (tri.a, tri.b, tri.c);
        juce::dsp::Matrix<float> L (3, 3);
        for (int i = 0; i < 3; ++i)
        {
            L (0, i) = points[triangleIndices[i]].x;
            L (1, i) = points[triangleIndices[i]].y;
            L (2, i) = points[triangleIndices[i]].z;
            if (points[triangleIndices[i]].isImaginary)
            {
                const float factor = 1.0f
                                     / std::sqrt (juce::square (points[triangleIndices[i]].x)
                                                  + juce::square (points[triangleIndices[i]].y)
                                                  + juce::square (points[triangleIndices[i]].z));
                L (0, i) *= factor;
                L (1, i) *= factor;
                L (2, i) *= factor;
            }
        }

        inverseArray.add (getInverse (L));
    }

    std::vector<float> sh;
    sh.resize (nCoeffs);

    for (int i = 0; i < 5200; ++i) //iterate over each tDesign point
    {
        const juce::dsp::Matrix<float> source (3, 1, tDesign5200[i]);
        SHEval (N, source (0, 0), source (1, 0), source (2, 0), &sh[0], false);

        const juce::dsp::Matrix<float> gains (3, 1);

        int t = 0;
        for (; t < triangles.size(); ++t) //iterate over each triangle
        {
            Tri tri = triangles[t];
            juce::Array<int> triangleIndices (tri.a, tri.b, tri.c);
            juce::Array<bool> imagFlags (points[tri.a].isImaginary,
                                         points[tri.b].isImaginary,
                                         points[tri.c].isImaginary);
            juce::dsp::Matrix<float> gains (3, 1);
            gains = inverseArray.getUnchecked (t) * source;

            if (gains (0, 0) >= -FLT_EPSILON && gains (1, 0) >= -FLT_EPSILON
                && gains (2, 0) >= -FLT_EPSILON)
            {
                // we found the corresponding triangle!
                const float foo =
                    1.0f
                    / std::sqrt (juce::square (gains (0, 0)) + juce::square (gains (1, 0))
                                 + juce::square (gains (2, 0)));
                gains = gains * foo;

                if (imagFlags.contains (true))
                {
                    const int imagGainIdx = imagFlags.indexOf (
                        true); // which of the three corresponds to the imaginary loudspeaker
                    const int imaginaryLspIdx = triangleIndices[imagGainIdx];

                    juce::Array<int> realGainIndex (0, 1, 2);
                    realGainIndex.remove (imagGainIdx);

                    juce::Array<int> connectedLsps;
                    connectedLsps.add (imaginaryLspIdx);
                    for (int k = 0; k < triangles.size(); ++k) //iterate over each triangle
                    {
                        Tri probe = triangles[k];
                        juce::Array<int> probeTriangleIndices (probe.a, probe.b, probe.c);
                        if (probeTriangleIndices.contains (imaginaryLspIdx))
                        { // found searched imaginaryLspIdx in that triangle
                            for (int j = 0; j < 3; ++j)
                            {
                                if (! connectedLsps.contains (probeTriangleIndices[j]))
                                    connectedLsps.add (probeTriangleIndices[j]);
                            }
                        }
                    }

                    connectedLsps.remove (0); // remove imaginary loudspeaker again
                    juce::Array<float> gainVector;
                    gainVector.resize (connectedLsps.size());

                    const float kappa = getKappa (gains (imagGainIdx, 0),
                                                  gains (realGainIndex[0], 0),
                                                  gains (realGainIndex[1], 0),
                                                  connectedLsps.size());

                    gainVector.fill (gains (imagGainIdx, 0) * (points[imaginaryLspIdx].gain)
                                     * kappa);

                    for (int j = 0; j < 2; ++j)
                    {
                        const int idx = connectedLsps.indexOf (triangleIndices[realGainIndex[j]]);
                        gainVector.set (idx, gainVector[idx] + gains (realGainIndex[j], 0));
                    }

                    for (int n = 0; n < connectedLsps.size(); ++n)
                        juce::FloatVectorOperations::addWithMultiply (
                            &decoderMatrix (points[connectedLsps[n]].realLspNum, 0),
                            &sh[0],
                            gainVector[n],
                            nCoeffs);
                }
                else
                {
                    juce::FloatVectorOperations::addWithMultiply (
                        &decoderMatrix (points[tri.a].realLspNum, 0),
                        &sh[0],
                        gains (0, 0),
                        nCoeffs);
                    juce::FloatVectorOperations::addWithMultiply (
                        &decoderMatrix (points[tri.b].realLspNum, 0),
                        &sh[0],
                        gains (1, 0),
                        nCoeffs);
                    juce::FloatVectorOperations::addWithMultiply (
                        &decoderMatrix (points[tri.c].realLspNum, 0),
                        &sh[0],
                        gains (2, 0),
                        nCoeffs);
                }
                break;
            }
        }
        jassert (t < triangles.size());
    }

    // calculate max lsp gain
    float maxGain = 0.0f;
    for (int i = 0; i < nLsps; ++i)
    {
        const R3 point = points[i];
        if (! point.isImaginary)
        {
            SHEval (N, point.x, point.y, point.z, &sh[0]); // encoding at loudspeaker position
            float sumOfSquares = 0.0f;
            for (int m = 0; m < nRealLsps; ++m)
            {
                float sum = 0.0f;
                for (int n = 0; n < nCoeffs; ++n)
                    sum += sh[n] * decoderMatrix (m, n);
                sumOfSquares += juce::square (sum);
            }
            sumOfSquares = sqrt (sumOfSquares);
            if (sumOfSquares > maxGain)
                maxGain = sumOfSquares;
        }
    }

    decoderMatrix = decoderMatrix * (1.0f / maxGain);

    // calculate energy and rE vector
    juce::Array<juce::Vector3D<float>> realLspsCoordinates;
    realLspsCoordinates.resize (nRealLsps);
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
            realLspsCoordinates.set (points[i].realLspNum,
                                     juce::Vector3D<float> (points[i].x, points[i].y, points[i].z)
                                         .normalised()); // zero count
    }

    const int w = energyDistribution.getWidth();
    const float wHalf = w / 2;
    const int h = energyDistribution.getHeight();
    const float hHalf = h / 2;
    float minLvl = 0.0f;
    float maxLvl = 0.0f;
    float sumLvl = 0.0f;
    auto levelValues = juce::dsp::Matrix<float> (w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            juce::Vector3D<float> spher (1.0f, 0.0f, 0.0f);
            HammerAitov::XYToSpherical ((x - wHalf) / wHalf, (hHalf - y) / hHalf, spher.y, spher.z);
            juce::Vector3D<float> cart = sphericalInRadiansToCartesian (spher);
            SHEval (N, cart.x, cart.y, cart.z, &sh[0]); // encoding a source

            if (ambisonicWeights == ReferenceCountedDecoder::Weights::maxrE)
                multiplyMaxRE (N, sh.data());
            else if (ambisonicWeights == ReferenceCountedDecoder::Weights::inPhase)
                multiplyInPhase (N, sh.data());

            juce::Vector3D<float> rE (0.0f, 0.0f, 0.0f);
            float sumOfSquares = 0.0f;
            for (int m = 0; m < nRealLsps; ++m)
            {
                float sum = 0.0f;
                for (int n = 0; n < nCoeffs; ++n)
                    sum += (sh[n] * decoderMatrix (m, n));
                const float sumSquared = juce::square (sum);
                rE += realLspsCoordinates[m] * sumSquared;
                sumOfSquares += sumSquared;
            }

            const float lvl = 0.5f * juce::Decibels::gainToDecibels (sumOfSquares);
            levelValues (x, y) = lvl;
            sumLvl += lvl;

            if (lvl > maxLvl)
                maxLvl = lvl;
            if (lvl < minLvl)
                minLvl = lvl;

            rE /= sumOfSquares + FLT_EPSILON;
            const float width = 2.0f * std::acos (juce::jmin (1.0f, rE.length()));
            const float reMap = juce::jlimit (0.0f, 1.0f, width / juce::MathConstants<float>::pi);

            const juce::Colour rEPixelColour = juce::Colours::limegreen.withMultipliedAlpha (reMap);
            rEVector.setPixelAt (x, y, rEPixelColour);
        }

    const float meanLvl = sumLvl / (w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            constexpr float plusMinusRange = 1.5f;
            const float map =
                (juce::jlimit (-plusMinusRange, plusMinusRange, levelValues (x, y) - meanLvl)
                 + plusMinusRange)
                / (2 * plusMinusRange);

            const juce::Colour pixelColour = juce::Colours::red.withMultipliedAlpha (map);
            energyDistribution.setPixelAt (x, y, pixelColour);
        }

    DBG ("min: " << minLvl << " max: " << maxLvl);

    updateLoudspeakerVisualization = true;

    ReferenceCountedDecoder::Ptr newDecoder = new ReferenceCountedDecoder (
        "Decoder",
        "A " + getOrderString (N) + " order Ambisonics decoder using the AllRAD approach.",
        (int) decoderMatrix.getSize()[0],
        (int) decoderMatrix.getSize()[1]);
    newDecoder->getMatrix() = decoderMatrix;
    ReferenceCountedDecoder::Settings newSettings;
    newSettings.expectedNormalization = ReferenceCountedDecoder::Normalization::n3d;
    newSettings.weights = ambisonicWeights;
    newSettings.weightsAlreadyApplied = false;

    newDecoder->setSettings (newSettings);

    juce::Array<int>& routing = newDecoder->getRoutingArrayReference();
    routing.resize (nRealLsps);
    for (int i = 0; i < nLsps; ++i)
    {
        if (! points[i].isImaginary)
            routing.set (points[i].realLspNum, points[i].channel - 1); // zero count
    }

    decoder.setDecoder (newDecoder);
    decoderConfig = newDecoder;

    updateChannelCount = true;

    DBG ("finished");

    MailBox::Message newMessage;
    newMessage.messageColour = juce::Colours::green;
    newMessage.headline = "Decoder created";
    newMessage.text = "The decoder was calculated successfully.";
    messageToEditor = newMessage;
    updateMessage = true;

    return juce::Result::ok();
}

float AllRADecoderAudioProcessor::getKappa (float gIm, float gRe1, float gRe2, int N)
{
    const float p = gIm * (gRe1 + gRe2) / (N * juce::square (gIm));
    const float q = (juce::square (gRe1) + juce::square (gRe2) - 1.0f) / (N * juce::square (gIm));
    return -p + std::sqrt (juce::jmax (juce::square (p) - q, 0.0f));
}

juce::dsp::Matrix<float> AllRADecoderAudioProcessor::getInverse (juce::dsp::Matrix<float> A)
{
    const float det = A (0, 0) * (A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1))
                      + A (0, 1) * (A (1, 2) * A (2, 0) - A (1, 0) * A (2, 2))
                      + A (0, 2) * (A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0));

    const float factor = 1.0 / det;

    juce::dsp::Matrix<float> inverse (3, 3);

    inverse (0, 0) = (A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1)) * factor;
    inverse (0, 1) = (-A (0, 1) * A (2, 2) + A (0, 2) * A (2, 1)) * factor;
    inverse (0, 2) = (A (0, 1) * A (1, 2) - A (0, 2) * A (1, 1)) * factor;

    inverse (1, 0) = (-A (1, 0) * A (2, 2) + A (1, 2) * A (2, 0)) * factor;
    inverse (1, 1) = (A (0, 0) * A (2, 2) - A (0, 2) * A (2, 0)) * factor;
    inverse (1, 2) = (-A (0, 0) * A (1, 2) + A (0, 2) * A (1, 0)) * factor;

    inverse (2, 0) = (A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0)) * factor;
    inverse (2, 1) = (-A (0, 0) * A (2, 1) + A (0, 1) * A (2, 0)) * factor;
    inverse (2, 2) = (A (0, 0) * A (1, 1) - A (0, 1) * A (1, 0)) * factor;

    return inverse;
}

void AllRADecoderAudioProcessor::saveConfigurationToFile (juce::File destination)
{
    if (*exportDecoder < 0.5f && *exportLayout < 0.5f)
    {
        DBG ("nothing to export");
        MailBox::Message newMessage;
        newMessage.messageColour = juce::Colours::red;
        newMessage.headline = "Nothing to export.";
        newMessage.text = "Please select at least one of the export options.";
        messageToEditor = newMessage;
        updateMessage = true;
        return;
    }

    auto* jsonObj = new juce::DynamicObject();
    jsonObj->setProperty (
        "Name",
        juce::var ("All-Round Ambisonic decoder (AllRAD) and loudspeaker layout"));
    char versionString[10];
    strcpy (versionString, "v");
    strcat (versionString, JucePlugin_VersionString);
    jsonObj->setProperty (
        "Description",
        juce::var ("This configuration file was created with the IEM AllRADecoder "
                   + juce::String (versionString) + " plug-in. "
                   + juce::Time::getCurrentTime().toString (true, true)));

    if (*exportDecoder >= 0.5f)
    {
        if (decoderConfig != nullptr)
            jsonObj->setProperty ("Decoder",
                                  ConfigurationHelper::convertDecoderToVar (decoderConfig));
        else
        {
            DBG ("No decoder available");
            MailBox::Message newMessage;
            newMessage.messageColour = juce::Colours::red;
            newMessage.headline = "No decoder available for export.";
            newMessage.text = "Please calculate a decoder first.";
            messageToEditor = newMessage;
            updateMessage = true;
            return;
        }
    }
    if (*exportLayout >= 0.5f)
        jsonObj->setProperty (
            "LoudspeakerLayout",
            ConfigurationHelper::convertLoudspeakersToVar (loudspeakers, "A loudspeaker layout"));

    juce::Result result =
        ConfigurationHelper::writeConfigurationToFile (destination, juce::var (jsonObj));
    if (result.wasOk())
    {
        DBG ("Configuration successfully written to file.");
        MailBox::Message newMessage;
        newMessage.messageColour = juce::Colours::green;
        newMessage.headline = "Configuration exported successfully";
        newMessage.text =
            "The decoder was successfully written to " + destination.getFileName() + ".";
        messageToEditor = newMessage;
        updateMessage = true;
    }
    else
        DBG ("Could not write configuration file.");
}

void AllRADecoderAudioProcessor::setLastDir (juce::File newLastDir)
{
    lastDir = newLastDir;
    const juce::var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}

void AllRADecoderAudioProcessor::valueTreePropertyChanged (
    juce::ValueTree& treeWhosePropertyHasChanged,
    const juce::Identifier& property)
{
    DBG ("valueTreePropertyChanged");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildAdded (juce::ValueTree& parentTree,
                                                      juce::ValueTree& childWhichHasBeenAdded)
{
    DBG ("valueTreeChildAdded");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildRemoved (juce::ValueTree& parentTree,
                                                        juce::ValueTree& childWhichHasBeenRemoved,
                                                        int indexFromWhichChildWasRemoved)
{
    DBG ("valueTreeChildRemoved");
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::valueTreeChildOrderChanged (
    juce::ValueTree& parentTreeWhoseChildrenHaveMoved,
    int oldIndex,
    int newIndex)
{
    DBG ("valueTreeChildOrderChanged");
    prepareLayout();
}

void AllRADecoderAudioProcessor::valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged)
{
    DBG ("valueTreeParentChanged");
}

void AllRADecoderAudioProcessor::loadConfiguration (const juce::File& configFile)
{
    undoManager.beginNewTransaction();
    loudspeakers.removeAllChildren (&undoManager);

    juce::Result result =
        ConfigurationHelper::parseFileForLoudspeakerLayout (configFile, loudspeakers, &undoManager);
    if (! result.wasOk())
    {
        DBG ("Configuration could not be loaded.");
        MailBox::Message newMessage;
        newMessage.messageColour = juce::Colours::red;
        newMessage.headline = "Error loading configuration";
        newMessage.text = result.getErrorMessage();
        messageToEditor = newMessage;
        updateMessage = true;
    }
}

void AllRADecoderAudioProcessor::rotate (const float degreesAddedToAzimuth)
{
    loudspeakers.removeListener (this);
    undoManager.beginNewTransaction();

    bool amountIsPositive = degreesAddedToAzimuth > 0;
    const int nLsps = loudspeakers.getNumChildren();
    for (int i = 0; i < nLsps; ++i)
    {
        auto lsp = loudspeakers.getChild (i);
        float val = lsp.getProperty ("Azimuth");
        val += degreesAddedToAzimuth;
        if (amountIsPositive && val > 360.0f)
            val -= 360.0f;
        else if (! amountIsPositive && val < -360.0f)
            val += 360.0f;
        lsp.setProperty ("Azimuth", val, &undoManager);
    }
    loudspeakers.addListener (this);
    prepareLayout();
    updateTable = true;
}

void AllRADecoderAudioProcessor::wrapSphericalCoordinates()
{
    loudspeakers.removeListener (this);

    for (juce::ValueTree::Iterator it = loudspeakers.begin(); it != loudspeakers.end(); ++it)
    {
        bool updateElevation = false;

        float az = (*it).getProperty ("Azimuth");
        float el = (*it).getProperty ("Elevation");

        // Wrap elevation
        if (el > 180.0f || el < -180.0f)
        {
            float fullRotations = std::copysign (std::ceil (std::abs (el) / 360.0f), el);
            el -= fullRotations * 360.0f;
            updateElevation = true;
        }
        if (el > 90.0f || el < -90.0f)
        {
            el = std::copysign (180.0 - std::abs (el), el);
            az += 180.0f;
            updateElevation = true;
            (*it).setProperty ("Azimuth", az, &undoManager);
        }

        // Wrap azimuth
        if (az > 180.0f || az < -180.0f)
        {
            float fullRotations = std::copysign (std::ceil (std::abs (az) / 360.0f), az);
            az -= fullRotations * 360.0f;

            (*it).setProperty ("Azimuth", az, nullptr);
        }

        // Update coordinates
        if (updateElevation)
            (*it).setProperty ("Elevation", el, nullptr);
    }

    loudspeakers.addListener (this);
}

//==============================================================================
const bool AllRADecoderAudioProcessor::interceptOSCMessage (juce::OSCMessage& message)
{
    if (message.getAddressPattern().toString().equalsIgnoreCase (
            "/" + juce::String (JucePlugin_Name) + "/decoderOrder")
        && message.size() >= 1)
    {
        if (message[0].isInt32())
        {
            auto value = message[0].getInt32() - 1;
            message.clear();
            message.addInt32 (value);
        }
        else if (message[0].isFloat32())
        {
            auto value = message[0].getFloat32() - 1;
            message.clear();
            message.addFloat32 (value);
        }
    }

    return false;
}

const bool
    AllRADecoderAudioProcessor::processNotYetConsumedOSCMessage (const juce::OSCMessage& message)
{
    const juce::String prefix ("/" + juce::String (JucePlugin_Name));
    if (message.getAddressPattern().toString().startsWith (prefix))
    {
        juce::OSCMessage msg (message);
        msg.setAddressPattern (message.getAddressPattern().toString().substring (
            juce::String (JucePlugin_Name).length() + 1));

        if (msg.getAddressPattern().toString().equalsIgnoreCase ("/loadFile") && msg.size() >= 1)
        {
            if (msg[0].isString())
            {
                juce::File fileToLoad (msg[0].getString());
                loadConfiguration (fileToLoad);
                return true;
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/calculate")
                 || msg.getAddressPattern().toString().equalsIgnoreCase ("/calculateDecoder"))
        {
            calculateDecoder();
            return true;
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/export") && msg.size() >= 1)
        {
            if (msg[0].isString())
            {
                juce::File file (msg[0].getString());
                saveConfigurationToFile (file);
                return true;
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/playNoise")
                 && msg.size() >= 1)
        {
            if (msg[0].isInt32())
            {
                const int channel = msg[0].getInt32();
                if (channel <= 64)
                {
                    playNoiseBurst (channel);
                    return true;
                }
            }
        }
        else if (msg.getAddressPattern().toString().equalsIgnoreCase ("/playEncodedNoise")
                 && msg.size() >= 2)
        {
            float azimuth = 0.0f;
            float elevation = 0.0f;

            if (msg[0].isInt32())
                azimuth = msg[0].getInt32();
            else if (msg[0].isFloat32())
                azimuth = msg[0].getFloat32();
            else
                return false;

            if (msg[1].isInt32())
                elevation = msg[1].getInt32();
            else if (msg[1].isFloat32())
                elevation = msg[1].getFloat32();
            else
                return false;

            playAmbisonicNoiseBurst (azimuth, elevation);
            return true;
        }
    }

    return false;
}

// ==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>>
    AllRADecoderAudioProcessor::createParameterLayout()
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
        "decoderOrder",
        "Decoder Order",
        "",
        juce::NormalisableRange<float> (0.0f, 6.0f, 1.0f),
        0.0f,
        [] (float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "2nd";
            else if (value >= 1.5f && value < 2.5f)
                return "3rd";
            else if (value >= 2.5f && value < 3.5f)
                return "4th";
            else if (value >= 3.5f && value < 4.5f)
                return "5th";
            else if (value >= 4.5f && value < 5.5f)
                return "6th";
            else if (value >= 5.5f)
                return "7th";
            else
                return "1st";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "exportDecoder",
        "Export Decoder",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "Yes";
            else
                return "No";
        },
        nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay (
        "exportLayout",
        "Export Layout",
        "",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f),
        1.0f,
        [] (float value)
        {
            if (value >= 0.5f)
                return "Yes";
            else
                return "No";
        },
        nullptr));

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("weights",
                                                                    "Ambisonic Weights",
                                                                    weightsStrings,
                                                                    1));

    return params;
}
