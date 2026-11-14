cmake_minimum_required(VERSION 3.22)

project(TelephoneBeep VERSION 1.0.0)

# Clone JUCE next to this folder or adjust the path
add_subdirectory(JUCE)

juce_add_plugin(TelephoneBeep
    COMPANY_NAME "SimpleSounds"
    PLUGIN_MANUFACTURER_CODE Ssnd
    PLUGIN_CODE TelB
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Telephone Beep"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TelephoneBeep
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(TelephoneBeep
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TelephoneBeep
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
