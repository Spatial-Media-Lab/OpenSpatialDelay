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

const float inPhase0 = 1.0f;

const float inPhase1[4] = {
    1.0f,
    3.3333333333333331e-01f,
    3.3333333333333331e-01f,
    3.3333333333333331e-01f,
};

const float inPhase2[9] = { 1.0f,
                            5.0000000000000000e-01f,
                            5.0000000000000000e-01f,
                            5.0000000000000000e-01f,
                            1.0000000000000001e-01f,
                            1.0000000000000001e-01f,
                            1.0000000000000001e-01f,
                            1.0000000000000001e-01f,
                            1.0000000000000001e-01f };

const float inPhase3[16] = { 1.0f,
                             5.9999999999999998e-01f,
                             5.9999999999999998e-01f,
                             5.9999999999999998e-01f,
                             2.0000000000000001e-01f,
                             2.0000000000000001e-01f,
                             2.0000000000000001e-01f,
                             2.0000000000000001e-01f,
                             2.0000000000000001e-01f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f,
                             2.8571428571428571e-02f };

const float inPhase4[25] = { 1.0f,
                             6.6666666666666663e-01f,
                             6.6666666666666663e-01f,
                             6.6666666666666663e-01f,
                             2.8571428571428570e-01f,
                             2.8571428571428570e-01f,
                             2.8571428571428570e-01f,
                             2.8571428571428570e-01f,
                             2.8571428571428570e-01f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.1428571428571425e-02f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f,
                             7.9365079365079361e-03f };

const float inPhase5[36] = { 1.0f,
                             7.1428571428571430e-01f,
                             7.1428571428571430e-01f,
                             7.1428571428571430e-01f,
                             3.5714285714285715e-01f,
                             3.5714285714285715e-01f,
                             3.5714285714285715e-01f,
                             3.5714285714285715e-01f,
                             3.5714285714285715e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             1.1904761904761904e-01f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.3809523809523808e-02f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f,
                             2.1645021645021645e-03f };

const float inPhase6[49] = { 1.0f,
                             7.5000000000000000e-01f,
                             7.5000000000000000e-01f,
                             7.5000000000000000e-01f,
                             4.1666666666666669e-01f,
                             4.1666666666666669e-01f,
                             4.1666666666666669e-01f,
                             4.1666666666666669e-01f,
                             4.1666666666666669e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             1.6666666666666666e-01f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             4.5454545454545456e-02f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             7.5757575757575760e-03f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f,
                             5.8275058275058275e-04f };

const float inPhase7[64] = { 1.0f,
                             7.7777777777777779e-01f,
                             7.7777777777777779e-01f,
                             7.7777777777777779e-01f,
                             4.6666666666666667e-01f,
                             4.6666666666666667e-01f,
                             4.6666666666666667e-01f,
                             4.6666666666666667e-01f,
                             4.6666666666666667e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             2.1212121212121213e-01f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             7.0707070707070704e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             1.6317016317016316e-02f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             2.3310023310023310e-03f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f,
                             1.5540015540015540e-04f };

// as inPhase attenuates higher orders, encoding and sampling at same directions won't result the same amplitude
// these are the correction factors for that problem
// calculated with the Matlab RUMS toolbox: n = (N + 1)^2; correction = n / sum (inPhase (N, true));
const float inPhaseCorrection[8] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };

// energy correction
// calculated with the Matlab RUMS toolbox: n = (N + 1)^2; correction = sqrt (sqrt ((N+1) / sum (inPhase(N))));
const float inPhaseCorrectionEnergy[8] = { 1.0f,         1.10668192f, 1.17017366f,  1.21614964f,
                                           1.252492535f, 1.28269475f, 1.308620875f, 1.331388035f };

inline void multiplyInPhase (const int N, float* data)
{
    switch (N)
    {
        case 0:
            break;
        case 1:
            juce::FloatVectorOperations::multiply (data, inPhase1, 4);
            break;
        case 2:
            juce::FloatVectorOperations::multiply (data, inPhase2, 9);
            break;
        case 3:
            juce::FloatVectorOperations::multiply (data, inPhase3, 16);
            break;
        case 4:
            juce::FloatVectorOperations::multiply (data, inPhase4, 25);
            break;
        case 5:
            juce::FloatVectorOperations::multiply (data, inPhase5, 36);
            break;
        case 6:
            juce::FloatVectorOperations::multiply (data, inPhase6, 47);
            break;
        case 7:
            juce::FloatVectorOperations::multiply (data, inPhase7, 64);
            break;
    }
}

inline void copyInPhase (const int N, float* data)
{
    switch (N)
    {
        case 0:
            *data = 1.0f;
            break;
        case 1:
            juce::FloatVectorOperations::copy (data, inPhase1, 4);
            break;
        case 2:
            juce::FloatVectorOperations::copy (data, inPhase2, 9);
            break;
        case 3:
            juce::FloatVectorOperations::copy (data, inPhase3, 16);
            break;
        case 4:
            juce::FloatVectorOperations::copy (data, inPhase4, 25);
            break;
        case 5:
            juce::FloatVectorOperations::copy (data, inPhase5, 36);
            break;
        case 6:
            juce::FloatVectorOperations::copy (data, inPhase6, 47);
            break;
        case 7:
            juce::FloatVectorOperations::copy (data, inPhase7, 64);
            break;
    }
}

inline const float* getInPhaseLUT (const int N)
{
    switch (N)
    {
        case 1:
            return &inPhase1[0];
        case 2:
            return &inPhase2[0];
        case 3:
            return &inPhase3[0];
        case 4:
            return &inPhase4[0];
        case 5:
            return &inPhase5[0];
        case 6:
            return &inPhase6[0];
        case 7:
            return &inPhase7[0];
        default:
            return &inPhase0;
    }
}
