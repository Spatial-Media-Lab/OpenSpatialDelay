#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

//==============================================================================
// Binaural profile: defines virtual head characteristics for simplified HRTF
//==============================================================================
struct BinauralProfile
{
    float headRadius;   // meters
    float ildScale;     // ILD multiplier
    float shadowFreqHz; // head shadow cutoff frequency
    const char* name;
};

//==============================================================================
// Virtual speaker position for the modular spatial audio core
//==============================================================================
struct VirtualSpeaker
{
    float azimuthRad;
    float elevationRad;
};

//==============================================================================
// VBAP speaker triplet: 3 speaker indices + pre-computed 3x3 inverse matrix
// for 3D Vector Base Amplitude Panning (Pulkki 1997)
//==============================================================================
struct VBAPTriplet
{
    int i, j, k;            // speaker indices into virtual speaker array
    float inv[3][3];        // inverse of [spk_i, spk_j, spk_k] direction matrix
};

//==============================================================================
// Speaker layout for multi-channel output (v0.2)
// Defines speaker positions for each supported output format.
// LFE is tracked but excluded from spatialization.
//==============================================================================
struct SpeakerLayout
{
    int numSpeakers;        // number of spatial speakers (excluding LFE)
    int lfeChannelIndex;    // output channel index for LFE, or -1 if no LFE
    int totalChannels;      // total output channels (including LFE)

    struct Speaker
    {
        float azimuthRad;
        float elevationRad;
        int   channelIndex;  // output buffer channel index
    };

    Speaker speakers[16];   // max 16 spatial speakers
};

//==============================================================================
// Per-object spatial state
//==============================================================================
struct ObjectState
{
    float azimuthDeg   = 0.0f;
    float elevationDeg = 0.0f;
    float distance     = 0.5f;
    float delayTimeMs  = 500.0f;
    bool  enabled      = false;
};

//==============================================================================
// Binaural gain result for one source position
//==============================================================================
struct BinauralGains
{
    float leftGain         = 0.0f;
    float rightGain        = 0.0f;
    float leftDelaySamples = 0.0f;  // ITD: additional delay for left ear
    float rightDelaySamples = 0.0f; // ITD: additional delay for right ear
};

//==============================================================================
class OpenSpatialDelayProcessor : public juce::AudioProcessor
{
public:
    static constexpr int MAX_OBJECTS = 12;
    static constexpr int MAX_DELAY_SECONDS = 24;  // 12 objects × 2s max base delay
    static constexpr int PITCH_GRAIN_SIZE = 1024;  // ~23 ms at 44.1 kHz
    static constexpr float MAX_CUMULATIVE_SEMITONES = 36.0f;  // Cap cumulative pitch to avoid grain artifacts

    // Modular 3D Audio Core constants
    static constexpr int NUM_VIRTUAL_SPEAKERS = 16;
    static constexpr int HOA_ORDER = 3;
    static constexpr int HOA_CHANNELS = (HOA_ORDER + 1) * (HOA_ORDER + 1); // = 16

    // v0.2: Multi-channel output format — auto-detected from host bus config
    enum class OutputFormat { Binaural = 0, Quad, Surround5_1, Surround7_1, Surround7_1_4, Surround9_1_6 };
    OutputFormat getActiveOutputFormat() const { return activeOutputFormat; }

    //--------------------------------------------------------------------------
    OpenSpatialDelayProcessor();
    ~OpenSpatialDelayProcessor() override;

    //--------------------------------------------------------------------------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //--------------------------------------------------------------------------
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //--------------------------------------------------------------------------
    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return MAX_DELAY_SECONDS; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //--------------------------------------------------------------------------
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //--------------------------------------------------------------------------
    juce::AudioProcessorValueTreeState apvts;

    // Access for the editor
    ObjectState getObjectState (int objectIndex) const;

    static const std::array<BinauralProfile, 5> binauralProfiles;

    // Modular 3D Audio Core: 9.1.6 virtual speaker layout + zenith (9 ear + 6 top + 1 zenith)
    static const std::array<VirtualSpeaker, NUM_VIRTUAL_SPEAKERS> virtualSpeakers;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //--- DSP helpers ----------------------------------------------------------
    void   writeDelayLine (float sample);
    float  readDelayLine  (float delaySamples) const;
    float  readPitchShifted (float delaySamples, float semitones, int objectIndex);
    BinauralGains computeBinauralGains (float azimuthRad, float elevationRad,
                                        float distance, int profileIndex) const;
    float getTempoSyncedDelayMs (int noteDivisionIndex) const;

