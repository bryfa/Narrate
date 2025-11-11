#include "NarrateLookAndFeel.h"

NarrateLookAndFeel::NarrateLookAndFeel()
{
    setTheme(Theme::Dark);
}

void NarrateLookAndFeel::setTheme(Theme newTheme)
{
    currentTheme = newTheme;
    applyColorScheme(currentTheme == Theme::Dark ? darkScheme : lightScheme);
}

void NarrateLookAndFeel::toggleTheme()
{
    setTheme(currentTheme == Theme::Dark ? Theme::Light : Theme::Dark);
}

void NarrateLookAndFeel::applyColorScheme(const ColorScheme& scheme)
{
    // Window colours
    setColour(juce::ResizableWindow::backgroundColourId, scheme.background);
    setColour(juce::DocumentWindow::backgroundColourId, scheme.background);

    // General component colours
    setColour(juce::TextButton::buttonColourId, scheme.surface);
    setColour(juce::TextButton::buttonOnColourId, scheme.primary);
    setColour(juce::TextButton::textColourOffId, scheme.text);
    setColour(juce::TextButton::textColourOnId, scheme.text);

    setColour(juce::ComboBox::backgroundColourId, scheme.surface);
    setColour(juce::ComboBox::textColourId, scheme.text);
    setColour(juce::ComboBox::outlineColourId, scheme.border);
    setColour(juce::ComboBox::arrowColourId, scheme.textSecondary);

    setColour(juce::Label::textColourId, scheme.text);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::Slider::backgroundColourId, scheme.surface);
    setColour(juce::Slider::thumbColourId, scheme.primary);
    setColour(juce::Slider::trackColourId, scheme.secondary);
    setColour(juce::Slider::rotarySliderFillColourId, scheme.primary);
    setColour(juce::Slider::rotarySliderOutlineColourId, scheme.border);
    setColour(juce::Slider::textBoxTextColourId, scheme.text);
    setColour(juce::Slider::textBoxBackgroundColourId, scheme.surface);
    setColour(juce::Slider::textBoxOutlineColourId, scheme.border);

    setColour(juce::TextEditor::backgroundColourId, scheme.surface);
    setColour(juce::TextEditor::textColourId, scheme.text);
    setColour(juce::TextEditor::outlineColourId, scheme.border);
    setColour(juce::TextEditor::focusedOutlineColourId, scheme.primary);

    setColour(juce::ToggleButton::textColourId, scheme.text);
    setColour(juce::ToggleButton::tickColourId, scheme.primary);
    setColour(juce::ToggleButton::tickDisabledColourId, scheme.textSecondary);

    setColour(juce::ListBox::backgroundColourId, scheme.surface);
    setColour(juce::ListBox::outlineColourId, scheme.border);
    setColour(juce::ListBox::textColourId, scheme.text);

    setColour(juce::ScrollBar::backgroundColourId, scheme.background);
    setColour(juce::ScrollBar::thumbColourId, scheme.textSecondary);
    setColour(juce::ScrollBar::trackColourId, scheme.surface);

    setColour(juce::PopupMenu::backgroundColourId, scheme.surface);
    setColour(juce::PopupMenu::textColourId, scheme.text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, scheme.highlight);
    setColour(juce::PopupMenu::highlightedTextColourId, scheme.text);
}

void NarrateLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(backgroundColour);

    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 6.0f;

    auto& scheme = currentTheme == Theme::Dark ? darkScheme : lightScheme;

    // Background
    auto baseColour = button.getToggleState() ? scheme.primary : scheme.surface;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Border
    g.setColour(scheme.border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
}

void NarrateLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    auto& scheme = currentTheme == Theme::Dark ? darkScheme : lightScheme;

    auto bounds = button.getLocalBounds().toFloat();
    auto tickBounds = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);

    // Draw checkbox/toggle box
    g.setColour(button.getToggleState() ? scheme.primary : scheme.surface);
    g.fillRoundedRectangle(tickBounds, 3.0f);

    g.setColour(scheme.border);
    g.drawRoundedRectangle(tickBounds, 3.0f, 1.0f);

    // Draw tick if toggled
    if (button.getToggleState())
    {
        g.setColour(scheme.text);
        auto tick = tickBounds.reduced(tickBounds.getWidth() * 0.25f);

        juce::Path tickPath;
        tickPath.startNewSubPath(tick.getX(), tick.getCentreY());
        tickPath.lineTo(tick.getCentreX(), tick.getBottom());
        tickPath.lineTo(tick.getRight(), tick.getY());

        g.strokePath(tickPath, juce::PathStrokeType(2.0f));
    }

    // Draw label text
    g.setColour(scheme.text);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft);
}
