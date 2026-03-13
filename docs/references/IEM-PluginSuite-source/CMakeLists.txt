cmake_minimum_required (VERSION 3.13)
include(CheckIncludeFiles)

#  _____ ______ __  __   _____  _                    _          _____       _ _
# |_   _|  ____|  \/  | |  __ \| |                  (_)        / ____|     (_) |
#   | | | |__  | \  / | | |__) | |_   _  __ _        _ _ __   | (___  _   _ _| |_ ___
#   | | |  __| | |\/| | |  ___/| | | | |/ _` |______| | '_ \   \___ \| | | | | __/ _ \
#  _| |_| |____| |  | | | |    | | |_| | (_| |______| | | | |  ____) | |_| | | ||  __/
# |_____|______|_|  |_| |_|    |_|\__,_|\__, |      |_|_| |_| |_____/ \__,_|_|\__\___|
#                                        __/ |
#                                       |___/

# The following list will hold all the plug-ins which will be built.
# Simply comment them out if you want to build only a subset of the plug-ins.
set (PLUGINS_TO_BUILD
        AllRADecoder
        # BinauralDecoder
        # CoordinateConverter
        # DirectionalCompressor
        # DirectivityShaper
        DistanceCompensator
        # DualDelay
        # EnergyVisualizer
        # FdnReverb
        # GranularEncoder
        # MatrixMultiplier
        # MultiBandCompressor
        # MultiEncoder
        # MultiEQ
        # OmniCompressor
        # ProbeDecoder
        # RoomEncoder
        # SceneRotator
        # SimpleDecoder
        # StereoEncoder
        # ToolBox
    )

# The following options let you select the plug-ins formats you want to build.
# You can either change them here or use the command line to switch them on/off.
option (IEM_USE_SYSTEM_JUCE "Use JUCE found via CMake rather than the bundled one." OFF)
option (IEM_BUILD_VST2 "Build VST2 version of the plug-ins." OFF)
option (IEM_BUILD_VST3 "Build VST3 version of the plug-ins." OFF)
option (IEM_BUILD_LV2 "Build LV2 version of the plug-ins." OFF)
option (IEM_BUILD_STANDALONE "Build standalones of the plug-ins." ON)
option (IEM_STANDALONE_HEADLESS "Build standalones without GUI." ON)
option (IEM_STANDALONE_JACK_SUPPORT "Build standalones with JACK support." ON)

set (IEM_POST_BUILD_INSTALL "USER" CACHE STRING "Define if plug-ins are installed for USER, SYSTEM or if you want NO installation")
set (prefix "/usr/local" CACHE STRING "Path prefix for custom installation")

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# Actual project configuration
project (IEMPluginSuite VERSION 1.14.1 LANGUAGES C CXX)

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

# set copy paths for built plug-ins
if (IEM_POST_BUILD_INSTALL STREQUAL "NO")
    set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD OFF)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)

    if (IEM_POST_BUILD_INSTALL STREQUAL "SYSTEM")
        set (prefix "/Library/Audio/Plug-Ins")
    else()  # Defaulting to USER
        set (prefix "$ENV{HOME}/Library/Audio/Plug-Ins")
    endif()

    set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "${prefix}/VST/IEM")
    set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "${prefix}/VST3/IEM")
    set_directory_properties (PROPERTIES JUCE_LV2_COPY_DIR "${prefix}/LV2")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)

    if ((IEM_POST_BUILD_INSTALL STREQUAL "USER") AND (prefix STREQUAL "/usr/local"))
        if (IEM_BUILD_VST3 OR IEM_BUILD_VST2)
            message (WARNING "VST and VST3 plug-ins are installed for all users by default as there is no officially supported consistent directory for local installation. Using SYSTEM for those. You can provide a costum prefix to change this behaviour.")
            set (IEM_POST_BUILD_INSTALL "SYSTEM")
        endif()

        set_directory_properties(PROPERTIES JUCE_LV2_COPY_DIR "$ENV{APPDATA}/LV2")
    endif()

    if (IEM_POST_BUILD_INSTALL STREQUAL "USER")
        set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "${prefix}/VST")
        set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "${prefix}/VST3")
        set_directory_properties(PROPERTIES JUCE_LV2_COPY_DIR "${prefix}/LV2")

    else()  # Defaulting to SYSTEM as this is the only valid path on Windows for VST and VST3
        set_directory_properties(PROPERTIES JUCE_LV2_COPY_DIR "$ENV{CommonProgramFiles}/LV2")

        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "$ENV{ProgramW6432}/Steinberg/VSTplugins/IEM")
            set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "$ENV{CommonProgramFiles}/VST3/IEM")
        else()
            set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "$ENV{ProgramFiles(x86)}/Steinberg/Vstplugins/IEM")
            set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "$ENV{CommonProgramFiles(x86)}/VST3/IEM")
        endif()

    endif()


