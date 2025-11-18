#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../NarrateConfig.h"

class NarrateAudioProcessor;

/**
 * DawSyncPanel
 *
 * UI panel for DAW synchronization indicator and controls.
 * Full implementation in Plugin, no-op in Standalone.
 */
class DawSyncPanel : public juce::Component
{
public:
    DawSyncPanel(NarrateAudioProcessor* processor);
    ~DawSyncPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    // Only available in Plugin
    void updateSyncStatus(bool isPlaying, double position, double bpm);
#endif

private:
    NarrateAudioProcessor* audioProcessor;

#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    // Plugin-only members
    juce::Label dawSyncLabel {"", "DAW Sync: Disabled"};
    juce::ToggleButton enableSyncButton {"Enable DAW Sync"};

    void syncToggleChanged();
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DawSyncPanel)
};
