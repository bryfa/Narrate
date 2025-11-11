cmake_minimum_required(VERSION 3.22)

project(Narrate VERSION 0.1.0)

# Add JUCE
add_subdirectory(JUCE)

# Create the plugin/standalone target
juce_add_plugin(Narrate
    COMPANY_NAME "MulhacenLabs"
    PLUGIN_MANUFACTURER_CODE Mlhn
    PLUGIN_CODE Nrrt
    FORMATS VST3 Standalone
    PRODUCT_NAME "Narrate"
    VST3_CATEGORIES Fx
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Add source files
target_sources(Narrate
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/EditorView.cpp
        Source/RunningView.cpp
        Source/NarrateLookAndFeel.cpp
)

# Set C++ standard
target_compile_features(Narrate PUBLIC cxx_std_20)

# Add JUCE modules
target_link_libraries(Narrate
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Preprocessor definitions
target_compile_definitions(Narrate
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
