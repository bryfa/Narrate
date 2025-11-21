#include "SummaryDialog.h"

SummaryDialog::SummaryDialog()
{
    // Title label
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Status label
    statusLabel.setFont(juce::Font(16.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // Stats label
    statsLabel.setFont(juce::Font(14.0f));
    statsLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statsLabel);

    // Warnings button
    warningsButton.setButtonText("▸ Warnings");
    warningsButton.onClick = [this]()
    {
        warningsExpanded = !warningsExpanded;
        warningsButton.setButtonText(warningsExpanded ? "▾ Warnings" : "▸ Warnings");
        warningsEditor.setVisible(warningsExpanded);
        resized();
    };

    // Warnings editor
    warningsEditor.setMultiLine(true);
    warningsEditor.setReadOnly(true);
    warningsEditor.setScrollbarsShown(true);
    warningsEditor.setCaretVisible(false);
    warningsEditor.setPopupMenuEnabled(true);
    warningsEditor.setVisible(false);

    // Errors button
    errorsButton.setButtonText("▸ Errors");
    errorsButton.onClick = [this]()
    {
        errorsExpanded = !errorsExpanded;
        errorsButton.setButtonText(errorsExpanded ? "▾ Errors" : "▸ Errors");
        errorsEditor.setVisible(errorsExpanded);
        resized();
    };

    // Errors editor
    errorsEditor.setMultiLine(true);
    errorsEditor.setReadOnly(true);
    errorsEditor.setScrollbarsShown(true);
    errorsEditor.setCaretVisible(false);
    errorsEditor.setPopupMenuEnabled(true);
    errorsEditor.setVisible(false);

    // Copy button
    copyButton.setButtonText("Copy Details");
    copyButton.onClick = [this]()
    {
        juce::String details;
        details << "=== " << operationResult.operationName << " ===\n\n";
        details << "Status: " << (operationResult.success ? "SUCCESS" : "FAILED") << "\n";

        if (operationResult.itemsProcessed > 0)
        {
            details << "Items processed: " << operationResult.itemsProcessed << "\n";
            details << "Items successful: " << operationResult.itemsSuccessful << "\n";
            details << "Items skipped: " << operationResult.itemsSkipped << "\n";
        }

        if (operationResult.timeElapsedSeconds > 0.0)
        {
            details << "Time: " << formatDuration(operationResult.timeElapsedSeconds) << "\n";
        }

        int warningCount = operationResult.getWarningCount();
        int errorCount = operationResult.getErrorCount();

        if (warningCount > 0)
        {
            details << "\n=== Warnings (" << warningCount << ") ===\n";
            for (const auto& msg : operationResult.getWarnings())
            {
                if (msg.context.isNotEmpty())
                    details << "[" << msg.context << "] ";
                details << msg.message << "\n";
            }
        }

        if (errorCount > 0)
        {
            details << "\n=== Errors (" << errorCount << ") ===\n";
            for (const auto& msg : operationResult.getErrors())
            {
                if (msg.context.isNotEmpty())
                    details << "[" << msg.context << "] ";
                details << msg.message << "\n";
            }
        }

        juce::SystemClipboard::copyTextToClipboard(details);
    };
    addAndMakeVisible(copyButton);

    // Close button
    closeButton.setButtonText("Close");
    closeButton.onClick = [this]()
    {
        if (auto* window = findParentComponentOfClass<SummaryDialogWindow>())
            window->closeButtonPressed();
    };
    addAndMakeVisible(closeButton);
}

SummaryDialog::~SummaryDialog()
{
}

void SummaryDialog::show(const Narrate::OperationResult& result, juce::Component* parentComponent)
{
    auto* window = new SummaryDialogWindow(result);
    window->centreAroundComponent(parentComponent, window->getWidth(), window->getHeight());
    window->setVisible(true);
    window->enterModalState(true, juce::ModalCallbackFunction::create([window](int)
    {
        delete window;
    }));
}

void SummaryDialog::setResult(const Narrate::OperationResult& result)
{
    operationResult = result;
    buildContent();
}

void SummaryDialog::paint(juce::Graphics& g)
{
    g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));
}

void SummaryDialog::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Title
    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);

    // Status
    statusLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(15);

    // Stats
    if (statsLabel.getText().isNotEmpty())
    {
        int statsHeight = 60;
        statsLabel.setBounds(area.removeFromTop(statsHeight));
        area.removeFromTop(15);
    }

    // Warnings section
    int warningCount = operationResult.getWarningCount();
    if (warningCount > 0)
    {
        warningsButton.setBounds(area.removeFromTop(30));
        area.removeFromTop(5);

        if (warningsExpanded)
        {
            int editorHeight = juce::jmin(150, area.getHeight() / 2);
            warningsEditor.setBounds(area.removeFromTop(editorHeight));
            area.removeFromTop(10);
        }
    }

    // Errors section
    int errorCount = operationResult.getErrorCount();
    if (errorCount > 0)
    {
        errorsButton.setBounds(area.removeFromTop(30));
        area.removeFromTop(5);

        if (errorsExpanded)
        {
            int editorHeight = juce::jmin(150, area.getHeight() / 2);
            errorsEditor.setBounds(area.removeFromTop(editorHeight));
            area.removeFromTop(10);
        }
    }

    // Bottom buttons
    area.removeFromTop(10);
    auto buttonArea = area.removeFromBottom(40);

    int buttonWidth = 120;
    closeButton.setBounds(buttonArea.removeFromRight(buttonWidth));
    buttonArea.removeFromRight(10);

    if (warningCount > 0 || errorCount > 0)
    {
        copyButton.setBounds(buttonArea.removeFromRight(buttonWidth));
    }
}

