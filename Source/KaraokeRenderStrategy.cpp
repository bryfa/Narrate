#include "KaraokeRenderStrategy.h"

KaraokeRenderStrategy::KaraokeRenderStrategy()
{
}

KaraokeRenderStrategy::~KaraokeRenderStrategy()
{
}

void KaraokeRenderStrategy::render (juce::Graphics& g, const RenderContext& context)
{
    g.fillAll (juce::Colours::black);

    if (context.project.getNumClips() == 0)
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("No project loaded", context.bounds, juce::Justification::centred);
        return;
    }

    // Only render the current clip in karaoke mode
    if (context.currentClipIndex < 0 || context.currentClipIndex >= context.project.getNumClips())
        return;

    const auto& clip = context.project.getClip (context.currentClipIndex);

    auto area = context.bounds.reduced (20);
    area.removeFromBottom (60); // Space for stop button

    float baseFontSize = context.project.getDefaultFontSize() * 1.2f; // Slightly larger for karaoke
    float lineHeight = baseFontSize * lineSpacing;
    float maxWidth = static_cast<float>(area.getWidth()) - 40.0f;

    // Calculate line breaks for the entire clip
    auto lines = calculateLineBreaks (clip, baseFontSize, maxWidth);

    if (lines.empty())
        return;

    // Find which line contains the current word
    int currentLineIndex = -1;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (context.wordIndex >= lines[i].startWordIndex &&
            context.wordIndex <= lines[i].endWordIndex)
        {
            currentLineIndex = static_cast<int>(i);
            break;
        }
    }

    // If no word is active, show first line as current
    if (currentLineIndex < 0)
        currentLineIndex = 0;

    // Calculate vertical center
    float centerY = area.getY() + (area.getHeight() / 2.0f);

    // Render previous line (dimmed)
    if (showPreviousLine && currentLineIndex > 0)
    {
        float prevY = centerY - lineHeight * 1.5f;
        renderLine (g, context, clip, lines[static_cast<size_t>(currentLineIndex - 1)],
                    context.currentClipIndex, prevY, baseFontSize, lineHeight,
                    true, false);
    }

    // Render current line (highlighted)
    renderLine (g, context, clip, lines[static_cast<size_t>(currentLineIndex)],
                context.currentClipIndex, centerY, baseFontSize, lineHeight,
                false, false);

    // Render next line (preview)
    if (showNextLine && currentLineIndex < static_cast<int>(lines.size()) - 1)
    {
        float nextY = centerY + lineHeight * 1.5f;
        renderLine (g, context, clip, lines[static_cast<size_t>(currentLineIndex + 1)],
                    context.currentClipIndex, nextY, baseFontSize, lineHeight,
                    false, true);
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

std::vector<KaraokeRenderStrategy::LineInfo> KaraokeRenderStrategy::calculateLineBreaks (
    const Narrate::NarrateClip& clip, float baseFontSize, float maxWidth) const
{
    std::vector<LineInfo> lines;
    const auto& words = clip.getWords();

    if (words.isEmpty())
        return lines;

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

void KaraokeRenderStrategy::renderLine (juce::Graphics& g, const RenderContext& context,
                                         const Narrate::NarrateClip& clip, const LineInfo& line,
                                         int clipIndex, float y, float baseFontSize, float lineHeight,
                                         bool isDimmed, bool isPreview)
{
    juce::ignoreUnused(clipIndex);
    const auto& words = clip.getWords();
    float areaWidth = static_cast<float>(context.bounds.getWidth());
    float x = calculateLineStartX (areaWidth, line.totalWidth);

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
        bool isCurrentWord = (wordIndex == context.wordIndex) && !isDimmed && !isPreview;
        bool shouldHighlight = isCurrentWord && context.isRunning &&
                               context.currentTime < context.project.getTotalDuration();

        if (shouldHighlight)
        {
            // Draw highlight background with glow effect
            g.setColour (context.project.getHighlightColour().withAlpha (0.3f));
            g.fillRoundedRectangle (x - 8.0f, y - 8.0f, wordWidth + 16.0f, lineHeight, 4.0f);

            g.setColour (context.project.getHighlightColour());
            g.fillRoundedRectangle (x - 5.0f, y - 5.0f, wordWidth + 10.0f, lineHeight - 5.0f, 4.0f);
        }

        // Determine text color
        juce::Colour textColour;
        if (shouldHighlight)
        {
            textColour = juce::Colours::black;
        }
        else if (isDimmed)
        {
            // Previous line - dimmed
            textColour = formatting.colour.withAlpha (0.4f);
        }
        else if (isPreview)
        {
            // Next line - slightly dimmed
            textColour = formatting.colour.withAlpha (0.7f);
        }
        else
        {
            // Current line, non-highlighted words
            textColour = formatting.colour;
        }

        // Draw word text
        g.setColour (textColour);
        g.drawText (word.text,
                    juce::Rectangle<float>(x, y, wordWidth, lineHeight - 5.0f),
                    juce::Justification::left);

        x += wordWidth + wordSpacing;
    }
}

float KaraokeRenderStrategy::calculateLineStartX (float areaWidth, float lineWidth) const
{
    // Always center in karaoke mode
    return (areaWidth / 2.0f) - (lineWidth / 2.0f);
}

int KaraokeRenderStrategy::findCurrentWordIndex (const Narrate::NarrateClip& clip, double currentTime) const
{
    const auto& words = clip.getWords();
    double clipStartTime = clip.getStartTime();

    for (int i = 0; i < words.size(); ++i)
    {
        double wordAbsoluteTime = clipStartTime + words[i].relativeTime;

        // Find the next word's time to determine current word's duration
        double wordEndTime;
        if (i < words.size() - 1)
            wordEndTime = clipStartTime + words[i + 1].relativeTime;
        else
            wordEndTime = clip.getEndTime();

        // Check if current time falls within this word's duration
        if (currentTime >= wordAbsoluteTime && currentTime < wordEndTime)
            return i;
    }

    return -1; // No word found
}

juce::String KaraokeRenderStrategy::getName() const
{
    return "Karaoke";
}
