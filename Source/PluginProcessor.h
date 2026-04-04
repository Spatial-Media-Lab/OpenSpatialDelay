#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include "PresetData.h"
#include "PhaseVocoderPitchShifter.h"
#include "DopplerVelocity.h"
#include "TrajectoryEngine.h"
#include "FilterBank.h"

// libmysofa — SOFA file reader for HRTF data
struct MYSOFA_EASY;  // Forward declaration (avoids including mysofa.h in header)

// #############################################################################
// SPATIAL MEDIA LIBRARY — Extraction Guide
// Sections marked "SPATIAL MEDIA LIBRARY" or "SPATIAL FRAMEWORK" are reusable
// across future SML plugins. To create a new plugin:
//   1. Keep all SPATIAL MEDIA LIBRARY sections (IO, algorithms, HRTF, layouts,
//      OSC receive/send, bus negotiation, utility DSP)
//   2. Replace all DELAY-SPECIFIC sections (delay line, pitch shift, feedback,
//      wobble, trajectories, render loop internals)
//   3. Parameter layout (MIXED) — keep spatial params, replace effect-specific
// #############################################################################

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
    bool  enabled      = false;
};

//==============================================================================
// Per-object trajectory state (for editor visualization)
//==============================================================================
struct TrajectoryState
{
    float originAzDeg  = 0.0f;   // Base/origin position (captured at shape change)
    float originElDeg  = 0.0f;
    float originDist   = 0.5f;
    int   shape        = 0;      // 0 = None, 1+ = active trajectory
    float phase        = 0.0f;   // 0..1 animation progress
    bool  reverse      = false;
    float randomTime   = 0.0f;   // Random trajectory: current time accumulator
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

    /** Get ITD-free interpolated HRIR pair for a direction.
        The returned HRIRs have ITD removed (time-aligned onsets). The ITD values
        are returned separately in delayL/delayR (in samples, fractional).
        This produces phase-coherent HRIRs that can be smoothly crossfaded
        without comb-filtering artifacts from ITD misalignment. */
    void getAlignedHRIR (float azimuthRad, float elevationRad,
                         float* irL, float* irR,
                         float& delayL, float& delayR) const;

    int getIRLength() const { return irLength; }
    int getNumPositions() const { return numPositions; }
    bool isLoaded() const { return loaded; }

    /** Unload current profile and free resources. */
    void unload();

    /** Convert a raw HRIR to minimum-phase in-place using cepstral decomposition.
        Preserves magnitude spectrum but removes excess phase, so time-domain
        interpolation between adjacent HRIRs produces smooth spectral transitions
        without comb filtering (issue #47). workBuf must be >= fftSize * 2 floats. */
    static void convertToMinPhase (float* ir, int irLength, int fftOrder, float* workBuf);

    /** Detect onset sample index of an IR using threshold of peak amplitude.
        Returns the index of the first sample exceeding thresholdFraction * peakAbs.
        Used to compute ITD when SOFA delay values are zero (ITD baked into waveform).
        Returns 0 if no clear onset found or if onset > irLength/2. */
    static int detectOnset (const float* ir, int length, float thresholdFraction = 0.1f);

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
    ~PartitionedConvolver();

    /** Prepare the convolver for a given max block size and IR length. */
    void prepare (int maxBlockSize, int irLength);

    /** Set or update the impulse response.
        v1.0.5: Dual-convolver crossfade — new IR is loaded into the inactive
        slot and crossfaded over kCrossfadeBlocks blocks using equal-power
        (cos/sin) gains.  This eliminates overlap-save boundary discontinuities
        that caused audible pops during HRTF transitions (issue #50). */
    void setIR (const float* ir, int length);

    /** Process one block: convolve input with IR, write to output.
        in and out must be numSamples long. Can be called in-place. */
    void process (const float* in, float* out, int numSamples);

    /** Reset internal state (overlap buffers, input accumulators). */
    void reset();

    /** Reset and clear all IR data so the next setIR is a direct load (no crossfade). */
    void clearAll();

    bool isPrepared() const { return fftSize > 0; }

private:
    std::unique_ptr<juce::dsp::FFT> fft;  // Owned via unique_ptr for locked destruction (issue #137)
    int fftOrder = 1;
    int fftSize = 0;                 // 2^fftOrder
    int irLen = 0;
    int blockSize = 0;

