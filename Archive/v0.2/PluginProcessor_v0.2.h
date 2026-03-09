#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

// libmysofa — SOFA file reader for HRTF data
struct MYSOFA_EASY;  // Forward declaration (avoids including mysofa.h in header)

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
// Source position for spatialization algorithm input
//==============================================================================
struct SourcePosition
{
    float azimuthRad;
    float elevationRad;
    float distance;
};

//==============================================================================
// Context structs passed to spatialization algorithms
//==============================================================================
struct LayoutContext
{
    const SpeakerLayout& layout;
    const std::vector<VBAPTriplet>& triplets;   // empty for 2D-only layouts
    const float (*ambiDecodeMatrix)[16];         // Ambisonics decode matrix [speaker][channel]
    int ambiNumSpeakers;
};

struct BinauralContext
{
    int profileIndex;
    double sampleRate;
    const BinauralProfile* profiles;             // pointer to the 5-profile array
    const float* binauralSHWeightsL;             // pre-computed SH weights (Ambisonics only)
    const float* binauralSHWeightsR;
};

//==============================================================================
// Abstract spatialization algorithm interface
// Shared across the Spatial Media Library plugin suite
//==============================================================================
class SpatializationAlgorithm
{
public:
    virtual ~SpatializationAlgorithm() = default;

    /** Compute speaker gains for a source position in the given layout. */
    virtual void computeGains (const SourcePosition& source,
                               const LayoutContext& ctx,
                               float* outputGains,
                               int numSpeakers) const = 0;

    /** Whether this algorithm can produce direct binaural output (bypassing speakers). */
    virtual bool supportsBinauralDirect() const { return false; }

    /** Compute direct binaural gains (only called if supportsBinauralDirect() is true). */
    virtual BinauralGains computeBinauralGains (const SourcePosition& source,
                                                const BinauralContext& ctx) const { return {}; }

    /** Whether this algorithm supports surround speaker output. */
    virtual bool supportsSurround() const { return true; }

    /** Whether this algorithm uses SH-domain (spherical harmonic) accumulation
        for HRTF convolution, rather than speaker-domain accumulation.
        When true, processBlock accumulates into SH buffers and uses SH-projected HRIRs.
        Only Ambisonics returns true; speaker-based algorithms (VBAP, VBIP, KNN) return false. */
    virtual bool supportsSHDomain() const { return false; }

    virtual juce::String getName() const = 0;
};

//==============================================================================
// Concrete algorithm implementations (stateless, lightweight)
//==============================================================================

/** Woodworth ITD+ILD binaural model — used internally for "Simple (Low CPU)" profile.
    Not in the user-facing algorithm dropdown; kept for Woodworth speaker cache gains. */
class DirectBinauralAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    bool supportsBinauralDirect() const override { return true; }
    BinauralGains computeBinauralGains (const SourcePosition& source,
                                        const BinauralContext& ctx) const override;
    bool supportsSurround() const override { return false; }
    juce::String getName() const override { return "Direct Binaural"; }
};

/** Vector Base Amplitude Panning (Pulkki 1997). 2D or 3D. */
class VBAPAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    juce::String getName() const override { return "VBAP"; }
};

/** 3rd-order Ambisonics (ACN/SN3D) with max-rE weighting.
    Uses SH-domain HRTF convolution for binaural output. */
class AmbisonicsAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    bool supportsBinauralDirect() const override { return true; }
    BinauralGains computeBinauralGains (const SourcePosition& source,
                                        const BinauralContext& ctx) const override;
    bool supportsSHDomain() const override { return true; }
    juce::String getName() const override { return "Ambisonics (HOA)"; }
};

/** Vector Base Intensity Panning — VBAP with squared gains for tighter focus. */
class VBIPAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    juce::String getName() const override { return "VBIP"; }
};

/** K-Nearest Neighbor panning — inverse-distance-squared weighting. */
class KNNAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    juce::String getName() const override { return "KNN"; }
};

//==============================================================================
// HRTF Database — loads SOFA files and provides HRIR lookup
// Wraps libmysofa for SOFA parsing and nearest-neighbor interpolation
//==============================================================================
class HRTFDatabase
{
public:
    HRTFDatabase();
    ~HRTFDatabase();

    /** Load a SOFA file from memory (BinaryData). Resamples to targetSampleRate. */
    bool loadFromMemory (const void* data, int dataSize, float targetSampleRate);

    /** Get interpolated HRIR pair for a direction (our convention: radians).
        Writes irLength samples to irL and irR buffers (must be pre-allocated). */
    void getInterpolatedHRIR (float azimuthRad, float elevationRad,
                              float* irL, float* irR,
                              float& delayL, float& delayR) const;

    /** For Ambisonics: compute SH-projected HRIRs.
        Integrates Y_c(dir) × HRIR(dir) over all measurement positions.
        Writes numSHChannels × irLength floats to shIRsL and shIRsR. */
    void computeSHProjectedHRIRs (float* shIRsL, float* shIRsR,
                                   int numSHChannels) const;

