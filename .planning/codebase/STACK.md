# Technology Stack

**Analysis Date:** 2026-04-14

## Languages

**Primary:**
- C++ 17 - Core DSP and plugin logic
- C - libmysofa HRTF data reading

**Secondary:**
- CMake - Build configuration
- Bash - Build and deployment scripting
- Python - Documentation and audio analysis tools (not part of runtime)

## Runtime

**Environment:**
- CMake 3.22+ (minimum required)
- Xcode toolchain (macOS: arm64 Apple Silicon and x86_64)
- Visual Studio 17 2022 (Windows x64)

**Package Manager:**
- CMake FetchContent - Dependency resolution for libmysofa and Catch2
- vcpkg (Windows only) - System dependency management for zlib

## Frameworks

**Core:**
- JUCE 8.0.3 (git submodule at `JUCE/`) - Audio plugin framework (VST3 + AU)
  - Modules used: `juce_audio_utils`, `juce_audio_plugin_client`, `juce_dsp`, `juce_osc`
  - Subcomponents: `juce_audio_processors`, `juce_audio_basics`, `juce_core`, `juce_events`, `juce_graphics`

**Testing:**
- Catch2 v3.7.1 - Unit test framework with FetchContent

**Build/Dev:**
- CMake 3.22+ - Build system with custom plugin code patching
- Bash scripts (`scripts/build_version.sh`) - Versioned build automation with bundle ID patching
- codesign (macOS) - Ad-hoc code signing for plugin bundles

## Key Dependencies

**Critical:**
- libmysofa v1.3.2 (GitHub: https://github.com/hoene/libmysofa.git) - SOFA file reader for HRTF data
  - Static linking: `-static_libs=ON`, tests disabled
  - Used in `Source/PluginProcessor.cpp` via `#include "mysofa.h"`
  - Handles binaural convolution data for 5 HRTF profiles (MIT KEMAR, SADIE D2-KU100, CIPIC Subject 003, HUTUBS PP2, BernSchuetz KU100)

**Infrastructure:**
- zlib (macOS: system; Windows: vcpkg x64-windows-static) - Compression library (transitive dependency of libmysofa)
  - Linked as `ZLIB::ZLIB` in CMakeLists.txt line 139

## Configuration

**Environment:**
- CMake cache variables control plugin format selection (VST3 + AU on macOS, VST3 only on Windows)
- Fast-math flags enabled: `-ffast-math` (Apple), `/fp:fast /arch:AVX2` (MSVC)
- Compile definitions disable JUCE web browser and CURL integration: `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`

**Build:**
- CMakeLists.txt (root) - Main plugin target configuration
- JUCE/CMakeLists.txt (submodule) - JUCE framework configuration
- scripts/build_version.sh - Versioned build pipeline with JUCE patches for Ableton VST3 compatibility (issue #122) and AU channel probe limit (issue #88)

**Binary Data Embedding:**
- 5 HRTF SOFA files embedded via `juce_add_binary_data(HRTFData)` - `HRTF/mit_kemar_large_pinna.sofa`, `HRTF/sadie_d2_ku100.sofa`, etc.
- 8 Font files embedded via `juce_add_binary_data(FontData)` - `fonts/DM_Sans*.ttf`, `fonts/JetBrains_Mono*.ttf`, `fonts/Roboto-Medium.ttf`
- Generated headers: `HRTFData.h`, `FontData.h`

## Platform Requirements

**Development:**
- macOS 12.0+ with Apple Silicon or Intel support
- CMake 3.22+
- Git (for submodule checkout)
- Xcode command-line tools or clang

**Production:**
- **macOS:**
  - Deployment target: macOS 12.0
  - Architecture: arm64 (Apple Silicon) and x86_64 (Intel)
  - Plugin formats: AU (.component) + VST3 (.vst3)
  - Installation: `~/Library/Audio/Plug-Ins/Components/` (AU), `~/Library/Audio/Plug-Ins/VST3/` (VST3)

- **Windows:**
  - Windows 10+
  - x64 architecture
  - Plugin format: VST3 (.vst3)
  - Deployment: Visual Studio 17 2022, MSVC compiler with standard-conforming preprocessor (`/Zc:preprocessor`)

## Optimization Flags

**macOS:**
- Fast-math mode enabled: `-ffast-math`
- Suppressed NaN/infinity warnings: `-Wno-nan-infinity-disabled`

**Windows:**
- Fast floating-point mode: `/fp:fast`
- AVX2 SIMD instructions: `/arch:AVX2`
- Suppressed CRT deprecation warnings: `_CRT_SECURE_NO_WARNINGS`

## CI/CD

**GitHub Actions:**
- macOS workflow: `build-macos.yml` - Runs on `macos-14` (Apple Silicon), builds arm64 AU + VST3
- Windows workflow: `build-windows.yml` - Runs on `windows-latest`, builds x64 VST3
- Artifact retention: 90 days
- SignPath integration (commented, available for code signing)

---

*Stack analysis: 2026-04-14*
