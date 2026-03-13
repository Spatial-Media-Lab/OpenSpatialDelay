#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

// libmysofa — SOFA file reader for HRTF data
struct MYSOFA_EASY;  // Forward declaration (avoids including mysofa.h in header)

// #############################################################################
// SPATIAL MEDIA LIBRARY — Reusable spatial audio data structures
// These structs are shared across all plugins in the Spatial Media Library suite.
// #############################################################################

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
};

// #############################################################################
// SPATIAL MEDIA LIBRARY — Spatialization algorithm interface & implementations
// Abstract base + 7 concrete algorithms. Reusable across all SML plugins.
// To add a new plugin: implement your DSP, use these algorithms via computeGains().
// #############################################################################

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
    virtual BinauralGains computeBinauralGains (const SourcePosition& /*source*/,
                                                const BinauralContext& /*ctx*/) const { return {}; }

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

/** Distance-Based Amplitude Panning (Lossius et al., ICMC 2009).
    Computes speaker gains from Euclidean distances in Cartesian space.
    Ideal for irregular/non-standard speaker layouts where VBAP triangulation fails. */
class DBAPAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    juce::String getName() const override { return "DBAP"; }
};

/** Multiple-Direction Amplitude Panning (Pulkki 2000).
    Creates source spread by rendering multiple VBAP sub-sources on a ring
    around the main direction. Produces wider, more stable spatial images. */
class MDAPAlgorithm : public SpatializationAlgorithm
{
public:
    void computeGains (const SourcePosition& source, const LayoutContext& ctx,
                       float* outputGains, int numSpeakers) const override;
    juce::String getName() const override { return "MDAP"; }
};

// #############################################################################
// SPATIAL MEDIA LIBRARY — HRTF & binaural rendering infrastructure
// HRTFDatabase, PartitionedConvolver, and BinauralRenderer are reusable
// by any SML plugin that needs binaural output via HRTF convolution.
// #############################################################################

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
    static constexpr int MAX_SOURCES = 12;     // Per-source direct binaural (v0.3)

    BinauralRenderer() = default;

    /** Prepare all convolvers for the given sample rate and block size. */
    void prepare (double sampleRate, int maxBlockSize);

    /** Load a new HRTF profile. Computes normGain and prepares source convolvers.
        v0.3: No longer sets up virtual speaker or SH convolvers. */
    void setProfile (int profileIndex, HRTFDatabase& hrtfDb);

    /** Update a single source's HRIR based on its current 3D position.
        Realtime-safe: KD-tree lookup + in-place FFT, no allocation.
        Called from processBlock at block boundaries when position changes. */
    void updateSourceHRIR (int sourceIndex, float azRad, float elRad,
                           HRTFDatabase& db);

    /** Render per-source accumulation buffers through HRTF convolvers.
        sourceBufs: [numSources][numSamples], outL/outR: [numSamples]
        Only enabled sources are convolved. */
    void renderSourceBuffers (const float* const* sourceBufs,
                              const bool* sourceEnabled,
                              int numSources, int numSamples,
                              float* outL, float* outR);

    /** Check if the renderer is in Simple (Woodworth) mode. */
    bool isSimpleMode() const { return activeProfile == 0; }

    /** Get the active profile index. */
    int getActiveProfile() const { return activeProfile; }

    /** Reset all convolver states (e.g., on playback restart). */
    void reset();

private:
    // v0.3: Per-source direct binaural convolvers
    PartitionedConvolver sourceConvL[MAX_SOURCES];
    PartitionedConvolver sourceConvR[MAX_SOURCES];
    float  cachedSourceAz[MAX_SOURCES] = {};     // radians, for ~1° change detection
    float  cachedSourceEl[MAX_SOURCES] = {};
    bool   sourceConvReady[MAX_SOURCES] = {};     // true after first HRIR loaded
    float  storedNormGain = 1.0f;                 // cross-profile normalization
    int    storedIRLength = 0;                    // cached for updateSourceHRIR

    int activeProfile = 0;   // Default = Simple (Woodworth fallback)

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Temporary work buffers for convolution output
    std::vector<float> convTmpL, convTmpR;
};

// #############################################################################
// PLUGIN-SPECIFIC — OpenSpatialDelay processor class
// This class wires the reusable spatial framework (above) to the delay engine.
// Other SML plugins would replace this class with their own DSP processor,
// reusing the structs, algorithms, HRTFDatabase, and BinauralRenderer above.
// #############################################################################

