#include "TimelineEventManager.h"
#include <algorithm>

TimelineEventManager::TimelineEventManager()
{
}

TimelineEventManager::~TimelineEventManager()
{
}

void TimelineEventManager::buildTimeline (const Narrate::NarrateProject& project)
{
    timeline.clear();

    // Build a sorted list of all time events in the project
    for (int clipIndex = 0; clipIndex < project.getNumClips(); ++clipIndex)
    {
        const auto& clip = project.getClip (clipIndex);

        // Add clip start event
        timeline.push_back ({clip.getStartTime(), EventType::ClipStart, clipIndex, -1});

        // Add clip end event
        timeline.push_back ({clip.getEndTime(), EventType::ClipEnd, clipIndex, -1});

        // Add word events
        for (int wordIndex = 0; wordIndex < clip.getNumWords(); ++wordIndex)
        {
            const auto& word = clip.getWords()[wordIndex];
            double wordAbsoluteTime = clip.getStartTime() + word.relativeTime;

            // Add word start event
            timeline.push_back ({wordAbsoluteTime, EventType::WordStart, clipIndex, wordIndex});

            // Calculate word end time (start of next word, or clip end)
            double wordEndTime;
            if (wordIndex < clip.getNumWords() - 1)
            {
                const auto& nextWord = clip.getWords()[wordIndex + 1];
                wordEndTime = clip.getStartTime() + nextWord.relativeTime;
            }
            else
            {
                wordEndTime = clip.getEndTime();
            }

            // Add word end event
            timeline.push_back ({wordEndTime, EventType::WordEnd, clipIndex, wordIndex});
        }
    }

    // Sort timeline by time
    std::sort (timeline.begin(), timeline.end());

    // Reset the event index
    nextEventIndex = 0;
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
        }

        nextEventIndex++;
    }
}

void TimelineEventManager::reset()
{
    timeline.clear();
    nextEventIndex = 0;
}
