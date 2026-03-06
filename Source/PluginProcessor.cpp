#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Binaural profiles: simplified head models for ITD + ILD rendering
//==============================================================================
const std::array<BinauralProfile, 5> OpenSpatialDelayProcessor::binauralProfiles = {{
    { 0.0875f, 1.0f, 1500.0f, "Studio Reference" },   // MIT KEMAR-inspired
    { 0.0920f, 1.3f, 1200.0f, "Immersive" },           // KU100-inspired (wider)
    { 0.0850f, 0.8f, 1800.0f, "Natural" },             // Human subject, subtler
    { 0.0900f, 1.1f, 1400.0f, "Precise" },             // Cross-validated, balanced
    { 0.0875f, 1.5f, 1100.0f, "Spatial" },             // High-res, exaggerated cues
}};

//==============================================================================
// Virtual speaker layout: 9.1.6 standard + Top Center = 16 speakers
// Based on Dolby Atmos / ITU-R BS.2051 9.1.6 bed (LFE excluded)
// Ear level (9) + Top (6) + Zenith (1) = 16
// Convention: 0° = front, positive azimuth = left, negative = right
//==============================================================================
static constexpr float degToRad (float deg) { return deg * juce::MathConstants<float>::pi / 180.0f; }

const std::array<VirtualSpeaker, OpenSpatialDelayProcessor::NUM_VIRTUAL_SPEAKERS>
    OpenSpatialDelayProcessor::virtualSpeakers = {{
    // --- Ear-level ring (9 speakers at elevation 0°) ---
    { degToRad(   0.0f),  degToRad(  0.0f) },  //  0: C   — Centre
    { degToRad(  30.0f),  degToRad(  0.0f) },  //  1: L   — Left
    { degToRad( -30.0f),  degToRad(  0.0f) },  //  2: R   — Right
    { degToRad(  60.0f),  degToRad(  0.0f) },  //  3: Lw  — Left Wide
    { degToRad( -60.0f),  degToRad(  0.0f) },  //  4: Rw  — Right Wide
    { degToRad(  90.0f),  degToRad(  0.0f) },  //  5: Ls  — Left Surround
    { degToRad( -90.0f),  degToRad(  0.0f) },  //  6: Rs  — Right Surround
    { degToRad( 135.0f),  degToRad(  0.0f) },  //  7: Lrs — Left Rear Surround
    { degToRad(-135.0f),  degToRad(  0.0f) },  //  8: Rrs — Right Rear Surround
    // --- Top ring (6 speakers at elevation +45°) ---
    { degToRad(  45.0f),  degToRad( 45.0f) },  //  9: Tfl — Top Front Left
    { degToRad( -45.0f),  degToRad( 45.0f) },  // 10: Tfr — Top Front Right
    { degToRad(  90.0f),  degToRad( 45.0f) },  // 11: Tsl — Top Side Left
    { degToRad( -90.0f),  degToRad( 45.0f) },  // 12: Tsr — Top Side Right
    { degToRad( 135.0f),  degToRad( 45.0f) },  // 13: Trl — Top Rear Left
    { degToRad(-135.0f),  degToRad( 45.0f) },  // 14: Trr — Top Rear Right
    // --- Zenith (1 speaker, keeps count at 16 for square Ambi decode) ---
    { degToRad(   0.0f),  degToRad( 90.0f) },  // 15: T   — Top Centre (zenith)
}};

//==============================================================================
// v0.2: Static speaker layouts for multi-channel output formats
// ITU-R BS.775 / BS.2051 standard positions
// Convention: 0° = front, positive azimuth = left, negative = right
// LFE is tracked but excluded from spatialization
//==============================================================================
static SpeakerLayout makeQuadLayout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 4;
    l.lfeChannelIndex = -1;
    l.totalChannels = 4;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(110.0f), 0.0f, 2 };  // Ls
    l.speakers[3] = { degToRad(-110.0f),0.0f, 3 };  // Rs
    return l;
}

static SpeakerLayout make5_1Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 5;
    l.lfeChannelIndex = 3;   // JUCE 5.1: L R C LFE Ls Rs
    l.totalChannels = 6;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad(110.0f), 0.0f, 4 };  // Ls
    l.speakers[4] = { degToRad(-110.0f),0.0f, 5 };  // Rs
    return l;
}

static SpeakerLayout make7_1Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 7;
    l.lfeChannelIndex = 3;   // JUCE 7.1: L R C LFE Lss Rss Lsr Rsr
    l.totalChannels = 8;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4] = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5] = { degToRad(135.0f), 0.0f, 6 };  // Lsr
    l.speakers[6] = { degToRad(-135.0f),0.0f, 7 };  // Rsr
    return l;
}

static SpeakerLayout make7_1_4Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 11;
    l.lfeChannelIndex = 3;   // JUCE 7.1.4: L R C LFE Lss Rss Lsr Rsr Tfl Tfr Trl Trr
    l.totalChannels = 12;
    // Ear level (7)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };  // Lsr
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };  // Rsr
    // Top (4)
    l.speakers[7]  = { degToRad( 45.0f), degToRad(45.0f), 8 };   // Tfl
    l.speakers[8]  = { degToRad(-45.0f), degToRad(45.0f), 9 };   // Tfr
    l.speakers[9]  = { degToRad(135.0f), degToRad(45.0f), 10 };  // Trl
    l.speakers[10] = { degToRad(-135.0f),degToRad(45.0f), 11 };  // Trr
    return l;
}

static SpeakerLayout make9_1_6Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 15;
    l.lfeChannelIndex = 3;   // JUCE 9.1.6: L R C LFE Lss Rss Lsr Rsr Lw Rw Tfl Tfr Tsl Tsr Trl Trr
    l.totalChannels = 16;
    // Ear level (9)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };   // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };   // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };   // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };   // Lss
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };   // Rss
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };   // Lsr
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };   // Rsr
    l.speakers[7]  = { degToRad( 60.0f), 0.0f, 8 };   // Lw
    l.speakers[8]  = { degToRad(-60.0f), 0.0f, 9 };   // Rw
    // Top (6)
    l.speakers[9]  = { degToRad( 45.0f), degToRad(45.0f), 10 };  // Tfl
    l.speakers[10] = { degToRad(-45.0f), degToRad(45.0f), 11 };  // Tfr
    l.speakers[11] = { degToRad( 90.0f), degToRad(45.0f), 12 };  // Tsl
    l.speakers[12] = { degToRad(-90.0f), degToRad(45.0f), 13 };  // Tsr
    l.speakers[13] = { degToRad(135.0f), degToRad(45.0f), 14 };  // Trl
    l.speakers[14] = { degToRad(-135.0f),degToRad(45.0f), 15 };  // Trr
    return l;
}

//==============================================================================
// Note division ratios (multiplied by beat duration to get delay in ms)
//==============================================================================
static const float noteDivisionRatios[] = {
    4.0f,       // 1/1 (whole note)
    2.0f,       // 1/2
    3.0f,       // 1/2 dotted
    4.0f/3.0f,  // 1/2 triplet
    1.0f,       // 1/4 (quarter note)
    1.5f,       // 1/4 dotted
    2.0f/3.0f,  // 1/4 triplet
    0.5f,       // 1/8
    0.75f,      // 1/8 dotted
    1.0f/3.0f,  // 1/8 triplet
    0.25f,      // 1/16
    0.375f,     // 1/16 dotted
    1.0f/6.0f,  // 1/16 triplet
};

//==============================================================================
// Parameter layout
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
    OpenSpatialDelayProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Global parameters ---------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("delayTime", 1), "Delay Time",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), 500.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 2) + " s";
                return juce::String (juce::roundToInt (value)) + " ms";
            })));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("tempoSync", 1), "Tempo Sync", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("noteDivision", 1), "Note Division",
        juce::NormalisableRange<float> (1.0f, 16.0f, 1.0f), 4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                // value is 1..16 representing number of 16th notes
                // 4 = 1/4 note, 2 = 1/8 note, 1 = 1/16 note, 8 = 1/2 note, 16 = 1/1
                int sixteenths = juce::roundToInt (value);
                if (sixteenths == 16) return juce::String ("1/1");
                if (sixteenths == 8)  return juce::String ("1/2");
                if (sixteenths == 4)  return juce::String ("1/4");
                if (sixteenths == 2)  return juce::String ("1/8");
                if (sixteenths == 1)  return juce::String ("1/16");
                // For irregular values, show x/16
                return juce::String (sixteenths) + "/16";
            })));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("syncMode", 1), "Sync Mode",
        juce::StringArray { "Notes", "Triplet", "Dotted", "16th" }, 0));


    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("feedback", 1), "Feedback",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt(value * 100.0f)) + "%"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLP", 1), "Low-Pass Filter",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 20000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 1) + " kHz";
                return juce::String (juce::roundToInt (value)) + " Hz";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHP", 1), "High-Pass Filter",
        juce::NormalisableRange<float> (20.0f, 5000.0f, 1.0f, 0.3f), 20.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 1) + " kHz";
                return juce::String (juce::roundToInt (value)) + " Hz";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("pitchShift", 1), "Pitch Shift",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " st"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("dryWet", 1), "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt(value * 100.0f)) + "%"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 1), "Input Gain",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f, 2.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " dB"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f, 2.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " dB"; })));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("algorithm", 1), "Algorithm",
        juce::StringArray { "Direct Binaural", "VBAP", "Ambisonics (HOA)", "VBIP", "KNN" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("hrtfProfile", 1), "HRTF Profile",
        juce::StringArray { "Studio Reference", "Immersive", "Natural",
                            "Precise", "Spatial" }, 0));

    // --- Per-object parameters ------------------------------------------------
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto id  = [&](const char* suffix) {
            return juce::ParameterID ("object" + juce::String (i + 1) + "_" + suffix, 1);
        };
        auto name = [&](const char* suffix) {
            return "Object " + juce::String (i + 1) + " " + suffix;
        };

        bool defaultEnabled = (i < 4);
        float defaultTime   = 100.0f * (i + 1);  // 100, 200, 300... ms
        const float defaultAzArray[] = {-45.0f, 45.0f, -135.0f, 135.0f,
                                         0.0f, 90.0f, -90.0f, 180.0f,
                                         -30.0f, 30.0f, -60.0f, 60.0f};
        float defaultAz = defaultAzArray[i];

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            id ("enabled"), name ("Enabled"), defaultEnabled));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("time"), name ("Time"),
            juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), defaultTime));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("azimuth"), name ("Azimuth"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.1f), defaultAz,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 1) + juce::String::charToString (0x00B0); })));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("elevation"), name ("Elevation"),
            juce::NormalisableRange<float> (-90.0f, 90.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 1) + juce::String::charToString (0x00B0); })));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("distance"), name ("Distance"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
// Constructor / Destructor
//==============================================================================
OpenSpatialDelayProcessor::OpenSpatialDelayProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::mono(),   true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // v0.2: Initialize LFE filter (will be configured in prepareToPlay)
}

