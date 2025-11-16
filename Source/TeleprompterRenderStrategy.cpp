#include "TeleprompterRenderStrategy.h"

TeleprompterRenderStrategy::TeleprompterRenderStrategy()
{
}

TeleprompterRenderStrategy::~TeleprompterRenderStrategy()
{
}

void TeleprompterRenderStrategy::render (juce::Graphics& g, const RenderContext& context)
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

    float baseFontSize = context.project.getDefaultFontSize() * 1.3f; // Larger for teleprompter
    float lineHeight = baseFontSize * lineSpacing;
    float maxWidth = static_cast<float>(area.getWidth()) - 40.0f;

    // Build all lines from all clips
    auto allLines = buildAllLines (context, baseFontSize, maxWidth);

    if (allLines.empty())
        return;

    // Calculate read line position
    float readLineY = area.getY() + (area.getHeight() * readLinePosition);

    // Calculate scroll offset to keep current word at read line
    float scrollOffset = calculateScrollOffset (allLines, context.clipIndex, context.wordIndex,
                                                 lineHeight, static_cast<float>(area.getY()), readLineY);

    // Render all lines with scrolling
    float y = area.getY() - scrollOffset;
    for (const auto& line : allLines)
    {
        // Skip lines that are way off screen (optimization)
        if (y > area.getBottom() + 100.0f)
            break;

        if (y >= area.getY() - lineHeight - 10.0f)
        {
            renderLine (g, context, line, y, baseFontSize, lineHeight);
        }

        y += lineHeight;
    }

    // Draw read line guide
    if (showReadLine)
    {
        g.setColour (juce::Colours::white.withAlpha (0.2f));
        g.drawLine (static_cast<float>(area.getX()),
                    readLineY,
                    static_cast<float>(area.getRight()),
                    readLineY,
                    2.0f);
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

std::vector<TeleprompterRenderStrategy::LineInfo> TeleprompterRenderStrategy::buildAllLines (
    const RenderContext& context, float baseFontSize, float maxWidth) const
{
    std::vector<LineInfo> allLines;

    // Build lines for all clips in the project
    for (int clipIndex = 0; clipIndex < context.project.getNumClips(); ++clipIndex)
    {
        auto clipLines = calculateLineBreaksForClip (context.project.getClip (clipIndex),
                                                      clipIndex,
                                                      baseFontSize,
                                                      maxWidth);

        allLines.insert (allLines.end(), clipLines.begin(), clipLines.end());

        // Add spacing between clips
        if (clipIndex < context.project.getNumClips() - 1)
        {
            // Add empty line as separator
            allLines.push_back ({clipIndex, -1, -1, 0.0f});
        }
    }

    return allLines;
}

std::vector<TeleprompterRenderStrategy::LineInfo> TeleprompterRenderStrategy::calculateLineBreaksForClip (
    const Narrate::NarrateClip& clip, int clipIndex, float baseFontSize, float maxWidth) const
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
            lines.push_back ({clipIndex, lineStartIndex, wordIndex - 1, currentLineWidth});
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
        lines.push_back ({clipIndex, lineStartIndex, static_cast<int>(words.size()) - 1, currentLineWidth});

    return lines;
}

float TeleprompterRenderStrategy::calculateScrollOffset (const std::vector<LineInfo>& allLines,
                                                          int currentClipIndex,
                                                          int currentWordIndex,
                                                          float lineHeight,
                                                          float areaY,
                                                          float readLineY) const
{
    // Find the line containing the current word from the current clip
    for (int lineIndex = 0; lineIndex < allLines.size(); ++lineIndex)
    {
        const auto& line = allLines[lineIndex];

        // Skip empty separator lines
        if (line.startWordIndex == -1)
            continue;

        // Only check lines from the current clip
        if (line.clipIndex != currentClipIndex)
            continue;

        if (currentWordIndex >= line.startWordIndex &&
            currentWordIndex <= line.endWordIndex)
        {
            // Current word is on this line
            // Calculate offset to position this line at the read line
            // The line should appear at readLineY, so we need to scroll by:
            // (lineIndex * lineHeight) - (readLineY - areaY)
            float targetY = lineIndex * lineHeight;
            float readLineOffset = readLineY - areaY;
            return targetY - readLineOffset;
        }
    }

    // Default: no scroll offset
    return 0.0f;
}

void TeleprompterRenderStrategy::renderLine (juce::Graphics& g, const RenderContext& context,
                                              const LineInfo& line, float y,
                                              float baseFontSize, float lineHeight)
{
    // Skip empty separator lines
    if (line.startWordIndex == -1)
        return;

    const auto& clip = context.project.getClip (line.clipIndex);
    const auto& words = clip.getWords();

    // Center the line
    float areaWidth = static_cast<float>(context.bounds.getWidth());
    float x = (areaWidth / 2.0f) - (line.totalWidth / 2.0f);

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
        bool isCurrentClip = (line.clipIndex == context.clipIndex);
        bool isCurrentWord = isCurrentClip && (wordIndex == context.wordIndex);
        bool shouldHighlight = isCurrentWord && context.isRunning &&
                               context.currentTime < context.project.getTotalDuration();

        if (shouldHighlight)
        {
            // Draw subtle highlight background
            g.setColour (context.project.getHighlightColour().withAlpha (0.3f));
            g.fillRoundedRectangle (x - 5.0f, y - 3.0f, wordWidth + 10.0f, lineHeight - 5.0f, 3.0f);
        }

        // Draw word text
        g.setColour (shouldHighlight ? context.project.getHighlightColour().brighter (0.5f) : formatting.colour);
        g.drawText (word.text,
                    juce::Rectangle<float>(x, y, wordWidth, lineHeight - 5.0f),
                    juce::Justification::left);

        x += wordWidth + wordSpacing;
    }
}

juce::String TeleprompterRenderStrategy::getName() const
{
    return "Teleprompter";
}
