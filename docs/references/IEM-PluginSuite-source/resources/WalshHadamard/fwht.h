#ifndef __FWHT__
#define __FWHT__

//================================================
// @title        fwht.h
// @author       Jonathan Hadida
// @contact      Jonathan.hadida [at] dtc.ox.ac.uk
//================================================

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

#define for_i(n) for (unsigned i = 0; i < n; ++i)

/********************     **********     ********************/
/********************     **********     ********************/

/**
 * Count the number of bits set.
 */
template <typename IntType>
unsigned bit_count (IntType n)
{
    unsigned c;
    for (c = 0; n; ++c)
        n &= n - 1;
    return c;
}

// ------------------------------------------------------------------------

/**
 * Reverse the bits (left-right) of an integer.
 */
template <typename IntType>
IntType bit_reverse (IntType n, unsigned nbits)
{
    IntType r = n;
    IntType l, m;

    // Compute shift and bitmask
    l = (IntType) (nbits - 1);
    m = (1 << nbits) - 1;

    // Permute
    while (n >>= 1)
    {
        r <<= 1;
        r |= n & 1;
        --l;
    }

    // Final shift and masking
    r <<= l;
    return r & m;
}

template <typename IntType>
inline IntType bit_reverse (IntType n)
{
    return bit_reverse (n, 8 * sizeof (n));
}

// ------------------------------------------------------------------------

/**
 * Print the bits of an integer.
 */
template <typename IntType>
void bit_print (IntType n)
{
    static const unsigned b = 8 * sizeof (IntType);
    static char buf[b + 1];
    buf[b] = '\0';

    std::cout << n;
    for (unsigned m = b; m; n >>= 1, --m)
        buf[m - 1] = (n & 1) + '0';
    printf (" = %s\n", buf);
}

// ------------------------------------------------------------------------

/**
 * Gray codes transforms.
 */
template <typename IntType>
inline IntType bin2gray (IntType n)
{
    return (n >> 1) ^ n;
}

template <typename IntType>
IntType gray2bin (IntType n)
{
    for (IntType m = n; m >>= 1; n = n ^ m)
        ;
    return n;
}

// ------------------------------------------------------------------------

/**
 * Integer logarithm base 2.
 */
template <typename IntType>
unsigned ilog2 (IntType n)
{
    unsigned l;
    for (l = 0; n; n >>= 1, ++l)
        ;
    return l;
}

template <typename IntType>
inline bool is_pow2 (IntType n)
{
    return (n > 1) && ((n & (n - 1)) == 0);
}

// ------------------------------------------------------------------------

/**
 * Orthogonal transformation.
 */
template <typename T>
void rotate (T& a, T& b)
{
    T A;
    A = a;
    a = A + b;
    b = A - b;
}

// ------------------------------------------------------------------------

/**
 * Compute the permutation of WH coefficients to sort them by sequency.
 */
template <typename IntType>
void fwht_sequency_permutation (std::vector<IntType>& perm, unsigned order)
{
    if (perm.size() == (size_t) (1 << order))
        return;

    perm.resize (1 << order);
    IntType k = 0;
    for (auto& p : perm)
        p = bit_reverse (bin2gray (k++), order);
}

// ------------------------------------------------------------------------

/**
 * Fast Walsh-Hadamard transform.
 */
template <typename T>
void fwht (T* data, unsigned n, bool sequency_ordered = false, bool normalize = true)
{
    // Require n to be a power of 2
    unsigned l2 = ilog2 (n) - 1;
    if (n != (unsigned) (1 << l2))
        throw std::length_error ("Data length should be a power of 2.");

    // Normalizer
    const T norm = sqrt (T (n));

    // Compute the WHT
    for (unsigned i = 0; i < l2; ++i)
    {
        for (unsigned j = 0; j < n; j += (1 << (i + 1)))
            for (unsigned k = 0; k < (unsigned) (1 << i); ++k)
                rotate (data[j + k], data[j + k + (unsigned) (1 << i)]);
    }

    static std::vector<unsigned> perm;
    if (sequency_ordered)
    {
        // Compute permutation
        fwht_sequency_permutation (perm, l2);

        // Copy transform
        std::vector<T> tmp (data, data + n);
        for_i (n) data[i] = tmp[perm[i]] / norm;
    }
    else if (normalize)
        for_i (n) data[i] /= norm;
}

// ------------------------------------------------------------------------

template <typename T>
void ifwht (T* data, unsigned n, bool sequency_ordered = false)
{
    fwht (data, n, sequency_ordered, false);
}

// ------------------------------------------------------------------------

template <typename T>
void fwht (std::vector<T>& data, bool sequency_ordered = false)
{
    // Round to the next power of 2
    unsigned n = data.size();
    unsigned l2 = ilog2 (n) - is_pow2 (n);
    unsigned N = 1 << l2;

    // 0-padding to the next power of 2
    data.resize (N, T (0));
    fwht (data.data(), N, sequency_ordered);
}

// ------------------------------------------------------------------------

template <typename T>
void ifwht (std::vector<T>& data, bool sequency_ordered = false)
{
    // Round to the next power of 2
    unsigned n = data.size();
    unsigned l2 = ilog2 (n) - is_pow2 (n);
    unsigned N = 1 << l2;

    // 0-padding to the next power of 2
    data.resize (N, T (0));
    ifwht (data.data(), N, sequency_ordered);
}

#endif
