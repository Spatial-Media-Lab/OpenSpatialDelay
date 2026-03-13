//================================================
// @title        mx_sequency_perm.cpp
// @author       Jonathan Hadida
// @contact      Jonathan.hadida [at] dtc.ox.ac.uk
//================================================

#include "fwht.h"
#include "mexArray.h"
#include <algorithm>

/********************     **********     ********************/
/********************     **********     ********************/

/**
 * Returns the sequency permutation (bitrev of Gray code) of order n.
 * Use the script start_matlab.sh to start Matlab, and compile with:
 * mex CXXFLAGS="\$CXXFLAGS -std=c++0x -Wall -O2" mx_sequency_perm.cpp
 */
void mexFunction (int nargout, mxArray* out[], int nargin, const mxArray* in[])
{
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: p = mx_sequency_perm(order);");
        return;
    }

    ndArray<unsigned, 2> output;

    unsigned n = static_cast<unsigned> (mxGetScalar (in[0]));
    std::vector<unsigned> p;
    fwht_sequency_permutation (p, n);

    int size[2];
    size[0] = 1;
    size[1] = p.size();
    out[0] = mxCreateNumericArray (2, size, mx_type<unsigned>::id, mxREAL);
    output.assign (out[0]);

    std::copy (p.begin(), p.end(), output.begin());
}