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

class ReferenceCountedMatrix : public juce::ReferenceCountedObject
{
public:
    typedef juce::ReferenceCountedObjectPtr<ReferenceCountedMatrix> Ptr;

    ReferenceCountedMatrix (const juce::String& nameToUse,
                            const juce::String& descriptionToUse,
                            int rows,
                            int columns) :
        name (nameToUse), description (descriptionToUse), matrix (rows, columns)
    {
        for (int i = 0; i < rows; ++i)
            routingArray.add (i);

        DBG (getConstructorMessage());
    }

    ~ReferenceCountedMatrix() { DBG (getDeconstructorMessage()); }

    virtual juce::String getConstructorMessage() const
    {
        return "Matrix named '" + name + "' constructed. Size: "
               + juce::String (matrix.getNumRows()) + "x" + juce::String (matrix.getNumColumns());
    }

    virtual juce::String getDeconstructorMessage() const
    {
        return "Matrix named '" + name + "' destroyed.";
    }

    juce::dsp::Matrix<float>& getMatrix() { return matrix; }
    juce::String getName() const { return name; }

    juce::String getDescription() const { return description; }

    const int getNumOutputChannels()
    {
        int maxChannel = 0;
        for (int i = routingArray.size(); --i >= 0;)
        {
            const int newValue = routingArray.getUnchecked (i);
            if (newValue > maxChannel)
                maxChannel = newValue;
        }
        return maxChannel + 1;
    }

    const int getNumInputChannels() { return (int) matrix.getNumColumns(); }

    juce::Array<int>& getRoutingArrayReference() { return routingArray; }

protected:
    juce::String name;
    juce::String description;
    juce::dsp::Matrix<float> matrix;
    juce::Array<int> routingArray;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedMatrix)
};
