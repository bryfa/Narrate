#include "DawSyncPanel.h"
#include "../PluginProcessor.h"

DawSyncPanel::DawSyncPanel(NarrateAudioProcessor* processor)
    : audioProcessor(processor)
{
#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    dawSyncLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    dawSyncLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(dawSyncLabel);

    enableSyncButton.onClick = [this] { syncToggleChanged(); };
    addAndMakeVisible(enableSyncButton);
#endif
}

DawSyncPanel::~DawSyncPanel()
{
}

void DawSyncPanel::paint(juce::Graphics& g)
{
#if !NARRATE_SHOW_DAW_SYNC_INDICATOR
    juce::ignoreUnused(g);
#endif
}

void DawSyncPanel::resized()
{
#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    auto area = getLocalBounds();

    enableSyncButton.setBounds(area.removeFromLeft(150));
    area.removeFromLeft(10);
    dawSyncLabel.setBounds(area);
#endif
}

#if NARRATE_SHOW_DAW_SYNC_INDICATOR

void DawSyncPanel::syncToggleChanged()
{
    // TODO: Implement DAW sync enable/disable
    bool enabled = enableSyncButton.getToggleState();

    if (enabled)
    {
        dawSyncLabel.setText("DAW Sync: Enabled", juce::dontSendNotification);
        dawSyncLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    else
    {
        dawSyncLabel.setText("DAW Sync: Disabled", juce::dontSendNotification);
        dawSyncLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
}

void DawSyncPanel::updateSyncStatus(bool isPlaying, double position, double bpm)
{
    juce::String status = juce::String::formatted(
        "DAW: %s | Pos: %.2fs | BPM: %.1f",
        isPlaying ? "Playing" : "Stopped",
        position,
        bpm);

    dawSyncLabel.setText(status, juce::dontSendNotification);
}

#endif // NARRATE_SHOW_DAW_SYNC_INDICATOR
