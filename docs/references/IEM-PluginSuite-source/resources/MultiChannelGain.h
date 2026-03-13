/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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

/*
 This processor is based on JUCE's juce::dsp::Gain processor.
 */

template <typename FloatType>
class MultiChannelGain
{
public:
    MultiChannelGain() {}

    //==============================================================================
    /** Applies a new gain as a linear value. */
    void setGainLinear (const int channel, FloatType newGain) noexcept
    {
        if (channel < gains.size())
            gains.getUnchecked (channel)->setTargetValue (newGain);
    }

    /** Applies a new gain as a decibel value. */
    void setGainDecibels (const int channel, FloatType newGainDecibels)
    {
        setGainLinear (channel, juce::Decibels::decibelsToGain<FloatType> (newGainDecibels));
    }

    /** Sets the length of the ramp used for smoothing gain changes. */
    void setRampDurationSeconds (double newDurationSeconds) noexcept
    {
        if (rampDurationSeconds != newDurationSeconds)
        {
            rampDurationSeconds = newDurationSeconds;
            reset();
        }
    }

    /** Returns the ramp duration in seconds. */
    double getRampDurationSeconds() const noexcept { return rampDurationSeconds; }

    /** Returns true if the current value is currently being interpolated. */
    bool isSmoothing (const int channel)
    {
        jassert (channel < gains.size());
        return gains.getUnchecked (channel).isSmoothing();
    }

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        gains.clear();
        for (int ch = 0; ch < spec.numChannels; ++ch)
            gains.add (new juce::LinearSmoothedValue<float>());

        reset();
    }

    /** Resets the internal state of the gain */
    void reset() noexcept
    {
        if (sampleRate > 0)
            for (int i = 0; i < gains.size(); ++i)
                gains.getUnchecked (i)->reset (sampleRate, rampDurationSeconds);
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& inBlock = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert (inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert (inBlock.getNumChannels() <= gains.size());
        jassert (inBlock.getNumSamples() == outBlock.getNumSamples());

        auto len = inBlock.getNumSamples();
        auto numChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            for (int i = 0; i < gains.size(); ++i)
                gains.getUnchecked (i)->skip (static_cast<int> (len));

            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom (inBlock);

            return;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* src = inBlock.getChannelPointer (ch);
            auto* dst = outBlock.getChannelPointer (ch);
            for (size_t i = 0; i < len; ++i)
                dst[i] = src[i] * gains.getUnchecked (ch)->getNextValue();
        }

        for (int ch = (int) numChannels; ch < gains.size(); ++ch)
        {
            gains.getUnchecked (ch)->skip (static_cast<int> (len));
        }
    }

private:
    juce::OwnedArray<juce::LinearSmoothedValue<FloatType>> gains;
    double sampleRate = 0, rampDurationSeconds = 0;
};
