#pragma once

#include "AudioPlaybackFeature.h"

/**
 * NoOpAudioPlayback
 *
 * No-op implementation of audio playback for Plugin builds.
 * All methods return safe defaults and do nothing.
 */
class NoOpAudioPlayback : public AudioPlaybackFeature
{
public:
    NoOpAudioPlayback() = default;
    ~NoOpAudioPlayback() override = default;

    // Feature availability
    bool isAvailable() const override { return false; }

    // Audio file management
    bool loadAudioFile(const juce::File&) override { return false; }
    bool hasAudioLoaded() const override { return false; }
    juce::File getLoadedAudioFile() const override { return juce::File(); }

    // Playback control
    void startPlayback() override {}
    void stopPlayback() override {}
    void pausePlayback() override {}
    bool isPlaying() const override { return false; }

    // Position control
    double getPosition() const override { return 0.0; }
    void setPosition(double) override {}
    double getDuration() const override { return 0.0; }

    // Waveform data access
    void getThumbnailData(int, double, double, float*, int) override {}
};