    /** Get the HRIR for a specific virtual speaker position.
        Used by the BinauralRenderer for speaker-based algorithms. */
    void getHRIRForSpeaker (const VirtualSpeaker& speaker,
                            float* irL, float* irR,
                            float& delayL, float& delayR) const;

    int getIRLength() const { return irLength; }
    int getNumPositions() const { return numPositions; }
    bool isLoaded() const { return loaded; }

    /** Unload current profile and free resources. */
    void unload();

private:
    MYSOFA_EASY* easyHandle = nullptr;
    int irLength = 0;
    int numPositions = 0;
    bool loaded = false;
};

//==============================================================================
// Partitioned Convolver — real-time FFT overlap-save convolution
// Uses juce::dsp::FFT for efficient per-block convolution
//==============================================================================
class PartitionedConvolver
{
public:
    PartitionedConvolver() = default;

    /** Prepare the convolver for a given max block size and IR length. */
    void prepare (int maxBlockSize, int irLength);

    /** Set or update the impulse response. Pre-computes FFT of IR. */
    void setIR (const float* ir, int length);

    /** Process one block: convolve input with IR, write to output.
        in and out must be numSamples long. Can be called in-place. */
    void process (const float* in, float* out, int numSamples);

    /** Reset internal state (overlap buffers, input accumulators). */
    void reset();

    bool isPrepared() const { return fftSize > 0; }

private:
    juce::dsp::FFT fft { 1 };      // Will be re-initialized in prepare()
    int fftOrder = 1;
    int fftSize = 0;                 // 2^fftOrder
    int irLen = 0;
    int blockSize = 0;

    std::vector<float> irFreqDomain;     // Pre-computed IR in frequency domain
    std::vector<float> inputAccum;       // Input accumulator for FFT
    std::vector<float> fftWorkBuf;       // FFT work buffer
    std::vector<float> overlapBuf;       // Overlap-save tail buffer
    int inputAccumPos = 0;               // Current position in input accumulator
};

//==============================================================================
// Binaural Renderer — manages HRTF convolution for all algorithms
// Routes spatial accumulation buffers through per-speaker or per-SH convolvers
//==============================================================================
class BinauralRenderer
{
public:
    static constexpr int MAX_CONVOLVERS = 16;  // Max speakers or SH channels

    BinauralRenderer() = default;

    /** Prepare all convolvers for the given sample rate and block size. */
    void prepare (double sampleRate, int maxBlockSize);

    /** Load a new HRTF profile. Sets up convolvers for all virtual speakers
        and (optionally) SH-domain HRIRs for Ambisonics. */
    void setProfile (int profileIndex, HRTFDatabase& hrtfDb,
                     const VirtualSpeaker* speakers, int numSpeakers,
                     int numSHChannels);

    /** Render speaker accumulation buffers through HRTF convolvers.
        Used by VBAP, VBIP, KNN (virtual speaker algorithms).
        speakerBufs: [numSpeakers][numSamples], outL/outR: [numSamples] */
    void renderSpeakerBuffers (const float* const* speakerBufs, int numSpeakers,
                                int numSamples, float* outL, float* outR);

    /** Render SH accumulation buffers through SH-domain HRTF convolvers.
        Used by Ambisonics algorithm.
        shBufs: [numSHChannels][numSamples], outL/outR: [numSamples] */
    void renderSHBuffers (const float* const* shBufs, int numSHChannels,
                           int numSamples, float* outL, float* outR);

    /** Check if the renderer is in Simple (Woodworth) mode. */
    bool isSimpleMode() const { return activeProfile == 0; }

    /** Get the active profile index. */
    int getActiveProfile() const { return activeProfile; }

    /** Reset all convolver states (e.g., on playback restart). */
    void reset();

private:
    // Virtual speaker convolvers: per-speaker × 2 ears
    PartitionedConvolver speakerConvL[MAX_CONVOLVERS];
    PartitionedConvolver speakerConvR[MAX_CONVOLVERS];

    // SH-domain convolvers: per-SH-channel × 2 ears
    PartitionedConvolver shConvL[MAX_CONVOLVERS];
    PartitionedConvolver shConvR[MAX_CONVOLVERS];

    int activeProfile = 0;   // Default = Simple (Woodworth fallback)
    int activeSpeakers = 0;
    int activeSHChannels = 0;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Temporary work buffers for convolution output
    std::vector<float> convTmpL, convTmpR;
};

//==============================================================================
class OpenSpatialDelayProcessor : public juce::AudioProcessor
{
public:
    static constexpr int MAX_OBJECTS = 12;
    static constexpr int MAX_DELAY_SECONDS = 24;  // 12 objects × 2s max base delay
    // Modular 3D Audio Core constants
    static constexpr int NUM_VIRTUAL_SPEAKERS = 16;
    static constexpr int HOA_ORDER = 3;
    static constexpr int HOA_CHANNELS = (HOA_ORDER + 1) * (HOA_ORDER + 1); // = 16

    // v0.2: Multi-channel output format
    enum class OutputFormat { Binaural = 0, Quad, Surround5_1, Surround7_1, Surround7_1_4, Surround9_1_6, Octaphonic };

