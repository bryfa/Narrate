#pragma once

#include "RenderStrategy.h"

/**
 * Karaoke render strategy - displays current line centered with highlighted word.
 * Shows previous line (dimmed), current line (active), and next line (preview).
 * Optimized for sing-along and performance use cases.
 */
class KaraokeRenderStrategy : public RenderStrategy
{
public:
    KaraokeRenderStrategy();
    ~KaraokeRenderStrategy() override;

    void render (juce::Graphics& g, const RenderContext& context) override;
    juce::String getName() const override;

    // Configuration setters
    void setWordSpacing (float spacing) { wordSpacing = spacing; }
    void setLineSpacing (float spacing) { lineSpacing = spacing; }
    void setShowPreviousLine (bool show) { showPreviousLine = show; }
    void setShowNextLine (bool show) { showNextLine = show; }

    // Configuration getters
    float getWordSpacing() const { return wordSpacing; }
    float getLineSpacing() const { return lineSpacing; }
    bool getShowPreviousLine() const { return showPreviousLine; }
    bool getShowNextLine() const { return showNextLine; }

private:
    // Helper to find which word is at the center of current playback
    int findCurrentWordIndex (const Narrate::NarrateClip& clip, double currentTime) const;

    // Line information structure
    struct LineInfo
    {
        int startWordIndex;
        int endWordIndex;
        float totalWidth;
    };

    // Helper methods
    std::vector<LineInfo> calculateLineBreaks (const Narrate::NarrateClip& clip,
                                                float baseFontSize, float maxWidth) const;

    void renderLine (juce::Graphics& g, const RenderContext& context,
                     const Narrate::NarrateClip& clip, const LineInfo& line,
                     int clipIndex, float y, float baseFontSize, float lineHeight,
                     bool isDimmed, bool isPreview);

    float calculateLineStartX (float areaWidth, float lineWidth) const;

    // Configurable properties
    float wordSpacing = 12.0f;
    float lineSpacing = 1.5f;  // Multiplier for line height
    bool showPreviousLine = true;
    bool showNextLine = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KaraokeRenderStrategy)
};
