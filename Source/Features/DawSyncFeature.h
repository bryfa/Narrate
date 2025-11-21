#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * DawSyncFeature
 *
 * Interface for DAW transport synchronization.
 */
class DawSyncFeature
{
public:
    virtual ~DawSyncFeature() = default;

    // Feature availability
    virtual bool isAvailable() const = 0;

    // Sync state
    virtual bool isSyncEnabled() const = 0;
    virtual void setSyncEnabled(bool enabled) = 0;

    // Position from DAW
    virtual double getCurrentPosition(juce::AudioPlayHead* playHead) = 0;
    virtual bool isPlaying(juce::AudioPlayHead* playHead) = 0;

    // Tempo information
    virtual double getBPM(juce::AudioPlayHead* playHead) = 0;
    virtual int getTimeSignatureNumerator(juce::AudioPlayHead* playHead) = 0;
    virtual int getTimeSignatureDenominator(juce::AudioPlayHead* playHead) = 0;
};
