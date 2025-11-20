#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

/**
 * ProgressWindow
 *
 * A modal window that displays import/export progress with a progress bar,
 * status message, and optional cancel button.
 */
class ProgressWindow : public juce::DocumentWindow
{
public:
    ProgressWindow(const juce::String& title);
    ~ProgressWindow() override;

    // Update progress (0.0 to 1.0) and status message
    void setProgress(double progress, const juce::String& message);

    // Check if user clicked cancel
    bool wasCancelled() const { return cancelled.load(); }

    // Show the window modally (non-blocking - updates via message thread)
    void showModal();

    // Close the window
    void closeWindow();

private:
    class ContentComponent;
    std::unique_ptr<ContentComponent> content;
    std::atomic<bool> cancelled{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressWindow)
};