    //--- Modular 3D Audio Core (plugin-agnostic, future: extract to SpatialCore) ---
    static const std::vector<VBAPTriplet>& getVBAPTriplets();
    void computeVBAPGains (float azimuthRad, float elevationRad, float* outGains) const;
    void computeAmbiSpeakerGains (float azimuthRad, float elevationRad, float* outGains) const;  // v0.2+ speaker output
    void updateSpeakerBinauralCache (int profileIndex);
    void computeAmbiDecodeMatrix();
    static float evalSH (int acnIndex, float azimuthRad, float elevationRad);

    // Direct Ambisonics-to-Binaural decode (v0.1 — bypasses virtual speakers)
    void computeBinauralSHWeights (int profileIndex);
    BinauralGains computeAmbiBinauralGains (float azimuthRad, float elevationRad,
                                             float distance, int profileIndex) const;

    //--- v0.2: Multi-channel output support ---
    OutputFormat detectOutputFormat (int numOutputChannels) const;
    void activateLayout (OutputFormat format);

    // 2D VBAP for flat layouts (Quad, 5.1, 7.1)
    void computeVBAPGains2D (const SpeakerLayout& layout, float azimuthRad, float* outGains) const;
    // 3D VBAP for height layouts (7.1.4, 9.1.6) — triplet-based
    void computeVBAPGains3D (const SpeakerLayout& layout, float azimuthRad, float elevationRad, float* outGains) const;
    // VBIP — intensity-weighted VBAP (tighter focus)
    void computeVBIPGains (const SpeakerLayout& layout, float azimuthRad, float elevationRad, float* outGains) const;
    // KNN — K-nearest neighbor panning
    void computeKNNGains (const SpeakerLayout& layout, float azimuthRad, float elevationRad, float* outGains, int k = 3) const;

    //--- DSP state ------------------------------------------------------------
    double currentSampleRate = 44100.0;

    // Main delay buffer (circular)
    std::vector<float> delayBuffer;
    int delayBufferSize = 0;
    int writePosition   = 0;

    // Feedback state
    float feedbackSample = 0.0f;
    juce::dsp::IIR::Filter<float> feedbackLPFilter;
    juce::dsp::IIR::Filter<float> feedbackHPFilter;

    // Per-object pitch shifter state (grain phase counters)
    // MAX_OBJECTS + 1: indices 0..11 for objects, index 12 for feedback pitch shifter
    float pitchPhase[MAX_OBJECTS + 1] = {};
    float feedbackPitchPhase = 0.0f;

    // Smoothing for delay time to create "Repitch" effect
    juce::LinearSmoothedValue<float> smoothedDelayTime;

    // Smooth loop multiplier — prevents clicks when enabling/disabling objects
    juce::LinearSmoothedValue<float> smoothedLoopMultiplier;

    // Modular 3D Audio Core state
    BinauralGains speakerBinauralCache[NUM_VIRTUAL_SPEAKERS] = {};
    int cachedProfileIndex = -1;
    float ambiDecodeMatrix[NUM_VIRTUAL_SPEAKERS][HOA_CHANNELS] = {};  // v0.2+ speaker decode

    // Direct Ambisonics-to-Binaural decode weights (v0.1)
    // Computed by projecting the ILD model onto the SH basis via Fibonacci sphere sampling
    float binauralSHWeightsL[HOA_CHANNELS] = {};
    float binauralSHWeightsR[HOA_CHANNELS] = {};
    int cachedBinauralProfileIndex = -1;

    // v0.2: Multi-channel output state
    OutputFormat activeOutputFormat = OutputFormat::Binaural;
    SpeakerLayout activeLayout = {};
    float discreteAmbiDecodeMatrix[16][16] = {};  // active layout's Ambi decode (speakers × SH channels)
    int discreteAmbiNumSpeakers = 0;
    std::vector<VBAPTriplet> layoutVBAPTriplets;   // 3D VBAP triplets for height layouts
    juce::dsp::IIR::Filter<float> lfeFilter;       // 120 Hz LP for LFE generation

    // Soft Clipper helper (NaN-safe, preserves natural asymptotic curve for self-oscillation)
    static float softClip (float x)
    {
        if (! std::isfinite (x))
            return 0.0f;

        const float threshold = 0.8f;
        if (x > threshold)
            return threshold + (x - threshold) / (1.0f + (x - threshold) * (x - threshold));
        if (x < -threshold)
            return -threshold + (x + threshold) / (1.0f + (x + threshold) * (x + threshold));
        return x;
    }

    // Pre-allocated work buffer (avoid allocation in processBlock)
    std::vector<float> monoInputBuffer;

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedDryWet;
    juce::SmoothedValue<float> smoothedFeedback;
    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;

    //--------------------------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayProcessor)
};