elseif ((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
    set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)

    if (IEM_POST_BUILD_INSTALL STREQUAL "SYSTEM")
        set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "${prefix}/lib/vst/IEM")
        set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "${prefix}/lib/vst3/IEM")
        set_directory_properties (PROPERTIES JUCE_LV2_COPY_DIR "${prefix}/lib/lv2")
    else()  # Defaulting to USER
        set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "$ENV{HOME}/.vst/IEM")
        set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "$ENV{HOME}/.vst3/IEM")
        set_directory_properties (PROPERTIES JUCE_LV2_COPY_DIR "$ENV{HOME}/.lv2")
    endif()
endif()

# add JUCE dependency
if (IEM_USE_SYSTEM_JUCE)
    find_package(JUCE CONFIG REQUIRED)
else()
    add_subdirectory (JUCE)
endif()

# formats
set (IEM_FORMATS "")

if (IEM_BUILD_VST2)
    SET (VST2SDKPATH "/usr/include" CACHE STRING "Path to the VST2 SDK")
    # checking VST2 SDK
    if (VST2SDKPATH STREQUAL "/usr/include")
        CHECK_INCLUDE_FILES(pluginterfaces/vst2.x/aeffectx.h HAVE_VST2_HEADERS)
        IF (NOT HAVE_VST2_HEADERS)
            message (FATAL_ERROR "You have to specify the VST2SDKPATH variable with the path to the VST2 SDK if you want to build VST2 versions.")
        endif()
    endif()
    juce_set_vst2_sdk_path (${VST2SDKPATH})
    list (APPEND IEM_FORMATS VST)
    message ("-- IEM: Building VST2 versions")
endif()

if (IEM_BUILD_VST3)
    message ("-- IEM: Building VST3 versions")
    list (APPEND IEM_FORMATS VST3)
endif()

if (IEM_BUILD_LV2)
    message ("-- IEM: Building LV2 versions")
    list (APPEND IEM_FORMATS LV2)
endif()

if (IEM_BUILD_STANDALONE)
    message ("-- IEM: Building Standalone versions")
    list (APPEND IEM_FORMATS Standalone)
endif()

include_directories (resources/ /usr/local/include)
add_compile_definitions (DONT_SET_USING_JUCE_NAMESPACE=1
                         JUCE_MODAL_LOOPS_PERMITTED=1)

juce_add_binary_data (LAF_fonts SOURCES
    resources/lookAndFeel/Roboto-Bold.ttf
    resources/lookAndFeel/Roboto-Light.ttf
    resources/lookAndFeel/Roboto-Medium.ttf
    resources/lookAndFeel/Roboto-Regular.ttf)
    
juce_add_binary_data (IEM_logo 
    HEADER_NAME "IEMLogo.h"
    NAMESPACE "IEMLogo"
    SOURCES
    resources/Standalone/IEMPluginSuiteSmall.png)

foreach (subproject IN LISTS PLUGINS_TO_BUILD)
    add_subdirectory (${subproject})
endforeach()


# link fftw if necessary, only needed by BinauralDecoder, and not on macOS
if (BinauralDecoder IN_LIST PLUGINS_TO_BUILD AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows") # build fftw3f on windows
        set (BUILD_SHARED_LIBS OFF)
        set (BUILD_TESTS OFF)
        set (ENABLE_FLOAT ON)
        set (ENABLE_SSE ON)

        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        add_subdirectory (fftw)
        set_target_properties ("fftw3f" PROPERTIES POSITION_INDEPENDET_CODE ON)
    endif()

    target_compile_definitions ("BinauralDecoder" PUBLIC JUCE_DSP_USE_STATIC_FFTW=1)
    target_link_libraries ("BinauralDecoder" PRIVATE fftw3f)
endif()


if (IEM_BUILD_STANDALONE)
    foreach (subproject IN LISTS PLUGINS_TO_BUILD)
        target_sources(${subproject} PRIVATE
            resources/Standalone/StandaloneApp.cpp
            resources/Standalone/MyStandaloneFilterWindow.h
            resources/Standalone/IEM_JackAudio.h
            resources/Standalone/IEM_AudioDeviceSelectorComponent.cpp
            resources/Standalone/IEM_AudioDeviceSelectorComponent.h)
    endforeach()

    if (IEM_STANDALONE_HEADLESS)
        foreach (subproject IN LISTS PLUGINS_TO_BUILD)
            target_compile_definitions(${subproject} PUBLIC HEADLESS_BUILD=1)
        endforeach()
    endif()
    if (NOT IEM_STANDALONE_JACK_SUPPORT)
        foreach (subproject IN LISTS PLUGINS_TO_BUILD)
            target_compile_definitions(${subproject} PUBLIC DONT_BUILD_WITH_JACK_SUPPORT=1)
        endforeach()
    endif()
endif()
