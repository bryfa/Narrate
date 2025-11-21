#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../OperationResult.h"

/**
 * ToastNotification
 *
 * Brief, auto-dismissing notification overlay for quick user feedback.
 * Appears at the bottom of the screen, shows a message, and fades out after a few seconds.
 *
 * Used for fast operations that don't need a full progress dialog or summary.
 */
class ToastNotification : public juce::Component,
                          private juce::Timer
{
public:
    /**
     * Toast appearance style
     */
    enum class Style
    {
        Success,    // Green background for successful operations
        Warning,    // Yellow background for operations with warnings
        Error,      // Red background for failed operations
        Info        // Blue background for informational messages
    };

    ToastNotification();
    ~ToastNotification() override;

    /**
     * Show a toast notification with custom text and style
     * @param message The message to display
     * @param style Visual style (Success/Warning/Error/Info)
     * @param durationMs How long to show the toast (default 3000ms)
     */
    void show(const juce::String& message, Style style = Style::Success, int durationMs = 3000);

    /**
     * Show a toast based on an OperationResult
     * Automatically chooses style based on result status and message count
     * @param result The operation result to display
     * @param durationMs How long to show the toast (default 3000ms)
     */
    void showResult(const Narrate::OperationResult& result, int durationMs = 3000);

    /**
     * Dismiss the toast immediately
     */
    void dismiss();

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;

private:
    void timerCallback() override;
    void updatePosition();
    juce::Colour getBackgroundColour() const;
    juce::Colour getTextColour() const;

    juce::String message;
    Style currentStyle = Style::Info;
    float alpha = 0.0f;

    // Animation states
    enum class AnimationState
    {
        FadingIn,
        Visible,
        FadingOut,
        Hidden
    };

    AnimationState animationState = AnimationState::Hidden;
    int frameCount = 0;
    int visibleDurationMs = 3000;
    static constexpr int fadeInFrames = 10;    // ~166ms at 60fps
    static constexpr int fadeOutFrames = 15;   // ~250ms at 60fps
    static constexpr int animationIntervalMs = 16;  // ~60fps

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToastNotification)
};
