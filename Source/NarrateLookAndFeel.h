#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

class NarrateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class Theme
    {
        Dark,
        Light
    };

    NarrateLookAndFeel();
    ~NarrateLookAndFeel() override = default;

    void setTheme(Theme newTheme);
    Theme getTheme() const { return currentTheme; }
    void toggleTheme();

    // Override drawing methods for custom appearance
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    Theme currentTheme = Theme::Dark;

    struct ColorScheme
    {
        juce::Colour background;
        juce::Colour surface;
        juce::Colour primary;
        juce::Colour secondary;
        juce::Colour text;
        juce::Colour textSecondary;
        juce::Colour border;
        juce::Colour highlight;
    };

    ColorScheme darkScheme{
        juce::Colour(0xff1a1a1a),  // background
        juce::Colour(0xff2d2d2d),  // surface
        juce::Colour(0xff4a90e2),  // primary (blue)
        juce::Colour(0xff7b68ee),  // secondary (purple)
        juce::Colour(0xffe0e0e0),  // text
        juce::Colour(0xffa0a0a0),  // textSecondary
        juce::Colour(0xff404040),  // border
        juce::Colour(0xff5aa5f5)   // highlight
    };

    ColorScheme lightScheme{
        juce::Colour(0xfff5f5f5),  // background
        juce::Colour(0xffffffff),  // surface
        juce::Colour(0xff2e7bd4),  // primary (blue)
        juce::Colour(0xff6b5ce7),  // secondary (purple)
        juce::Colour(0xff1a1a1a),  // text
        juce::Colour(0xff666666),  // textSecondary
        juce::Colour(0xffd0d0d0),  // border
        juce::Colour(0xff4a90e2)   // highlight
    };

    void applyColorScheme(const ColorScheme& scheme);
};
