#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "NarrateConfig.h"

class WaveformDisplay : public juce::Component,
                        public juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Load audio file and generate waveform thumbnail
    void loadURL (const juce::File& file);

    // Set current playback position (0.0 to 1.0)
    void setRelativePosition (double position);

    // ChangeListener callback for thumbnail
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    double relativePosition = 0.0;  // Current playback position (0.0 to 1.0)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
