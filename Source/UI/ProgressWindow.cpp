#include "ProgressWindow.h"

// ============================================================================
// ContentComponent - Internal UI for the progress window
// ============================================================================

class ProgressWindow::ContentComponent : public juce::Component
{
public:
    ContentComponent(std::atomic<bool>& cancelledFlag)
        : cancelled(cancelledFlag)
    {
        addAndMakeVisible(progressBar);
        addAndMakeVisible(statusLabel);
        addAndMakeVisible(cancelButton);

        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setFont(juce::Font(16.0f));

        cancelButton.setButtonText("Cancel");
        cancelButton.onClick = [this]
        {
            cancelled.store(true);
        };

        setSize(400, 150);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);

        statusLabel.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);

        progressBar.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);

        cancelButton.setBounds(area.removeFromTop(30).withSizeKeepingCentre(100, 30));
    }

    void setProgress(double progress, const juce::String& message)
    {
        currentProgress = progress;
        statusLabel.setText(message, juce::dontSendNotification);
        repaint();
    }

    double getProgress() const { return currentProgress; }

private:
    class CustomProgressBar : public juce::Component
    {
    public:
        void setProgress(double newProgress)
        {
            progress = juce::jlimit(0.0, 1.0, newProgress);
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            // Background
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(bounds, 3.0f);

            // Progress bar
            if (progress > 0.0)
            {
                auto progressWidth = bounds.getWidth() * static_cast<float>(progress);
                auto progressBounds = bounds.withWidth(progressWidth);

                g.setColour(juce::Colours::lightblue);
                g.fillRoundedRectangle(progressBounds, 3.0f);
            }

            // Border
            g.setColour(juce::Colours::lightgrey);
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

            // Percentage text
            g.setColour(juce::Colours::white);
            g.setFont(14.0f);
            juce::String percentText = juce::String(static_cast<int>(progress * 100)) + "%";
            g.drawText(percentText, bounds.toNearestInt(), juce::Justification::centred);
        }

    private:
        double progress = 0.0;
    };

    CustomProgressBar progressBar;
    juce::Label statusLabel;
    juce::TextButton cancelButton;
    std::atomic<bool>& cancelled;
    double currentProgress = 0.0;

    friend class ProgressWindow;
};

// ============================================================================
// ProgressWindow Implementation
// ============================================================================

ProgressWindow::ProgressWindow(const juce::String& title)
    : juce::DocumentWindow(title,
                           juce::Colours::darkgrey,
                           0)  // No buttons - users must use Cancel button
{
    content = std::make_unique<ContentComponent>(cancelled);
    setContentOwned(content.get(), true);
    setUsingNativeTitleBar(true);
    setResizable(false, false);
    centreWithSize(getWidth(), getHeight());
}

ProgressWindow::~ProgressWindow()
{
    // Content is owned by DocumentWindow, will be deleted automatically
    content.release();
}

void ProgressWindow::setProgress(double progress, const juce::String& message)
{
    if (content)
    {
        juce::MessageManager::callAsync([this, progress, message]()
        {
            if (content)
            {
                content->setProgress(progress, message);
                content->progressBar.setProgress(progress);
            }
        });
    }
}

void ProgressWindow::showModal()
{
    setVisible(true);
    toFront(true);
}

void ProgressWindow::closeWindow()
{
    juce::MessageManager::callAsync([this]()
    {
        setVisible(false);
    });
}
