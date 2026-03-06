cmake_minimum_required(VERSION 3.22)
project(OpenSpatialDelay VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE path - override with -DJUCE_DIR=... if needed
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Spatial64/JUCE")
endif()

add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(OpenSpatialDelay
    COMPANY_NAME "SpatialMediaLibrary"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Smli
    PLUGIN_CODE Osd2
    FORMATS VST3 AU
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

target_link_libraries(OpenSpatialDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# macOS: build for Apple Silicon
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build for Apple Silicon")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
endif()

# =============================================================================
# Post-build: install plugins to ~/Library/Audio/Plug-Ins/
# Runs scripts/install_plugins.sh after the final target (VST3) is built.
# =============================================================================
if(APPLE)
    add_custom_command(TARGET OpenSpatialDelay_VST3 POST_BUILD
        COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/scripts/install_plugins.sh" "${CMAKE_BINARY_DIR}"
        COMMENT "Installing AU and VST3 plugins..."
    )
endif()
