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
#include "../JuceLibraryCode/JuceHeader.h"
#include "ReferenceCountedDecoder.h"
#include "ReferenceCountedMatrix.h"

class MatrixMultiplication
{
public:
    MatrixMultiplication() {}

    void prepare (const juce::dsp::ProcessSpec& newSpec, bool prepareInputBuffering = true)
    {
        spec = newSpec;

        if (prepareInputBuffering)
        {
            buffer.setSize (buffer.getNumChannels(), spec.maximumBlockSize);
            bufferPrepared = true;
        }
        else
        {
            buffer.setSize (0, 0);
            bufferPrepared = false;
        }

        checkIfNewMatrixAvailable();
    }

    void processReplacing (juce::dsp::AudioBlock<float> data)
    {
        checkIfNewMatrixAvailable();

        // you have to call the prepare method with prepareInputBuffering set to true in order tu use the processReplacing method
        jassert (bufferPrepared);

        ReferenceCountedMatrix::Ptr retainedCurrentMatrix (currentMatrix);
        if (retainedCurrentMatrix == nullptr || ! bufferPrepared)
        {
            data.clear();
            return;
        }

        auto& T = retainedCurrentMatrix->getMatrix();

        const int nInputChannels = juce::jmin (static_cast<int> (data.getNumChannels()),
                                               static_cast<int> (T.getNumColumns()));
        const int nSamples = static_cast<int> (data.getNumSamples());

        // copy input data to buffer
        for (int ch = 0; ch < nInputChannels; ++ch)
            buffer.copyFrom (ch, 0, data.getChannelPointer (ch), nSamples);

        juce::dsp::AudioBlock<float> ab (buffer.getArrayOfWritePointers(),
                                         nInputChannels,
                                         0,
                                         nSamples);
        processNonReplacing (ab, data, false);
    }

    void processNonReplacing (const juce::dsp::AudioBlock<float> inputBlock,
                              juce::dsp::AudioBlock<float> outputBlock,
                              const bool checkNewMatrix = true)
    {
        // you should call the processReplacing instead, it will buffer the input data
        // this is a weak check, as e.g. if number channels differ, it won't trigger
        jassert (inputBlock != outputBlock);

        juce::ScopedNoDenormals noDenormals;

        if (checkNewMatrix)
            checkIfNewMatrixAvailable();

        ReferenceCountedMatrix::Ptr retainedCurrentMatrix (currentMatrix);
        if (retainedCurrentMatrix == nullptr)
        {
            outputBlock.clear();
            return;
        }

        auto& T = retainedCurrentMatrix->getMatrix();

        const int nInputChannels = juce::jmin (static_cast<int> (inputBlock.getNumChannels()),
                                               static_cast<int> (T.getNumColumns()));
        const int nSamples = static_cast<int> (inputBlock.getNumSamples());

        for (int row = 0; row < T.getNumRows(); ++row)
        {
            const int destCh = retainedCurrentMatrix->getRoutingArrayReference().getUnchecked (row);
            if (destCh < outputBlock.getNumChannels())
            {
                float* dest = outputBlock.getChannelPointer (destCh);
                juce::FloatVectorOperations::multiply (dest,
                                                       inputBlock.getChannelPointer (0),
                                                       T (row, 0),
                                                       nSamples); // first channel
                for (int i = 1; i < nInputChannels; ++i) // remaining channels
                    juce::FloatVectorOperations::addWithMultiply (dest,
                                                                  inputBlock.getChannelPointer (i),
                                                                  T (row, i),
                                                                  nSamples);
            }
        }

        juce::Array<int> routingCopy (retainedCurrentMatrix->getRoutingArrayReference());
        routingCopy.sort();
        int lastDest = -1;
        const int nElements = routingCopy.size();
        for (int i = 0; i < nElements; ++i)
        {
            const int destCh = routingCopy[i];
            for (; ++lastDest < destCh;)
                if (lastDest < outputBlock.getNumChannels())
                    juce::FloatVectorOperations::clear (outputBlock.getChannelPointer (lastDest),
                                                        (int) outputBlock.getNumSamples());
            lastDest = destCh;
        }

        for (int ch = routingCopy.getLast() + 1; ch < outputBlock.getNumChannels(); ++ch)
            juce::FloatVectorOperations::clear (outputBlock.getChannelPointer (ch),
                                                (int) outputBlock.getNumSamples());
    }

    const bool checkIfNewMatrixAvailable()
    {
        if (newMatrixAvailable)
        {
            newMatrixAvailable = false;
            currentMatrix = newMatrix;
            newMatrix = nullptr;

            if (currentMatrix != nullptr)
            {
                DBG ("MatrixTransformer: New matrix with name '" << currentMatrix->getName()
                                                                 << "' set.");
                const int cols = (int) currentMatrix->getMatrix().getNumColumns();
                buffer.setSize (cols, buffer.getNumSamples());
                DBG ("MatrixTransformer: buffer resized to " << buffer.getNumChannels() << "x"
                                                             << buffer.getNumSamples());
            }

            return true;
        }
        return false;
    };

    void setMatrix (ReferenceCountedMatrix::Ptr newMatrixToUse, bool force = false)
    {
        newMatrix = newMatrixToUse;
        newMatrixAvailable = true;
        if (force)
            checkIfNewMatrixAvailable();
    }

    ReferenceCountedMatrix::Ptr getMatrix() { return currentMatrix; }

private:
    //==============================================================================
    juce::dsp::ProcessSpec spec = { -1, 0, 0 };
    ReferenceCountedMatrix::Ptr currentMatrix { nullptr };
    ReferenceCountedMatrix::Ptr newMatrix { nullptr };

    juce::AudioBuffer<float> buffer;
    bool bufferPrepared { false };

    bool newMatrixAvailable { false };
};
