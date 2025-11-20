#pragma once

#include "NarrateDataModel.h"
#include "HighlightSettings.h"
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
        WordEnd,
        HighlightEnd  // NEW: Separate event for when highlight should end
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
    std::function<void(int clipIndex, int wordIndex)> onHighlightEnd;  // NEW

    // Initialize the timeline from a project with highlight settings
    void buildTimeline (const Narrate::NarrateProject& project,
                        const HighlightSettings& settings = HighlightSettings());

    // Process all events that occurred between previousTime and currentTime
    void processEvents (double previousTime, double currentTime);

    // Reset the timeline
    void reset();

    // Seek to a specific time position (resets event index to process events from this time)
    void seekToTime (double time);

    // Get the complete timeline (for debugging/inspection)
    const std::vector<TimeEvent>& getTimeline() const { return timeline; }

private:
    std::vector<TimeEvent> timeline;
    size_t nextEventIndex = 0;

    // Helper to calculate highlight duration based on settings
    double calculateHighlightDuration (double wordDuration, double wordStartTime,
                                        const HighlightSettings& settings) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineEventManager)
};
