#pragma once
#include <JuceHeader.h>
#include <map>
#include <memory>

//==============================================================================
// Process-global FFT cache — prevents vDSP twiddle table use-after-free
// when multiple plugin instances create/destroy FFT setups concurrently.
// vDSP shares internal twiddle factor memory across setups of the same order;
// destroying the last setup frees the shared table even if another thread's
// vDSP_fft_zrip is reading from it.  The cache creates once, never destroys.
// (issue #131)
//==============================================================================
struct SharedFFTCache
{
    juce::SpinLock lock;
    std::map<int, std::shared_ptr<juce::dsp::FFT>> cache;
    std::shared_ptr<juce::dsp::FFT> getOrCreate (int fftOrder);
};

SharedFFTCache& getSharedFFTCache();
