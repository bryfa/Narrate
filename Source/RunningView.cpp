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

    // Setup navigation buttons
    previousClipButton.onClick = [this] { previousClipClicked(); };
    addAndMakeVisible (previousClipButton);

    nextClipButton.onClick = [this] { nextClipClicked(); };
    addAndMakeVisible (nextClipButton);

    jumpBackButton.onClick = [this] { jumpBackClicked(); };
    addAndMakeVisible (jumpBackButton);

    jumpForwardButton.onClick = [this] { jumpForwardClicked(); };
    addAndMakeVisible (jumpForwardButton);

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

    // Bottom control bar - two rows
    auto controlBar = area.removeFromBottom (90);

    // Top row: Navigation buttons (4 buttons)
    auto navRow = controlBar.removeFromTop (40);
    int buttonWidth = navRow.getWidth() / 4;

    previousClipButton.setBounds (navRow.removeFromLeft (buttonWidth).reduced (2));
    jumpBackButton.setBounds (navRow.removeFromLeft (buttonWidth).reduced (2));
    jumpForwardButton.setBounds (navRow.removeFromLeft (buttonWidth).reduced (2));
    nextClipButton.setBounds (navRow.removeFromLeft (buttonWidth).reduced (2));

    controlBar.removeFromTop (5);  // Spacing

    // Bottom row: Stop button (full width)
    stopButton.setBounds (controlBar.removeFromTop (40));
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

void RunningView::previousClipClicked()
{
    if (!isRunning || currentClipIndex <= 0)
        return;

    // Jump to the previous clip
    currentClipIndex--;

    // Set time to the start of the previous clip
    if (currentClipIndex >= 0 && currentClipIndex < project.getNumClips())
    {
        auto& clip = project.getClip(currentClipIndex);
        currentTime = clip.getStartTime();
        currentWordIndex = -1;  // Reset word index

        // Seek event manager to new time position
        eventManager.seekToTime(currentTime);

        // Update previousTime to enable proper event processing
        // Set it slightly before currentTime so processEvents will fire
        previousTime = juce::jmax(0.0, currentTime - 0.001);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
        // Update audio position if audio is loaded
        if (audioProcessor && audioProcessor->hasAudioLoaded())
        {
            audioProcessor->setAudioPosition(currentTime);
        }
#endif

        // Trigger waveform position update callback
        if (onPositionChanged)
            onPositionChanged();

        repaint();
    }
}

void RunningView::nextClipClicked()
{
    if (!isRunning || currentClipIndex >= project.getNumClips() - 1)
        return;

    // Jump to the next clip
    currentClipIndex++;

    // Set time to the start of the next clip
    if (currentClipIndex >= 0 && currentClipIndex < project.getNumClips())
    {
        auto& clip = project.getClip(currentClipIndex);
        currentTime = clip.getStartTime();
        currentWordIndex = -1;  // Reset word index

        // Seek event manager to new time position
        eventManager.seekToTime(currentTime);

        // Update previousTime to enable proper event processing
        // Set it slightly before currentTime so processEvents will fire
        previousTime = juce::jmax(0.0, currentTime - 0.001);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
        // Update audio position if audio is loaded
        if (audioProcessor && audioProcessor->hasAudioLoaded())
        {
            audioProcessor->setAudioPosition(currentTime);
        }
#endif

        // Trigger waveform position update callback
        if (onPositionChanged)
            onPositionChanged();

        repaint();
    }
}

void RunningView::jumpBackClicked()
{
    if (!isRunning)
        return;

    // Jump back 5 seconds
    constexpr double jumpAmount = 5.0;
    currentTime = juce::jmax(0.0, currentTime - jumpAmount);

    // Seek event manager to new time position
    eventManager.seekToTime(currentTime);

    // Update previousTime to enable proper event processing
    // Set it slightly before currentTime so processEvents will fire
    previousTime = juce::jmax(0.0, currentTime - 0.001);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Update audio position if audio is loaded
    if (audioProcessor && audioProcessor->hasAudioLoaded())
    {
        audioProcessor->setAudioPosition(currentTime);
    }
#endif

    // Update clip index based on new time
    for (int i = 0; i < project.getNumClips(); ++i)
    {
        auto& clip = project.getClip(i);
        if (currentTime >= clip.getStartTime() && currentTime < clip.getEndTime())
        {
            currentClipIndex = i;
            currentWordIndex = -1;  // Reset word index
            break;
        }
    }

    // Trigger waveform position update callback
    if (onPositionChanged)
        onPositionChanged();

    repaint();
}

void RunningView::jumpForwardClicked()
{
    if (!isRunning)
        return;

    // Jump forward 5 seconds
    constexpr double jumpAmount = 5.0;
    double totalDuration = project.getTotalDuration();
    currentTime = juce::jmin(totalDuration, currentTime + jumpAmount);

    // Seek event manager to new time position
    eventManager.seekToTime(currentTime);

    // Update previousTime to enable proper event processing
    // Set it slightly before currentTime so processEvents will fire
    previousTime = juce::jmax(0.0, currentTime - 0.001);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Update audio position if audio is loaded
    if (audioProcessor && audioProcessor->hasAudioLoaded())
    {
        audioProcessor->setAudioPosition(currentTime);
    }
#endif

    // Update clip index based on new time
    for (int i = 0; i < project.getNumClips(); ++i)
    {
        auto& clip = project.getClip(i);
        if (currentTime >= clip.getStartTime() && currentTime < clip.getEndTime())
        {
            currentClipIndex = i;
            currentWordIndex = -1;  // Reset word index
            break;
        }
    }

    // Trigger waveform position update callback
    if (onPositionChanged)
        onPositionChanged();

    repaint();
}
