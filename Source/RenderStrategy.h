#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"

/**
 * Base class for different rendering strategies.
 * Allows switching between different visual effects and presentation styles.
 */
class RenderStrategy
{
public:
    /**
     * Context passed to render strategies containing all necessary state.
     */
    struct RenderContext
    {
        const Narrate::NarrateProject& project;
        double currentTime;
        int currentClipIndex;
        bool isRunning;
        juce::Rectangle<int> bounds;
        int clipIndex;      // From getCurrentDisplayState()
        int wordIndex;      // From getCurrentDisplayState()
    };

    virtual ~RenderStrategy() = default;

    /**
     * Render the project content with this strategy's visual style.
     * @param g Graphics context to draw into
     * @param context Current rendering context with project state
     */
    virtual void render (juce::Graphics& g, const RenderContext& context) = 0;

    /**
     * Get the name of this rendering strategy.
     * @return Human-readable name (e.g., "Scrolling", "Karaoke", "Teleprompter")
     */
    virtual juce::String getName() const = 0;

protected:
    RenderStrategy() = default;
};
