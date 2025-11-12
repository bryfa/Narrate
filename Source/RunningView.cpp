#include "RunningView.h"

RunningView::RunningView()
{
    // Setup stop button
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this]
    {
        stop();
        if (onStopClicked)
            onStopClicked();
    };
    addAndMakeVisible (stopButton);
}

RunningView::~RunningView()
{
    stopTimer();
}

RunningView::DisplayState RunningView::getCurrentDisplayState() const
{
    DisplayState state;

    // Find the active clip at current time
    state.clipIndex = project.getClipIndexAtTime (currentTime);

    if (state.clipIndex < 0)
        return state;

    const auto& clip = project.getClip (state.clipIndex);

    // Find the active word within the clip
    double relativeTime = currentTime - clip.getStartTime();

    // Find the word that should be displayed at this time
    for (int i = clip.getNumWords() - 1; i >= 0; --i)
    {
        const auto& word = clip.getWords()[i];

        // If we're past this word's start time, this is the active word
        if (relativeTime >= word.relativeTime)
        {
            state.wordIndex = i;
            break;
        }
    }

    return state;
}

void RunningView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (project.getNumClips() == 0)
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("No project loaded", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto displayState = getCurrentDisplayState();

    if (displayState.clipIndex < 0)
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("Waiting for content...", getLocalBounds(), juce::Justification::centred);
        return;
    }

    const auto& clip = project.getClip (displayState.clipIndex);
    const auto& words = clip.getWords();

    auto area = getLocalBounds().reduced (20);
    area.removeFromBottom (60); // Space for stop button

    // Draw all words in current clip
    float x = static_cast<float>(area.getX());
    float y = static_cast<float>(area.getY());
    float lineHeight = project.getDefaultFontSize() * 1.5f;
    float wordSpacing = 10.0f;
    float baseFontSize = project.getDefaultFontSize();

    for (int i = 0; i < words.size(); ++i)
    {
        const auto& word = words[i];
        auto formatting = word.getEffectiveFormatting (clip.getDefaultFormatting());

        // Set font with formatting
        auto fontHeight = baseFontSize * formatting.fontSizeMultiplier;
        juce::Font font {juce::FontOptions {fontHeight}};
        if (formatting.bold) font.setBold (true);
        if (formatting.italic) font.setItalic (true);
        g.setFont (font);

        // Calculate word width using GlyphArrangement (not deprecated)
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (font, word.text, 0, 0);
        float wordWidth = glyphs.getBoundingBox (0, -1, false).getWidth();

        // Wrap to next line if needed
        if (x + wordWidth > static_cast<float>(area.getRight()) - 20.0f)
        {
            x = static_cast<float>(area.getX());
            y += lineHeight;
        }

        // Highlight current word
        bool isCurrentWord = (i == displayState.wordIndex);

        if (isCurrentWord && isRunning)
        {
            // Draw highlight background
            g.setColour (project.getHighlightColour());
            g.fillRect (x - 5.0f, y - 5.0f, wordWidth + 10.0f, lineHeight);
        }

        // Draw word text
        g.setColour (isCurrentWord ? juce::Colours::black : formatting.colour);
        g.drawText (word.text, juce::Rectangle<float>(x, y, wordWidth, lineHeight - 5.0f),
                    juce::Justification::left);

        x += wordWidth + wordSpacing;
    }

    // Draw timer at bottom
    g.setColour (juce::Colours::grey);
    g.setFont (14.0f);
    auto timerText = juce::String::formatted ("Time: %.2fs / %.2fs", currentTime, project.getTotalDuration());
    g.drawText (timerText, area.removeFromBottom (20), juce::Justification::centredLeft);
}

void RunningView::resized()
{
    auto area = getLocalBounds().reduced (10);
    stopButton.setBounds (area.removeFromBottom (40));
}

void RunningView::start (const Narrate::NarrateProject& newProject)
{
    project = newProject;
    currentTime = 0.0;
    isRunning = true;
    startTimer (timerIntervalMs);  // ~60fps
    repaint();
}

void RunningView::stop()
{
    isRunning = false;
    stopTimer();
    currentTime = 0.0;
    repaint();
}

void RunningView::timerCallback()
{
    if (!isRunning)
        return;

    // Advance time
    currentTime += timerIntervalMs / 1000.0;  // Convert ms to seconds

    // Check if we've finished
    if (currentTime >= project.getTotalDuration())
    {
        stop();
        if (onStopClicked)
            onStopClicked();
        return;
    }

    repaint();
}
