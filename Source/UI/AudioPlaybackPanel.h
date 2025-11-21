#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../NarrateConfig.h"

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    #include "../WaveformDisplay.h"
#endif

class NarrateAudioProcessor;

/**
 * AudioPlaybackPanel
 *
 * UI panel for audio playback controls and waveform display.
 * Full implementation in Standalone, no-op in Plugin.
 */
class AudioPlaybackPanel : public juce::Component
#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
                          , private juce::Timer
#endif
{
public:
    AudioPlaybackPanel(NarrateAudioProcessor* processor);
    ~AudioPlaybackPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Only available in Standalone
    void updateUI();
#endif

private:
    NarrateAudioProcessor* audioProcessor;

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Standalone-only members
    juce::TextButton loadAudioButton {"Load Audio"};
    juce::Label audioFileLabel {"", "No audio loaded"};
    juce::TextButton playPauseButton {"Play"};
    juce::TextButton stopButton {"Stop"};
    juce::Slider positionSlider;
    juce::Label positionLabel {"", "00:00 / 00:00"};
    WaveformDisplay waveformDisplay;

    void loadAudioClicked();
    void playPauseClicked();
    void stopClicked();
    void positionSliderChanged();
    void timerCallback() override;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlaybackPanel)
};
