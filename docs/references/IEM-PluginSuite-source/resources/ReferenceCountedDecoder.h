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
#include "MaxRE.h"
#include "ReferenceCountedMatrix.h"
#include "ambisonicTools.h"
#include "inPhase.h"

class ReferenceCountedDecoder : public ReferenceCountedMatrix
{
public:
    typedef juce::ReferenceCountedObjectPtr<ReferenceCountedDecoder> Ptr;

    enum class Normalization
    {
        n3d,
        sn3d
    };

    enum class Weights
    {
        none,
        maxrE,
        inPhase
    };

    struct Settings
    {
        Normalization expectedNormalization = Normalization::sn3d;
        Weights weights = Weights::none;
        bool weightsAlreadyApplied = false;
        int subwooferChannel = -1;
    };

    ReferenceCountedDecoder (const juce::String& nameToUse,
                             const juce::String& descriptionToUse,
                             int rows,
                             int columns) :
        ReferenceCountedMatrix (nameToUse, descriptionToUse, rows, columns),
        order (isqrt (columns) - 1)
    {
    }

    ~ReferenceCountedDecoder() override = default;

    virtual juce::String getConstructorMessage() const override
    {
        return "Decoder named '" + name + "' constructed. Size: "
               + juce::String (matrix.getNumRows()) + "x" + juce::String (matrix.getNumColumns());
    }

    virtual juce::String getDeconstructorMessage() const override
    {
        return "Decoder named '" + name + "' destroyed.";
    }

    const juce::String getName() { return name; }

    const juce::String getDescription() { return description; }

    void setSettings (const Settings newSettings) { settings = newSettings; }

    const Settings getSettings() { return settings; }

    const juce::String getWeightsString() const
    {
        switch (settings.weights)
        {
            case Weights::maxrE:
                return "maxrE";
            case Weights::inPhase:
                return "inPhase";
            default:
                return "none";
        }
    }

    /**
     Applies the inverse weights to the decoder matrix, so it can be used with different orders. This method has to be called before the decoder processes audio input.
    */
    void removeAppliedWeights()
    {
        if (settings.weightsAlreadyApplied && settings.weights != Weights::none)
        {
            const auto nCols = static_cast<int> (matrix.getNumColumns());
            const auto nRows = static_cast<int> (matrix.getNumRows());
            if (settings.weights == Weights::maxrE)
                for (int i = 0; i < nCols; ++i)
                    for (int j = 0; j < nRows; ++j)
                        matrix (j, i) /= getMaxRELUT (order)[i];
            else if (settings.weights == Weights::inPhase)
                for (int i = 0; i < nCols; ++i)
                    for (int j = 0; j < nRows; ++j)
                        matrix (j, i) /= getInPhaseLUT (order)[i];
            settings.weightsAlreadyApplied = false;
        }
    }

    const int getOrder() { return order; }

private:
    Settings settings;
    const int order;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedDecoder)
};
