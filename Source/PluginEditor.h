#pragma once

#include "PluginProcessor.h"
#include "EditorView.h"
#include "RunningView.h"
#include "NarrateLookAndFeel.h"
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
    void toggleTheme();
    void loadTheme();
    void saveTheme();

    NarrateAudioProcessor& processorRef;

    juce::TextButton fullscreenButton;
    juce::TextButton themeToggleButton;
    bool isFullScreen = false;
    bool isStandalone = false;

    // View components
    EditorView editorView;
    RunningView runningView;
    bool showingEditor = true;

    // Custom LookAndFeel
    NarrateLookAndFeel narrateLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NarrateAudioProcessorEditor)
};