OpenSpatialDelayProcessor::~OpenSpatialDelayProcessor() {}

//==============================================================================
bool OpenSpatialDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Input can be mono or stereo (stereo will be summed to mono internally)
    auto inputSet = layouts.getMainInputChannelSet();
    if (inputSet != juce::AudioChannelSet::mono() &&
        inputSet != juce::AudioChannelSet::stereo())
        return false;

    // v0.2: Support multiple output formats
    auto outputSet = layouts.getMainOutputChannelSet();
    if (outputSet == juce::AudioChannelSet::stereo())          return true;
    if (outputSet == juce::AudioChannelSet::quadraphonic())    return true;
    if (outputSet == juce::AudioChannelSet::create5point1())   return true;
    if (outputSet == juce::AudioChannelSet::create7point1())   return true;
    if (outputSet == juce::AudioChannelSet::create7point1point4()) return true;
    if (outputSet == juce::AudioChannelSet::create9point1point6()) return true;

    return false;
}

//==============================================================================
// v0.2: Detect output format from channel count
//==============================================================================
OpenSpatialDelayProcessor::OutputFormat
    OpenSpatialDelayProcessor::detectOutputFormat (int numOutputChannels) const
{
    switch (numOutputChannels)
    {
        case 4:  return OutputFormat::Quad;
        case 6:  return OutputFormat::Surround5_1;
        case 8:  return OutputFormat::Surround7_1;
        case 12: return OutputFormat::Surround7_1_4;
        case 16: return OutputFormat::Surround9_1_6;
        default: return OutputFormat::Binaural;  // 2ch or unknown → stereo binaural
    }
}

