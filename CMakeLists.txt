
cmake_minimum_required(VERSION 3.20)
project(AIVocalAssistantEZ VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(AIVocalAssistantEZ
    COMPANY_NAME "SofianeAudio"
    BUNDLE_ID com.sofiane.aivocalassistantez
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST3_CATEGORY "Fx"
    COPY_PLUGIN_AFTER_BUILD TRUE
    FORMATS VST3
    PRODUCT_NAME "AI Vocal Assistant EZ")

target_sources(AIVocalAssistantEZ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Analyzer/FeatureExtractor.cpp
    Source/Analyzer/AutoSuggest.cpp
    Source/Processors/Denoiser.cpp
    Source/Processors/Denoiser.h
    Source/Processors/HPF.h
    Source/Processors/HPF.cpp
    Source/Processors/DeEsser.h
    Source/Processors/DeEsser.cpp
    Source/Processors/AutoGain.h
    Source/Processors/AutoGain.cpp
    Source/Processors/OneKnobSaturator.h
    Source/Processors/OneKnobSaturator.cpp
)

target_include_directories(AIVocalAssistantEZ PRIVATE Source)

target_compile_definitions(AIVocalAssistantEZ
    PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISABLE_ASSERTIONS=1
            JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AIVocalAssistantEZ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
)
