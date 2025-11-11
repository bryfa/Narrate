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

void RunningView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (words.isEmpty())
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("No text to display", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto area = getLocalBounds().reduced (20);
    area.removeFromBottom (60); // Space for stop button

    // Draw all words
    float x = 20.0f;
    float y = 20.0f;
    float lineHeight = 40.0f;
    float wordSpacing = 10.0f;

    g.setFont (24.0f);

    for (int i = 0; i < words.size(); ++i)
    {
        auto word = words[i];
        auto wordWidth = g.getCurrentFont().getStringWidthFloat (word);

        // Wrap to next line if needed
        if (x + wordWidth > static_cast<float>(getWidth()) - 40.0f)
        {
            x = 20.0f;
            y += lineHeight;
        }

        // Highlight current word
        if (i == currentWordIndex && isRunning)
        {
            g.setColour (juce::Colours::yellow);
            g.fillRect (x - 5.0f, y - 5.0f, wordWidth + 10.0f, 35.0f);
            g.setColour (juce::Colours::black);
        }
        else
        {
            g.setColour (juce::Colours::white);
        }

        g.drawText (word, juce::Rectangle<float>(x, y, wordWidth, 30.0f),
                    juce::Justification::left);

        x += wordWidth + wordSpacing;
    }
}

void RunningView::resized()
{
    auto area = getLocalBounds().reduced (10);
    stopButton.setBounds (area.removeFromBottom (40));
}

void RunningView::start (const juce::String& text)
{
    parseWords (text);
    currentWordIndex = 0;
    isRunning = true;
    startTimer (1000); // 1 second per word
    repaint();
}

void RunningView::stop()
{
    isRunning = false;
    stopTimer();
    currentWordIndex = 0;
    repaint();
}

void RunningView::timerCallback()
{
    if (!isRunning)
        return;

    currentWordIndex++;

    if (currentWordIndex >= words.size())
    {
        // Finished all words, stop
        stop();
        if (onStopClicked)
            onStopClicked();
        return;
    }

    repaint();
}

void RunningView::parseWords (const juce::String& text)
{
    words.clear();

    // Split by whitespace
    auto tokens = juce::StringArray::fromTokens (text, " \n\t\r", "");

    for (auto& token : tokens)
    {
        if (token.isNotEmpty())
            words.add (token);
    }
}
