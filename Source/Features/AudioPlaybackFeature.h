#pragma once

#include <juce_core/juce_core.h>

/**
 * AudioPlaybackFeature
 *
 * Interface for audio playback functionality.
 * Standalone builds provide full implementation, plugin builds provide no-op.
 */
class AudioPlaybackFeature
{
public:
    virtual ~AudioPlaybackFeature() = default;

    // Feature availability
    virtual bool isAvailable() const = 0;

    // Audio file management
    virtual bool loadAudioFile(const juce::File& file) = 0;
    virtual bool hasAudioLoaded() const = 0;
    virtual juce::File getLoadedAudioFile() const = 0;

    // Playback control
    virtual void startPlayback() = 0;
    virtual void stopPlayback() = 0;
    virtual void pausePlayback() = 0;
    virtual bool isPlaying() const = 0;

    // Position control
    virtual double getPosition() const = 0;
    virtual void setPosition(double positionInSeconds) = 0;
    virtual double getDuration() const = 0;

    // Waveform data access (for visualization)
    virtual void getThumbnailData(int channel, double startTime, double endTime,
                                   float* samples, int numSamples) = 0;
};
