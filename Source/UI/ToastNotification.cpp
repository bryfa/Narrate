#include "ToastNotification.h"

ToastNotification::ToastNotification()
{
    setAlwaysOnTop(true);
    setVisible(false);
}

ToastNotification::~ToastNotification()
{
    stopTimer();
}

void ToastNotification::show(const juce::String& msg, Style style, int durationMs)
{
    message = msg;
    currentStyle = style;
    visibleDurationMs = durationMs;

    // Reset animation
    animationState = AnimationState::FadingIn;
    frameCount = 0;
    alpha = 0.0f;

    setVisible(true);
    updatePosition();
    toFront(true);

    startTimer(animationIntervalMs);
    repaint();
}

void ToastNotification::showResult(const Narrate::OperationResult& result, int durationMs)
{
    // Determine style based on result
    Style style;
    if (!result.success)
    {
        style = Style::Error;
    }
    else if (result.getErrorCount() > 0)
    {
        style = Style::Error;
    }
    else if (result.getWarningCount() > 0)
    {
        style = Style::Warning;
    }
    else
    {
        style = Style::Success;
    }

    // Create message text
    juce::String msg = result.getSummary();

    show(msg, style, durationMs);
}

void ToastNotification::dismiss()
{
    if (animationState != AnimationState::Hidden)
    {
        animationState = AnimationState::FadingOut;
        frameCount = 0;
    }
}

void ToastNotification::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Draw rounded rectangle background with shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f * alpha));
    g.fillRoundedRectangle(bounds.toFloat().translated(2.0f, 2.0f), 8.0f);

    // Draw main background
    g.setColour(getBackgroundColour().withAlpha(alpha));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

    // Draw border
    g.setColour(getBackgroundColour().brighter(0.2f).withAlpha(alpha));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 2.0f);

    // Draw text
    g.setColour(getTextColour().withAlpha(alpha));
    g.setFont(16.0f);
    g.drawText(message, bounds.reduced(15, 10), juce::Justification::centredLeft, true);
}

void ToastNotification::resized()
{
    updatePosition();
}

void ToastNotification::parentSizeChanged()
{
    updatePosition();
}

void ToastNotification::timerCallback()
{
    frameCount++;

    switch (animationState)
    {
        case AnimationState::FadingIn:
        {
            alpha = frameCount / static_cast<float>(fadeInFrames);
            if (frameCount >= fadeInFrames)
            {
                alpha = 1.0f;
                animationState = AnimationState::Visible;
                frameCount = 0;
            }
            repaint();
            break;
        }

        case AnimationState::Visible:
        {
            // Wait for visible duration
            if (frameCount * animationIntervalMs >= visibleDurationMs)
            {
                animationState = AnimationState::FadingOut;
                frameCount = 0;
            }
            break;
        }

        case AnimationState::FadingOut:
        {
            alpha = 1.0f - (frameCount / static_cast<float>(fadeOutFrames));
            if (frameCount >= fadeOutFrames || alpha <= 0.0f)
            {
                alpha = 0.0f;
                animationState = AnimationState::Hidden;
                setVisible(false);
                stopTimer();
            }
            repaint();
            break;
        }

        case AnimationState::Hidden:
        {
            stopTimer();
            break;
        }
    }
}

void ToastNotification::updatePosition()
{
    auto* parent = getParentComponent();
    if (parent == nullptr)
        return;

    const int toastWidth = juce::jmin(500, parent->getWidth() - 40);
    const int toastHeight = 60;
    const int bottomMargin = 20;

    int x = (parent->getWidth() - toastWidth) / 2;
    int y = parent->getHeight() - toastHeight - bottomMargin;

    setBounds(x, y, toastWidth, toastHeight);
}

juce::Colour ToastNotification::getBackgroundColour() const
{
    switch (currentStyle)
    {
        case Style::Success:
            return juce::Colour(0xff4caf50);  // Material Green

        case Style::Warning:
            return juce::Colour(0xffff9800);  // Material Orange

        case Style::Error:
            return juce::Colour(0xfff44336);  // Material Red

        case Style::Info:
            return juce::Colour(0xff2196f3);  // Material Blue

        default:
            return juce::Colour(0xff2196f3);
    }
}

juce::Colour ToastNotification::getTextColour() const
{
    return juce::Colours::white;
}
