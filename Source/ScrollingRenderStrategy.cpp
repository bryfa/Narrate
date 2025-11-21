#include "ScrollingRenderStrategy.h"

ScrollingRenderStrategy::ScrollingRenderStrategy()
{
}

ScrollingRenderStrategy::~ScrollingRenderStrategy()
{
}

void ScrollingRenderStrategy::render (juce::Graphics& g, const RenderContext& context)
{
    g.fillAll (juce::Colours::black);

    if (context.project.getNumClips() == 0)
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("No project loaded", context.bounds, juce::Justification::centred);
        return;
    }

    auto area = context.bounds.reduced (20);
    area.removeFromBottom (60); // Space for stop button

    float baseFontSize = context.project.getDefaultFontSize();
    float lineHeight = baseFontSize * lineSpacing;
    float extraClipSpacing = lineHeight * clipSpacing;

    // Calculate vertical center position
    float centerY = area.getY() + (area.getHeight() / 2.0f) - (lineHeight / 2.0f);

    // Draw all clips with scrolling centered on current clip
    for (int clipIndex = 0; clipIndex < context.project.getNumClips(); ++clipIndex)
    {
        // Calculate vertical offset for this clip
        float clipYOffset = (clipIndex - context.currentClipIndex) * (lineHeight + extraClipSpacing);
        float clipY = centerY + clipYOffset;

        // Skip clips that are way off screen (optimization)
        if (clipY < area.getY() - 200.0f || clipY > area.getBottom() + 200.0f)
            continue;

        drawClip (g, context, clipIndex, clipY, area, baseFontSize, lineHeight);
    }

    // Draw timer at bottom
    g.setColour (juce::Colours::grey);
    g.setFont (14.0f);
    auto timerText = juce::String::formatted ("Time: %.2fs / %.2fs",
                                               context.currentTime,
                                               context.project.getTotalDuration());
    auto timerArea = area.reduced (0, 0);
    timerArea.setY (area.getBottom() - 20);
    timerArea.setHeight (20);
    g.drawText (timerText, timerArea, juce::Justification::centredLeft);
}

void ScrollingRenderStrategy::drawClip (juce::Graphics& g, const RenderContext& context,
                                        int clipIndex, float clipY,
                                        const juce::Rectangle<int>& area,
                                        float baseFontSize, float lineHeight)
{
    const auto& clip = context.project.getClip (clipIndex);
    float maxWidth = static_cast<float>(area.getWidth()) - 40.0f;

    // Calculate line breaks
    auto lines = calculateLineBreaks (clip, baseFontSize, maxWidth);

    // Render each line
    float y = clipY;
    for (const auto& line : lines)
    {
        renderLine (g, context, clip, line, clipIndex, y, area, baseFontSize, lineHeight);
        y += lineHeight;
    }
}

std::vector<ScrollingRenderStrategy::LineInfo> ScrollingRenderStrategy::calculateLineBreaks (
    const Narrate::NarrateClip& clip, float baseFontSize, float maxWidth) const
{
    std::vector<LineInfo> lines;
    const auto& words = clip.getWords();
    float currentLineWidth = 0.0f;
    int lineStartIndex = 0;

    for (int wordIndex = 0; wordIndex < words.size(); ++wordIndex)
    {
        const auto& word = words[wordIndex];
        auto formatting = word.getEffectiveFormatting (clip.getDefaultFormatting());

        // Calculate word width
        auto fontHeight = baseFontSize * formatting.fontSizeMultiplier;
        juce::Font font {juce::FontOptions {fontHeight}};
        if (formatting.bold) font.setBold (true);
        if (formatting.italic) font.setItalic (true);

        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (font, word.text, 0, 0);
        float wordWidth = glyphs.getBoundingBox (0, -1, false).getWidth();

        // Check if word fits on current line
        float widthWithWord = currentLineWidth + wordWidth;
        if (wordIndex > lineStartIndex)
            widthWithWord += wordSpacing;

        if (widthWithWord > maxWidth && wordIndex > lineStartIndex)
        {
            // Save current line and start new one
            lines.push_back ({lineStartIndex, wordIndex - 1, currentLineWidth});
            lineStartIndex = wordIndex;
            currentLineWidth = wordWidth;
        }
        else
        {
            if (wordIndex > lineStartIndex)
                currentLineWidth += wordSpacing;
            currentLineWidth += wordWidth;
        }
    }

    // Add the last line
    if (lineStartIndex < words.size())
        lines.push_back ({lineStartIndex, static_cast<int>(words.size()) - 1, currentLineWidth});

    return lines;
}

void ScrollingRenderStrategy::renderLine (juce::Graphics& g, const RenderContext& context,
                                           const Narrate::NarrateClip& clip, const LineInfo& line,
                                           int clipIndex, float y, const juce::Rectangle<int>& area,
                                           float baseFontSize, float lineHeight)
{
    const auto& words = clip.getWords();
    float x = calculateLineStartX (area, line.totalWidth);

    for (int wordIndex = line.startWordIndex; wordIndex <= line.endWordIndex; ++wordIndex)
    {
        const auto& word = words[wordIndex];
        auto formatting = word.getEffectiveFormatting (clip.getDefaultFormatting());

        // Set font with formatting
        auto fontHeight = baseFontSize * formatting.fontSizeMultiplier;
        juce::Font font {juce::FontOptions {fontHeight}};
        if (formatting.bold) font.setBold (true);
        if (formatting.italic) font.setItalic (true);
        g.setFont (font);

        // Calculate word width
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (font, word.text, 0, 0);
        float wordWidth = glyphs.getBoundingBox (0, -1, false).getWidth();

        // Determine if this word should be highlighted
        bool isCurrentClip = (clipIndex == context.clipIndex);
        bool isCurrentWord = isCurrentClip && (wordIndex == context.wordIndex);
        bool shouldHighlight = isCurrentWord && context.isRunning &&
                               context.currentTime < context.project.getTotalDuration();

        if (shouldHighlight)
        {
            // Draw highlight background
            g.setColour (context.project.getHighlightColour());
            g.fillRect (x - 5.0f, y - 5.0f, wordWidth + 10.0f, lineHeight);
        }

        // Draw word text
        g.setColour (shouldHighlight ? juce::Colours::black : formatting.colour);
        g.drawText (word.text,
                    juce::Rectangle<float>(x, y, wordWidth, lineHeight - 5.0f),
                    juce::Justification::left);

        x += wordWidth + wordSpacing;
    }
}

float ScrollingRenderStrategy::calculateLineStartX (const juce::Rectangle<int>& area, float lineWidth) const
{
    switch (textAlignment)
    {
        case TextAlignment::Left:
            return static_cast<float>(area.getX());

        case TextAlignment::Center:
            return static_cast<float>(area.getCentreX()) - (lineWidth / 2.0f);

        case TextAlignment::Right:
            return static_cast<float>(area.getRight()) - lineWidth;

        default:
            return static_cast<float>(area.getX());
    }
}

juce::String ScrollingRenderStrategy::getName() const
{
    return "Scrolling";
}
