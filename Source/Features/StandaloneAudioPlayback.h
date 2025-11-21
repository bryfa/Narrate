#pragma once

#include "AudioPlaybackFeature.h"
#include "../NarrateConfig.h"

#if NARRATE_ENABLE_AUDIO_PLAYBACK

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>

/**
 * StandaloneAudioPlayback
 *
 * Full implementation of audio playback for Standalone builds.
 */
class StandaloneAudioPlayback : public AudioPlaybackFeature
{
public:
    StandaloneAudioPlayback();
    ~StandaloneAudioPlayback() override;

    // Feature availability
    bool isAvailable() const override { return true; }

    // Audio file management
    bool loadAudioFile(const juce::File& file) override;
    bool hasAudioLoaded() const override;
    juce::File getLoadedAudioFile() const override { return loadedAudioFile; }

    // Playback control
    void startPlayback() override;
    void stopPlayback() override;
    void pausePlayback() override;
    bool isPlaying() const override;

    // Position control
    double getPosition() const override;
    void setPosition(double positionInSeconds) override;
    double getDuration() const override;

    // Waveform data access
    void getThumbnailData(int channel, double startTime, double endTime,
                         float* samples, int numSamples) override;

    // Audio processing (called by PluginProcessor)
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::File loadedAudioFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneAudioPlayback)
};

#endif // NARRATE_ENABLE_AUDIO_PLAYBACK
