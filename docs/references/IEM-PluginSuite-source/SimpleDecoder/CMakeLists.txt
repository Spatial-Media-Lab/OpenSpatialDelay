
juce_add_plugin (SimpleDecoder
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE SiDe
    ICON_BIG "../resources/Standalone/IEMPluginSuiteSmall.png"
    ICON_SMALL "../resources/Standalone/IEMPluginSuiteSmall.png"
    COMPANY_NAME "IEM"
    PRODUCT_NAME "SimpleDecoder"
    FORMATS ${IEM_FORMATS}
    VERSION "0.5.4"
    LV2URI "http://plugins.iem.at/SimpleDecoder")


juce_generate_juce_header(SimpleDecoder)

target_sources(SimpleDecoder PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/DecoderInfoBox.h

    ../resources/OSC/OSCInputStream.h
    ../resources/OSC/OSCParameterInterface.cpp
    ../resources/OSC/OSCParameterInterface.h
    ../resources/OSC/OSCStatus.cpp
    ../resources/OSC/OSCStatus.h
    ../resources/OSC/OSCUtilities.h

    ../resources/efficientSHvanilla.cpp
    )

target_compile_definitions(SimpleDecoder PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data (SimpleDecoderPresets
    HEADER_NAME "Presets.h"
    NAMESPACE Presets
    SOURCES
    Source/Presets/5point1.json
    Source/Presets/5point1point4.json
    Source/Presets/7point1.json
    Source/Presets/7point1point4.json
    Source/Presets/22_2_NHK.json
    Source/Presets/Cube_8ch.json
    Source/Presets/CUBE.json
    Source/Presets/MSDecoder.json
    Source/Presets/Produktionsstudio.json
    Source/Presets/Quadraphonic.json)


target_include_directories(SimpleDecoder PRIVATE Source)

target_link_libraries(SimpleDecoder PRIVATE
    LAF_fonts
    IEM_logo
    SimpleDecoderPresets
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
