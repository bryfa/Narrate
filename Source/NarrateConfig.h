#pragma once

/**
 * NarrateConfig.h
 *
 * Configuration and feature flags for different build targets.
 * This file determines which features are available in Standalone vs VST3 builds.
 */

//==============================================================================
// Build Target Detection
//==============================================================================

#if JUCE_STANDALONE_APPLICATION || JucePlugin_Build_Standalone
    #define NARRATE_STANDALONE 1
    #define NARRATE_PLUGIN 0
#elif defined(NARRATE_ENABLE_SUBTITLE_EXPORT)
    // Console build - explicitly enables export via CMake preprocessor
    #define NARRATE_STANDALONE 1
    #define NARRATE_PLUGIN 0
#else
    #define NARRATE_STANDALONE 0
    #define NARRATE_PLUGIN 1
#endif

//==============================================================================
// Feature Flags - Standalone Application
//==============================================================================

#if NARRATE_STANDALONE

    // Audio playback features (standalone only)
    #define NARRATE_ENABLE_AUDIO_PLAYBACK 1
    #define NARRATE_ENABLE_AUDIO_FILE_LOADING 1
    #define NARRATE_ENABLE_WAVEFORM_DISPLAY 1
    #define NARRATE_ENABLE_TRANSPORT_CONTROLS 1

    // Export features (standalone only)
    #define NARRATE_ENABLE_SUBTITLE_EXPORT 1  // SRT, WebVTT export

    // UI features (standalone only)
    #define NARRATE_SHOW_LOAD_AUDIO_BUTTON 1
    #define NARRATE_SHOW_TIMELINE_EDITOR 1
    #define NARRATE_SHOW_EXPORT_MENU 1

    // DAW-specific features (disabled in standalone)
    #define NARRATE_ENABLE_DAW_TRANSPORT_SYNC 0
    #define NARRATE_ENABLE_DAW_AUTOMATION 0
    #define NARRATE_SHOW_DAW_SYNC_INDICATOR 0

#endif

//==============================================================================
// Feature Flags - VST3 Plugin
//==============================================================================

#if NARRATE_PLUGIN

    // Audio playback features (disabled in plugin - DAW handles audio)
    #define NARRATE_ENABLE_AUDIO_PLAYBACK 0
    #define NARRATE_ENABLE_AUDIO_FILE_LOADING 0
    #define NARRATE_ENABLE_WAVEFORM_DISPLAY 0  // Will use DAW's waveform
    #define NARRATE_ENABLE_TRANSPORT_CONTROLS 0  // DAW controls transport

    // Export features (disabled - use DAW export)
    #define NARRATE_ENABLE_SUBTITLE_EXPORT 0

    // UI features (simplified in plugin)
    #define NARRATE_SHOW_LOAD_AUDIO_BUTTON 0
    #define NARRATE_SHOW_TIMELINE_EDITOR 1  // Still useful for manual timing
    #define NARRATE_SHOW_EXPORT_MENU 0

    // DAW-specific features (enabled in plugin)
    #define NARRATE_ENABLE_DAW_TRANSPORT_SYNC 1  // TODO: Implement in Phase 2
    #define NARRATE_ENABLE_DAW_AUTOMATION 1      // TODO: Implement in Phase 2
    #define NARRATE_SHOW_DAW_SYNC_INDICATOR 1

#endif

//==============================================================================
// Common Features (Available in Both)
//==============================================================================

#define NARRATE_ENABLE_PROJECT_MANAGEMENT 1
#define NARRATE_ENABLE_CLIP_EDITING 1
#define NARRATE_ENABLE_WORD_TIMING 1
#define NARRATE_ENABLE_RENDER_STRATEGIES 1
#define NARRATE_ENABLE_FORMATTING 1
#define NARRATE_ENABLE_QUANTIZATION 1

//==============================================================================
// Build Info
//==============================================================================

namespace NarrateConfig
{
    inline const char* getBuildTarget()
    {
        #if NARRATE_STANDALONE
            return "Standalone";
        #else
            return "VST3 Plugin";
        #endif
    }

    inline const char* getProductName()
    {
        #if NARRATE_STANDALONE
            return "Narrate";
        #else
            return "Narrate VST3";
        #endif
    }

    inline bool hasAudioPlayback()
    {
        return NARRATE_ENABLE_AUDIO_PLAYBACK;
    }

    inline bool hasDawSync()
    {
        return NARRATE_ENABLE_DAW_TRANSPORT_SYNC;
    }

    inline bool canExportSubtitles()
    {
        return NARRATE_ENABLE_SUBTITLE_EXPORT;
    }
}