    // Output format registry — single source of truth for all supported formats
    // Reusable across Spatial Media Library plugins
    struct OutputFormatInfo
    {
        OutputFormat format;
        const char* name;           // UI display name (e.g., "7.1.4 Atmos")
        const char* shortName;      // Compact name (e.g., "7.1.4")
        int requiredChannels;       // Minimum bus channels needed
        bool hasLFE;
        bool hasHeight;
    };
    static constexpr int NUM_OUTPUT_FORMATS = 7;
    static const std::array<OutputFormatInfo, NUM_OUTPUT_FORMATS> outputFormatRegistry;

    // Double-buffered layout state for lock-free audio thread reads
    struct OutputLayoutState
    {
        OutputFormat format = OutputFormat::Binaural;
        SpeakerLayout layout = {};
        float ambiDecodeMatrix[16][16] = {};
        int ambiNumSpeakers = 0;
        std::vector<VBAPTriplet> vbapTriplets;
    };

    OutputFormat getActiveOutputFormat() const { return getActiveLayout().format; }
    const OutputLayoutState& getActiveLayout() const
    {
        return layoutBuffers[activeLayoutIndex.load (std::memory_order_acquire)];
    }
    int getMaxBusChannels() const { return maxBusChannels; }
    void requestOutputFormatChange (int formatIndex);

    static juce::String getOutputFormatName (OutputFormat format);

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

    // HRTF profile names for UI (6 profiles: 5 HRTF + 1 Simple)
    static constexpr int NUM_HRTF_PROFILES = 6;
    static const char* const hrtfProfileNames[NUM_HRTF_PROFILES];

    // Modular 3D Audio Core: 9.1.6 virtual speaker layout + zenith (9 ear + 6 top + 1 zenith)
    static const std::array<VirtualSpeaker, NUM_VIRTUAL_SPEAKERS> virtualSpeakers;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //--- DSP helpers ----------------------------------------------------------
    void   writeDelayLine (float sample);
    float  readDelayLine  (float delaySamples) const;
    float  readPitchShifted (float delaySamples, float semitones, int phaseIndex);
    float getTempoSyncedDelayMs (int noteDivisionIndex) const;

    //--- Modular 3D Audio Core ------------------------------------------------
    static const std::vector<VBAPTriplet>& getVBAPTriplets();
    void updateSpeakerBinauralCache (int profileIndex);
    void computeAmbiDecodeMatrix();

    // Direct Ambisonics-to-Binaural decode (v0.1 — bypasses virtual speakers)
    void computeBinauralSHWeights (int profileIndex);

    //--- v0.2: Multi-channel output support ---
    OutputFormat detectOutputFormat (int numOutputChannels) const;
    OutputFormat resolveEffectiveFormat (OutputFormat requested, int busChannels) const;
    void activateLayout (OutputFormat format);

    //--- Spatialization algorithms (polymorphic dispatch via SpatializationAlgorithm*) ---
    // 4 user-facing algorithms (alphabetical): Ambisonics (0), KNN (1), VBAP (2), VBIP (3)
    // DirectBinaural is internal-only — used for Woodworth binaural cache in "Simple (Low CPU)"
    DirectBinauralAlgorithm  algDirectBinaural;  // Kept for Simple profile Woodworth gains
    VBAPAlgorithm            algVBAP;
    AmbisonicsAlgorithm      algAmbisonics;
    VBIPAlgorithm            algVBIP;
    KNNAlgorithm             algKNN;
    static constexpr int NUM_ALGORITHMS = 4;
    SpatializationAlgorithm* algorithms[NUM_ALGORITHMS] = {};

    //--- HRTF convolution system ---
    HRTFDatabase   hrtfDatabase;
    BinauralRenderer binauralRenderer;
    void loadHRTFProfile (int profileIndex);
    int loadedHRTFProfileIndex = -1;

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

    // v0.2: Double-buffered output layout for lock-free format switching
    OutputLayoutState layoutBuffers[2];
    std::atomic<int> activeLayoutIndex { 0 };
    int prepareLayoutIndex = 1;                  // Message thread writes to the non-active buffer
    int maxBusChannels = 2;                      // Set in prepareToPlay()

    juce::dsp::IIR::Filter<float> lfeFilter;     // 120 Hz LP for LFE generation

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

    // Pre-allocated work buffers (avoid allocation in processBlock)
    std::vector<float> monoInputBuffer;

    // HRTF 3-pass work buffers (pre-allocated in prepareToPlay)
    // Pass 1 accumulates per-speaker or per-SH-channel signals
    std::vector<float> speakerAccumBufs[NUM_VIRTUAL_SPEAKERS];  // [speaker][sample]
    std::vector<float> shAccumBufs[HOA_CHANNELS];               // [sh_channel][sample]
    std::vector<float> wetBufL, wetBufR;                        // Pass 2 output / Pass 3 input

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedDryWet;
    juce::SmoothedValue<float> smoothedFeedback;
    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;

    //--------------------------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayProcessor)
};
