#include "AudioPlaybackPanel.h"
#include "../PluginProcessor.h"

AudioPlaybackPanel::AudioPlaybackPanel(NarrateAudioProcessor* processor)
    : audioProcessor(processor)
{
#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Setup audio loading button
    loadAudioButton.onClick = [this] { loadAudioClicked(); };
    addAndMakeVisible(loadAudioButton);

    audioFileLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    audioFileLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(audioFileLabel);

    // Transport controls
    playPauseButton.onClick = [this] { playPauseClicked(); };
    addAndMakeVisible(playPauseButton);

    stopButton.onClick = [this] { stopClicked(); };
    addAndMakeVisible(stopButton);

    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.onValueChange = [this] { positionSliderChanged(); };
    addAndMakeVisible(positionSlider);

    positionLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    positionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(positionLabel);

    // Waveform display
    addAndMakeVisible(waveformDisplay);

    // Start timer for updating UI (10Hz)
    startTimer(100);
#endif
}

AudioPlaybackPanel::~AudioPlaybackPanel()
{
#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    stopTimer();
#endif
}

void AudioPlaybackPanel::paint(juce::Graphics& g)
{
#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Draw background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(getLocalBounds());

    // Draw border
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);
#else
    juce::ignoreUnused(g);
#endif
}

void AudioPlaybackPanel::resized()
{
#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    auto area = getLocalBounds().reduced(5);

    // First row: Load Audio button and file label
    auto topRow = area.removeFromTop(25);
    loadAudioButton.setBounds(topRow.removeFromLeft(100));
    topRow.removeFromLeft(5);
    audioFileLabel.setBounds(topRow);

    area.removeFromTop(5);

    // Second row: Transport controls
    auto transportRow = area.removeFromTop(30);
    playPauseButton.setBounds(transportRow.removeFromLeft(60));
    transportRow.removeFromLeft(5);
    stopButton.setBounds(transportRow.removeFromLeft(60));
    transportRow.removeFromLeft(10);
    positionLabel.setBounds(transportRow.removeFromLeft(100));
    transportRow.removeFromLeft(5);
    positionSlider.setBounds(transportRow);

    // Waveform takes remaining space (if visible)
    if (waveformDisplay.isVisible())
    {
        area.removeFromTop(5);
        waveformDisplay.setBounds(area);
    }
#endif
}

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON

void AudioPlaybackPanel::loadAudioClicked()
{
    auto chooser = std::make_shared<juce::FileChooser>("Select an audio file...",
                                                         juce::File(),
                                                         "*.wav;*.mp3;*.aif;*.aiff;*.flac");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File())
            return;

        if (audioProcessor->loadAudioFile(file))
        {
            audioFileLabel.setText(file.getFileName(), juce::dontSendNotification);
            waveformDisplay.loadURL(file);
            waveformDisplay.setVisible(true);
            updateUI();
            resized();
        }
    });
}

void AudioPlaybackPanel::playPauseClicked()
{
    if (audioProcessor->isAudioPlaying())
    {
        audioProcessor->pauseAudioPlayback();
        playPauseButton.setButtonText("Play");
    }
    else
    {
        audioProcessor->startAudioPlayback();
        playPauseButton.setButtonText("Pause");
    }
}

void AudioPlaybackPanel::stopClicked()
{
    audioProcessor->stopAudioPlayback();
    playPauseButton.setButtonText("Play");
}

void AudioPlaybackPanel::positionSliderChanged()
{
    if (!positionSlider.isMouseButtonDown())
        return;

    double duration = audioProcessor->getAudioDuration();
    double newPosition = positionSlider.getValue() * duration;
    audioProcessor->setAudioPosition(newPosition);
}

void AudioPlaybackPanel::updateUI()
{
    if (!audioProcessor->hasAudioLoaded())
        return;

    double position = audioProcessor->getAudioPosition();
    double duration = audioProcessor->getAudioDuration();

    if (duration > 0.0)
    {
        positionSlider.setValue(position / duration, juce::dontSendNotification);
    }

    // Update time label
    int posMin = static_cast<int>(position) / 60;
    int posSec = static_cast<int>(position) % 60;
    int durMin = static_cast<int>(duration) / 60;
    int durSec = static_cast<int>(duration) % 60;

    juce::String timeText = juce::String::formatted("%02d:%02d / %02d:%02d",
                                                     posMin, posSec, durMin, durSec);
    positionLabel.setText(timeText, juce::dontSendNotification);

    // Update waveform playback position
    if (duration > 0.0)
    {
        waveformDisplay.setRelativePosition(position / duration);
    }
}

void AudioPlaybackPanel::timerCallback()
{
    updateUI();
}

#endif // NARRATE_SHOW_LOAD_AUDIO_BUTTON
