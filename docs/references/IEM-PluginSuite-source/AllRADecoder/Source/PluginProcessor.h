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

#pragma once

#include "../../resources/AudioProcessorBase.h"
#include "../JuceLibraryCode/JuceHeader.h"

#include "../../resources/AmbisonicDecoder.h"
#include "../../resources/NewtonApple/NewtonApple_hull3D.h"
#include "../../resources/ReferenceCountedDecoder.h"
#include "../../resources/customComponents/MailBox.h"
#include "../../resources/efficientSHvanilla.h"
#include "tDesign5200.h"

#define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 1
#define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 1
#include "../../resources/ConfigurationHelper.h"
#include "../../resources/HammerAitov.h"
#include "../../resources/ambisonicTools.h"
#include "AmbisonicNoiseBurst.h"
#include "NoiseBurst.h"

#define ProcessorClass AllRADecoderAudioProcessor

//==============================================================================

class AllRADecoderAudioProcessor
    : public AudioProcessorBase<IOTypes::Ambisonics<7>, IOTypes::AudioChannels<64>>,
      public juce::ValueTree::Listener
{
public:
    constexpr static int numberOfInputChannels = 64;
    constexpr static int numberOfOutputChannels = 64;
    static const juce::StringArray weightsStrings;

    //==============================================================================
    AllRADecoderAudioProcessor();
    ~AllRADecoderAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void updateBuffers() override; // use this to implement a buffer update method

    //==============================================================================
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& parentTree,
                              juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved (juce::ValueTree& parentTree,
                                juce::ValueTree& childWhichHasBeenRemoved,
                                int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged (juce::ValueTree& parentTreeWhoseChildrenHaveMoved,
                                     int oldIndex,
                                     int newIndex) override;
    void valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged) override;

    void playNoiseBurst (const int channel);
    void playAmbisonicNoiseBurst (const float azimuth, const float elevation);
    void addRandomPoint();
    void addImaginaryLoudspeakerBelow();

    void undo() { undoManager.undo(); }
    void redo() { undoManager.redo(); }

    void rotate (const float degreesAddedToAzimuth);

    void saveConfigurationToFile (juce::File destination);
    void loadConfiguration (const juce::File& presetFile);

    juce::ValueTree& getLoudspeakersValueTree() { return loudspeakers; }

    juce::var lsps;
    juce::Atomic<bool> updateLoudspeakerVisualization = false;
    juce::Atomic<bool> updateTable = true;
    juce::Atomic<bool> updateMessage = true;
    juce::Atomic<bool> updateChannelCount = true;

    ReferenceCountedDecoder::Ptr getCurrentDecoder() { return decoderConfig; }

    std::vector<R3> points;
    std::vector<Tri> triangles;
    std::vector<juce::Vector3D<float>> normals;

    juce::BigInteger imaginaryFlags;
    juce::UndoManager undoManager;

    juce::Result calculateDecoder();

    void setLastDir (juce::File newLastDir);
    juce::File getLastDir() { return lastDir; };

    juce::Image energyDistribution;
    juce::Image rEVector;

    MailBox::Message messageToEditor;

    //======= Parameters ===========================================================
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> createParameterLayout();

    //==============================================================================
    inline const bool interceptOSCMessage (juce::OSCMessage& message) override;
    inline const bool processNotYetConsumedOSCMessage (const juce::OSCMessage& message) override;

private:
    //==============================================================================
    // list of used audio parameters
    std::atomic<float>* inputOrderSetting;
    std::atomic<float>* useSN3D;
    std::atomic<float>* decoderOrder;
    std::atomic<float>* exportDecoder;
    std::atomic<float>* exportLayout;
    std::atomic<float>* weights;

    juce::ValueTree loudspeakers { "Loudspeakers" };

    AmbisonicDecoder decoder;
    ReferenceCountedDecoder::Ptr decoderConfig { nullptr };

    bool isLayoutReady = false;

    int highestChannelNumber;

    juce::File lastDir;
    std::unique_ptr<juce::PropertiesFile> properties;

    // ========== METHODS
    void prepareLayout();
    juce::Result checkLayout();
    juce::Result verifyLoudspeakers();
    juce::Result calculateTris();
    void convertLoudspeakersToArray();
    void wrapSphericalCoordinates();

    float getKappa (float gIm, float gRe1, float gRe2, int N);
    juce::dsp::Matrix<float> getInverse (juce::dsp::Matrix<float> A);

    juce::ValueTree createLoudspeakerFromCartesian (juce::Vector3D<float> cartesianCoordinates,
                                                    int channel,
                                                    bool isImaginary = false,
                                                    float gain = 1.0f);
    juce::ValueTree createLoudspeakerFromSpherical (juce::Vector3D<float> sphericalCoordinates,
                                                    int channel,
                                                    bool isImaginary = false,
                                                    float gain = 1.0f);
    juce::Vector3D<float> cartesianToSpherical (juce::Vector3D<float> cartvect);
    juce::Vector3D<float> sphericalToCartesian (juce::Vector3D<float> sphervect);
    juce::Vector3D<float> sphericalInRadiansToCartesian (juce::Vector3D<float> sphervect);

    NoiseBurst noiseBurst;
    AmbisonicNoiseBurst ambisonicNoiseBurst;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AllRADecoderAudioProcessor)
};
