#include "RunningView.h"
#include "ScrollingRenderStrategy.h"
#include "PluginProcessor.h"

RunningView::RunningView(NarrateAudioProcessor* processor)
    : audioProcessor(processor)
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

    eventManager.onHighlightEnd = [this] (int clipIndex, int wordIndex)
    {
        // Highlight ended - just trigger repaint to update visual state
        // Don't clear currentWordIndex - it should remain until next word starts
        repaint();
    };

    // Build the timeline of events with current highlight settings
    eventManager.buildTimeline (project, highlightSettings);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Start audio playback if loaded
    if (audioProcessor && audioProcessor->hasAudioLoaded())
    {
        audioProcessor->setAudioPosition (0.0);
        audioProcessor->startAudioPlayback();
    }
#endif

    startTimer (timerIntervalMs);  // ~60fps
    repaint();
}

void RunningView::stop()
{
    isRunning = false;
    stopTimer();
    currentTime = 0.0;

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Stop audio playback
    if (audioProcessor && audioProcessor->isAudioPlaying())
    {
        audioProcessor->stopAudioPlayback();
    }
#endif

    repaint();
}

void RunningView::setRenderStrategy (std::unique_ptr<RenderStrategy> strategy)
{
    renderStrategy = std::move (strategy);
    repaint();
}

void RunningView::setHighlightSettings (const HighlightSettings& newSettings)
{
    highlightSettings = newSettings;

    // Rebuild timeline if project is loaded
    if (project.getNumClips() > 0 && isRunning)
    {
        eventManager.buildTimeline (project, highlightSettings);
    }
}

void RunningView::timerCallback()
{
    if (!isRunning)
        return;

    // Store previous time for event detection
    previousTime = currentTime;

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Sync time from audio position if audio is playing
    if (audioProcessor && audioProcessor->isAudioPlaying())
    {
        currentTime = audioProcessor->getAudioPosition();
    }
    else
    {
        // No audio or not playing: advance time manually
        currentTime += timerIntervalMs / 1000.0;  // Convert ms to seconds
    }
#else
    // Plugin: Always advance time manually (no audio playback)
    currentTime += timerIntervalMs / 1000.0;  // Convert ms to seconds
#endif

    // Process events with look-ahead from settings to compensate for render latency
    double lookAheadTime = currentTime + (highlightSettings.lookAheadMs / 1000.0);
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