//==============================================================================
class OpenSpatialDelayProcessor : public juce::AudioProcessor,
                                  private juce::Timer
{
public:
    static constexpr int MAX_OBJECTS = 12;
    static constexpr int MAX_DELAY_SECONDS = 24;  // 12 objects × 2s max base delay
    // Modular 3D Audio Core constants
    static constexpr int NUM_VIRTUAL_SPEAKERS = 16;
    static constexpr int HOA_ORDER = 3;
    static constexpr int HOA_CHANNELS = (HOA_ORDER + 1) * (HOA_ORDER + 1); // = 16 (surround decode cap)
    static constexpr int MAX_AMBI_ORDER = 6;
    static constexpr int MAX_AMBI_CHANNELS = (MAX_AMBI_ORDER + 1) * (MAX_AMBI_ORDER + 1); // = 49

    // v0.5: Output formats — Binaural first, then Stereo, Surround, Ambisonics
    // 1 Binaural + 1 Stereo + 13 Surround + 6 Ambisonics = 21 total
    // Stereo mode (Equal Power, VBAP, XY, MS, Blumlein) selected via algorithm parameter
    enum class OutputFormat {
        // Binaural (HRTF head model) — default
        Binaural = 0,
        // Stereo (mode selected by algorithm param indices 6-10)
        Stereo,
        // Surround (ascending channel count)
        Quad, Surround5_0, Surround5_1, Surround7_0,
        Surround5_1_2, Surround7_1, Octaphonic,
        Surround7_0_2, Surround5_1_4, Surround7_1_2,
        Surround7_1_4, Surround7_1_6, Surround9_1_6,
        // Ambisonics output (AmbiX ACN/SN3D)
        AmbisonicsFOA, AmbisonicsSOA, AmbisonicsHOA,
        Ambisonics4OA, Ambisonics5OA, Ambisonics6OA
    };

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
        bool isAmbisonicsOutput;    // true for FOA/SOA/HOA output encoding
        int  ambiOrder;             // 0 for non-ambi, 1-6 for Ambisonics output
        bool isStereoVariant;       // true for Stereo (single entry, mode via algorithm param)
    };
    static constexpr int NUM_OUTPUT_FORMATS = 21;
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

    //--- DELAY-SPECIFIC: DSP helpers ------------------------------------------
    void   writeDelayLine (float sample);
    float  readDelayLine  (float delaySamples) const;
    float  readPitchShifted (float delaySamples, float semitones, int phaseIndex);
    float getTempoSyncedDelayMs (int noteDivisionIndex) const;

    //--- DELAY-SPECIFIC: Render path methods (v0.5 refactor) ------------------
    /** Read one tap sample: delay + pitch shift + air absorption. */
    float readObjectSample (int objectIndex, float baseDelaySamples, float pitchSemitones);
    /** Process feedback: read from end of chain, filter, soft-clip, NaN guard. */
    void  processFeedbackSample (float currentLoopMult, float baseDelaySamples,
                                 float pitchSemitones, float fb);

    void renderDirectBinauralHRTF (juce::AudioBuffer<float>& buffer, int numSamples,
                                   const ObjectState* objects, const float* objDistGain,
                                   float pitchSemitones);
    void renderSimpleBinauralWoodworth (juce::AudioBuffer<float>& buffer, int numSamples,
                                       const ObjectState* objects, const BinauralGains* objGains,
                                       float pitchSemitones);
    void renderAmbisonicsOutput (juce::AudioBuffer<float>& buffer, int numSamples,
                                const ObjectState* objects, const float* objDistGain,
                                float pitchSemitones, int ambiOrder);
    void renderStereoVariant (juce::AudioBuffer<float>& buffer, int numSamples,
                             const ObjectState* objects, const float* objDistGain,
                             float pitchSemitones, int stereoMode);
    void renderDiscreteSurround (juce::AudioBuffer<float>& buffer, int numSamples,
                                const ObjectState* objects,
                                const float (*objChannelGains)[16],
                                const float* objDistGain, float pitchSemitones,
                                const SpeakerLayout& surLayout);

    //--- SPATIAL FRAMEWORK: Modular 3D Audio Core ----------------------------
    static const std::vector<VBAPTriplet>& getVBAPTriplets();
    void computeAmbiDecodeMatrix();


    // Shared Ambisonics decode matrix computation (used by activateLayout)
    static void computeAmbiDecodeForLayout (const SpeakerLayout& layout,
                                            float outMatrix[][16], int& outNumSpeakers);

    //--- SPATIAL FRAMEWORK: Multi-channel output support ---
    OutputFormat detectOutputFormat (int numOutputChannels) const;
    OutputFormat resolveEffectiveFormat (OutputFormat requested, int busChannels) const;
    void activateLayout (OutputFormat format);

    //--- SPATIAL FRAMEWORK: Spatialization algorithms (polymorphic dispatch) ---
    // 4 user-facing algorithms (alphabetical): Ambisonics (0), KNN (1), VBAP (2), VBIP (3)
    // DirectBinaural is internal-only — used for Woodworth binaural cache in "Simple (Low CPU)"
    DirectBinauralAlgorithm  algDirectBinaural;  // Kept for Simple profile Woodworth gains
    VBAPAlgorithm            algVBAP;
    AmbisonicsAlgorithm      algAmbisonics;
    VBIPAlgorithm            algVBIP;
    KNNAlgorithm             algKNN;
    DBAPAlgorithm            algDBAP;            // v0.4: Distance-Based Amplitude Panning
    MDAPAlgorithm            algMDAP;            // v0.5: Multiple-Direction Amplitude Panning
    static constexpr int NUM_ALGORITHMS = 6;
    SpatializationAlgorithm* algorithms[NUM_ALGORITHMS] = {};

    //--- SPATIAL FRAMEWORK: HRTF convolution (double-buffered for thread safety) ---
    HRTFDatabase   hrtfDatabase;
    BinauralRenderer binauralRenderers[2];
    std::atomic<int> activeRendererIndex { 0 };
    int prepareRendererIndex = 1;
    void loadHRTFProfile (int profileIndex);
    void loadHRTFProfileIntoRenderer (int profileIndex, BinauralRenderer& renderer);
    int loadedHRTFProfileIndex = -1;

    //--- SPATIAL FRAMEWORK: Background HRTF loading (via Timer) ---
    void timerCallback() override;
    std::atomic<int> targetHRTFProfile { 0 };

    //--- DELAY-SPECIFIC: DSP state --------------------------------------------
    double currentSampleRate = 44100.0;

    // Main delay buffer (circular, power-of-2 size for bitmask indexing)
    std::vector<float> delayBuffer;
    int delayBufferSize = 0;
    int delayBufferMask = 0;   // v0.5: = delayBufferSize - 1, for & instead of %
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

    //--- SPATIAL FRAMEWORK: Layout & decode state ----------------------------
    float ambiDecodeMatrix[NUM_VIRTUAL_SPEAKERS][HOA_CHANNELS] = {};  // v0.2+ surround decode


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

    // v0.5: Contiguous per-source accumulation buffers for direct HRTF convolution
    std::vector<float> sourceAccumBufStorage;          // MAX_OBJECTS * maxBlockSize (contiguous)
    float* sourceAccumBufPtrs[MAX_OBJECTS] = {};       // pointers into storage
    std::vector<float> wetBufL, wetBufR;               // Convolution output / dry-wet mix input

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedDryWet;
    juce::SmoothedValue<float> smoothedFeedback;
    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;

    // v0.4: Air absorption — global toggle, per-object LP filter driven by distance
    juce::dsp::IIR::Filter<float> airAbsorptionFilter[MAX_OBJECTS];

    // v0.5: NFC-HOA — per-order shelf filters for near-field compensation (Ambisonics output only)
    // Applied internally in renderAmbisonicsOutput(), not exposed to user
    static constexpr float NFC_REFERENCE_RADIUS = 1.5f;  // meters (typical studio monitoring distance)
    juce::dsp::IIR::Filter<float> nfcFilters[MAX_OBJECTS][MAX_AMBI_ORDER];  // 12 objects × 6 orders
    float prevNfcDistance[MAX_OBJECTS] = {};

    // v0.5: Cached per-object parameter pointers (avoid string lookup in processBlock)
    std::atomic<float>* cachedParam_enabled[MAX_OBJECTS]       = {};
    std::atomic<float>* cachedParam_azimuth[MAX_OBJECTS]       = {};
    std::atomic<float>* cachedParam_elevation[MAX_OBJECTS]     = {};
    std::atomic<float>* cachedParam_distance[MAX_OBJECTS]      = {};
    std::atomic<float>* cachedParam_dopplerAmount[MAX_OBJECTS]  = {};

    // v0.5: Cached feedback filter frequencies (skip recalculation when unchanged)
    float cachedFeedbackLPFreq = -1.0f;
    float cachedFeedbackHPFreq = -1.0f;

    // v0.5: Cached pitch shifter window size (set in prepareToPlay, constant within session)
    float cachedPitchWindowSamples = 0.0f;

    // v0.4: Doppler effect — per-object checkbox, global amount, velocity tracking
    float prevAzimuth[MAX_OBJECTS]   = {};   // radians, previous block
    float prevElevation[MAX_OBJECTS] = {};   // radians, previous block
    float prevDistance[MAX_OBJECTS]   = {};   // normalized 0..1, previous block
    float dopplerSemitones[MAX_OBJECTS] = {};  // computed per-block
    float smoothedRadialVelocity[MAX_OBJECTS] = {};  // EMA-smoothed velocity

    //--------------------------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayProcessor)
};
