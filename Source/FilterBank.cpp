#include "FilterBank.h"

//==============================================================================
void FilterBank::prepare (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 1;

    fbLP_.prepare (spec);
    fbHP_.prepare (spec);
    fbLP_.reset();
    fbHP_.reset();

    for (int t = 0; t < kMaxObjects; ++t)
    {
        tapLP_[t].prepare (spec);
        tapHP_[t].prepare (spec);
        tapLP_[t].reset();
        tapHP_[t].reset();
    }

    // Pre-compute transparent (20kHz) air absorption coefficients
    airTransparentCoeffs_ =
        *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f);

    for (int i = 0; i < kMaxObjects; ++i)
    {
        airFilter_[i].prepare (spec);
        airFilter_[i].reset();
        *airFilter_[i].coefficients = airTransparentCoeffs_;
        smoothedAirCutoff_[i] = 20000.0f;
    }
}

//==============================================================================
void FilterBank::updateCoefficients (double sampleRate, float lpFreq, float hpFreq,
                                      float lpQ, float hpQ, bool filterEnabled)
{
    bypassed_ = ! filterEnabled;

    constexpr float alpha = 0.3f;
    smoothedLP_  += alpha * (lpFreq - smoothedLP_);
    smoothedHP_  += alpha * (hpFreq - smoothedHP_);
    smoothedLPQ_ += alpha * (lpQ - smoothedLPQ_);
    smoothedHPQ_ += alpha * (hpQ - smoothedHPQ_);

    bool lpChanged = std::abs (smoothedLP_ - cachedLP_) > 0.01f
                  || std::abs (smoothedLPQ_ - cachedLPQ_) > 0.0001f;
    bool hpChanged = std::abs (smoothedHP_ - cachedHP_) > 0.01f
                  || std::abs (smoothedHPQ_ - cachedHPQ_) > 0.0001f;

    if (lpChanged)
    {
        auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, smoothedLP_, smoothedLPQ_);
        fbLP_.coefficients = lpCoeffs;
        for (int t = 0; t < kMaxObjects; ++t)
            tapLP_[t].coefficients = lpCoeffs;
        cachedLP_ = smoothedLP_;
        cachedLPQ_ = smoothedLPQ_;
    }

    if (hpChanged)
    {
        auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, smoothedHP_, smoothedHPQ_);
        fbHP_.coefficients = hpCoeffs;
        for (int t = 0; t < kMaxObjects; ++t)
            tapHP_[t].coefficients = hpCoeffs;
        cachedHP_ = smoothedHP_;
        cachedHPQ_ = smoothedHPQ_;
    }
}

//==============================================================================
void FilterBank::updateAirAbsorption (int objectIndex, double sampleRate, float dist,
                                       bool airActive, bool positionChanged, bool stateChanged)
{
    airActive_ = airActive;

    if (airActive && (positionChanged || stateChanged))
    {
        float mappedDist = dist * dist;
        constexpr float absorbCoeff = 3.1f;
        float targetCutoff = 20000.0f * std::exp (-absorbCoeff * mappedDist);
        targetCutoff = juce::jlimit (500.0f, 20000.0f, targetCutoff);

        constexpr float airSmoothAlpha = 0.2f;
        smoothedAirCutoff_[objectIndex] += airSmoothAlpha * (targetCutoff - smoothedAirCutoff_[objectIndex]);

        *airFilter_[objectIndex].coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, smoothedAirCutoff_[objectIndex]);
    }
}

//==============================================================================
void FilterBank::clearAirAbsorption (int objectIndex)
{
    *airFilter_[objectIndex].coefficients = airTransparentCoeffs_;
}

//==============================================================================
float FilterBank::processTapSample (int objectIndex, float input)
{
    if (bypassed_)
        return input;
    return tapHP_[objectIndex].processSample (
               tapLP_[objectIndex].processSample (input));
}

//==============================================================================
float FilterBank::processFeedbackSample (float input)
{
    if (bypassed_)
        return input;
    return fbHP_.processSample (fbLP_.processSample (input));
}

//==============================================================================
float FilterBank::processAirSample (int objectIndex, float input)
{
    if (! airActive_)
        return input;
    return airFilter_[objectIndex].processSample (input);
}

//==============================================================================
void FilterBank::resetAll()
{
    fbLP_.reset();
    fbHP_.reset();
    for (int t = 0; t < kMaxObjects; ++t)
    {
        tapLP_[t].reset();
        tapHP_[t].reset();
        airFilter_[t].reset();
    }
}

void FilterBank::resetObject (int objectIndex)
{
    tapLP_[objectIndex].reset();
    tapHP_[objectIndex].reset();
    airFilter_[objectIndex].reset();
}

//==============================================================================
void FilterBank::invalidateCoefficients()
{
    cachedLP_ = -1.0f;
    cachedHP_ = -1.0f;
    cachedLPQ_ = -1.0f;
    cachedHPQ_ = -1.0f;
}

//==============================================================================
void FilterBank::setSmoothedFrequencies (float lp, float hp, float lpQ, float hpQ)
{
    smoothedLP_ = lp;
    smoothedHP_ = hp;
    smoothedLPQ_ = lpQ;
    smoothedHPQ_ = hpQ;
}
