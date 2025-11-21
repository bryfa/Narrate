#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../OperationResult.h"

/**
 * SummaryDialog
 *
 * Detailed modal dialog showing operation results with expandable sections
 * for warnings and errors. Provides comprehensive feedback after long-running
 * operations complete.
 *
 * Shows:
 * - Operation name and status (success/failure)
 * - Statistics (items processed, time taken, etc.)
 * - Expandable list of warnings (if any)
 * - Expandable list of errors (if any)
 * - Copy to clipboard button for diagnostic info
 */
class SummaryDialog : public juce::Component
{
public:
    SummaryDialog();
    ~SummaryDialog() override;

    /**
     * Show the summary dialog with operation result
     * @param result The operation result to display
     * @param parentComponent Component to center dialog over (can be nullptr)
     */
    static void show(const Narrate::OperationResult& result, juce::Component* parentComponent = nullptr);

    /**
     * Set the result to display
     */
    void setResult(const Narrate::OperationResult& result);

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void buildContent();
    juce::String formatDuration(double seconds) const;
    juce::Colour getStatusColour() const;

    Narrate::OperationResult operationResult;

    // UI Components
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label statsLabel;

    juce::TextButton warningsButton;
    juce::TextEditor warningsEditor;
    bool warningsExpanded = false;

    juce::TextButton errorsButton;
    juce::TextEditor errorsEditor;
    bool errorsExpanded = false;

    juce::TextButton copyButton;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SummaryDialog)
};


/**
 * SummaryDialogWindow
 *
 * Modal window wrapper for SummaryDialog
 */
class SummaryDialogWindow : public juce::DocumentWindow
{
public:
    SummaryDialogWindow(const Narrate::OperationResult& result);
    ~SummaryDialogWindow() override;

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SummaryDialogWindow)
};
