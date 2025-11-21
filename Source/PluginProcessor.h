#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Features/FeatureFactory.h"
#include "Features/AudioPlaybackFeature.h"
#include "Features/ExportFeature.h"
#include "Features/ImportFeature.h"
#include "Features/DawSyncFeature.h"
#include <memory>

/**
 * NarrateAudioProcessor
 *
 * Refactored to use feature components instead of conditional compilation.
 * Audio playback, export, and DAW sync are now handled by polymorphic feature objects.
 */
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

    // Audio processing
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Plugin info
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Programs
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // State saving
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Feature access (no conditional compilation needed!)
    AudioPlaybackFeature& getAudioPlayback() { return *audioPlayback; }
    ExportFeature& getExportFeature() { return *exportFeature; }
    ImportFeature& getImportFeature() { return *importFeature; }
    DawSyncFeature& getDawSync() { return *dawSync; }

    // Convenience methods (delegate to features)
    bool loadAudioFile(const juce::File& file) { return audioPlayback->loadAudioFile(file); }
    void startAudioPlayback() { audioPlayback->startPlayback(); }
    void stopAudioPlayback() { audioPlayback->stopPlayback(); }
    void pauseAudioPlayback() { audioPlayback->pausePlayback(); }
    bool isAudioPlaying() const { return audioPlayback->isPlaying(); }
    double getAudioPosition() const { return audioPlayback->getPosition(); }
    void setAudioPosition(double pos) { audioPlayback->setPosition(pos); }
    double getAudioDuration() const { return audioPlayback->getDuration(); }
    bool hasAudioLoaded() const { return audioPlayback->hasAudioLoaded(); }

private:
    std::unique_ptr<juce::PropertiesFile> settings;
    juce::ValueTree state;

    // Feature components (created by FeatureFactory based on build target)
    std::unique_ptr<AudioPlaybackFeature> audioPlayback;
    std::unique_ptr<ExportFeature> exportFeature;
    std::unique_ptr<ImportFeature> importFeature;
    std::unique_ptr<DawSyncFeature> dawSync;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NarrateAudioProcessor)
};
