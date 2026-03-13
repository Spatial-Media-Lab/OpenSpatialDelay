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
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

// A simple queue of arbitrary sample type (SampleType) with fixed numbers of samples (BufferSize).
// A good thing to transfer data between processor and editoras it should be lock-free.
// IMPORTANT INFORMATION: If this queue is full, new data WON'T be inserted!
// The two methods return the number of actually written or read samples.

template <typename SampleType, int BufferSize>
class Queue
{
public:
    Queue() : abstractFifo (BufferSize) {}

    int addToQueue (const SampleType* samples, size_t numSamples)
    {
        jassert (numSamples <= BufferSize); // don't push more samples than the buffer holds

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (numSamples, start1, size1, start2, size2);

        if (size1 > 0)
            for (int i = 0; i < size1; ++i)
                buffer[start1 + i] = samples[i];
        if (size2 > 0)
            for (int i = 0; i < size2; ++i)
                buffer[i] = samples[size1 + i];
        abstractFifo.finishedWrite (size1 + size2);

        return size1 + size2;
    }

    int readFromQueue (SampleType* outputBuffer, int numItems)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (numItems, start1, size1, start2, size2);

        if (size1 > 0)
            for (int i = 0; i < size1; ++i)
                outputBuffer[i] = buffer[start1 + i];
        if (size2 > 0)
            for (int i = 0; i < size2; ++i)
                outputBuffer[start1 + i] = buffer[i];

        abstractFifo.finishedRead (size1 + size2);

        return size1 + size2;
    }

private:
    AbstractFifo abstractFifo;
    std::array<SampleType, BufferSize> buffer;
};
