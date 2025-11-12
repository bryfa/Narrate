#pragma once

#include "RenderStrategy.h"

/**
 * Scrolling render strategy - displays all clips with the current clip centered.
 * Clips scroll vertically as playback progresses.
 */
class ScrollingRenderStrategy : public RenderStrategy
{
public:
    enum class TextAlignment
    {
        Left,
        Center,
        Right
    };

    ScrollingRenderStrategy();
    ~ScrollingRenderStrategy() override;

    void render (juce::Graphics& g, const RenderContext& context) override;
    juce::String getName() const override;

    // Configuration setters
    void setWordSpacing (float spacing) { wordSpacing = spacing; }
    void setLineSpacing (float spacing) { lineSpacing = spacing; }
    void setClipSpacing (float spacing) { clipSpacing = spacing; }
    void setTextAlignment (TextAlignment alignment) { textAlignment = alignment; }

    // Configuration getters
    float getWordSpacing() const { return wordSpacing; }
    float getLineSpacing() const { return lineSpacing; }
    float getClipSpacing() const { return clipSpacing; }
    TextAlignment getTextAlignment() const { return textAlignment; }

private:
    // Line information structure
    struct LineInfo
    {
        int startWordIndex;
        int endWordIndex;
        float totalWidth;
    };

    // Helper methods to simplify render logic
    void drawClip (juce::Graphics& g, const RenderContext& context,
                   int clipIndex, float clipY, const juce::Rectangle<int>& area,
                   float baseFontSize, float lineHeight);

    std::vector<LineInfo> calculateLineBreaks (const Narrate::NarrateClip& clip,
                                                float baseFontSize, float maxWidth) const;

    void renderLine (juce::Graphics& g, const RenderContext& context,
                     const Narrate::NarrateClip& clip, const LineInfo& line,
                     int clipIndex, float y, const juce::Rectangle<int>& area,
                     float baseFontSize, float lineHeight);

    float calculateLineStartX (const juce::Rectangle<int>& area, float lineWidth) const;

    // Configurable properties
    float wordSpacing = 10.0f;
    float lineSpacing = 1.1f;  // Multiplier for line height
    float clipSpacing = 2.0f;  // Multiplier for extra space between clips
    TextAlignment textAlignment = TextAlignment::Center;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingRenderStrategy)
};