void SummaryDialog::buildContent()
{
    // Set title
    titleLabel.setText(operationResult.operationName, juce::dontSendNotification);

    // Set status with color
    juce::String statusText = operationResult.success ? "✓ Success" : "✗ Failed";
    statusLabel.setText(statusText, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, getStatusColour());

    // Build stats text
    juce::String stats;
    if (operationResult.itemsProcessed > 0)
    {
        stats << "Processed: " << operationResult.itemsProcessed;

        if (operationResult.itemsSuccessful < operationResult.itemsProcessed)
        {
            stats << " (successful: " << operationResult.itemsSuccessful;
            if (operationResult.itemsSkipped > 0)
                stats << ", skipped: " << operationResult.itemsSkipped;
            stats << ")";
        }
        stats << "\n";
    }

    if (operationResult.timeElapsedSeconds > 0.0)
    {
        stats << "Time: " << formatDuration(operationResult.timeElapsedSeconds) << "\n";
    }

    if (operationResult.operationDetail.isNotEmpty())
    {
        stats << "File: " << operationResult.operationDetail;
    }

    statsLabel.setText(stats, juce::dontSendNotification);

    // Setup warnings
    int warningCount = operationResult.getWarningCount();
    if (warningCount > 0)
    {
        warningsButton.setButtonText("▸ Warnings (" + juce::String(warningCount) + ")");
        addAndMakeVisible(warningsButton);

        juce::String warningText;
        for (const auto& msg : operationResult.getWarnings())
        {
            if (msg.context.isNotEmpty())
                warningText << "[" << msg.context << "] ";
            warningText << msg.message << "\n";
        }
        warningsEditor.setText(warningText, false);
        addAndMakeVisible(warningsEditor);
    }
    else
    {
        warningsButton.setVisible(false);
        warningsEditor.setVisible(false);
    }

    // Setup errors
    int errorCount = operationResult.getErrorCount();
    if (errorCount > 0)
    {
        errorsButton.setButtonText("▸ Errors (" + juce::String(errorCount) + ")");
        addAndMakeVisible(errorsButton);

        juce::String errorText;
        for (const auto& msg : operationResult.getErrors())
        {
            if (msg.context.isNotEmpty())
                errorText << "[" << msg.context << "] ";
            errorText << msg.message << "\n";
        }
        errorsEditor.setText(errorText, false);
        addAndMakeVisible(errorsEditor);
    }
    else
    {
        errorsButton.setVisible(false);
        errorsEditor.setVisible(false);
    }

    resized();
}

juce::String SummaryDialog::formatDuration(double seconds) const
{
    if (seconds < 1.0)
        return juce::String(static_cast<int>(seconds * 1000)) + " ms";
    else if (seconds < 60.0)
        return juce::String(seconds, 2) + " seconds";
    else
    {
        int mins = static_cast<int>(seconds / 60.0);
        int secs = static_cast<int>(seconds) % 60;
        return juce::String(mins) + "m " + juce::String(secs) + "s";
    }
}

juce::Colour SummaryDialog::getStatusColour() const
{
    if (!operationResult.success)
        return juce::Colour(0xfff44336);  // Red
    else if (operationResult.getWarningCount() > 0)
        return juce::Colour(0xffff9800);  // Orange
    else
        return juce::Colour(0xff4caf50);  // Green
}

//==============================================================================
// SummaryDialogWindow

SummaryDialogWindow::SummaryDialogWindow(const Narrate::OperationResult& result)
    : DocumentWindow("Operation Summary",
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::closeButton)
{
    setUsingNativeTitleBar(true);

    auto* dialog = new SummaryDialog();
    dialog->setResult(result);

    setContentOwned(dialog, true);

    // Size based on content
    int width = 500;
    int baseHeight = 300;
    int warningCount = result.getWarningCount();
    int errorCount = result.getErrorCount();

    // Add height for expandable sections
    if (warningCount > 0) baseHeight += 40;
    if (errorCount > 0) baseHeight += 40;

    setResizable(true, false);
    setResizeLimits(400, 250, 800, 800);
    centreWithSize(width, baseHeight);
}

SummaryDialogWindow::~SummaryDialogWindow()
{
}

void SummaryDialogWindow::closeButtonPressed()
{
    setVisible(false);
    exitModalState(0);
}
