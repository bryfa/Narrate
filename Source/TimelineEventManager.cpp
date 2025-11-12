#include "TimelineEventManager.h"
#include <algorithm>
#include <cmath>

TimelineEventManager::TimelineEventManager()
{
}

TimelineEventManager::~TimelineEventManager()
{
}

void TimelineEventManager::buildTimeline (const Narrate::NarrateProject& project,
                                          const HighlightSettings& settings)
{
    timeline.clear();

    // Get quantization interval if enabled
    double snapInterval = settings.getSnapInterval();

    // Build a sorted list of all time events in the project
    for (int clipIndex = 0; clipIndex < project.getNumClips(); ++clipIndex)
    {
        const auto& clip = project.getClip (clipIndex);

        // Quantize clip start time if enabled
        double clipStartTime = clip.getStartTime();
        if (settings.quantizeEnabled)
            clipStartTime = settings.quantizeTime (clipStartTime);

        // Add clip start event
        timeline.push_back ({clipStartTime, EventType::ClipStart, clipIndex, -1});

        // Track the current time for quantization
        double currentTime = clipStartTime;

        // Add word events
        for (int wordIndex = 0; wordIndex < clip.getNumWords(); ++wordIndex)
        {
            const auto& word = clip.getWords()[wordIndex];
            double wordAbsoluteTime = clip.getStartTime() + word.relativeTime;

            // Quantize word start time if enabled
            if (settings.quantizeEnabled)
            {
                wordAbsoluteTime = settings.quantizeTime (wordAbsoluteTime);
                // Ensure word doesn't start before current time
                if (wordAbsoluteTime < currentTime)
                    wordAbsoluteTime = currentTime;
            }

            // Add word start event
            timeline.push_back ({wordAbsoluteTime, EventType::WordStart, clipIndex, wordIndex});

            // Calculate original word duration
            double wordDuration;
            if (wordIndex < clip.getNumWords() - 1)
            {
                const auto& nextWord = clip.getWords()[wordIndex + 1];
                wordDuration = (clip.getStartTime() + nextWord.relativeTime) - wordAbsoluteTime;
            }
            else
            {
                wordDuration = clip.getEndTime() - wordAbsoluteTime;
            }

            // Calculate highlight duration based on settings
            double highlightDuration = calculateHighlightDuration (wordDuration, wordAbsoluteTime, settings);

            // Add highlight end event (when highlight should disappear)
            timeline.push_back ({wordAbsoluteTime + highlightDuration, EventType::HighlightEnd, clipIndex, wordIndex});

            // Calculate word end time (for data tracking)
            double wordEndTime = wordAbsoluteTime + wordDuration;

            // Add word end event
            timeline.push_back ({wordEndTime, EventType::WordEnd, clipIndex, wordIndex});

            // Update current time for next word
            currentTime = wordEndTime;
        }

        // Add clip end event
        timeline.push_back ({clip.getEndTime(), EventType::ClipEnd, clipIndex, -1});
    }

    // Sort timeline by time
    std::sort (timeline.begin(), timeline.end());

    // Reset the event index
    nextEventIndex = 0;
}

double TimelineEventManager::calculateHighlightDuration (double wordDuration,
                                                         double wordStartTime,
                                                         const HighlightSettings& settings) const
{
    using DurationMode = HighlightSettings::DurationMode;

    switch (settings.durationMode)
    {
        case DurationMode::Original:
            // Use original word duration
            return wordDuration;

        case DurationMode::Minimum:
            // Use at least the minimum duration
            return std::max (wordDuration, settings.minimumDuration);

        case DurationMode::Fixed:
            // Use fixed duration for all words
            return settings.fixedDuration;

        case DurationMode::GridBased:
        {
            // Highlight until next grid position
            double snapInterval = settings.getSnapInterval();
            if (snapInterval > 0.0)
            {
                // Calculate next grid position
                double nextGridTime = std::ceil (wordStartTime / snapInterval) * snapInterval;
                if (nextGridTime <= wordStartTime)
                    nextGridTime += snapInterval;

                return nextGridTime - wordStartTime;
            }
            // Fall back to original if no valid grid
            return wordDuration;
        }

        default:
            return wordDuration;
    }
}

void TimelineEventManager::processEvents (double previousTime, double currentTime)
{
    // Process all events that fall within the time range [previousTime, currentTime)
    while (nextEventIndex < timeline.size())
    {
        const auto& event = timeline[nextEventIndex];

        // Check if this event should fire
        if (event.time < previousTime)
        {
            // This event is in the past, skip it
            nextEventIndex++;
            continue;
        }

        if (event.time >= currentTime)
        {
            // This event is in the future, stop processing
            break;
        }

        // Fire the event
        switch (event.type)
        {
            case EventType::ClipStart:
                if (onClipStart)
                    onClipStart (event.clipIndex);
                break;

            case EventType::ClipEnd:
                if (onClipEnd)
                    onClipEnd (event.clipIndex);
                break;

            case EventType::WordStart:
                if (onWordStart)
                    onWordStart (event.clipIndex, event.wordIndex);
                break;

            case EventType::WordEnd:
                if (onWordEnd)
                    onWordEnd (event.clipIndex, event.wordIndex);
                break;

            case EventType::HighlightEnd:
                if (onHighlightEnd)
                    onHighlightEnd (event.clipIndex, event.wordIndex);
                break;
        }

        nextEventIndex++;
    }
}

void TimelineEventManager::reset()
{
    timeline.clear();
    nextEventIndex = 0;
}