//==============================================================================
// v0.2: Activate speaker layout for the detected output format
//==============================================================================
void OpenSpatialDelayProcessor::activateLayout (OutputFormat format)
{
    activeOutputFormat = format;

    switch (format)
    {
        case OutputFormat::Quad:          activeLayout = makeQuadLayout();    break;
        case OutputFormat::Surround5_1:   activeLayout = make5_1Layout();    break;
        case OutputFormat::Surround7_1:   activeLayout = make7_1Layout();    break;
        case OutputFormat::Surround7_1_4: activeLayout = make7_1_4Layout();  break;
        case OutputFormat::Surround9_1_6: activeLayout = make9_1_6Layout();  break;
        case OutputFormat::Binaural:
        default:
            activeLayout = {};
            activeLayout.numSpeakers = 0;
            activeLayout.lfeChannelIndex = -1;
            activeLayout.totalChannels = 2;
            return;  // No layout-specific processing needed for binaural
    }

    discreteAmbiNumSpeakers = activeLayout.numSpeakers;

    // --- Compute Ambisonics decode matrix for this layout ---
    // D = E^T (E E^T + epsilon I)^{-1}  (Tikhonov-regularized pseudo-inverse)
    const int N = activeLayout.numSpeakers;
    const int M = HOA_CHANNELS;

    // Build encoding matrix E[c][s] = evalSH(c, speaker_s_position)
    float E[16][16] = {};
    for (int s = 0; s < N; ++s)
        for (int c = 0; c < M; ++c)
            E[c][s] = evalSH (c, activeLayout.speakers[s].azimuthRad,
                                 activeLayout.speakers[s].elevationRad);

    // Compute EET = E * E^T  (M × M)
    float EET[16][16] = {};
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < M; ++j)
        {
            float sum = 0.0f;
            for (int s = 0; s < N; ++s)
                sum += E[i][s] * E[j][s];
            EET[i][j] = sum;
        }

    // Tikhonov regularization: EET += epsilon * I
    float epsilon = 0.01f;
    for (int i = 0; i < M; ++i)
        EET[i][i] += epsilon;

    // Invert EET via Gauss-Jordan (M × M, small matrix)
    float inv[16][16] = {};
    for (int i = 0; i < M; ++i)
        inv[i][i] = 1.0f;

    float aug[16][16];
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < M; ++j)
            aug[i][j] = EET[i][j];

    for (int col = 0; col < M; ++col)
    {
        // Find pivot
        int pivot = col;
        for (int row = col + 1; row < M; ++row)
            if (std::abs (aug[row][col]) > std::abs (aug[pivot][col]))
                pivot = row;

        if (pivot != col)
        {
            std::swap_ranges (aug[col], aug[col] + M, aug[pivot]);
            std::swap_ranges (inv[col], inv[col] + M, inv[pivot]);
        }

        float diagVal = aug[col][col];
        if (std::abs (diagVal) < 1e-10f) continue;

        for (int j = 0; j < M; ++j)
        {
            aug[col][j] /= diagVal;
            inv[col][j] /= diagVal;
        }

        for (int row = 0; row < M; ++row)
        {
            if (row == col) continue;
            float factor = aug[row][col];
            for (int j = 0; j < M; ++j)
            {
                aug[row][j] -= factor * aug[col][j];
                inv[row][j] -= factor * inv[col][j];
            }
        }
    }

    // D[s][c] = sum_k E^T[s][k] * inv[k][c] = sum_k E[k][s] * inv[k][c]
    for (int s = 0; s < N; ++s)
        for (int c = 0; c < M; ++c)
        {
            float sum = 0.0f;
            for (int k = 0; k < M; ++k)
                sum += E[k][s] * inv[k][c];
            discreteAmbiDecodeMatrix[s][c] = sum;
        }

    // --- Build 3D VBAP triplets for height layouts ---
    layoutVBAPTriplets.clear();

    if (format == OutputFormat::Surround7_1_4 || format == OutputFormat::Surround9_1_6)
    {
        // Build triplets from layout speakers using same Delaunay approach as virtual speakers
        // For simplicity, use brute-force: test all triplets, keep those whose inverse exists
        // and that contain the source direction (validated at runtime)
        for (int a = 0; a < N - 2; ++a)
        {
            for (int b = a + 1; b < N - 1; ++b)
            {
                for (int cc = b + 1; cc < N; ++cc)
                {
                    auto toCart = [](float az, float el) -> std::array<float, 3> {
                        return { std::cos(el) * std::sin(az),
                                 std::cos(el) * std::cos(az),
                                 std::sin(el) };
                    };

                    auto ca = toCart (activeLayout.speakers[a].azimuthRad, activeLayout.speakers[a].elevationRad);
                    auto cb = toCart (activeLayout.speakers[b].azimuthRad, activeLayout.speakers[b].elevationRad);
                    auto ccc = toCart (activeLayout.speakers[cc].azimuthRad, activeLayout.speakers[cc].elevationRad);

                    // 3×3 matrix [ca cb ccc]
                    float m[3][3] = {
                        { ca[0], cb[0], ccc[0] },
                        { ca[1], cb[1], ccc[1] },
                        { ca[2], cb[2], ccc[2] }
                    };

                    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                              - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                              + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

                    if (std::abs (det) < 0.01f)
                        continue;  // Degenerate triplet

                    float invDet = 1.0f / det;
                    VBAPTriplet t;
                    t.i = a;
                    t.j = b;
                    t.k = cc;
                    t.inv[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
                    t.inv[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
                    t.inv[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
                    t.inv[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
                    t.inv[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
                    t.inv[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;
                    t.inv[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
                    t.inv[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
                    t.inv[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

                    layoutVBAPTriplets.push_back (t);
                }
            }
        }
    }
}

//==============================================================================
void OpenSpatialDelayProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Allocate delay buffer (24 seconds max: 12 objects × 2s)
    delayBufferSize = static_cast<int> (sampleRate * MAX_DELAY_SECONDS) + 1;
    delayBuffer.assign (delayBufferSize, 0.0f);
    writePosition = 0;
    feedbackSample = 0.0f;
    
    // Reset all pitch shifter phases (including the feedback shifter at index MAX_OBJECTS)
    for (int i = 0; i < MAX_OBJECTS + 1; ++i)
        pitchPhase[i] = 0.0f;
    
    feedbackPitchPhase = 0.0f;

    // Pre-allocate mono input buffer
    monoInputBuffer.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Initialize filters
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    feedbackLPFilter.prepare (spec);
    feedbackHPFilter.prepare (spec);
    feedbackLPFilter.reset();
    feedbackHPFilter.reset();

    // Initialize smoothed values
    smoothedDryWet.reset     (sampleRate, 0.02);
    smoothedFeedback.reset   (sampleRate, 0.02);
    smoothedInputGain.reset  (sampleRate, 0.02);
    smoothedOutputGain.reset (sampleRate, 0.02);
    
    // Slower smoothing for delay time to create audible pitch bend (100ms)
    smoothedDelayTime.reset  (sampleRate, 0.1);

    // Smooth loop multiplier — 50ms ramp prevents clicks when enabling/disabling objects
    smoothedLoopMultiplier.reset (sampleRate, 0.05);
    smoothedLoopMultiplier.setCurrentAndTargetValue (1.0f);

    // Initialize modular 3D audio core
    cachedProfileIndex = -1;          // Force VBAP binaural cache re-computation
    cachedBinauralProfileIndex = -1;  // Force Ambisonics SH weights re-computation
    computeAmbiDecodeMatrix();        // Pre-compute 3rd-order decode matrix for virtual speakers

    // v0.2: Detect output format and activate speaker layout
    auto format = detectOutputFormat (getTotalNumOutputChannels());
    activateLayout (format);

    // v0.2: Configure LFE low-pass filter (120 Hz, 2nd order Butterworth)
    lfeFilter.prepare (spec);
    lfeFilter.reset();
    *lfeFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 120.0f);
}

void OpenSpatialDelayProcessor::releaseResources()
{
    delayBuffer.clear();
    monoInputBuffer.clear();
}

//==============================================================================
// Delay line helpers
//==============================================================================
void OpenSpatialDelayProcessor::writeDelayLine (float sample)
{
    delayBuffer[writePosition] = sample;
    writePosition = (writePosition + 1) % delayBufferSize;
}

float OpenSpatialDelayProcessor::readDelayLine (float delaySamples) const
{
    // readPos is floating point index relative to write head
    float readPos = static_cast<float> (writePosition) - delaySamples - 1.0f;
    
    // Handle wrap-around
    const float bufSize = static_cast<float>(delayBufferSize);
    while (readPos < 0.0f) readPos += bufSize;
    readPos = std::fmod(readPos, bufSize);

    int   i1 = static_cast<int> (readPos);
    float f  = readPos - static_cast<float>(i1);

    // Get 4 points for Cubic Hermite Interpolation
    int i0 = (i1 - 1 + delayBufferSize) % delayBufferSize;
    int i2 = (i1 + 1) % delayBufferSize;
    int i3 = (i1 + 2) % delayBufferSize;

    float y0 = delayBuffer[static_cast<size_t>(i0)];
    float y1 = delayBuffer[static_cast<size_t>(i1)];
    float y2 = delayBuffer[static_cast<size_t>(i2)];
    float y3 = delayBuffer[static_cast<size_t>(i3)];

    // Cubic Hermite spline interpolation (Catmull-Rom variant)
    // This significantly reduces high-frequency roll-off compared to linear interpolation,
    // which is essential for preserving the brightness required for self-oscillation.
    return y1 + 0.5f * f * (y2 - y0 + f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3 + f * (3.0f * (y1 - y2) + y3 - y0)));
}

//==============================================================================
// High-Fidelity Dual-Head Pitch Shifter
// Uses two large drifting read-heads with sinusoidal crossfading.
// Much more transparent than granular methods for recursive delay paths.
//==============================================================================
float OpenSpatialDelayProcessor::readPitchShifted (float delaySamples,
                                                     float semitones, int phaseIndex)
{
    // Bypass if negligible pitch shift
    if (std::abs (semitones) < 0.005f)
        return readDelayLine (delaySamples);

    // Use a large 80ms window for high-fidelity shifting
    const float windowSamples = static_cast<float>(currentSampleRate) * 0.080f;
    const float ratio = std::pow (2.0f, semitones / 12.0f);
    
    float& phase = pitchPhase[phaseIndex];
    
    // Head 1 (clamp defensively in case of floating-point drift)
    float p1 = phase;
    if (p1 < 0.0f || p1 >= windowSamples)
    {
        p1 = std::fmod (p1, windowSamples);
        if (p1 < 0.0f) p1 += windowSamples;
    }
    // Head 2 (offset by exactly half the window)
    float p2 = std::fmod (phase + windowSamples * 0.5f, windowSamples);
    
    // Sinusoidal/Hann crossfade window (ensures constant power and unity gain)
    const float twoPi = 2.0f * juce::MathConstants<float>::pi;
    float gain1 = 0.5f - 0.5f * std::cos (twoPi * p1 / windowSamples);
    float gain2 = 1.0f - gain1; 

    // Read from the delay line using the cubic interpolator
    float s1 = readDelayLine (delaySamples + p1);
    float s2 = readDelayLine (delaySamples + p2);

    // Advance phase based on the drift required for the specific pitch ratio
    // This allows the read head to smoothly move relative to the write head.
    phase += (1.0f - ratio);
    phase = std::fmod (phase, windowSamples);
    if (phase < 0.0f) phase += windowSamples;

    return s1 * gain1 + s2 * gain2;
}

//==============================================================================
// Binaural rendering (simplified physical model: ITD + ILD)
//==============================================================================
BinauralGains OpenSpatialDelayProcessor::computeBinauralGains (
    float azimuthRad, float elevationRad, float distance, int profileIndex) const
{
    const auto& profile = binauralProfiles[juce::jlimit (0, 4, profileIndex)];

    // Effective lateral angle (azimuth projected by elevation)
    float sinAz  = std::sin (azimuthRad);
    float cosEl  = std::cos (elevationRad);
    float lateral = sinAz * cosEl;  // effective sine of lateral angle

    // Woodworth ITD model: t = (r/c) * (sin(theta) + theta)
    // Simplified: use lateral angle directly
    float itdSeconds = (profile.headRadius / 343.0f)
                     * (std::abs (lateral) + std::asin (std::abs (lateral)));
    float itdSamples = itdSeconds * static_cast<float> (currentSampleRate);

    // ILD: frequency-dependent, but simplified to a broadband gain difference
    // Empirical: ~1.5 dB per 10 degrees for low frequencies, more for high
    float ildDb = profile.ildScale * 8.0f * std::abs (lateral); // up to ~8 dB at 90°
    float farEarGain = juce::Decibels::decibelsToGain (-ildDb);

    // Distance attenuation (inverse-distance law, clamped)
    float distGain = 1.0f / std::max (0.1f, distance * 4.0f + 0.25f);

    BinauralGains gains;

    if (lateral >= 0.0f)  // Source on the LEFT (positive azimuth = left per ADM)
    {
        gains.leftGain          = distGain;
        gains.rightGain         = distGain * farEarGain;
        gains.leftDelaySamples  = 0.0f;
        gains.rightDelaySamples = itdSamples;
    }
    else  // Source on the RIGHT
    {
        gains.leftGain          = distGain * farEarGain;
        gains.rightGain         = distGain;
        gains.leftDelaySamples  = itdSamples;
        gains.rightDelaySamples = 0.0f;
    }

    return gains;
}

//==============================================================================
// Tempo sync
//==============================================================================
float OpenSpatialDelayProcessor::getTempoSyncedDelayMs (int noteDivisionIndex) const
{
    auto playHead = getPlayHead();
    if (playHead == nullptr)
        return 500.0f;

    auto posInfo = playHead->getPosition();
    if (! posInfo.hasValue() || ! posInfo->getBpm().hasValue())
        return 500.0f;

    double bpm = *posInfo->getBpm();
    if (bpm <= 0.0)
        bpm = 120.0;

    // Unit is 1/16th note in milliseconds
    double sixteenthMs = (60000.0 / bpm) / 4.0;

    // Echo knob value (noteDivisionIndex) represents number of 16th units
    double duration = (double)noteDivisionIndex * sixteenthMs;

    // Apply mode multiplier
    int mode = static_cast<int> (apvts.getRawParameterValue ("syncMode")->load());
    if (mode == 1) // Triplet
        duration *= (2.0 / 3.0);
    else if (mode == 2) // Dotted
        duration *= 1.5;
    // Notes and 16th use face value (1.0x)

    return static_cast<float> (duration);
}

//==============================================================================
// Modular 3D Audio Core — VBAP 3D (Pulkki 1997)
//==============================================================================

// Helper: convert spherical to Cartesian unit vector
static inline void sphericalToCartesian (float azRad, float elRad,
                                          float& x, float& y, float& z)
{
    float cosEl = std::cos (elRad);
    x = std::cos (azRad) * cosEl;   // front/back
    y = std::sin (azRad) * cosEl;   // left/right
    z = std::sin (elRad);           // up/down
}

// Helper: 3x3 matrix inverse, returns false if singular
static bool invert3x3 (const float m[3][3], float inv[3][3])
{
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
              - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
              + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (std::abs (det) < 1e-10f)
        return false;

    float invDet = 1.0f / det;
    inv[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    inv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet;
    inv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
    inv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet;
    inv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    inv[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet;
    inv[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    inv[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet;
    inv[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
    return true;
}

// Pre-computed VBAP triangulation of 16 virtual speakers on the unit sphere
// Layout: 0-8 ear-level (9.1.6), 9-14 top ring (+45° el), 15 zenith (+90°)
// Triangulation covers the upper hemisphere only (sources below horizon are
// projected up); the 9 ear-level speakers provide full 360° horizontal coverage.
const std::vector<VBAPTriplet>& OpenSpatialDelayProcessor::getVBAPTriplets()
{
    static std::vector<VBAPTriplet> triplets;
    static bool initialized = false;

    if (initialized)
        return triplets;

    // Speaker indices for reference:
    //  0: C (0°)      1: L (30°)      2: R (-30°)
    //  3: Lw (60°)    4: Rw (-60°)    5: Ls (90°)
    //  6: Rs (-90°)   7: Lrs (135°)   8: Rrs (-135°)
    //  9: Tfl (45°)  10: Tfr (-45°)  11: Tsl (90°)
    // 12: Tsr (-90°) 13: Trl (135°)  14: Trr (-135°)
    // 15: T (zenith)

    // Triangulation connectivity:
    // Tier 1: Ear-level ring to top ring (connect adjacent ear-level pairs
    //         to the top speaker that sits above the arc between them)
    // Tier 2: Top ring to zenith (6 triangles forming the dome cap)
    const int tris[][3] = {
        // --- Tier 1: Ear-level ↔ Top ring ---
        // Front sector
        { 0,  1,  9 },  // C + L → Tfl
        { 0,  2, 10 },  // C + R → Tfr
        { 1,  3,  9 },  // L + Lw → Tfl
        { 2,  4, 10 },  // R + Rw → Tfr
        // Side sector
        { 3,  5,  9 },  // Lw + Ls → Tfl (Tfl bridges front-to-side left)
        { 4,  6, 10 },  // Rw + Rs → Tfr (Tfr bridges front-to-side right)
        { 5,  7, 11 },  // Ls + Lrs → Tsl
        { 6,  8, 12 },  // Rs + Rrs → Tsr
        // Rear sector
        { 7,  8, 13 },  // Lrs + Rrs → Trl (using Trl at 135°)
        { 7,  8, 14 },  // Lrs + Rrs → Trr (using Trr at -135°)
        // Bridging: connect top speakers to each other through ear-level ring
        { 5,  9, 11 },  // Ls + Tfl + Tsl
        { 6, 10, 12 },  // Rs + Tfr + Tsr
        { 7, 11, 13 },  // Lrs + Tsl + Trl
        { 8, 12, 14 },  // Rrs + Tsr + Trr
        // Front bridging between Tfl/Tfr through C
        { 0,  9, 10 },  // C + Tfl + Tfr
        // Rear bridging between Trl/Trr through rear ear-level
        { 7, 13, 14 },  // Lrs + Trl + Trr  (left rear bridge)
        { 8, 13, 14 },  // Rrs + Trl + Trr  (right rear bridge)

        // --- Tier 2: Top ring → Zenith (dome cap) ---
        {  9, 10, 15 }, // Tfl + Tfr + T  (front cap)
        {  9, 11, 15 }, // Tfl + Tsl + T  (front-left cap)
        { 10, 12, 15 }, // Tfr + Tsr + T  (front-right cap)
        { 11, 13, 15 }, // Tsl + Trl + T  (rear-left cap)
        { 12, 14, 15 }, // Tsr + Trr + T  (rear-right cap)
        { 13, 14, 15 }, // Trl + Trr + T  (rear cap)

        // --- Below-horizon fallback triangles ---
        // For sources below the ear plane, these triangles between adjacent
        // ear-level speakers provide panning across the lower hemisphere.
        // (The signal is effectively "projected" into the ear-level ring.)
        { 0,  1,  2 },  // C + L + R (front)
        { 1,  2,  3 },  // L + R + Lw
        { 2,  3,  4 },  // R + Lw + Rw
        { 3,  4,  5 },  // Lw + Rw + Ls
        { 4,  5,  6 },  // Rw + Ls + Rs
        { 5,  6,  7 },  // Ls + Rs + Lrs
        { 6,  7,  8 },  // Rs + Lrs + Rrs
        { 0,  7,  8 },  // C + Lrs + Rrs (rear wrap — through C for full coverage)
        { 0,  1,  7 },  // C + L + Lrs (left rear quadrant)
        { 0,  2,  8 },  // C + R + Rrs (right rear quadrant)
    };

    constexpr int numTris = sizeof(tris) / sizeof(tris[0]);

    for (int t = 0; t < numTris; ++t)
    {
        VBAPTriplet tri;
        tri.i = tris[t][0];
        tri.j = tris[t][1];
        tri.k = tris[t][2];

        // Build 3x3 matrix of speaker direction vectors
        float xi, yi, zi, xj, yj, zj, xk, yk, zk;
        sphericalToCartesian (virtualSpeakers[tri.i].azimuthRad,
                              virtualSpeakers[tri.i].elevationRad, xi, yi, zi);
        sphericalToCartesian (virtualSpeakers[tri.j].azimuthRad,
                              virtualSpeakers[tri.j].elevationRad, xj, yj, zj);
        sphericalToCartesian (virtualSpeakers[tri.k].azimuthRad,
                              virtualSpeakers[tri.k].elevationRad, xk, yk, zk);

        // Matrix L = [spk_i | spk_j | spk_k] as rows for g = L^-1 * p
        float L[3][3] = {
            { xi, yi, zi },
            { xj, yj, zj },
            { xk, yk, zk }
        };

        if (invert3x3 (L, tri.inv))
            triplets.push_back (tri);
    }

    initialized = true;
    return triplets;
}

void OpenSpatialDelayProcessor::computeVBAPGains (float azimuthRad, float elevationRad,
                                                    float* outGains) const
{
    // Zero all gains
    for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
        outGains[s] = 0.0f;

    // Source direction as unit Cartesian vector
    float px, py, pz;
    sphericalToCartesian (azimuthRad, elevationRad, px, py, pz);

    const auto& triplets = getVBAPTriplets();
    float bestGainSum = -1.0f;
    int bestTri = -1;
    float bestG[3] = {};

    for (int t = 0; t < static_cast<int> (triplets.size()); ++t)
    {
        const auto& tri = triplets[t];

        // g = inv * p (matrix-vector multiply)
        float g0 = tri.inv[0][0] * px + tri.inv[0][1] * py + tri.inv[0][2] * pz;
        float g1 = tri.inv[1][0] * px + tri.inv[1][1] * py + tri.inv[1][2] * pz;
        float g2 = tri.inv[2][0] * px + tri.inv[2][1] * py + tri.inv[2][2] * pz;

        // Source is inside this triangle if all gains are non-negative
        if (g0 >= -1e-6f && g1 >= -1e-6f && g2 >= -1e-6f)
        {
            float sum = g0 + g1 + g2;
            if (sum > bestGainSum)
            {
                bestGainSum = sum;
                bestTri = t;
                bestG[0] = std::max (0.0f, g0);
                bestG[1] = std::max (0.0f, g1);
                bestG[2] = std::max (0.0f, g2);
            }
        }
    }

    if (bestTri >= 0)
    {
        // Constant-power normalization
        float power = bestG[0] * bestG[0] + bestG[1] * bestG[1] + bestG[2] * bestG[2];
        float scale = (power > 1e-12f) ? (1.0f / std::sqrt (power)) : 0.0f;

        outGains[triplets[bestTri].i] = bestG[0] * scale;
        outGains[triplets[bestTri].j] = bestG[1] * scale;
        outGains[triplets[bestTri].k] = bestG[2] * scale;
    }
    else
    {
        // Fallback: nearest speaker (should not happen with correct triangulation)
        float bestDot = -2.0f;
        int bestSpeaker = 0;
        for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
        {
            float sx, sy, sz;
            sphericalToCartesian (virtualSpeakers[s].azimuthRad,
                                  virtualSpeakers[s].elevationRad, sx, sy, sz);
            float dot = px * sx + py * sy + pz * sz;
            if (dot > bestDot)
            {
                bestDot = dot;
                bestSpeaker = s;
            }
        }
        outGains[bestSpeaker] = 1.0f;
    }
}

//==============================================================================
// Modular 3D Audio Core — 3rd-Order Ambisonics (ACN/SN3D)
//==============================================================================

float OpenSpatialDelayProcessor::evalSH (int acn, float az, float el)
{
    // Real spherical harmonics, ACN ordering, SN3D normalization
    // az = azimuth (radians), el = elevation (radians)
    float cosAz  = std::cos (az);
    float sinAz  = std::sin (az);
    float cos2Az = std::cos (2.0f * az);
    float sin2Az = std::sin (2.0f * az);
    float cos3Az = std::cos (3.0f * az);
    float sin3Az = std::sin (3.0f * az);
    float sinEl  = std::sin (el);
    float cosEl  = std::cos (el);
    float sinEl2 = sinEl * sinEl;
    float cosEl2 = cosEl * cosEl;

    switch (acn)
    {
        // Order 0
        case 0: return 1.0f;

        // Order 1 (SN3D: no extra factor needed)
        case 1: return sinAz * cosEl;              // Y1^-1
        case 2: return sinEl;                       // Y1^0
        case 3: return cosAz * cosEl;              // Y1^1

        // Order 2 (SN3D normalization)
        case 4: return std::sqrt (3.0f) * 0.5f * sin2Az * cosEl2;                     // Y2^-2 (SN3D: √3/2)
        case 5: return std::sqrt (3.0f) * sinAz * sinEl * cosEl;                      // Y2^-1 (SN3D: √3) [= √3 * sinAz * sin(2el)/2 simplified]
        case 6: return 0.5f * (3.0f * sinEl2 - 1.0f);                                 // Y2^0
        case 7: return std::sqrt (3.0f) * cosAz * sinEl * cosEl;                      // Y2^1
        case 8: return std::sqrt (3.0f) * 0.5f * cos2Az * cosEl2;                     // Y2^2 (SN3D: √3/2)

        // Order 3 (SN3D normalization)
        case  9: return std::sqrt (5.0f / 8.0f) * sin3Az * cosEl * cosEl2;            // Y3^-3
        case 10: return std::sqrt (15.0f) * 0.5f * sin2Az * sinEl * cosEl2;           // Y3^-2 (SN3D: √15/2)
        case 11: return std::sqrt (3.0f / 8.0f) * sinAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^-1
        case 12: return 0.5f * sinEl * (5.0f * sinEl2 - 3.0f);                        // Y3^0
        case 13: return std::sqrt (3.0f / 8.0f) * cosAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^1
        case 14: return std::sqrt (15.0f) * 0.5f * cos2Az * sinEl * cosEl2;           // Y3^2 (SN3D: √15/2)
        case 15: return std::sqrt (5.0f / 8.0f) * cos3Az * cosEl * cosEl2;            // Y3^3

        default: return 0.0f;
    }
}

void OpenSpatialDelayProcessor::computeAmbiDecodeMatrix()
{
    // Build the encoding matrix M[channel][speaker] where M[c][s] = Y_c(speaker_s)
    // Mode-matching decode: D = M^{-1}, so gain = D · coeffs gives correct speaker gains
    // Note: M has channels as rows, speakers as columns (standard Ambisonics convention)
    float M[HOA_CHANNELS][NUM_VIRTUAL_SPEAKERS];

    for (int c = 0; c < HOA_CHANNELS; ++c)
        for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
            M[c][s] = evalSH (c, virtualSpeakers[s].azimuthRad,
                                  virtualSpeakers[s].elevationRad);

    // Compute D = M^{-1} via Gauss-Jordan elimination on [M | I]
    // Since we have 16 channels and 16 speakers, M is square (16x16)
    float augmented[16][32];

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            augmented[i][j]      = M[i][j];
            augmented[i][j + 16] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Forward elimination with partial pivoting
    for (int col = 0; col < 16; ++col)
    {
        // Find pivot
        int maxRow = col;
        float maxVal = std::abs (augmented[col][col]);
        for (int row = col + 1; row < 16; ++row)
        {
            if (std::abs (augmented[row][col]) > maxVal)
            {
                maxVal = std::abs (augmented[row][col]);
                maxRow = row;
            }
        }

        // Swap rows
        if (maxRow != col)
            for (int j = 0; j < 32; ++j)
                std::swap (augmented[col][j], augmented[maxRow][j]);

        // Scale pivot row
        float pivot = augmented[col][col];
        if (std::abs (pivot) < 1e-10f)
            continue; // Singular — skip (should not happen with our layout)

        float invPivot = 1.0f / pivot;
        for (int j = 0; j < 32; ++j)
            augmented[col][j] *= invPivot;

        // Eliminate column in other rows
        for (int row = 0; row < 16; ++row)
        {
            if (row == col) continue;
            float factor = augmented[row][col];
            for (int j = 0; j < 32; ++j)
                augmented[row][j] -= factor * augmented[col][j];
        }
    }

    // Extract D = M^{-1} from right half of augmented matrix
    // D[s][c] = M^{-1}[s][c] → gain[s] = sum_c D[s][c] * coeffs[c]
    for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
        for (int c = 0; c < HOA_CHANNELS; ++c)
            ambiDecodeMatrix[s][c] = augmented[s][c + 16];
}

void OpenSpatialDelayProcessor::computeAmbiSpeakerGains (float azimuthRad, float elevationRad,
                                                          float* outGains) const
{
    // Max-rE weights per order for 3rd-order Ambisonics
    // These attenuate higher orders to maximize energy concentration of the decode,
    // critical for non-uniform layouts (our 9.1.6 has no below-horizon speakers).
    // Values: cos(order * π / (2*(N+1))) where N = HOA_ORDER = 3 (Zotter & Frank 2012)
    static const float maxrE[4] = {
        1.0f,                                                     // order 0
        std::cos (juce::MathConstants<float>::pi / 8.0f),         // order 1: cos(π/8) ≈ 0.924
        std::cos (2.0f * juce::MathConstants<float>::pi / 8.0f),  // order 2: cos(π/4) ≈ 0.707
        std::cos (3.0f * juce::MathConstants<float>::pi / 8.0f),  // order 3: cos(3π/8) ≈ 0.383
    };

    // Map ACN index to order: 0→0, 1-3→1, 4-8→2, 9-15→3
    auto acnToOrder = [](int acn) -> int {
        if (acn < 1) return 0;
        if (acn < 4) return 1;
        if (acn < 9) return 2;
        return 3;
    };

    // Step 1: Encode source direction into 16 ACN/SN3D coefficients with max-rE weighting
    float coeffs[HOA_CHANNELS];
    for (int c = 0; c < HOA_CHANNELS; ++c)
        coeffs[c] = evalSH (c, azimuthRad, elevationRad) * maxrE[acnToOrder (c)];

    // Step 2: Decode via matrix multiply: gain[s] = sum_c D[s][c] * coeffs[c]
    float totalPower = 0.0f;
    for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
    {
        float gain = 0.0f;
        for (int c = 0; c < HOA_CHANNELS; ++c)
            gain += ambiDecodeMatrix[s][c] * coeffs[c];

        // Clamp negative gains to zero (negative = out-of-phase artefact)
        outGains[s] = std::max (0.0f, gain);
        totalPower += outGains[s] * outGains[s];
    }

    // Step 3: Constant-power normalization for consistent loudness
    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
            outGains[s] *= scale;
    }
}

//==============================================================================
// Direct Ambisonics-to-Binaural: compute SH decode weights via sphere sampling
// Projects the ILD binaural model onto the SH basis using Fibonacci sphere
//==============================================================================
void OpenSpatialDelayProcessor::computeBinauralSHWeights (int profileIndex)
{
    if (profileIndex == cachedBinauralProfileIndex)
        return;

    const auto& profile = binauralProfiles[juce::jlimit (0, 4, profileIndex)];

    // Clear accumulators
    for (int c = 0; c < HOA_CHANNELS; ++c)
    {
        binauralSHWeightsL[c] = 0.0f;
        binauralSHWeightsR[c] = 0.0f;
    }

    // Fibonacci sphere: ~2000 near-uniform samples on the unit sphere
    constexpr int N = 2000;
    const float goldenRatio = (1.0f + std::sqrt (5.0f)) * 0.5f;
    const float goldenAngle = 2.0f * juce::MathConstants<float>::pi / (goldenRatio * goldenRatio);

    for (int i = 0; i < N; ++i)
    {
        // Fibonacci sphere sampling: sinEl distributed uniformly in [-1, 1]
        float sinEl = 1.0f - (2.0f * static_cast<float>(i) + 1.0f) / static_cast<float>(N);
        float el = std::asin (sinEl);
        float az = goldenAngle * static_cast<float>(i);
        // Wrap azimuth to [-π, π]
        az = std::fmod (az, 2.0f * juce::MathConstants<float>::pi);
        if (az > juce::MathConstants<float>::pi) az -= 2.0f * juce::MathConstants<float>::pi;

        // Compute ILD-only binaural model at this direction (unit distance, no ITD)
        float lateral = std::sin (az) * std::cos (el);
        float ildDb = profile.ildScale * 8.0f * std::abs (lateral);
        float farEarGain = juce::Decibels::decibelsToGain (-ildDb);

        float hL, hR;
        if (lateral >= 0.0f)  // source on left
        {
            hL = 1.0f;
            hR = farEarGain;
        }
        else  // source on right
        {
            hL = farEarGain;
            hR = 1.0f;
        }

        // Accumulate SH projection: weights[c] += Y_c(dir) * H(dir)
        for (int c = 0; c < HOA_CHANNELS; ++c)
        {
            float sh = evalSH (c, az, el);
            binauralSHWeightsL[c] += sh * hL;
            binauralSHWeightsR[c] += sh * hR;
        }
    }

    // Normalize by (4π / N) — the quadrature weight for uniform sphere sampling
    float norm = 4.0f * juce::MathConstants<float>::pi / static_cast<float>(N);
    for (int c = 0; c < HOA_CHANNELS; ++c)
    {
        binauralSHWeightsL[c] *= norm;
        binauralSHWeightsR[c] *= norm;
    }

    cachedBinauralProfileIndex = profileIndex;
}

//==============================================================================
// Direct Ambisonics-to-Binaural: encode source to SH, decode directly to L/R
//==============================================================================
BinauralGains OpenSpatialDelayProcessor::computeAmbiBinauralGains (
    float azimuthRad, float elevationRad, float distance, int profileIndex) const
{
    const auto& profile = binauralProfiles[juce::jlimit (0, 4, profileIndex)];

    // Max-rE weights per order (same as computeAmbiSpeakerGains)
    static const float maxrE[4] = {
        1.0f,
        std::cos (juce::MathConstants<float>::pi / 8.0f),
        std::cos (2.0f * juce::MathConstants<float>::pi / 8.0f),
        std::cos (3.0f * juce::MathConstants<float>::pi / 8.0f),
    };
    auto acnToOrder = [](int acn) -> int {
        if (acn < 1) return 0;
        if (acn < 4) return 1;
        if (acn < 9) return 2;
        return 3;
    };

    // Step 1: SH Encode with max-rE weighting
    float coeffs[HOA_CHANNELS];
    for (int c = 0; c < HOA_CHANNELS; ++c)
        coeffs[c] = evalSH (c, azimuthRad, elevationRad) * maxrE[acnToOrder (c)];

    // Step 2: ILD via SH decode — dot product with pre-computed binaural weights
    float rawL = 0.0f, rawR = 0.0f;
    for (int c = 0; c < HOA_CHANNELS; ++c)
    {
        rawL += coeffs[c] * binauralSHWeightsL[c];
        rawR += coeffs[c] * binauralSHWeightsR[c];
    }

    // Clamp to non-negative (SH reconstruction can produce small negatives)
    rawL = std::max (0.0f, rawL);
    rawR = std::max (0.0f, rawR);

    // Constant-power normalization to preserve energy
    float power = rawL * rawL + rawR * rawR;
    if (power > 1e-12f)
    {
        float scale = std::sqrt (2.0f) / std::sqrt (power);  // √2 so centered source gives ~1.0 per ear
        rawL *= scale;
        rawR *= scale;
    }

    // Step 3: ITD from first-order direction extraction
    // ACN 1 = sinAz * cosEl = the lateral component — identical to computeBinauralGains
    float lateral = coeffs[1] / std::max (0.001f, maxrE[1]);  // undo max-rE to get true lateral
    float itdSeconds = (profile.headRadius / 343.0f)
                     * (std::abs (lateral) + std::asin (juce::jlimit (-1.0f, 1.0f, std::abs (lateral))));
    float itdSamples = itdSeconds * static_cast<float> (currentSampleRate);

    // Step 4: Distance attenuation (same formula as computeBinauralGains)
    float distGain = 1.0f / std::max (0.1f, distance * 4.0f + 0.25f);

    BinauralGains gains;
    gains.leftGain  = rawL * distGain;
    gains.rightGain = rawR * distGain;

    if (lateral >= 0.0f)  // source on left
    {
        gains.leftDelaySamples  = 0.0f;
        gains.rightDelaySamples = itdSamples;
    }
    else  // source on right
    {
        gains.leftDelaySamples  = itdSamples;
        gains.rightDelaySamples = 0.0f;
    }

    return gains;
}

//==============================================================================
// v0.2: 2D VBAP for flat layouts (Quad, 5.1, 7.1) — azimuth-only panning
//==============================================================================
void OpenSpatialDelayProcessor::computeVBAPGains2D (const SpeakerLayout& layout,
                                                      float azimuthRad, float* outGains) const
{
    const int N = layout.numSpeakers;
    for (int s = 0; s < N; ++s)
        outGains[s] = 0.0f;

    if (N < 2) return;

    // Normalize azimuth to [-π, π]
    while (azimuthRad > juce::MathConstants<float>::pi)  azimuthRad -= 2.0f * juce::MathConstants<float>::pi;
    while (azimuthRad < -juce::MathConstants<float>::pi) azimuthRad += 2.0f * juce::MathConstants<float>::pi;

    // Find the two speakers that span the source azimuth
    // Sort speaker azimuths for efficient pair finding
    struct SpkAz { int index; float az; };
    SpkAz sorted[16];
    for (int s = 0; s < N; ++s)
    {
        sorted[s].index = s;
        sorted[s].az = layout.speakers[s].azimuthRad;
        // Normalize to [-π, π]
        while (sorted[s].az > juce::MathConstants<float>::pi)  sorted[s].az -= 2.0f * juce::MathConstants<float>::pi;
        while (sorted[s].az < -juce::MathConstants<float>::pi) sorted[s].az += 2.0f * juce::MathConstants<float>::pi;
    }
    std::sort (sorted, sorted + N, [](const SpkAz& a, const SpkAz& b) { return a.az < b.az; });

    // Find spanning pair
    int leftIdx = -1, rightIdx = -1;
    for (int s = 0; s < N; ++s)
    {
        int next = (s + 1) % N;
        float az1 = sorted[s].az;
        float az2 = sorted[next].az;

        // Handle wrap-around
        if (next == 0)
            az2 += 2.0f * juce::MathConstants<float>::pi;

        float srcAz = azimuthRad;
        if (next == 0 && srcAz < az1)
            srcAz += 2.0f * juce::MathConstants<float>::pi;

        if (srcAz >= az1 && srcAz <= az2)
        {
            leftIdx = s;
            rightIdx = next;
            break;
        }
    }

    if (leftIdx < 0)
    {
        // Fallback: nearest speaker
        float minDist = 999.0f;
        int nearest = 0;
        for (int s = 0; s < N; ++s)
        {
            float d = std::abs (sorted[s].az - azimuthRad);
            if (d > juce::MathConstants<float>::pi) d = 2.0f * juce::MathConstants<float>::pi - d;
            if (d < minDist) { minDist = d; nearest = s; }
        }
        outGains[sorted[nearest].index] = 1.0f;
        return;
    }

    float az1 = sorted[leftIdx].az;
    float az2 = sorted[rightIdx].az;
    float srcAz = azimuthRad;

    // Handle wrap
    if (rightIdx == 0)
    {
        az2 += 2.0f * juce::MathConstants<float>::pi;
        if (srcAz < az1) srcAz += 2.0f * juce::MathConstants<float>::pi;
    }

    float span = az2 - az1;
    if (span < 1e-6f)
    {
        outGains[sorted[leftIdx].index] = 1.0f;
        return;
    }

    // Sine law panning (VBAP 2D)
    float g1 = std::sin (az2 - srcAz) / std::sin (span);
    float g2 = std::sin (srcAz - az1) / std::sin (span);

    // Constant-power normalization
    float power = g1 * g1 + g2 * g2;
    if (power > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (power);
        g1 *= scale;
        g2 *= scale;
    }

    outGains[sorted[leftIdx].index] = std::max (0.0f, g1);
    outGains[sorted[rightIdx].index] = std::max (0.0f, g2);
}

//==============================================================================
// v0.2: 3D VBAP for height layouts (7.1.4, 9.1.6) — uses pre-computed triplets
//==============================================================================
void OpenSpatialDelayProcessor::computeVBAPGains3D (const SpeakerLayout& layout,
                                                      float azimuthRad, float elevationRad,
                                                      float* outGains) const
{
    const int N = layout.numSpeakers;
    for (int s = 0; s < N; ++s)
        outGains[s] = 0.0f;

    // Source direction as unit Cartesian vector
    float px = std::cos (elevationRad) * std::sin (azimuthRad);
    float py = std::cos (elevationRad) * std::cos (azimuthRad);
    float pz = std::sin (elevationRad);

    float bestGainSum = -1.0f;
    int bestTri = -1;
    float bestG[3] = {};

    for (int t = 0; t < static_cast<int> (layoutVBAPTriplets.size()); ++t)
    {
        const auto& tri = layoutVBAPTriplets[t];

        float g0 = tri.inv[0][0] * px + tri.inv[0][1] * py + tri.inv[0][2] * pz;
        float g1 = tri.inv[1][0] * px + tri.inv[1][1] * py + tri.inv[1][2] * pz;
        float g2 = tri.inv[2][0] * px + tri.inv[2][1] * py + tri.inv[2][2] * pz;

        if (g0 >= -1e-6f && g1 >= -1e-6f && g2 >= -1e-6f)
        {
            float sum = g0 + g1 + g2;
            if (sum > bestGainSum)
            {
                bestGainSum = sum;
                bestTri = t;
                bestG[0] = std::max (0.0f, g0);
                bestG[1] = std::max (0.0f, g1);
                bestG[2] = std::max (0.0f, g2);
            }
        }
    }

    if (bestTri >= 0)
    {
        float power = bestG[0] * bestG[0] + bestG[1] * bestG[1] + bestG[2] * bestG[2];
        float scale = (power > 1e-12f) ? (1.0f / std::sqrt (power)) : 0.0f;

        outGains[layoutVBAPTriplets[bestTri].i] = bestG[0] * scale;
        outGains[layoutVBAPTriplets[bestTri].j] = bestG[1] * scale;
        outGains[layoutVBAPTriplets[bestTri].k] = bestG[2] * scale;
    }
    else
    {
        // Fallback: nearest speaker
        float bestDot = -2.0f;
        int bestSpeaker = 0;
        for (int s = 0; s < N; ++s)
        {
            float sx = std::cos (layout.speakers[s].elevationRad) * std::sin (layout.speakers[s].azimuthRad);
            float sy = std::cos (layout.speakers[s].elevationRad) * std::cos (layout.speakers[s].azimuthRad);
            float sz = std::sin (layout.speakers[s].elevationRad);
            float dot = px * sx + py * sy + pz * sz;
            if (dot > bestDot)
            {
                bestDot = dot;
                bestSpeaker = s;
            }
        }
        outGains[bestSpeaker] = 1.0f;
    }
}

//==============================================================================
// v0.2: VBIP — Vector Base Intensity Panning (intensity-weighted VBAP)
//==============================================================================
void OpenSpatialDelayProcessor::computeVBIPGains (const SpeakerLayout& layout,
                                                    float azimuthRad, float elevationRad,
                                                    float* outGains) const
{
    const int N = layout.numSpeakers;

    // Check if layout has height speakers
    bool hasHeight = (activeOutputFormat == OutputFormat::Surround7_1_4 ||
                      activeOutputFormat == OutputFormat::Surround9_1_6);

    // Start with VBAP gains
    if (hasHeight)
        computeVBAPGains3D (layout, azimuthRad, elevationRad, outGains);
    else
        computeVBAPGains2D (layout, azimuthRad, outGains);

    // Square all gains for intensity weighting
    float sum = 0.0f;
    for (int s = 0; s < N; ++s)
    {
        outGains[s] = outGains[s] * outGains[s];
        sum += outGains[s];
    }

    // Normalize to constant power
    if (sum > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (sum);
        for (int s = 0; s < N; ++s)
            outGains[s] *= scale;
    }
}

//==============================================================================
// v0.2: KNN — K-Nearest Neighbor panning
//==============================================================================
void OpenSpatialDelayProcessor::computeKNNGains (const SpeakerLayout& layout,
                                                   float azimuthRad, float elevationRad,
                                                   float* outGains, int k) const
{
    const int N = layout.numSpeakers;
    for (int s = 0; s < N; ++s)
        outGains[s] = 0.0f;

    if (N == 0) return;
    k = std::min (k, N);

    // Source direction as unit Cartesian vector
    float px = std::cos (elevationRad) * std::sin (azimuthRad);
    float py = std::cos (elevationRad) * std::cos (azimuthRad);
    float pz = std::sin (elevationRad);

    // Compute angular distances
    struct SpkDist { int index; float dist; };
    SpkDist dists[16];
    for (int s = 0; s < N; ++s)
    {
        float sx = std::cos (layout.speakers[s].elevationRad) * std::sin (layout.speakers[s].azimuthRad);
        float sy = std::cos (layout.speakers[s].elevationRad) * std::cos (layout.speakers[s].azimuthRad);
        float sz = std::sin (layout.speakers[s].elevationRad);
        float dot = juce::jlimit (-1.0f, 1.0f, px * sx + py * sy + pz * sz);
        dists[s] = { s, std::acos (dot) };
    }

    // Partial sort to find K nearest
    std::partial_sort (dists, dists + k, dists + N,
                       [](const SpkDist& a, const SpkDist& b) { return a.dist < b.dist; });

    // Check if source is exactly at a speaker
    if (dists[0].dist < 1e-4f)
    {
        outGains[dists[0].index] = 1.0f;
        return;
    }

    // Inverse-distance-squared weighting
    float totalWeight = 0.0f;
    float weights[16] = {};
    for (int i = 0; i < k; ++i)
    {
        float w = 1.0f / (dists[i].dist * dists[i].dist + 1e-6f);
        weights[i] = w;
        totalWeight += w;
    }

    // Constant-power normalization
    float totalPower = 0.0f;
    for (int i = 0; i < k; ++i)
    {
        float g = weights[i] / totalWeight;
        outGains[dists[i].index] = g;
        totalPower += g * g;
    }

    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < N; ++s)
            outGains[s] *= scale;
    }
}

//==============================================================================
// v0.1 Output Renderer: pre-compute binaural gains for virtual speakers
//==============================================================================
void OpenSpatialDelayProcessor::updateSpeakerBinauralCache (int profileIndex)
{
    if (profileIndex == cachedProfileIndex)
        return;

    // Nominal distance 0.1875 yields distGain = 1.0 from the formula
    // 1/(0.1875*4+0.25) = 1.0, so speaker binaural gains are purely directional.
    // Per-object distance attenuation is applied separately.
    const float nominalDistance = 0.1875f;

    for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
    {
        speakerBinauralCache[s] = computeBinauralGains (
            virtualSpeakers[s].azimuthRad,
            virtualSpeakers[s].elevationRad,
            nominalDistance, profileIndex);
    }

    cachedProfileIndex = profileIndex;
}

//==============================================================================
// Access object state for editor
//==============================================================================
ObjectState OpenSpatialDelayProcessor::getObjectState (int objectIndex) const
{
    ObjectState state;
    if (objectIndex < 0 || objectIndex >= MAX_OBJECTS)
        return state;

    auto prefix = "object" + juce::String (objectIndex + 1) + "_";
    if (auto* p = apvts.getRawParameterValue (prefix + "enabled"))
        state.enabled = p->load() > 0.5f;
    if (auto* p = apvts.getRawParameterValue (prefix + "time"))
        state.delayTimeMs = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "azimuth"))
        state.azimuthDeg = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "elevation"))
        state.elevationDeg = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "distance"))
        state.distance = p->load();

    return state;
}

//==============================================================================
// Main audio processing
//==============================================================================
void OpenSpatialDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto numSamples       = buffer.getNumSamples();
    auto numInputChannels = getTotalNumInputChannels();

    if (delayBuffer.empty() || numSamples <= 0)
        return;

    // --- Read parameter values -----------------------------------------------
    bool  tempoSync       = apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
    int   noteDivision    = static_cast<int> (apvts.getRawParameterValue ("noteDivision")->load());
    float lpFreq          = apvts.getRawParameterValue ("filterLP")->load();
    float hpFreq          = apvts.getRawParameterValue ("filterHP")->load();
    int   profileIndex    = static_cast<int> (apvts.getRawParameterValue ("hrtfProfile")->load());
    float pitchSemitones  = apvts.getRawParameterValue ("pitchShift")->load();

    // (#3) Calculate target base delay
    float targetBaseDelayMs;
    if (tempoSync)
        targetBaseDelayMs = getTempoSyncedDelayMs (noteDivision);
    else
        targetBaseDelayMs = apvts.getRawParameterValue ("delayTime")->load();

    smoothedDelayTime.setTargetValue (targetBaseDelayMs);

    float dryWetTarget    = apvts.getRawParameterValue ("dryWet")->load();
    float fbTarget        = apvts.getRawParameterValue ("feedback")->load();
    float inGainDb        = apvts.getRawParameterValue ("inputGain")->load();
    float outGainDb       = apvts.getRawParameterValue ("outputGain")->load();

    smoothedDryWet.setTargetValue     (dryWetTarget);
    smoothedFeedback.setTargetValue   (fbTarget);
    smoothedInputGain.setTargetValue  (juce::Decibels::decibelsToGain (inGainDb));
    smoothedOutputGain.setTargetValue (juce::Decibels::decibelsToGain (outGainDb));

    // --- Update feedback filters (block-rate) --------------------------------
    feedbackLPFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, lpFreq);
    feedbackHPFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpFreq);

    // --- Read algorithm selection (block-rate) --------------------------------
    int algorithmIndex = static_cast<int> (apvts.getRawParameterValue ("algorithm")->load());
    bool isBinaural = (activeOutputFormat == OutputFormat::Binaural);

    // For surround output, force algorithm ≥ 1 (Direct Binaural not available)
    if (! isBinaural && algorithmIndex == 0)
        algorithmIndex = 1;  // Fall back to VBAP

    // Update spatial caches based on algorithm (block-rate, binaural path only)
    if (isBinaural)
    {
        if (algorithmIndex == 1 || algorithmIndex == 3)  // VBAP or VBIP: virtual speakers → binaural
            updateSpeakerBinauralCache (profileIndex);
        else if (algorithmIndex == 2)                     // Ambisonics: direct SH → binaural
            computeBinauralSHWeights (profileIndex);
        else if (algorithmIndex == 4)                     // KNN: virtual speakers → binaural
            updateSpeakerBinauralCache (profileIndex);
    }

    // --- Read object states and pre-compute spatial gains -------------------
    ObjectState objects[MAX_OBJECTS];
    BinauralGains objGains[MAX_OBJECTS];                              // Binaural path (algo 0, 2)
    float objSpeakerGains[MAX_OBJECTS][NUM_VIRTUAL_SPEAKERS] = {};    // Binaural VBAP/VBIP/KNN (16 virtual speakers)
    float objChannelGains[MAX_OBJECTS][16] = {};                      // Discrete surround path (all algos)
    float objDistGain[MAX_OBJECTS] = {};                              // Distance attenuation
    int numActiveObjects = 0;
    int lastEnabledObjectIndex = -1;

    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        objects[t] = getObjectState (t);

        if (objects[t].enabled)
        {
            ++numActiveObjects;
            lastEnabledObjectIndex = t;
            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            objDistGain[t] = 1.0f / std::max (0.1f, objects[t].distance * 4.0f + 0.25f);

            if (isBinaural)
            {
                // --- BINAURAL PATH (stereo output) ---
                switch (algorithmIndex)
                {
                    case 0: // Direct Binaural
                        objGains[t] = computeBinauralGains (azRad, elRad, objects[t].distance, profileIndex);
                        break;
                    case 1: // VBAP → virtual speakers → binaural
                        computeVBAPGains (azRad, elRad, objSpeakerGains[t]);
                        break;
                    case 2: // Ambisonics → direct SH → binaural
                        objGains[t] = computeAmbiBinauralGains (azRad, elRad, objects[t].distance, profileIndex);
                        break;
                    case 3: // VBIP → virtual speakers → binaural
                    {
                        computeVBAPGains (azRad, elRad, objSpeakerGains[t]);
                        // Square gains for intensity weighting
                        float sum = 0.0f;
                        for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                        {
                            objSpeakerGains[t][sp] *= objSpeakerGains[t][sp];
                            sum += objSpeakerGains[t][sp];
                        }
                        if (sum > 1e-12f)
                        {
                            float scale = 1.0f / std::sqrt (sum);
                            for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                                objSpeakerGains[t][sp] *= scale;
                        }
                        break;
                    }
                    case 4: // KNN → virtual speakers → binaural
                    {
                        // KNN for 16-speaker virtual layout
                        float px = std::cos (elRad) * std::sin (azRad);
                        float py = std::cos (elRad) * std::cos (azRad);
                        float pz = std::sin (elRad);

                        struct SpkD { int idx; float dist; };
                        SpkD dists[NUM_VIRTUAL_SPEAKERS];
                        for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                        {
                            float sx = std::cos (virtualSpeakers[sp].elevationRad) * std::sin (virtualSpeakers[sp].azimuthRad);
                            float sy = std::cos (virtualSpeakers[sp].elevationRad) * std::cos (virtualSpeakers[sp].azimuthRad);
                            float sz = std::sin (virtualSpeakers[sp].elevationRad);
                            float dot = juce::jlimit (-1.0f, 1.0f, px * sx + py * sy + pz * sz);
                            dists[sp] = { sp, std::acos (dot) };
                        }
                        constexpr int knn = 3;
                        std::partial_sort (dists, dists + knn, dists + NUM_VIRTUAL_SPEAKERS,
                                           [](const SpkD& a, const SpkD& b) { return a.dist < b.dist; });

                        for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                            objSpeakerGains[t][sp] = 0.0f;

                        if (dists[0].dist < 1e-4f)
                        {
                            objSpeakerGains[t][dists[0].idx] = 1.0f;
                        }
                        else
                        {
                            float totalW = 0.0f;
                            float weights[knn];
                            for (int i = 0; i < knn; ++i)
                            {
                                weights[i] = 1.0f / (dists[i].dist * dists[i].dist + 1e-6f);
                                totalW += weights[i];
                            }
                            float totalP = 0.0f;
                            for (int i = 0; i < knn; ++i)
                            {
                                float g = weights[i] / totalW;
                                objSpeakerGains[t][dists[i].idx] = g;
                                totalP += g * g;
                            }
                            if (totalP > 1e-12f)
                            {
                                float sc = 1.0f / std::sqrt (totalP);
                                for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                                    objSpeakerGains[t][sp] *= sc;
                            }
                        }
                        break;
                    }
                    default: break;
                }
            }
            else
            {
                // --- DISCRETE SURROUND PATH (multi-channel output) ---
                bool hasHeight = (activeOutputFormat == OutputFormat::Surround7_1_4 ||
                                  activeOutputFormat == OutputFormat::Surround9_1_6);

                switch (algorithmIndex)
                {
                    case 1: // VBAP direct to output speakers
                        if (hasHeight)
                            computeVBAPGains3D (activeLayout, azRad, elRad, objChannelGains[t]);
                        else
                            computeVBAPGains2D (activeLayout, azRad, objChannelGains[t]);
                        break;
                    case 2: // Ambisonics → layout decode matrix
                    {
                        // Max-rE weights per order
                        static const float maxrE[4] = {
                            1.0f,
                            std::cos (juce::MathConstants<float>::pi / 8.0f),
                            std::cos (2.0f * juce::MathConstants<float>::pi / 8.0f),
                            std::cos (3.0f * juce::MathConstants<float>::pi / 8.0f),
                        };
                        auto acnToOrder = [](int acn) -> int {
                            if (acn < 1) return 0; if (acn < 4) return 1;
                            if (acn < 9) return 2; return 3;
                        };
                        float coeffs[HOA_CHANNELS];
                        for (int c = 0; c < HOA_CHANNELS; ++c)
                            coeffs[c] = evalSH (c, azRad, elRad) * maxrE[acnToOrder (c)];

                        float totalPower = 0.0f;
                        for (int sp = 0; sp < discreteAmbiNumSpeakers; ++sp)
                        {
                            float gain = 0.0f;
                            for (int c = 0; c < HOA_CHANNELS; ++c)
                                gain += discreteAmbiDecodeMatrix[sp][c] * coeffs[c];
                            objChannelGains[t][sp] = std::max (0.0f, gain);
                            totalPower += objChannelGains[t][sp] * objChannelGains[t][sp];
                        }
                        if (totalPower > 1e-12f)
                        {
                            float sc = 1.0f / std::sqrt (totalPower);
                            for (int sp = 0; sp < discreteAmbiNumSpeakers; ++sp)
                                objChannelGains[t][sp] *= sc;
                        }
                        break;
                    }
                    case 3: // VBIP direct to output speakers
                        computeVBIPGains (activeLayout, azRad, elRad, objChannelGains[t]);
                        break;
                    case 4: // KNN direct to output speakers
                        computeKNNGains (activeLayout, azRad, elRad, objChannelGains[t]);
                        break;
                    default: // Fallback to VBAP
                        if (hasHeight)
                            computeVBAPGains3D (activeLayout, azRad, elRad, objChannelGains[t]);
                        else
                            computeVBAPGains2D (activeLayout, azRad, objChannelGains[t]);
                        break;
                }
            }
        }
    }

    // --- Prepare mono input --------------------------------------------------
    if (monoInputBuffer.size() < static_cast<size_t> (numSamples))
        monoInputBuffer.resize (static_cast<size_t> (numSamples));

    if (numInputChannels == 1)
    {
        auto* in = buffer.getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[i] = in[i];
    }
    else
    {
        auto* inL = buffer.getReadPointer (0);
        auto* inR = buffer.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[i] = (inL[i] + inR[i]) * 0.5f;
    }

    // --- Clear output buffer -------------------------------------------------
    buffer.clear();

    // --- Per-sample processing -----------------------------------------------
    // Signal flow (SEQUENTIAL SPATIAL PING-PONG):
    //   1. INPUT: gain mono input signal
    //   2. WRITE: input + (processed_feedback × fb) → delay line
    //   3. READ OBJECTS: read from delay line at offsets (1..N) × baseDelay
    //   4. SPATIALIZE: Each object applies sequential pitch + spatial rendering
    //   5. UPDATE FEEDBACK: mono read from END of chain
    //   6. OUTPUT MIX: dry/wet blend → output channels

    // Determine the loop length multiplier based on the highest enabled object index
    // Smoothed to prevent clicks when enabling/disabling objects mid-playback
    float loopMultiplierTarget = static_cast<float>(std::max (1, lastEnabledObjectIndex + 1));
    smoothedLoopMultiplier.setTargetValue (loopMultiplierTarget);

    if (isBinaural)
    {
        // =====================================================================
        // BINAURAL PATH (stereo output — identical to v0.1)
        // =====================================================================
        auto* outL = buffer.getWritePointer (0);
        auto* outR = buffer.getWritePointer (1);

        for (int s = 0; s < numSamples; ++s)
        {
            float currentDelayMs = smoothedDelayTime.getNextValue();
            float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
            float currentLoopMult = smoothedLoopMultiplier.getNextValue();

            float inGain  = smoothedInputGain.getNextValue();
            float dw      = smoothedDryWet.getNextValue();
            float fb      = smoothedFeedback.getNextValue();
            float outGain = smoothedOutputGain.getNextValue();

            float rawInput    = monoInputBuffer[s];
            float inputSample = rawInput * inGain;

            // === STAGE 1: WRITE ===
            float delayInput = inputSample + feedbackSample * fb;
            delayInput = softClip (delayInput * 0.98f);
            writeDelayLine (delayInput);

            // === STAGE 2: READ & SPATIALIZE ===
            float wetL = 0.0f, wetR = 0.0f;

            for (int t = 0; t < MAX_OBJECTS; ++t)
            {
                if (! objects[t].enabled) continue;

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;
                objPitch = juce::jlimit (-MAX_CUMULATIVE_SEMITONES, MAX_CUMULATIVE_SEMITONES, objPitch);
                float objMono = readPitchShifted (objDelaySamples, objPitch, t);

                if (algorithmIndex == 1 || algorithmIndex == 3 || algorithmIndex == 4)
                {
                    // VBAP / VBIP / KNN — route through virtual speakers → binaural renderer
                    float dist = objDistGain[t];
                    for (int sp = 0; sp < NUM_VIRTUAL_SPEAKERS; ++sp)
                    {
                        float spkSig = objMono * dist * objSpeakerGains[t][sp];
                        wetL += spkSig * speakerBinauralCache[sp].leftGain;
                        wetR += spkSig * speakerBinauralCache[sp].rightGain;
                    }
                }
                else
                {
                    // Direct Binaural (algo 0) and Ambisonics Binaural (algo 2)
                    wetL += objMono * objGains[t].leftGain;
                    wetR += objMono * objGains[t].rightGain;
                }
            }

            // === STAGE 3: FEEDBACK ===
            float fbDelaySamples = currentLoopMult * baseDelaySamples;
            fbDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), fbDelaySamples);
            float fbPitchShiftAmount = currentLoopMult * pitchSemitones;
            fbPitchShiftAmount = juce::jlimit (-MAX_CUMULATIVE_SEMITONES, MAX_CUMULATIVE_SEMITONES, fbPitchShiftAmount);
            float feedbackRaw = readPitchShifted (fbDelaySamples, fbPitchShiftAmount, MAX_OBJECTS);
            float filtered = feedbackLPFilter.processSample (feedbackRaw);
            filtered = feedbackHPFilter.processSample (filtered);
            const float makeupGain = 1.0f + (fb * fb * 0.2f);
            feedbackSample = softClip (filtered * makeupGain);
            if (! std::isfinite (feedbackSample))
            {
                feedbackSample = 0.0f;
                feedbackLPFilter.reset();
                feedbackHPFilter.reset();
            }

            // === STAGE 4: OUTPUT MIX ===
            outL[s] = (rawInput * (1.0f - dw) + wetL * dw) * outGain;
            outR[s] = (rawInput * (1.0f - dw) + wetR * dw) * outGain;
        }
    }
    else
    {
        // =====================================================================
        // DISCRETE SURROUND PATH (multi-channel output — v0.2)
        // =====================================================================
        const int numSpeakers = activeLayout.numSpeakers;
        const int lfeIdx = activeLayout.lfeChannelIndex;

        // Get output channel write pointers
        float* outChannels[16] = {};
        int numOutCh = buffer.getNumChannels();
        for (int ch = 0; ch < numOutCh && ch < 16; ++ch)
            outChannels[ch] = buffer.getWritePointer (ch);

        for (int s = 0; s < numSamples; ++s)
        {
            float currentDelayMs = smoothedDelayTime.getNextValue();
            float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
            float currentLoopMult = smoothedLoopMultiplier.getNextValue();

            float inGain  = smoothedInputGain.getNextValue();
            float dw      = smoothedDryWet.getNextValue();
            float fb      = smoothedFeedback.getNextValue();
            float outGain = smoothedOutputGain.getNextValue();

            float rawInput    = monoInputBuffer[s];
            float inputSample = rawInput * inGain;

            // === STAGE 1: WRITE ===
            float delayInput = inputSample + feedbackSample * fb;
            delayInput = softClip (delayInput * 0.98f);
            writeDelayLine (delayInput);

            // === STAGE 2: READ & SPATIALIZE ===
            float channelAccum[16] = {};
            float wetMono = 0.0f;  // For LFE generation

            for (int t = 0; t < MAX_OBJECTS; ++t)
            {
                if (! objects[t].enabled) continue;

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;
                objPitch = juce::jlimit (-MAX_CUMULATIVE_SEMITONES, MAX_CUMULATIVE_SEMITONES, objPitch);
                float objMono = readPitchShifted (objDelaySamples, objPitch, t);
                float dist = objDistGain[t];

                for (int sp = 0; sp < numSpeakers; ++sp)
                    channelAccum[sp] += objMono * dist * objChannelGains[t][sp];

                wetMono += objMono * dist;
            }

            // === STAGE 3: FEEDBACK ===
            float fbDelaySamples = currentLoopMult * baseDelaySamples;
            fbDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), fbDelaySamples);
            float fbPitchShiftAmount = currentLoopMult * pitchSemitones;
            fbPitchShiftAmount = juce::jlimit (-MAX_CUMULATIVE_SEMITONES, MAX_CUMULATIVE_SEMITONES, fbPitchShiftAmount);
            float feedbackRaw = readPitchShifted (fbDelaySamples, fbPitchShiftAmount, MAX_OBJECTS);
            float filtered = feedbackLPFilter.processSample (feedbackRaw);
            filtered = feedbackHPFilter.processSample (filtered);
            const float makeupGain = 1.0f + (fb * fb * 0.2f);
            feedbackSample = softClip (filtered * makeupGain);
            if (! std::isfinite (feedbackSample))
            {
                feedbackSample = 0.0f;
                feedbackLPFilter.reset();
                feedbackHPFilter.reset();
            }

            // === STAGE 4: OUTPUT MIX ===
            // Route spatialized signal to output channels
            for (int sp = 0; sp < numSpeakers; ++sp)
            {
                int ch = activeLayout.speakers[sp].channelIndex;
                if (ch >= 0 && ch < numOutCh && outChannels[ch] != nullptr)
                    outChannels[ch][s] = channelAccum[sp] * dw * outGain;
            }

            // Dry signal → L and R only (channels 0 and 1)
            float drySignal = rawInput * (1.0f - dw) * outGain;
            if (outChannels[0] != nullptr) outChannels[0][s] += drySignal;
            if (outChannels[1] != nullptr) outChannels[1][s] += drySignal;

            // LFE generation — low-pass filtered mono sum at −10 dB
            if (lfeIdx >= 0 && lfeIdx < numOutCh && outChannels[lfeIdx] != nullptr)
            {
                float lfeSig = lfeFilter.processSample (wetMono) * 0.316f * dw * outGain;  // −10 dB ≈ 0.316
                outChannels[lfeIdx][s] = lfeSig;
            }
        }
    }
}

//==============================================================================
// State save / restore
//==============================================================================
void OpenSpatialDelayProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OpenSpatialDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorEditor* OpenSpatialDelayProcessor::createEditor()
{
    return new OpenSpatialDelayEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpenSpatialDelayProcessor();
}
