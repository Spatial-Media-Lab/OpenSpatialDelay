//================================================
// @title        mx_fwht.cpp
// @author       Jonathan Hadida
// @contact      Jonathan.hadida [at] dtc.ox.ac.uk
//================================================

#include "fwht.h"
#include "ndArray.h"
#include <algorithm>

#define SEQUENCY_ORDER false

/********************     **********     ********************/
/********************     **********     ********************/

/**
 * Dependency to ndArray (see https://github.com/Sheljohn/ndArray).
 * Use the script start_matlab.sh to start Matlab, and compile with:
 * mex CXXFLAGS="\$CXXFLAGS -std=c++0x -Wall -O2" mx_fwht.cpp
 */
void mexFunction (int nargout, mxArray* out[], int nargin, const mxArray* in[])
{
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: w = mx_fwht(sequence);");
        return;
    }

    ndArray<const double, 2> input (in[0]);
    ndArray<double, 2> output;

    std::vector<double> d (input.begin(), input.end());
    fwht (d, SEQUENCY_ORDER);

    int size[2];
    size[0] = 1;
    size[1] = d.size();
    out[0] = mxCreateNumericArray (2, size, mx_type<double>::id, mxREAL);
    output.assign (out[0]);

    std::copy (d.begin(), d.end(), output.begin());
}