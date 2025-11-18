#pragma once

#include "NarrateConfig.h"

/**
 * FeatureRegistry
 *
 * Centralized feature availability queries.
 * This provides a clean API for checking feature availability without
 * scattering #if directives throughout the codebase.
 */
class FeatureRegistry
{
public:
    enum class Feature
    {
        AudioPlayback,
        AudioFileLoading,
        WaveformDisplay,
        TransportControls,
        SubtitleExport,
        DawTransportSync,
        DawAutomation,
        TimelineEditor
    };

    // Feature availability queries
    static constexpr bool isEnabled(Feature feature)
    {
        switch (feature)
        {
            case Feature::AudioPlayback:
                return NARRATE_ENABLE_AUDIO_PLAYBACK;
            case Feature::AudioFileLoading:
                return NARRATE_ENABLE_AUDIO_FILE_LOADING;
            case Feature::WaveformDisplay:
                return NARRATE_ENABLE_WAVEFORM_DISPLAY;
            case Feature::TransportControls:
                return NARRATE_ENABLE_TRANSPORT_CONTROLS;
            case Feature::SubtitleExport:
                return NARRATE_ENABLE_SUBTITLE_EXPORT;
            case Feature::DawTransportSync:
                return NARRATE_ENABLE_DAW_TRANSPORT_SYNC;
            case Feature::DawAutomation:
                return NARRATE_ENABLE_DAW_AUTOMATION;
            case Feature::TimelineEditor:
                return NARRATE_SHOW_TIMELINE_EDITOR;
            default:
                return false;
        }
    }

    // UI component visibility queries
    static constexpr bool shouldShowLoadAudioButton()
    {
        return NARRATE_SHOW_LOAD_AUDIO_BUTTON;
    }

    static constexpr bool shouldShowExportMenu()
    {
        return NARRATE_SHOW_EXPORT_MENU;
    }

    static constexpr bool shouldShowDawSyncIndicator()
    {
        return NARRATE_SHOW_DAW_SYNC_INDICATOR;
    }

    static constexpr bool shouldShowTimelineEditor()
    {
        return NARRATE_SHOW_TIMELINE_EDITOR;
    }

    // Build target queries
    static constexpr bool isStandalone()
    {
        return NARRATE_STANDALONE;
    }

    static constexpr bool isPlugin()
    {
        return NARRATE_PLUGIN;
    }

    // Convenience helper: check if any audio features are enabled
    static constexpr bool hasAudioFeatures()
    {
        return isEnabled(Feature::AudioPlayback) ||
               isEnabled(Feature::AudioFileLoading);
    }

    // Convenience helper: check if any export features are enabled
    static constexpr bool hasExportFeatures()
    {
        return isEnabled(Feature::SubtitleExport);
    }

    // Convenience helper: check if any DAW features are enabled
    static constexpr bool hasDawFeatures()
    {
        return isEnabled(Feature::DawTransportSync) ||
               isEnabled(Feature::DawAutomation);
    }
};
