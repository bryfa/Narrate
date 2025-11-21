#pragma once

#include <algorithm>

/**
 * Configurable settings for word highlighting behavior.
 * Supports different use cases: storytelling (natural timing),
 * rhythmic (quantized to tempo), and teleprompter modes.
 */
struct HighlightSettings
{
    // Quantization settings
    bool quantizeEnabled = false;    // Toggle quantization on/off
    double bpm = 120.0;              // Tempo in beats per minute
    int subdivision = 4;             // 1=whole, 2=half, 4=quarter, 8=eighth notes

    // Highlight duration control
    enum class DurationMode
    {
        Original,      // Use word's original duration (natural, story telling)
        Minimum,       // Ensure minimum readability duration
        Fixed,         // All words have same duration
        GridBased      // Highlight until next grid position (rhythmic)
    };

    DurationMode durationMode = DurationMode::Original;
    double minimumDuration = 0.0;    // Minimum duration in seconds (0 = disabled)
    double fixedDuration = 0.5;      // Fixed duration in seconds (when mode = Fixed)

    // Look-ahead for render timing compensation
    double lookAheadMs = 25.0;       // Milliseconds to compensate for render latency

    // Preset factory methods for common use cases

    /**
     * Natural preset - Original timing, no quantization.
     * Best for: Story telling, audiobooks, natural speech.
     */
    static HighlightSettings naturalPreset()
    {
        HighlightSettings settings;
        settings.quantizeEnabled = false;
        settings.durationMode = DurationMode::Original;
        settings.minimumDuration = 0.0;
        return settings;
    }

    /**
     * Storytelling preset - Natural timing with minimum duration for readability.
     * Best for: Story telling, audiobooks where short words need visibility.
     */
    static HighlightSettings storytellingPreset()
    {
        HighlightSettings settings;
        settings.quantizeEnabled = false;
        settings.durationMode = DurationMode::Minimum;
        settings.minimumDuration = 0.2;  // 200ms minimum for readability
        return settings;
    }

    /**
     * Rhythmic preset - Quantized to tempo grid with grid-based durations.
     * Best for: Music, rhythmic content, synchronized timing.
     */
    static HighlightSettings rhythmicPreset (double bpm = 120.0, int subdivision = 4)
    {
        HighlightSettings settings;
        settings.quantizeEnabled = true;
        settings.bpm = bpm;
        settings.subdivision = subdivision;
        settings.durationMode = DurationMode::GridBased;
        return settings;
    }

    /**
     * Teleprompter preset - Good readability with minimum duration.
     * Best for: Teleprompter, voice acting, scripts.
     */
    static HighlightSettings teleprompterPreset()
    {
        HighlightSettings settings;
        settings.quantizeEnabled = false;
        settings.durationMode = DurationMode::Minimum;
        settings.minimumDuration = 0.3;  // 300ms for comfortable reading
        return settings;
    }

    /**
     * Fixed duration preset - All words highlighted for same duration.
     * Best for: Consistent timing, educational content.
     */
    static HighlightSettings fixedDurationPreset (double duration = 0.5)
    {
        HighlightSettings settings;
        settings.quantizeEnabled = false;
        settings.durationMode = DurationMode::Fixed;
        settings.fixedDuration = duration;
        return settings;
    }

    // Helper methods

    /**
     * Calculate the snap interval based on BPM and subdivision.
     * Returns 0.0 if quantization is disabled.
     */
    double getSnapInterval() const
    {
        if (!quantizeEnabled || bpm <= 0.0)
            return 0.0;

        double beatDuration = 60.0 / bpm;
        return beatDuration / subdivision;
    }

    /**
     * Quantize a time value to the nearest grid position.
     */
    double quantizeTime (double time) const
    {
        double snapInterval = getSnapInterval();
        if (snapInterval <= 0.0)
            return time;

        return std::round(time / snapInterval) * snapInterval;
    }
};
