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

/* Hard coded convolution of IIR coefficients.
 Use double! */

template <typename type>
class FilterVisualizerHelper
{
public:
    static juce::Array<type> cascadeSecondOrderCoefficients (juce::Array<type>& c0,
                                                             juce::Array<type>& c1)
    {
        juce::Array<type> c12;
        c12.resize (9);
        const int o = 2;

        c12.setUnchecked (0, c0[0] * c1[0]);
        c12.setUnchecked (1, c0[0] * c1[1] + c0[1] * c1[0]);
        c12.setUnchecked (2, c0[0] * c1[2] + c0[1] * c1[1] + c0[2] * c1[0]);
        c12.setUnchecked (3, c0[1] * c1[2] + c0[2] * c1[1]);
        c12.setUnchecked (4, c0[2] * c1[2]);

        c12.setUnchecked (5, c1[1 + o] + c0[1 + o]);
        c12.setUnchecked (6, c1[2 + o] + c0[1 + o] * c1[1 + o] + c0[2 + o]);
        c12.setUnchecked (7, c0[1 + o] * c1[2 + o] + c0[2 + o] * c1[1 + o]);
        c12.setUnchecked (8, c0[2 + o] * c1[2 + o]);

        return c12;
    }

    static juce::Array<type> cascadeFirstAndSecondOrderCoefficients (juce::Array<type>& firstOrder,
                                                                     juce::Array<type>& secondOrder)
    {
        juce::Array<type>& c1 = firstOrder;
        juce::Array<type>& c2 = secondOrder;

        juce::Array<type> c12;
        c12.resize (7);

        //b
        c12.setUnchecked (0, c1[0] * c2[0]);
        c12.setUnchecked (1, c1[0] * c2[1] + c1[1] * c2[0]);
        c12.setUnchecked (2, c1[0] * c2[2] + c1[1] * c2[1]);
        c12.setUnchecked (3, c1[1] * c2[2]);

        //a
        c12.setUnchecked (4, c1[2] + c2[3]);
        c12.setUnchecked (5, c1[2] * c2[3] + c2[4]);
        c12.setUnchecked (6, c1[2] * c2[4]);

        return c12;
    }
};
