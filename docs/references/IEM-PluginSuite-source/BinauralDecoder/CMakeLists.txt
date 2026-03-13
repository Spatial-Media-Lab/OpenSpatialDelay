
juce_add_plugin (BinauralDecoder
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE BiDe
    ICON_BIG "../resources/Standalone/IEMPluginSuiteSmall.png"
    ICON_SMALL "../resources/Standalone/IEMPluginSuiteSmall.png"
    COMPANY_NAME "IEM"
    PRODUCT_NAME "BinauralDecoder"
    FORMATS ${IEM_FORMATS}
    VERSION "0.6.5"
    LV2URI "http://plugins.iem.at/BinauralDecoder")


juce_generate_juce_header(BinauralDecoder)

target_sources (BinauralDecoder PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h

    ../resources/OSC/OSCInputStream.h
    ../resources/OSC/OSCParameterInterface.cpp
    ../resources/OSC/OSCParameterInterface.h
    ../resources/OSC/OSCStatus.cpp
    ../resources/OSC/OSCStatus.h
    ../resources/OSC/OSCUtilities.h

    ../resources/efficientSHvanilla.cpp
    )

target_compile_definitions (BinauralDecoder PRIVATE
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

target_include_directories (BinauralDecoder PRIVATE Source)


juce_add_binary_data (BinauralDecoderEQs
    HEADER_NAME "EQData.h"
    NAMESPACE EQData
    SOURCES
        Source/EQ/AKG-K141MK2.wav
        Source/EQ/AKG-K240DF.wav
        Source/EQ/AKG-K240MK2.wav
        Source/EQ/AKG-K271MK2.wav
        Source/EQ/AKG-K271STUDIO.wav
        Source/EQ/AKG-K601.wav
        Source/EQ/AKG-K701.wav
        Source/EQ/AKG-K702.wav
        Source/EQ/AKG-K1000-Closed.wav
        Source/EQ/AKG-K1000-Open.wav
        Source/EQ/AudioTechnica-ATH-M50.wav
        Source/EQ/Beyerdynamic-DT250.wav
        Source/EQ/Beyerdynamic-DT770PRO-250Ohms.wav
        Source/EQ/Beyerdynamic-DT880.wav
        Source/EQ/Beyerdynamic-DT990PRO.wav
        Source/EQ/Presonus-HD7.wav
        Source/EQ/Sennheiser-HD430.wav
        Source/EQ/Sennheiser-HD560ovationII.wav
        Source/EQ/Sennheiser-HD480.wav
        Source/EQ/Sennheiser-HD565ovation.wav
        Source/EQ/Sennheiser-HD600.wav
        Source/EQ/Sennheiser-HD650.wav
        Source/EQ/SHURE-SRH940.wav)

juce_add_binary_data (BinauralDecoderIRs
    HEADER_NAME "IRData.h"
    NAMESPACE IRData
    SOURCES
        Source/IRs/irsOrd1.wav
        Source/IRs/irsOrd7.wav
        Source/IRs/irsOrd6.wav
        Source/IRs/irsOrd5.wav
        Source/IRs/irsOrd4.wav
        Source/IRs/irsOrd3.wav
        Source/IRs/irsOrd2.wav)

target_link_libraries (BinauralDecoder PRIVATE
    LAF_fonts
    IEM_logo
    BinauralDecoderEQs
    BinauralDecoderIRs
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
