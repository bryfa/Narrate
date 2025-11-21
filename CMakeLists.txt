cmake_minimum_required(VERSION 3.22)

project(Narrate VERSION 0.1.0)

# ============================================================================
# Build Configuration Options
# ============================================================================

# BUILD_TARGET: Controls what gets built
# Options: ALL, PLUGIN, STANDALONE, CONSOLE
# - ALL: Build everything (VST3 plugin + Standalone app + Console tool)
# - PLUGIN: Build only VST3 plugin
# - STANDALONE: Build only standalone application
# - CONSOLE: Build only console export tool
set(BUILD_TARGET "ALL" CACHE STRING "Choose what to build: ALL, PLUGIN, STANDALONE, CONSOLE")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS "ALL" "PLUGIN" "STANDALONE" "CONSOLE")

# Determine what formats to build based on BUILD_TARGET
if(BUILD_TARGET STREQUAL "ALL")
    set(BUILD_FORMATS "VST3" "Standalone")
    set(BUILD_CONSOLE_APP ON)
    message(STATUS "Building ALL targets: VST3 Plugin + Standalone + Console")
elseif(BUILD_TARGET STREQUAL "PLUGIN")
    set(BUILD_FORMATS "VST3")
    set(BUILD_CONSOLE_APP OFF)
    message(STATUS "Building PLUGIN target: VST3 only")
elseif(BUILD_TARGET STREQUAL "STANDALONE")
    set(BUILD_FORMATS "Standalone")
    set(BUILD_CONSOLE_APP OFF)
    message(STATUS "Building STANDALONE target: Standalone app only")
elseif(BUILD_TARGET STREQUAL "CONSOLE")
    set(BUILD_FORMATS "")
    set(BUILD_CONSOLE_APP ON)
    message(STATUS "Building CONSOLE target: Console tool only")
else()
    message(FATAL_ERROR "Invalid BUILD_TARGET: ${BUILD_TARGET}. Must be ALL, PLUGIN, STANDALONE, or CONSOLE")
endif()

# Add JUCE
add_subdirectory(JUCE)

# ============================================================================
# Plugin/Standalone Target (only if formats are specified)
# ============================================================================

if(BUILD_FORMATS)
    # Create the plugin/standalone target
    juce_add_plugin(Narrate
        COMPANY_NAME "MulhacenLabs"
        PLUGIN_MANUFACTURER_CODE Mlhn
        PLUGIN_CODE Nrrt
        FORMATS ${BUILD_FORMATS}
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
        Source/TimelineEventManager.cpp
        Source/ScrollingRenderStrategy.cpp
        Source/KaraokeRenderStrategy.cpp
        Source/TeleprompterRenderStrategy.cpp
        Source/NarrateLookAndFeel.cpp
        Source/NarrateDataModel.cpp
        Source/WaveformDisplay.cpp

        # Feature implementations
        Source/Features/StandaloneAudioPlayback.cpp
        Source/Features/StandaloneExportFeature.cpp
        Source/Features/StandaloneImportFeature.cpp
        Source/Features/PluginDawSyncFeature.cpp

        # UI Panels
        Source/UI/AudioPlaybackPanel.cpp
        Source/UI/ExportPanel.cpp
        Source/UI/DawSyncPanel.cpp
        Source/UI/ProgressWindow.cpp
        Source/UI/ToastNotification.cpp
        Source/UI/SummaryDialog.cpp
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

endif() # BUILD_FORMATS

# ============================================================================
# Console Application (for command-line export)
# ============================================================================

if(BUILD_CONSOLE_APP)
    message(STATUS "Building console application is enabled")

    # Create console executable
    add_executable(NarrateConsole
        Source/Console/NarrateConsole.cpp

        # Shared source files needed for import/export
        Source/NarrateDataModel.cpp
        Source/Features/StandaloneExportFeature.cpp
        Source/Features/StandaloneImportFeature.cpp
    )

    # Set C++ standard for console app
    target_compile_features(NarrateConsole PUBLIC cxx_std_20)

    # Link required libraries
    target_link_libraries(NarrateConsole
        PRIVATE
            juce::juce_core
            juce::juce_graphics
    )

    # Add include directories
    target_include_directories(NarrateConsole
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
    )

    # Add preprocessor definitions
    target_compile_definitions(NarrateConsole
        PRIVATE
            NARRATE_ENABLE_SUBTITLE_EXPORT=1
    )

    # On Linux, link required system libraries
    if(UNIX AND NOT APPLE)
        target_link_libraries(NarrateConsole
            PRIVATE
                curl
                pthread
                dl
        )
    endif()

endif()

# ============================================================================
# Testing with Catch2 (Optional - only enabled for development builds)
# ============================================================================

option(BUILD_TESTS "Build the test suite" OFF)

if(BUILD_TESTS)
    message(STATUS "Building tests is enabled")

    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )

    FetchContent_MakeAvailable(Catch2)

    # Create test executable
    add_executable(NarrateTests
        Tests/TestMain.cpp
        Tests/Unit/NarrateDataModelTests.cpp

        # Add source files needed for testing
        Source/NarrateDataModel.cpp
    )

    # Set C++ standard for tests
    target_compile_features(NarrateTests PUBLIC cxx_std_20)

    # Link test dependencies
    target_link_libraries(NarrateTests
        PRIVATE
            Catch2::Catch2WithMain
            juce::juce_core
            juce::juce_data_structures
            juce::juce_graphics
    )

    # On Linux, link required system libraries
    if(UNIX AND NOT APPLE)
        target_link_libraries(NarrateTests
            PRIVATE
                curl
                pthread
                dl
        )
    endif()

    # Add include directories for tests
    target_include_directories(NarrateTests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
    )

    # Enable testing and register tests
    enable_testing()
    include(CTest)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(NarrateTests)
endif()
