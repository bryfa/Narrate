#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"
#include <functional>
#include <vector>

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

    // Time event callbacks
    std::function<void(int clipIndex)> onClipStart;
    std::function<void(int clipIndex)> onClipEnd;
    std::function<void(int clipIndex, int wordIndex)> onWordStart;
    std::function<void(int clipIndex, int wordIndex)> onWordEnd;

private:
    void timerCallback() override;

    // Time event system
    enum class EventType
    {
        ClipStart,
        ClipEnd,
        WordStart,
        WordEnd
    };

    struct TimeEvent
    {
        double time;
        EventType type;
        int clipIndex;
        int wordIndex;  // Only used for word events

        bool operator< (const TimeEvent& other) const { return time < other.time; }
    };

    void buildTimeline();
    void processEvents (double previousTime, double newTime);

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
    double previousTime = 0.0;  // Previous time (for event detection)
    bool isRunning = false;
    int currentClipIndex = 0;  // Index of the currently active clip

    // Timer runs at 60fps for smooth updates
    static constexpr int timerIntervalMs = 16;  // ~60fps

    // Time event system
    std::vector<TimeEvent> timeline;
    size_t nextEventIndex = 0;  // Index of next event to fire

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunningView)
};
