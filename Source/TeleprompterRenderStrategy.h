#pragma once

#include "RenderStrategy.h"

/**
 * Teleprompter render strategy - displays scrolling text with current word at fixed position.
 * Text continuously scrolls upward, keeping the current word at the "read line".
 * Optimized for voice acting, presentations, and script reading.
 */
class TeleprompterRenderStrategy : public RenderStrategy
{
public:
    TeleprompterRenderStrategy();
    ~TeleprompterRenderStrategy() override;

    void render (juce::Graphics& g, const RenderContext& context) override;
    juce::String getName() const override;

    // Configuration setters
    void setWordSpacing (float spacing) { wordSpacing = spacing; }
    void setLineSpacing (float spacing) { lineSpacing = spacing; }
    void setReadLinePosition (float position) { readLinePosition = position; }
    void setShowReadLine (bool show) { showReadLine = show; }

    // Configuration getters
    float getWordSpacing() const { return wordSpacing; }
    float getLineSpacing() const { return lineSpacing; }
    float getReadLinePosition() const { return readLinePosition; }
    bool getShowReadLine() const { return showReadLine; }

private:
    // Line information structure
    struct LineInfo
    {
        int clipIndex;
        int startWordIndex;
        int endWordIndex;
        float totalWidth;
    };

    // Helper methods to build and render lines
    std::vector<LineInfo> buildAllLines (const RenderContext& context,
                                          float baseFontSize, float maxWidth) const;

    std::vector<LineInfo> calculateLineBreaksForClip (const Narrate::NarrateClip& clip,
                                                       int clipIndex,
                                                       float baseFontSize,
                                                       float maxWidth) const;

    void renderLine (juce::Graphics& g, const RenderContext& context,
                     const LineInfo& line, float y,
                     float baseFontSize, float lineHeight);

    float calculateScrollOffset (const std::vector<LineInfo>& allLines,
                                  int currentClipIndex,
                                  int currentWordIndex,
                                  float lineHeight,
                                  float areaY,
                                  float readLineY) const;

    // Configurable properties
    float wordSpacing = 10.0f;
    float lineSpacing = 1.4f;          // Multiplier for line height
    float readLinePosition = 0.33f;    // Position of read line (0.0 = top, 1.0 = bottom)
    bool showReadLine = true;          // Show visual guide line

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TeleprompterRenderStrategy)
};
