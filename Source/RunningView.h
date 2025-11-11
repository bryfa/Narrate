#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class RunningView : public juce::Component, private juce::Timer
{
public:
    RunningView();
    ~RunningView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Start highlighting words from the given text
    void start (const juce::String& text);

    // Stop the highlighting
    void stop();

    // Set a callback for when the Stop button is clicked
    std::function<void()> onStopClicked;

private:
    void timerCallback() override;
    void parseWords (const juce::String& text);

    juce::TextButton stopButton;
    juce::StringArray words;
    int currentWordIndex = 0;
    bool isRunning = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunningView)
};
