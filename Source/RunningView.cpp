#include "RunningView.h"
#include "ScrollingRenderStrategy.h"

RunningView::RunningView()
{
    // Setup stop button
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this]
    {
        stop();
        if (onStopClicked)
            onStopClicked();
    };
    addAndMakeVisible (stopButton);

    // Initialize with default scrolling strategy
    renderStrategy = std::make_unique<ScrollingRenderStrategy>();
}

RunningView::~RunningView()
{
    stopTimer();
}

void RunningView::paint (juce::Graphics& g)
{
    if (!renderStrategy)
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("No render strategy", getLocalBounds(), juce::Justification::centred);
        return;
    }

    // Create render context with event-based indices
    RenderStrategy::RenderContext context {
        project,
        currentTime,
        currentClipIndex,
        isRunning,
        getLocalBounds(),
        currentClipIndex,  // clipIndex for rendering (same as currentClipIndex)
        currentWordIndex   // wordIndex from events
    };

    // Delegate to the strategy
    renderStrategy->render (g, context);
}

void RunningView::resized()
{
    auto area = getLocalBounds().reduced (10);
    stopButton.setBounds (area.removeFromBottom (40));
}

void RunningView::start (const Narrate::NarrateProject& newProject)
{
    project = newProject;
    currentTime = 0.0;
    previousTime = 0.0;
    isRunning = true;
    currentClipIndex = 0;
    currentWordIndex = -1;

    // Setup event callbacks on the event manager
    eventManager.onClipStart = [this] (int clipIndex)
    {
        currentClipIndex = clipIndex;
        repaint();
    };

    eventManager.onWordStart = [this] (int clipIndex, int wordIndex)
    {
        currentWordIndex = wordIndex;
        repaint();
    };

    // Build the timeline of events
    eventManager.buildTimeline (project);

    startTimer (timerIntervalMs);  // ~60fps
    repaint();
}

void RunningView::stop()
{
    isRunning = false;
    stopTimer();
    currentTime = 0.0;
    repaint();
}

void RunningView::setRenderStrategy (std::unique_ptr<RenderStrategy> strategy)
{
    renderStrategy = std::move (strategy);
    repaint();
}

void RunningView::timerCallback()
{
    if (!isRunning)
        return;

    // Store previous time for event detection
    previousTime = currentTime;

    // Advance time
    currentTime += timerIntervalMs / 1000.0;  // Convert ms to seconds

    // Process events with look-ahead to compensate for render latency
    // This helps ensure highlights appear at the right time on screen
    constexpr double lookAheadMs = 25.0;  // 25ms look-ahead
    double lookAheadTime = currentTime + (lookAheadMs / 1000.0);
    eventManager.processEvents (previousTime, lookAheadTime);

    // Check if we've finished
    if (currentTime >= project.getTotalDuration())
    {
        stop();
        if (onStopClicked)
            onStopClicked();
        return;
    }

    repaint();
}