    // v1.0.5: Dual-convolver architecture (issue #50).
    // Two independent convolution slots run in parallel during crossfades.
    // This avoids the overlap-save boundary discontinuity: each slot keeps
    // its own overlap buffer tied to its own IR, so the tail is never
    // contaminated by a mismatched kernel.
    struct ConvSlot
    {
        std::vector<float> irFreqDomain;     // IR in frequency domain
        std::vector<float> inputAccum;       // Input accumulator for FFT
        std::vector<float> fftWorkBuf;       // FFT work buffer
        std::vector<float> overlapBuf;       // Overlap-save tail buffer
        int inputAccumPos = 0;               // Current position in input accumulator
    };

    ConvSlot slots[2];

    // State machine for IR transitions
    enum class State { Idle, Warmup, Crossfading };
    State state = State::Idle;

    static constexpr int kCrossfadeBlocks = 4;   // Equal-power crossfade duration (~21ms)
    static constexpr int kWarmupBlocks = 1;      // Let inactive slot build overlap before crossfade

    int activeSlot = 0;                          // Index of the currently active slot (0 or 1)
    int stateBlockCount = 0;                     // Blocks elapsed in current state

    // Per-sample gain interpolation for glitch-free crossfade
    float fadeOutGain = 1.0f;                    // Current fade-out gain (active → old)
    float fadeInGain  = 0.0f;                    // Current fade-in gain  (inactive → new)
    float prevFadeOutGain = 1.0f;                // Previous block's ending fade-out gain
    float prevFadeInGain  = 0.0f;               // Previous block's ending fade-in gain

    // Deferred IR: if setIR() is called mid-transition, store it for later
    std::vector<float> pendingIR;
    int pendingIRLen = 0;
    bool hasPendingIR = false;

    // Work buffers for dual-slot output mixing
    std::vector<float> slotOutputA;
    std::vector<float> slotOutputB;

    // Helpers
    void processSlot (ConvSlot& slot, const float* in, float* out, int numSamples);
    void resetSlot (ConvSlot& slot);
    void loadIRIntoSlot (ConvSlot& slot, const float* ir, int length);
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

    /** Per-renderer HRTF database (issue #96: eliminates shared-state race
        between timer thread loading and audio thread HRIR lookups). */
    HRTFDatabase hrtfDatabase;

    /** Prepare all convolvers for the given sample rate and block size. */
    void prepare (double sampleRate, int maxBlockSize);

    /** Load a new HRTF profile. Computes normGain and prepares source convolvers.
        v0.3: No longer sets up virtual speaker or SH convolvers. */
    void setProfile (int profileIndex);

    /** Update a single source's HRIR based on its current 3D position.
        Realtime-safe: KD-tree lookup + in-place FFT, no allocation.
        Called from processBlock at block boundaries when position changes. */
    void updateSourceHRIR (int sourceIndex, float azRad, float elRad);

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

    /** Invalidate all source convolvers so the next HRIR load is a direct load
        (no crossfade from stale IR). Used during preset transitions to prevent
        clicks from crossfading between unrelated HRIR positions. */
    void invalidateSources();

    /** Test-only: override ITD processing state for diagnostic isolation. */
    void setITDEnabled (bool enabled) { itdActive = enabled; }

    /** Test-only: read current ITD values for diagnostic logging. */
    float getCurrentITDL (int src) const { return (src >= 0 && src < MAX_SOURCES) ? currentITDL[src] : 0.0f; }
    float getCurrentITDR (int src) const { return (src >= 0 && src < MAX_SOURCES) ? currentITDR[src] : 0.0f; }
    float getTargetITDL (int src) const { return (src >= 0 && src < MAX_SOURCES) ? targetITDL[src] : 0.0f; }
    float getTargetITDR (int src) const { return (src >= 0 && src < MAX_SOURCES) ? targetITDR[src] : 0.0f; }

    /** Test-only: no-op, retained for API compatibility. */
    void setMinPhaseEnabled (bool) {}

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

    // v1.0: ITD (Inter-aural Time Difference) tracking for smooth HRIR transitions.
    // When using getAlignedHRIR(), HRIRs are time-aligned (ITD removed).
    // ITD is applied as a separate fractional-sample delay, smoothly interpolated
    // between blocks to prevent timing discontinuities.
    float currentITDL[MAX_SOURCES] = {};    // Current applied ITD (samples, fractional)
    float currentITDR[MAX_SOURCES] = {};
    float targetITDL[MAX_SOURCES] = {};     // Target ITD from latest HRIR lookup
    float targetITDR[MAX_SOURCES] = {};

    // Short delay lines for ITD application (max ITD ≈ 0.7ms ≈ 34 samples @ 48kHz)
    static constexpr int kITDBufferSize = 64;
    float itdBufferL[MAX_SOURCES][kITDBufferSize] = {};
    float itdBufferR[MAX_SOURCES][kITDBufferSize] = {};
    int itdWritePos[MAX_SOURCES] = {};
    bool itdActive = false;  // true when using aligned HRIRs (non-Simple profiles)

};

