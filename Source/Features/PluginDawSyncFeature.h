#pragma once

#include "DawSyncFeature.h"
#include "../NarrateConfig.h"

#if NARRATE_ENABLE_DAW_TRANSPORT_SYNC

/**
 * PluginDawSyncFeature
 *
 * Full implementation of DAW transport synchronization for Plugin builds.
 */
class PluginDawSyncFeature : public DawSyncFeature
{
public:
    PluginDawSyncFeature();
    ~PluginDawSyncFeature() override = default;

    // Feature availability
    bool isAvailable() const override { return true; }

    // Sync state
    bool isSyncEnabled() const override { return syncEnabled; }
    void setSyncEnabled(bool enabled) override;

    // Position from DAW
    double getCurrentPosition(juce::AudioPlayHead* playHead) override;
    bool isPlaying(juce::AudioPlayHead* playHead) override;

    // Tempo information
    double getBPM(juce::AudioPlayHead* playHead) override;
    int getTimeSignatureNumerator(juce::AudioPlayHead* playHead) override;
    int getTimeSignatureDenominator(juce::AudioPlayHead* playHead) override;

private:
    bool syncEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginDawSyncFeature)
};

#endif // NARRATE_ENABLE_DAW_TRANSPORT_SYNC
