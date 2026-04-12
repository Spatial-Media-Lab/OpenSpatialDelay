---
name: plugin-architecture-patterns
description: Use when designing JUCE plugin architecture, separating DSP from UI, setting up APVTS parameters, building preset systems, handling MIDI, or implementing modulation routing.
---

# Plugin Architecture Patterns

## Architecture Overview

```
Plugin Host → Processor (Audio) ←→ Editor (UI)
                    ↓
              DSP Engine (Domain)
                    ↓
           Filter │ Envelope │ Oscillator
```

**Dependency rule:** Outer layers depend on inner. Never the reverse.

| Layer | Responsibility | Realtime-Safe? |
|-------|---------------|----------------|
| Presentation (UI) | Rendering, interaction | No |
| Application (Processor) | Parameters, host comms | Partially |
| Domain (DSP Core) | Pure audio algorithms | YES - mandatory |

## Core: Clean Layer Separation

```cpp
// Domain Layer — Pure DSP, minimal JUCE deps
class FilterCore {
public:
    void setFrequency(float hz, float sr) { /* coefficients */ }
    float processSample(float in) noexcept { return /* output */; }
    void reset() noexcept { /* clear state */ }
};

// Application Layer — Processor with APVTS
class MyProcessor : public juce::AudioProcessor {
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float>* cutoffParam;  // Cached in constructor
    FilterCore filter;
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override {
        juce::ScopedNoDenormals noDenormals;
        filter.setFrequency(cutoffParam->load(), getSampleRate());
        // ... process through domain layer
    }
};

// Presentation Layer — Editor with APVTS attachments
class MyEditor : public juce::AudioProcessorEditor {
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttach;
};
```

## APVTS Quick Setup

```cpp
namespace Params {
    inline auto createLayout() {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"cutoff", 1}, "Cutoff",
            juce::NormalisableRange<float>(20.f, 20000.f, 0.01f, 0.3f), 1000.f));
        return { params.begin(), params.end() };
    }
}
// Constructor: apvts(*this, nullptr, "Params", Params::createLayout())
// Cache:      cutoffParam = apvts.getRawParameterValue("cutoff");
```

## State Save/Restore

```cpp
void getStateInformation(juce::MemoryBlock& dest) override {
    auto xml = apvts.copyState().createXml();
    copyXmlToBinary(*xml, dest);
}
void setStateInformation(const void* data, int size) override {
    auto xml = getXmlFromBinary(data, size);
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
```

## Common Mistakes

| Mistake | Fix |
|---|---|
| God class Processor (DSP + UI + state) | Separate into Domain / Application / Presentation layers |
| DSP on UI thread or UI on audio thread | DSP only in `processBlock()`; UI only on message thread |
| No state separation (params vs non-param) | Params in APVTS; UI size, preset name in separate ValueTree child |
| Allocating in `processBlock()` | Pre-allocate everything in `prepareToPlay()` |
| `getParameter()` instead of `getRawParameterValue()` | Cache `std::atomic<float>*` pointers in constructor |
| Shared static/global state across instances | All state must be instance members |

---

See **[REFERENCE.md](REFERENCE.md)** for full patterns: preset system design (user + factory), MIDI handling with MPE, modulation matrix architecture, per-voice modulation, voice management with steal strategies, multi-format conditional compilation, unit testing DSP with Catch2, and performance optimization.
