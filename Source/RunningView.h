#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"
#include <functional>

class RunningView : public juce::Component, private juce::Timer
{
public:
    RunningView();
    ~RunningView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Start playing from a NarrateProject
    void start (const Narrate::NarrateProject& project);

    // Stop the playback
    void stop();

    // Set a callback for when the Stop button is clicked
    std::function<void()> onStopClicked;

private:
    void timerCallback() override;

    // Find which clip and word should be displayed at current time
    struct DisplayState
    {
        int clipIndex = -1;
        int wordIndex = -1;
    };
    DisplayState getCurrentDisplayState() const;

    juce::TextButton stopButton;
    Narrate::NarrateProject project;

    double currentTime = 0.0;  // Current playback time in seconds
    bool isRunning = false;

    // Timer runs at 60fps for smooth updates
    static constexpr int timerIntervalMs = 16;  // ~60fps

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunningView)
};
