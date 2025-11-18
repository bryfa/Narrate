#pragma once

#include "DawSyncFeature.h"

/**
 * NoOpDawSyncFeature
 *
 * No-op implementation for Standalone builds (no DAW to sync with).
 */
class NoOpDawSyncFeature : public DawSyncFeature
{
public:
    NoOpDawSyncFeature() = default;
    ~NoOpDawSyncFeature() override = default;

    bool isAvailable() const override { return false; }

    bool isSyncEnabled() const override { return false; }
    void setSyncEnabled(bool) override {}

    double getCurrentPosition(juce::AudioPlayHead*) override { return 0.0; }
    bool isPlaying(juce::AudioPlayHead*) override { return false; }

    double getBPM(juce::AudioPlayHead*) override { return 120.0; }
    int getTimeSignatureNumerator(juce::AudioPlayHead*) override { return 4; }
    int getTimeSignatureDenominator(juce::AudioPlayHead*) override { return 4; }
};
