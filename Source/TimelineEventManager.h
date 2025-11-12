#pragma once

#include "NarrateDataModel.h"
#include <functional>
#include <vector>

class TimelineEventManager
{
public:
    TimelineEventManager();
    ~TimelineEventManager();

    // Event types that can occur on the timeline
    enum class EventType
    {
        ClipStart,
        ClipEnd,
        WordStart,
        WordEnd
    };

    // A single timed event
    struct TimeEvent
    {
        double time;
        EventType type;
        int clipIndex;
        int wordIndex;  // Only used for word events

        bool operator< (const TimeEvent& other) const { return time < other.time; }
    };

    // Event callbacks
    std::function<void(int clipIndex)> onClipStart;
    std::function<void(int clipIndex)> onClipEnd;
    std::function<void(int clipIndex, int wordIndex)> onWordStart;
    std::function<void(int clipIndex, int wordIndex)> onWordEnd;

    // Initialize the timeline from a project
    void buildTimeline (const Narrate::NarrateProject& project);

    // Process all events that occurred between previousTime and currentTime
    void processEvents (double previousTime, double currentTime);

    // Reset the timeline
    void reset();

    // Get the complete timeline (for debugging/inspection)
    const std::vector<TimeEvent>& getTimeline() const { return timeline; }

private:
    std::vector<TimeEvent> timeline;
    size_t nextEventIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineEventManager)
};
