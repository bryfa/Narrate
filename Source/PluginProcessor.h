#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "NarrateConfig.h"

#if NARRATE_ENABLE_AUDIO_PLAYBACK
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#endif

class NarrateAudioProcessor : public juce::AudioProcessor
{
public:
    NarrateAudioProcessor();
    ~NarrateAudioProcessor() override;

    // Settings management (for global preferences like theme)
    juce::PropertiesFile::Options getSettingsOptions();
    juce::PropertiesFile* getSettings() { return settings.get(); }

    // Text state management (per-instance, saved in DAW projects)
    void setEditorText(const juce::String& text);
    juce::String getEditorText() const;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Audio playback functionality
    bool loadAudioFile (const juce::File& file);
    void startAudioPlayback();
    void stopAudioPlayback();
    void pauseAudioPlayback();
    bool isAudioPlaying() const;
    double getAudioPosition() const;
    void setAudioPosition (double positionInSeconds);
    double getAudioDuration() const;
    bool hasAudioLoaded() const;
    juce::File getLoadedAudioFile() const { return loadedAudioFile; }
#endif

private:
    std::unique_ptr<juce::PropertiesFile> settings;
    juce::ValueTree state;

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Audio playback members
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::MixerAudioSource mixerSource;
    juce::File loadedAudioFile;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NarrateAudioProcessor)
};
