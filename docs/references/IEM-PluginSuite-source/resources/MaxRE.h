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

const float maxre0 = 1.0f;

const float maxre1[4] = {
    1.0f,
    5.7754104119288496e-01f,
    5.7754104119288496e-01f,
    5.7754104119288496e-01f,
};

const float maxre2[9] = { 1.0f,
                          7.7520766107019334e-01f,
                          7.7520766107019334e-01f,
                          7.7520766107019334e-01f,
                          4.0142037667287966e-01f,
                          4.0142037667287966e-01f,
                          4.0142037667287966e-01f,
                          4.0142037667287966e-01f,
                          4.0142037667287966e-01f };

const float maxre3[16] = { 1.0f,
                           8.6155075887658639e-01f,
                           8.6155075887658639e-01f,
                           8.6155075887658639e-01f,
                           6.1340456518123299e-01f,
                           6.1340456518123299e-01f,
                           6.1340456518123299e-01f,
                           6.1340456518123299e-01f,
                           6.1340456518123299e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f,
                           3.0643144179936538e-01f };

const float maxre4[25] = { 1.0f,
                           9.0644136637224459e-01f,
                           9.0644136637224459e-01f,
                           9.0644136637224459e-01f,
                           7.3245392600617265e-01f,
                           7.3245392600617265e-01f,
                           7.3245392600617265e-01f,
                           7.3245392600617265e-01f,
                           7.3245392600617265e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           5.0224998490808703e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f,
                           2.4736484001129033e-01f };

const float maxre5[36] = { 1.0f,
                           9.3263709143129281e-01f,
                           9.3263709143129281e-01f,
                           9.3263709143129281e-01f,
                           8.0471791647013236e-01f,
                           8.0471791647013236e-01f,
                           8.0471791647013236e-01f,
                           8.0471791647013236e-01f,
                           8.0471791647013236e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           6.2909156744472861e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           4.2321128963220900e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f,
                           2.0719132924646289e-01f };

const float maxre6[49] = { 1.0f,
                           9.4921830632793713e-01f,
                           9.4921830632793713e-01f,
                           9.4921830632793713e-01f,
                           8.5152308960211620e-01f,
                           8.5152308960211620e-01f,
                           8.5152308960211620e-01f,
                           8.5152308960211620e-01f,
                           8.5152308960211620e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           7.1432330396679700e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           5.4794300713180655e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           3.6475291657556469e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f,
                           1.7813609450688817e-01f };

const float maxre7[64] = { 1.0f,
                           9.6036452263662697e-01f,
                           9.6036452263662697e-01f,
                           9.6036452263662697e-01f,
                           8.8345002450861454e-01f,
                           8.8345002450861454e-01f,
                           8.8345002450861454e-01f,
                           8.8345002450861454e-01f,
                           8.8345002450861454e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           7.7381375334313540e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           6.3791321433685355e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           4.8368159255186721e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           3.2000849790781744e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f,
                           1.5616185043093761e-01f };

// as max re attenuates higher orders, encoding and sampling at same directions won't result the same amplitude
// these are the correction factors for that problem
// calculated with the Matlab RUMS toolbox: n = (N + 1)^2; correction = n / sum (maxrE (N, true));
const float maxRECorrection[8] = { 1.0f,
                                   1.463794976147894f,
                                   1.687692544652202f,
                                   1.818864885628318f,
                                   1.904961155192695f,
                                   1.965800739863925f,
                                   2.011075537215868f,
                                   2.046081944498225f };

// energy correction
// calculated with the Matlab RUMS toolbox: n = (N + 1)^2; correction = sqrt (sqrt ((N+1) / sum (maxrE(N))));
const float maxRECorrectionEnergy[8] = { 1.0f,         1.061114703f, 1.083513331f, 1.095089593f,
                                         1.102148983f, 1.106899961f, 1.110314372f, 1.112886124f };

inline void multiplyMaxRE (const int N, float* data)
{
    switch (N)
    {
        case 0:
            break;
        case 1:
            juce::FloatVectorOperations::multiply (data, maxre1, 4);
            break;
        case 2:
            juce::FloatVectorOperations::multiply (data, maxre2, 9);
            break;
        case 3:
            juce::FloatVectorOperations::multiply (data, maxre3, 16);
            break;
        case 4:
            juce::FloatVectorOperations::multiply (data, maxre4, 25);
            break;
        case 5:
            juce::FloatVectorOperations::multiply (data, maxre5, 36);
            break;
        case 6:
            juce::FloatVectorOperations::multiply (data, maxre6, 47);
            break;
        case 7:
            juce::FloatVectorOperations::multiply (data, maxre7, 64);
            break;
    }
}

inline void copyMaxRE (const int N, float* data)
{
    switch (N)
    {
        case 0:
            *data = 1.0f;
            break;
        case 1:
            juce::FloatVectorOperations::copy (data, maxre1, 4);
            break;
        case 2:
            juce::FloatVectorOperations::copy (data, maxre2, 9);
            break;
        case 3:
            juce::FloatVectorOperations::copy (data, maxre3, 16);
            break;
        case 4:
            juce::FloatVectorOperations::copy (data, maxre4, 25);
            break;
        case 5:
            juce::FloatVectorOperations::copy (data, maxre5, 36);
            break;
        case 6:
            juce::FloatVectorOperations::copy (data, maxre6, 47);
            break;
        case 7:
            juce::FloatVectorOperations::copy (data, maxre7, 64);
            break;
    }
}

inline const float* getMaxRELUT (const int N)
{
    switch (N)
    {
        case 1:
            return &maxre1[0];
        case 2:
            return &maxre2[0];
        case 3:
            return &maxre3[0];
        case 4:
            return &maxre4[0];
        case 5:
            return &maxre5[0];
        case 6:
            return &maxre6[0];
        case 7:
            return &maxre7[0];
        default:
            return &maxre0;
    }
}
