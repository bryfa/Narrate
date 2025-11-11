#pragma once

#include "PluginProcessor.h"
#include "EditorView.h"
#include "RunningView.h"
#include <juce_audio_processors/juce_audio_processors.h>

class NarrateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit NarrateAudioProcessorEditor (NarrateAudioProcessor&);
    ~NarrateAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    void toggleFullScreen();
    void switchToRunningView();
    void switchToEditorView();

    NarrateAudioProcessor& processorRef;

    juce::TextButton fullscreenButton;
    bool isFullScreen = false;
    bool isStandalone = false;

    // View components
    EditorView editorView;
    RunningView runningView;
    bool showingEditor = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NarrateAudioProcessorEditor)
};