// #############################################################################
// PLUGIN-SPECIFIC — OpenSpatialDelay processor class
// This class wires the reusable spatial framework (above) to the delay engine.
// Other SML plugins would replace this class with their own DSP processor,
// reusing the structs, algorithms, HRTFDatabase, and BinauralRenderer above.
// #############################################################################

//==============================================================================
class OpenSpatialDelayProcessor : public juce::AudioProcessor,
                                  private juce::Timer,
                                  private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
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

    // v0.7: ACN channel index → SH order lookup (constexpr for compile-time optimization)
    static constexpr int acnToOrder (int acn)
    {
        if (acn < 1)  return 0;  if (acn < 4)  return 1;
        if (acn < 9)  return 2;  if (acn < 16) return 3;
        if (acn < 25) return 4;  if (acn < 36) return 5;
        return 6;
    }

    // Output formats — Binaural, Stereo, Surround, Octaphonic, Atmos, SML, Ambisonics
    // 1 Binaural + 1 Stereo + 15 Surround + 6 Ambisonics = 23 total
    // Stereo mode (Equal Power, VBAP, XY, MS, Blumlein) selected via algorithm parameter
    enum class OutputFormat {
        // Binaural (HRTF head model) — default
        Binaural = 0,
        // Stereo (mode selected by algorithm param indices 6-10)
        Stereo,
        // Surround (ascending channel count)
        Quad, Surround5_0, Surround5_1, Surround7_0, Surround7_1,
        // 9.1 Surround (ITU-R BS.2051 System H — ear level only, no height)
        Surround9_1,
        // Octaphonic
        Octaphonic,
        // Atmos / Immersive (ascending channel count)
        Surround5_1_2, Surround5_1_4, Surround7_1_2,
        Surround7_1_4, Surround7_1_6, Surround9_1_4, Surround9_1_6,
        SurroundSML13_1,  // SML Multi-Use Room (13 speakers + LFE)
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
    static constexpr int NUM_OUTPUT_FORMATS = 23;
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
    TrajectoryState getTrajectoryState (int objectIndex) const;

    // v0.9: Evaluate Random trajectory noise at a given time (for look-ahead trail drawing)
    struct RandomPosition { float azDeg, elDeg, dist; };
    RandomPosition evaluateRandomNoise (int objectIndex, float time) const;

    // v0.6: OSC state accessors for editor
    bool isOscConnected() const { return oscConnected; }
    bool isOscOverrideActive (int objectIndex) const
    {
        return (objectIndex >= 0 && objectIndex < MAX_OBJECTS)
               ? oscOverrideActive[objectIndex].load (std::memory_order_relaxed) : false;
    }

    // v0.6: OSC port configuration (editable from editor)
    int getOscReceivePort() const { return oscReceivePort; }
    void setOscReceivePort (int port);

    // v1.0: Global tap drawer state (persisted for editor)
    bool getGlobalDrawerOpen() const { return globalDrawerOpen; }
    void setGlobalDrawerOpen (bool open) { globalDrawerOpen = open; }

    // v1.0: Global tap offset values (for OSC ↔ editor sync)
    static constexpr int kNumGlobalTapOffsets = 6;
    std::atomic<float> globalTapOffset[kNumGlobalTapOffsets] = {};  // AZ, EL, DIST, PITCH, DOPPLER, SPEED
    std::atomic<bool>  globalTapOffsetChanged { false };

    // Issue #68: Config params stored outside APVTS to hide from DAW automation lists.
    // Saved/restored in getStateInformation/setStateInformation.
    std::atomic<int> configAlgorithm { 0 };
    std::atomic<int> configHrtfProfile { 0 };
    std::atomic<int> configOutputFormat { 0 };
    std::atomic<int> configInputFormat { 0 };

    // Issue #122: Debounce updateHostDisplay — set by editor/OSC, flushed in timerCallback
    std::atomic<bool> configStateDirty { false };
    void markConfigStateDirty() { configStateDirty.store (true, std::memory_order_relaxed); }

    // v0.7: OSC Send accessors for editor
    bool isOscSendConnected() const { return oscSendConnected; }
    bool getOscSendEnabled() const { return oscSendEnabled; }
    void setOscSendEnabled (bool enabled);

    // v0.8: Delay channel routing for per-tap input selection
    enum class DelayChannel { Mono = 0, Left = 1, Right = 2 };

    // v0.7: Per-tap activity level (RMS) for UI glow animation
    float getTapActivityRMS (int i) const
    {
        return (i >= 0 && i < MAX_OBJECTS) ? tapActivityRMS[i].load (std::memory_order_relaxed) : 0.0f;
    }
    int getOscSendPort() const { return oscSendPort; }
    void setOscSendPort (int port);
    juce::String getOscSendIP() const { return oscSendIP; }
    void setOscSendIP (const juce::String& ip);

    //--- v0.6: Preset system --------------------------------------------------
    // PresetData struct, factory presets, and category names are in PresetData.h/cpp
    // (shared between plugin and build-time install_presets CLI tool)

    struct CategorizedPreset
    {
        juce::String category;
        juce::String name;
        int originalIndex;   // index into allPresets
        bool isFactory;
    };
    std::vector<CategorizedPreset> getCategorizedPresets() const;

    int  getNumPresets() const;
    int  getCurrentPresetIndex() const { return currentPresetIndex; }
    void setCurrentPresetIndex (int idx) { currentPresetIndex = idx; }
    juce::StringArray getPresetNames() const;
    void loadPreset (int index);
    void saveUserPreset (const juce::String& name);
    void saveUserPreset (const juce::String& name, const juce::String& category);
    void loadNextPreset();
    void loadPreviousPreset();
    static juce::File getPresetDirectory();

    static const std::array<BinauralProfile, 5> binauralProfiles;

    // HRTF profile names for UI (6 profiles: 5 HRTF + 1 Simple)
    static constexpr int NUM_HRTF_PROFILES = 6;
    static const char* const hrtfProfileNames[NUM_HRTF_PROFILES];

    // Modular 3D Audio Core: 9.1.6 virtual speaker layout + zenith (9 ear + 6 top + 1 zenith)
    static const std::array<VirtualSpeaker, NUM_VIRTUAL_SPEAKERS> virtualSpeakers;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //--- DELAY-SPECIFIC: DSP helpers ------------------------------------------
    void   writeDelayLine (float sampleL, float sampleR);
    float  readDelayLineL    (float delaySamples) const;
    float  readDelayLineR    (float delaySamples) const;
    float  readDelayLineMono (float delaySamples) const;
    float getTempoSyncedDelayMs (int noteDivisionIndex) const;

    //--- DELAY-SPECIFIC: Render path methods (v0.5 refactor) ------------------
    /** Read one tap sample: delay + per-tap pitch shift + air absorption.
        blockFraction: 0..1 position within block, for per-sample Doppler interpolation. */
    float readObjectSample (int objectIndex, float baseDelaySamples, float blockFraction);
    /** Process feedback: read from end of chain, filter, soft-clip, NaN guard. */
    void  processFeedbackSample (float currentLoopMult, float baseDelaySamples,
                                 float fb);

    void renderDirectBinauralHRTF (juce::AudioBuffer<float>& buffer, int numSamples,
                                   const ObjectState* objects, const float* objDistGain);
    void renderSimpleBinauralWoodworth (juce::AudioBuffer<float>& buffer, int numSamples,
                                       const ObjectState* objects, const BinauralGains* objGains);
    void renderAmbisonicsOutput (juce::AudioBuffer<float>& buffer, int numSamples,
                                const ObjectState* objects, const float* objDistGain,
                                int ambiOrder);
    void renderStereoVariant (juce::AudioBuffer<float>& buffer, int numSamples,
                             const ObjectState* objects, const float* objDistGain,
                             int stereoMode);
    void renderDiscreteSurround (juce::AudioBuffer<float>& buffer, int numSamples,
                                const ObjectState* objects,
                                const float (*objChannelGains)[16],
                                const float* objDistGain,
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
    // HRTFDatabase is now per-renderer (issue #96: eliminates shared-state race)
    BinauralRenderer binauralRenderers[2];
    std::atomic<int> activeRendererIndex { 0 };
    int prepareRendererIndex = 1;
    void loadHRTFProfile (int profileIndex);
    void loadHRTFProfileIntoRenderer (int profileIndex, BinauralRenderer& renderer);
    int loadedHRTFProfileIndex = -1;

    // v1.0.4: Renderer-level crossfade for smooth HRTF profile switching (issue #90).
    // When the active renderer swaps, the new convolver's overlap buffer is empty,
    // causing a one-block transient overshoot from the HRIR's positive early energy.
    // Crossfading old→new renderer output masks this startup ramp.
    int prevActiveRendererIdx_ = 0;
    bool rendererXfading_ = false;
    int rendererXfadeBlockCount_ = 0;
    int rendererXfadeFromIdx_ = 0;
    static constexpr int kRendererXfadeBlocks = 8;
    float prevRxFadeOut_ = 1.0f;
    float prevRxFadeIn_ = 0.0f;
    std::vector<float> xfadeWetL_, xfadeWetR_;
    std::atomic<bool> rendererXfadeActive_ { false };  // Signals timer to defer prepare (issue #137)

    //--- SPATIAL FRAMEWORK: Background HRTF loading (via Timer) ---
    void timerCallback() override;
    std::atomic<int> targetHRTFProfile { 0 };

    //--- SPATIAL MEDIA LIBRARY: ADM-OSC Receive --------------------------------
    void oscMessageReceived (const juce::OSCMessage& message) override;
    void handleOSCPosition (int objectIndex, float azDeg, float elDeg, float dist);
    void handleOSCParam (const juce::String& paramID, float denormValue);
    void syncGlobalTapOffsetAtomic (int index, float value);
public:
    // v1.0: Public test entry point — forwards to oscMessageReceived
    void testProcessOSCMessage (const juce::OSCMessage& msg) { oscMessageReceived (msg); }

    // v1.0.3: Test accessor for active binaural renderer (diagnostic isolation)
    BinauralRenderer& getActiveRenderer() { return binauralRenderers[activeRendererIndex.load (std::memory_order_acquire)]; }

    // v1.0.3: Synchronous HRTF profile load for tests (timer thread doesn't fire in test harness)
    void testLoadHRTFProfile (int profileIndex) { loadHRTFProfile (profileIndex); }
private:

    juce::OSCReceiver oscReceiver;
    int  oscReceivePort = 4002;                         // Default ADM-OSC receive port
    bool globalDrawerOpen = false;                      // v1.0: global tap drawer visibility
    bool oscConnected = false;                          // Current connection state
    bool prevAdmOscEnabled = false;                     // Edge-detect for enable/disable transitions
    std::atomic<float>* cachedParam_admOscEnabled = nullptr;

    //--- SPATIAL MEDIA LIBRARY: ADM-OSC Send state ---
    juce::OSCSender oscSender;
    bool oscSendEnabled = false;
    bool oscSendConnected = false;
    int  oscSendPort = 4003;
    juce::String oscSendIP = "127.0.0.1";
    int  oscSendTickCounter = 0;                           // 60Hz ticks → send every 2nd (30Hz)
    float oscSendPrevAz[MAX_OBJECTS]   = {};               // position-change gating
    float oscSendPrevEl[MAX_OBJECTS]   = {};
    float oscSendPrevDist[MAX_OBJECTS] = {};

    // v1.0: Per-object non-position send tracking (change-gated)
    float oscSendPrevDoppler[MAX_OBJECTS]    = {};
    float oscSendPrevObjPitch[MAX_OBJECTS]   = {};
    float oscSendPrevEnabled[MAX_OBJECTS]    = {};
    float oscSendPrevTrajShape[MAX_OBJECTS]  = {};
    float oscSendPrevTrajSpeed[MAX_OBJECTS]  = {};
    float oscSendPrevTrajDir[MAX_OBJECTS]    = {};
    float oscSendPrevInput[MAX_OBJECTS]      = {};

    // v1.0: Global param send tracking (change-gated)
    struct OscSendPrevGlobal {
        float delayTime = -1.0f, feedback = -1.0f, filterLP = -1.0f, filterHP = -1.0f;
        float filterLPQ = -1.0f, filterHPQ = -1.0f, dryWet = -1.0f;
        float inputGain = -999.0f, outputGain = -999.0f;
        float tempoSync = -1.0f, noteDivision = -1.0f, syncMode = -1.0f;
        float filterEnabled = -1.0f, algorithm = -1.0f, hrtfProfile = -1.0f;
        float outputFormat = -1.0f, airAbsorption = -1.0f;
        float wobbleEnabled = -1.0f, wobbleAmount = -1.0f, wobbleMorph = -1.0f;
        float tapAzimuth = -999.0f, tapElevation = -999.0f, tapDistance = -999.0f;
        float tapDoppler = -999.0f, tapPitch = -999.0f, tapSpeed = -999.0f;
    };
    OscSendPrevGlobal oscSendPrevGlobal;

    // Per-object OSC override: when active, OSC controls position (trajectory paused)
    std::atomic<bool> oscOverrideActive[MAX_OBJECTS] = {};
    double oscLastReceiveTime[MAX_OBJECTS] = {};        // juce::Time::getMillisecondCounterHiRes()

    // Partial Cartesian state (for individual /x, /y, /z messages)
    float oscCartesianX[MAX_OBJECTS] = {};
    float oscCartesianY[MAX_OBJECTS] = {};
    float oscCartesianZ[MAX_OBJECTS] = {};

    //--- v0.6: Per-object trajectory animation engine --------------------------
    std::atomic<float>* cachedParam_trajectoryShape[MAX_OBJECTS] = {};
    std::atomic<float>* cachedParam_trajectorySpeed[MAX_OBJECTS] = {};
    std::atomic<float>* cachedParam_trajectoryDirection[MAX_OBJECTS] = {};  // v0.8: 0=Forward, 1=Reverse

    // Cached RangedAudioParameter* for trajectory setValueNotifyingHost (avoids string lookups in timer)
    juce::RangedAudioParameter* trajParam_azimuth[MAX_OBJECTS]   = {};

    // Pre-built OSC address strings for ADM-OSC Send (avoids per-tick string allocation)
    juce::String oscSendAddress[MAX_OBJECTS];

    // v1.0.1: Trajectory animation — extracted to TrajectoryEngine class
    TrajectoryEngine trajectory;

public:
    // v1.0.1: Trajectory types — delegated to TrajectoryEngine
    using TrajectoryResult = TrajectoryEngine::TrajectoryResult;
    static TrajectoryResult computeTrajectory (int shape, float phase,
                                               float baseAz, float baseEl, float baseDist,
                                               bool reverse = false)
    {
        return TrajectoryEngine::computeTrajectory (shape, phase, baseAz, baseEl, baseDist, reverse);
    }

    // v1.0.1: Doppler pitch accessor — delegates to extracted DopplerVelocity module
    float getDopplerSemitones (int objectIndex) const { return doppler.getRawSemitones (objectIndex); }

private:

    //--- DELAY-SPECIFIC: DSP state --------------------------------------------
    // Tuning constants (named to avoid magic numbers in hot paths)
    static constexpr float kFeedbackInputHeadroom  = 0.98f;   // prevents feedback runaway at unity
    static constexpr float kMakeupGainCoeff        = 0.2f;    // self-oscillation loss compensation
    // v0.9: Threshold bypass constants removed — filterEnabled param drives bypass directly

    double currentSampleRate = 44100.0;

    // Main delay buffers (circular, power-of-2 size for bitmask indexing)
    // v0.8: Dual delay lines for stereo input routing
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int delayBufferSize = 0;
    int delayBufferMask = 0;   // v0.5: = delayBufferSize - 1, for & instead of %
    int writePosition   = 0;

    // Feedback state
    float feedbackSample = 0.0f;

    // v1.0.1: Filter management — extracted to FilterBank class
    FilterBank filters;

    // v1.0.6: Phase vocoder pitch shifter — replaces WSOLA-Lite (issue #60)
    PhaseVocoderPitchShifter pvPitchShifters[MAX_OBJECTS];

    // v1.0.2: Doppler via delay modulation — accumulates per-sample delay offset
    // from Doppler velocity. Natural pitch shift without PV artifacts (issue #77).
    float dopplerDelayAccum[MAX_OBJECTS] = {};

    // v1.0.8: Transport-aware PV reset — clears stale phase state on transport
    // stop/start/seek to prevent intermittent buzzing artifacts (issue #65)
    bool wasPlaying = false;
    juce::int64 expectedNextSample = 0;

    // v1.0.6: Transport fade-in — masks delay buffer discontinuity after scrub/seek (issue #103)
    float transportFadeGain = 1.0f;
    float transportFadeStep = 0.0f;  // 1/(0.005*sampleRate), set in prepareToPlay
    bool  transportFadeActive = false;

    // v1.0.1: Thread-safe preset reset — loadPreset() (message thread) stores pending
    // state here; processBlock() (audio thread) applies it, eliminating the data race
    // that caused intermittent WSOLA/Doppler corruption on preset changes (issue #42).
    struct PendingPresetReset {
        float prevAz[MAX_OBJECTS] = {};
        float prevEl[MAX_OBJECTS] = {};
        float prevDist[MAX_OBJECTS] = {};
    };
    PendingPresetReset pendingReset;
    std::atomic<bool>  presetResetPending { false };

    // v1.0.1: Preset transition — fade-out/reconfigure/fade-in (issue #84)
    // Mutes output before resetting state, preventing all transition artifacts.
    enum class PresetTransitionState { Idle, FadeOut, FadeIn };
    PresetTransitionState presetTransitionState { PresetTransitionState::Idle };
    float presetTransitionGain = 1.0f;
    float presetTransitionStep = 0.0f;  // 1/(0.005*sampleRate), set in prepareToPlay

    // v1.0: Per-tap fade envelope for glitch-free enable/disable transitions
    // 64-sample ramp (~1.3ms @ 48kHz) — fast enough to be inaudible, long enough to prevent clicks
    float tapFadeGain[MAX_OBJECTS] = {};
    float tapFadeTarget[MAX_OBJECTS] = {};
    float tapFadeIncrement = 0.0f;  // 1.0f / 64, set in prepareToPlay

    // Block-rate cached conversion factor: ms → samples (set at top of processBlock)
    float blockMsToSamples = 0.0f;

    // v1.0.7: Wobble modulation — 4-layer tape wow/flutter emulation (issue #92)
    float wobblePhases[4] = {};
    juce::SmoothedValue<float> smoothedWobbleAmount;  // v1.0: replaces blockWobbleAmount for click-free onset
    float blockWobbleMorph = 0.0f;
    inline float applyWobble (float baseDelaySamples, float currentDelayMs);

    // Smoothing for delay time to create "Repitch" effect
    juce::LinearSmoothedValue<float> smoothedDelayTime;

    // Smooth loop multiplier — prevents clicks when enabling/disabling objects
    juce::LinearSmoothedValue<float> smoothedLoopMultiplier;

    // v1.0.8: Feedback crossfade for loop multiplier changes (issue #65)
    // When the loop multiplier changes, crossfade the feedback read between old and new
    // positions instead of sweeping — sweeping causes a Doppler chirp.
    float prevLoopMultiplier = 1.0f;
    float fbCrossfadeProgress = 1.0f;  // 1.0 = crossfade complete (use new position only)
    static constexpr float kFbCrossfadeSamples = 1024.0f;  // ~21ms @ 48kHz
    float fbCrossfadeIncrement = 1.0f / kFbCrossfadeSamples;

    //--- SPATIAL FRAMEWORK: Layout & decode state ----------------------------
    float ambiDecodeMatrix[NUM_VIRTUAL_SPEAKERS][HOA_CHANNELS] = {};  // v0.2+ surround decode


    // v0.2: Double-buffered output layout for lock-free format switching
    OutputLayoutState layoutBuffers[2];
    std::atomic<int> activeLayoutIndex { 0 };
    int prepareLayoutIndex = 1;                  // Message thread writes to the non-active buffer
    int maxBusChannels = 2;                      // Set in prepareToPlay()

    juce::dsp::IIR::Filter<float> lfeFilter;     // 120 Hz LP for LFE generation

    //--- SPATIAL FRAMEWORK: Utility DSP (reusable by any SML plugin) ----------
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

    // Output Limiter — tanh-based soft ceiling for speaker protection during self-oscillation
    // C-infinity continuous (no derivative discontinuities), asymptotes to ±threshold
    static float outputLimiter (float x)
    {
        if (! std::isfinite (x))
            return 0.0f;
        const float threshold = 1.2589f;  // +2 dB
        return threshold * std::tanh (x / threshold);
    }

    // Pre-allocated work buffers (avoid allocation in processBlock)
    std::vector<float> monoInputBuffer;
    std::vector<float> inputBufferL;   // v0.8: per-channel input for stereo delay lines
    std::vector<float> inputBufferR;

    // v1.0.7: Dry path latency compensation (issue #63)
    // The phase vocoder adds kFFTSize (2048) samples of latency to the wet path.
    // The dry signal must be delayed by the same amount so the DAW's PDC is correct
    // at all dry/wet settings. Without this, the dry signal arrives 2048 samples early.
    // v1.0.1: Stereo dry buffers — dry path preserves stereo input (issue #73)
    std::vector<float> dryDelayLineL;       // circular buffer L, size = kFFTSize
    std::vector<float> dryDelayLineR;       // circular buffer R, size = kFFTSize
    int dryDelayWritePos = 0;
    std::vector<float> dryCompBufferL;      // pre-filled delayed dry signal L for current block
    std::vector<float> dryCompBufferR;      // pre-filled delayed dry signal R for current block

    // v1.0.1: Pre-computed per-sample dryWet / outputGain for post-render mix (issue #73)
    std::vector<float> perSampleDW;
    std::vector<float> perSampleOutGain;

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
    // v1.0.1: Air absorption state — now managed by FilterBank
    bool airAbsorptionActive = false;       // v0.9: block-rate true bypass (read by processBlock for coefficient path)
    bool prevAirAbsorptionActive = false;   // v0.9: edge detection for AIR toggle state changes

    // v0.5: NFC-HOA — per-order shelf filters for near-field compensation (Ambisonics output only)
    // Applied internally in renderAmbisonicsOutput(), not exposed to user
    static constexpr float NFC_REFERENCE_RADIUS = 1.5f;  // meters (typical studio monitoring distance)
    juce::dsp::IIR::Filter<float> nfcFilters[MAX_OBJECTS][MAX_AMBI_ORDER];  // 12 objects × 6 orders
    float prevNfcDistance[MAX_OBJECTS] = {};
    float smoothedNfcDistance[MAX_OBJECTS] = {};  // v1.0.1: EMA-smoothed to prevent coefficient transients

    // v0.7: Cached max-rE weights (recomputed only when ambi order changes)
    int cachedMaxrEOrder = -1;
    float cachedMaxrE[MAX_AMBI_ORDER + 1] = {};

    // v0.5: Cached per-object parameter pointers (avoid string lookup in processBlock)
    struct CachedObjectParams {
        std::atomic<float>* enabled       = nullptr;
        std::atomic<float>* azimuth       = nullptr;
        std::atomic<float>* elevation     = nullptr;
        std::atomic<float>* distance      = nullptr;
        std::atomic<float>* dopplerAmount = nullptr;
        std::atomic<float>* pitchShift    = nullptr;  // v0.7: per-tap additive pitch
        std::atomic<float>* inputChannel  = nullptr;  // v0.8: L+R/L/R selection
    };
    CachedObjectParams cachedObj[MAX_OBJECTS];

    // Cached global parameter pointers (avoid string lookup in processBlock)
    std::atomic<float>* cachedParam_tempoSync      = nullptr;
    std::atomic<float>* cachedParam_noteDivision    = nullptr;
    std::atomic<float>* cachedParam_filterLP        = nullptr;
    std::atomic<float>* cachedParam_filterHP        = nullptr;
    std::atomic<float>* cachedParam_filterHPQ       = nullptr;
    std::atomic<float>* cachedParam_filterLPQ       = nullptr;
    std::atomic<float>* cachedParam_filterEnabled   = nullptr;
    std::atomic<float>* cachedParam_airAbsorption   = nullptr;
    std::atomic<float>* cachedParam_wobbleEnabled   = nullptr;
    std::atomic<float>* cachedParam_wobbleAmount    = nullptr;
    std::atomic<float>* cachedParam_wobbleMorph     = nullptr;
    std::atomic<float>* cachedParam_delayTime       = nullptr;
    std::atomic<float>* cachedParam_dryWet          = nullptr;
    std::atomic<float>* cachedParam_feedback        = nullptr;
    std::atomic<float>* cachedParam_inputGain       = nullptr;
    std::atomic<float>* cachedParam_outputGain      = nullptr;
    // issue #68: Global tap offset APVTS cached pointers
    std::atomic<float>* cachedParam_globalTapAzimuth   = nullptr;
    std::atomic<float>* cachedParam_globalTapElevation = nullptr;
    std::atomic<float>* cachedParam_globalTapDistance   = nullptr;
    std::atomic<float>* cachedParam_globalTapPitch     = nullptr;
    std::atomic<float>* cachedParam_globalTapDoppler   = nullptr;
    std::atomic<float>* cachedParam_globalTapSpeed     = nullptr;

    // v0.5: Cached feedback filter frequencies + Q (skip recalculation when unchanged)
    // v1.0.1: Filter smoothing/cache state moved to FilterBank class

    // v1.0: Previous-block gains for per-sample interpolation (prevent clicks on rapid position changes)
    float prevStereoGainL[MAX_OBJECTS] = {};
    float prevStereoGainR[MAX_OBJECTS] = {};
    BinauralGains prevBinauralGains[MAX_OBJECTS] = {};
    float prevChannelGains[MAX_OBJECTS][16] = {};
    float prevSHCoeffs[MAX_OBJECTS][MAX_AMBI_CHANNELS] = {};
    float prevDistGain[MAX_OBJECTS] = {};

    // v1.0.1: Doppler velocity tracking — extracted to DopplerVelocity class
    DopplerVelocity doppler;

    // v0.7: Per-tap activity for UI glow (written in processBlock, read by editor timer)
    std::atomic<float> tapActivityRMS[MAX_OBJECTS] = {};
    float tapPeakAccum[MAX_OBJECTS] = {};  // per-block peak accumulator (reset each block)

    //--- v0.9: Preset system (private) — file-based, all presets on disk ------
    int currentPresetIndex = 0;
    std::vector<PresetData> allPresets;  // v1.0: factory from compiled array + user from disk
    std::vector<int> categorizedOrder;   // v0.9: maps sequential position → index into allPresets
    void loadAllPresets();               // v1.0: load factory from compiled-in array + user from disk
    void rebuildCategorizedOrder();
    PresetData captureCurrentState() const;
    // serializePresetToJson() and parsePresetJson() are now free functions in PresetData.h

    //--------------------------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenSpatialDelayProcessor)
};
