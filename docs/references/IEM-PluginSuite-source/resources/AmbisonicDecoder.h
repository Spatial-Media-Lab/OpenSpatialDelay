/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MatrixMultiplication.h"
#include "MaxRE.h"
#include "ReferenceCountedDecoder.h"
#include "ambisonicTools.h"
#include "inPhase.h"

class AmbisonicDecoder
{
public:
    AmbisonicDecoder() {}

    void prepare (const juce::dsp::ProcessSpec& newSpec)
    {
        spec = newSpec;
        matMult.prepare (newSpec, false); // we let do this class do the buffering

        buffer.setSize (buffer.getNumChannels(), spec.maximumBlockSize);
        buffer.clear();

        checkIfNewDecoderAvailable();
    }

    void setInputNormalization (ReferenceCountedDecoder::Normalization newNormalization)
    {
        inputNormalization = newNormalization;
    }

    /**
     Decodes the Ambisonic input signals to loudspeaker signals using the current decoder.
     This method takes care of buffering the input data, so inputBlock and outputBlock are
     allowed to be the same or overlap.
    */
    void process (juce::dsp::AudioBlock<float> inputBlock, juce::dsp::AudioBlock<float> outputBlock)
    {
        checkIfNewDecoderAvailable();

        ReferenceCountedDecoder::Ptr retainedDecoder = currentDecoder;

        auto& T = retainedDecoder->getMatrix();

        const int nInputChannels = juce::jmin (static_cast<int> (inputBlock.getNumChannels()),
                                               static_cast<int> (T.getNumColumns()));
        const int nSamples = static_cast<int> (inputBlock.getNumSamples());

        // copy input data to buffer
        for (int ch = 0; ch < nInputChannels; ++ch)
            buffer.copyFrom (ch, 0, inputBlock.getChannelPointer (ch), nSamples);

        juce::dsp::AudioBlock<float> ab (buffer.getArrayOfWritePointers(),
                                         nInputChannels,
                                         0,
                                         nSamples);
        processInternal (ab, outputBlock);
    }

    const bool checkIfNewDecoderAvailable()
    {
        if (newDecoderAvailable)
        {
            newDecoderAvailable = false;
            currentDecoder = newDecoder;
            newDecoder = nullptr;

            if (currentDecoder != nullptr)
            {
                currentDecoder->removeAppliedWeights();
                const int cols = (int) currentDecoder->getMatrix().getNumColumns();
                buffer.setSize (cols, buffer.getNumSamples());
            }

            matMult.setMatrix (currentDecoder, true);
            return true;
        }
        return false;
    };

    /** Giving the AmbisonicDecoder a new decoder for the audio processing. Note: The AmbisonicDecoder will call the removeAppliedWeights() of the ReferenceCountedDecoder before it processes audio! The matrix elements may change due to this method.
     */
    void setDecoder (ReferenceCountedDecoder::Ptr newDecoderToUse)
    {
        newDecoder = newDecoderToUse;
        newDecoderAvailable = true;
    }

    ReferenceCountedDecoder::Ptr getCurrentDecoder() { return currentDecoder; }

    /** Checks if a new decoder waiting to be used.
     */
    const bool isNewDecoderWaiting() { return newDecoderAvailable; }

private:
    /**
     Decodes the Ambisonic input signals to loudspeaker signals using the current decoder. Keep in mind that the input data will be changed!
     */
    void processInternal (juce::dsp::AudioBlock<float> inputBlock,
                          juce::dsp::AudioBlock<float> outputBlock)
    {
        // you should call the processReplacing instead, it will buffer the input data
        // this is a weak check, as e.g. if number channels differ, it won't trigger
        jassert (inputBlock != outputBlock);

        juce::ScopedNoDenormals noDenormals;

        ReferenceCountedDecoder::Ptr retainedDecoder = currentDecoder;

        if (retainedDecoder != nullptr) // if decoder is available, do the pre-processing
        {
            const int order = isqrt (static_cast<int> (inputBlock.getNumChannels())) - 1;
            const int chAmbi = juce::square (order + 1);
            const int numSamples = static_cast<int> (inputBlock.getNumSamples());

            float weights[64];
            const float correction =
                std::sqrt (std::sqrt ((static_cast<float> (retainedDecoder->getOrder()) + 1)
                                      / (static_cast<float> (order) + 1)));
            juce::FloatVectorOperations::fill (weights, correction, chAmbi);

            if (retainedDecoder->getSettings().weights == ReferenceCountedDecoder::Weights::maxrE)
            {
                multiplyMaxRE (order, weights);
                juce::FloatVectorOperations::multiply (weights,
                                                       maxRECorrectionEnergy[order],
                                                       chAmbi);
            }
            else if (retainedDecoder->getSettings().weights
                     == ReferenceCountedDecoder::Weights::inPhase)
            {
                multiplyInPhase (order, weights);
                juce::FloatVectorOperations::multiply (weights,
                                                       inPhaseCorrectionEnergy[order],
                                                       chAmbi);
            }

            if (retainedDecoder->getSettings().expectedNormalization != inputNormalization)
            {
                const float* conversionPtr (
                    inputNormalization == ReferenceCountedDecoder::Normalization::sn3d ? sn3d2n3d
                                                                                       : n3d2sn3d);
                juce::FloatVectorOperations::multiply (weights, conversionPtr, chAmbi);
            }

            for (int ch = 0; ch < chAmbi; ++ch)
                juce::FloatVectorOperations::multiply (inputBlock.getChannelPointer (ch),
                                                       weights[ch],
                                                       numSamples);
        }

        matMult.processNonReplacing (inputBlock, outputBlock);
    }

private:
    //==============================================================================
    juce::dsp::ProcessSpec spec = { -1, 0, 0 };
    ReferenceCountedDecoder::Ptr currentDecoder { nullptr };
    ReferenceCountedDecoder::Ptr newDecoder { nullptr };
    bool newDecoderAvailable { false };

    juce::AudioBuffer<float> buffer;

    ReferenceCountedDecoder::Normalization inputNormalization {
        ReferenceCountedDecoder::Normalization::sn3d
    };

    MatrixMultiplication matMult;
};
