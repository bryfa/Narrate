#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"
#include "TimelineEventManager.h"
#include "RenderStrategy.h"
#include "HighlightSettings.h"
#include <functional>
#include <memory>

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

    // Set the rendering strategy
    void setRenderStrategy (std::unique_ptr<RenderStrategy> strategy);

    // Highlight settings management
    void setHighlightSettings (const HighlightSettings& newSettings);
    HighlightSettings& getHighlightSettings() { return highlightSettings; }
    const HighlightSettings& getHighlightSettings() const { return highlightSettings; }

    // Set a callback for when the Stop button is clicked
    std::function<void()> onStopClicked;

private:
    void timerCallback() override;

    juce::TextButton stopButton;
    Narrate::NarrateProject project;

    double currentTime = 0.0;  // Current playback time in seconds
    double previousTime = 0.0;  // Previous time (for event detection)
    bool isRunning = false;
    int currentClipIndex = 0;  // Index of the currently active clip
    int currentWordIndex = -1;  // Index of the currently active word (event-based)

    // Timer runs at 60fps for smooth updates
    static constexpr int timerIntervalMs = 16;  // ~60fps

    // Time event system
    TimelineEventManager eventManager;

    // Rendering strategy
    std::unique_ptr<RenderStrategy> renderStrategy;

    // Highlight settings (configurable)
    HighlightSettings highlightSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunningView)
};
