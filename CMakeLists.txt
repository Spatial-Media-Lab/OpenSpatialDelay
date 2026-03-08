cmake_minimum_required(VERSION 3.22)
project(OpenSpatialDelay VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS: build for Apple Silicon
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build for Apple Silicon")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
endif()

# Windows: MSVC-specific flags for JUCE compatibility
if(MSVC)
    # JUCE 8 requires the standard-conforming preprocessor
    add_compile_options(/Zc:preprocessor)
    # Suppress C4996 deprecation warnings from JUCE / CRT
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# =============================================================================
# JUCE — bundled as git submodule at ./JUCE
# =============================================================================
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
endif()

add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

# =============================================================================
# libmysofa — SOFA file reader for HRTF data (fetched from GitHub)
# =============================================================================
include(FetchContent)

# Disable libmysofa tests and shared lib before declaring
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

FetchContent_Declare(
    libmysofa
    GIT_REPOSITORY https://github.com/hoene/libmysofa.git
    GIT_TAG        v1.3.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(libmysofa)

# =============================================================================
# SOFA HRTF BinaryData — 5 SOFA files embedded as binary resources
# =============================================================================
juce_add_binary_data(HRTFData
    HEADER_NAME "HRTFData.h"
    NAMESPACE HRTFData
    SOURCES
        HRTF/mit_kemar_large_pinna.sofa
        HRTF/sadie_d2_ku100.sofa
        HRTF/cipic_subject_003.sofa
        HRTF/hutubs_pp2.sofa
        HRTF/bernschuetz_ku100.sofa
)

# =============================================================================
# Plugin target
# =============================================================================
# AU is macOS-only; Windows gets VST3 only
if(APPLE)
    set(PLUGIN_FORMATS VST3 AU)
else()
    set(PLUGIN_FORMATS VST3)
endif()

juce_add_plugin(OpenSpatialDelay
    COMPANY_NAME "SpatialMediaLibrary"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Smli
    PLUGIN_CODE Osd2
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "OpenSpatialDelay v0.2"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(OpenSpatialDelay)

target_sources(OpenSpatialDelay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(OpenSpatialDelay
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Include libmysofa headers
target_include_directories(OpenSpatialDelay
    PRIVATE
        ${libmysofa_SOURCE_DIR}/src/hrtf
)

target_link_libraries(OpenSpatialDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_dsp
        HRTFData
        mysofa-static
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# =============================================================================
# Post-build: install plugins (macOS only)
# Runs scripts/install_plugins.sh after the final target (VST3) is built.
# =============================================================================
if(APPLE)
    add_custom_command(TARGET OpenSpatialDelay_VST3 POST_BUILD
        COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/scripts/install_plugins.sh" "${CMAKE_BINARY_DIR}"
        COMMENT "Installing AU and VST3 plugins..."
    )

    # Convenience target: download latest Windows VST3 from GitHub Actions
    add_custom_target(download_windows_build
        COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/scripts/download_windows_build.sh"
        COMMENT "Downloading latest Windows VST3 from GitHub Actions..."
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
