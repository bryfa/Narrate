#pragma once

#include "../NarrateConfig.h"
#include "AudioPlaybackFeature.h"
#include "ExportFeature.h"
#include "DawSyncFeature.h"

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    #include "StandaloneAudioPlayback.h"
#else
    #include "NoOpAudioPlayback.h"
#endif

#if NARRATE_ENABLE_SUBTITLE_EXPORT
    #include "StandaloneExportFeature.h"
#else
    #include "NoOpExportFeature.h"
#endif
#if NARRATE_ENABLE_DAW_TRANSPORT_SYNC
    #include "PluginDawSyncFeature.h"
#else
    #include "NoOpDawSyncFeature.h"
#endif

#include <memory>

/**
 * FeatureFactory
 *
 * Central factory for creating feature implementations based on build target.
 * This is the ONLY place where conditional compilation for feature selection occurs.
 */
class FeatureFactory
{
public:
    /**
     * Create appropriate AudioPlaybackFeature implementation.
     * - Standalone: Full audio playback with transport controls
     * - Plugin: No-op (DAW handles audio)
     */
    static std::unique_ptr<AudioPlaybackFeature> createAudioPlayback()
    {
#if NARRATE_ENABLE_AUDIO_PLAYBACK
        return std::make_unique<StandaloneAudioPlayback>();
#else
        return std::make_unique<NoOpAudioPlayback>();
#endif
    }

    /**
     * Create appropriate ExportFeature implementation.
     * - Standalone: Full export (SRT, WebVTT, etc.)
     * - Plugin: No-op (use DAW export)
     */
    static std::unique_ptr<ExportFeature> createExportFeature()
    {
#if NARRATE_ENABLE_SUBTITLE_EXPORT
        return std::make_unique<StandaloneExportFeature>();
#else
        return std::make_unique<NoOpExportFeature>();
#endif
    }

    /**
     * Create appropriate DawSyncFeature implementation.
     * - Standalone: No-op (no DAW to sync with)
     * - Plugin: Full DAW transport sync
     */
    static std::unique_ptr<DawSyncFeature> createDawSyncFeature()
    {
#if NARRATE_ENABLE_DAW_TRANSPORT_SYNC
        return std::make_unique<PluginDawSyncFeature>();
#else
        return std::make_unique<NoOpDawSyncFeature>();
#endif
    }
};
