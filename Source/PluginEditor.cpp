#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ScrollingRenderStrategy.h"
#include "KaraokeRenderStrategy.h"
#include "TeleprompterRenderStrategy.h"

NarrateAudioProcessorEditor::NarrateAudioProcessorEditor (NarrateAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      editorView(&p)  // Pass processor to EditorView
{
    juce::ignoreUnused (processorRef);

    // Apply custom LookAndFeel
    setLookAndFeel (&narrateLookAndFeel);

    // Load saved theme preference
    loadTheme();

    // Detect if running as standalone app
    isStandalone = juce::JUCEApplicationBase::isStandaloneApp();

    // Setup theme toggle button
    themeToggleButton.setButtonText ("Toggle Theme");
    themeToggleButton.onClick = [this] { toggleTheme(); };
    addAndMakeVisible (themeToggleButton);

    // Only setup fullscreen button for standalone apps
    if (isStandalone)
    {
        fullscreenButton.setButtonText ("Enter Fullscreen");
        fullscreenButton.onClick = [this] { toggleFullScreen(); };
        addAndMakeVisible (fullscreenButton);
    }

    // Setup view switching callbacks
    editorView.onPreviewClicked = [this] { switchToRunningView(); };
    runningView.onStopClicked = [this] { switchToEditorView(); };

    // Start with editor view
    addAndMakeVisible (editorView);
    runningView.setVisible (false);
    addChildComponent (runningView);

    // Enable keyboard focus so we can receive keyboard events
    setWantsKeyboardFocus (true);

    // Set the initial size of the plugin window (larger for better editing experience)
    setSize (1000, 700);
}

NarrateAudioProcessorEditor::~NarrateAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void NarrateAudioProcessorEditor::paint (juce::Graphics&)
{
    // Views handle their own painting
}

void NarrateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Position buttons in the top bar
    auto topBar = area.removeFromTop (40);

    // Theme toggle button on the left
    themeToggleButton.setBounds (topBar.removeFromLeft (120).reduced (5));

    // Fullscreen button on the right (standalone only)
    if (isStandalone)
    {
        fullscreenButton.setBounds (topBar.removeFromRight (150).reduced (5));
    }

    // Layout the current view to fill remaining space
    editorView.setBounds (area);
    runningView.setBounds (area);
}

bool NarrateAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    // Handle Ctrl+Shift+F for fullscreen toggle (standalone only)
    if (isStandalone && key == juce::KeyPress ('f', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        toggleFullScreen();
        return true;
    }

    return false;
}

void NarrateAudioProcessorEditor::toggleFullScreen()
{
    isFullScreen = !isFullScreen;

    if (auto* peer = getPeer())
    {
        peer->setFullScreen (isFullScreen);
        fullscreenButton.setButtonText (isFullScreen ? "Exit Fullscreen" : "Enter Fullscreen");
    }
}

void NarrateAudioProcessorEditor::switchToRunningView()
{
    // Get the current project from the editor
    auto project = editorView.getProject();

    // Set the render strategy based on project settings
    switch (project.getRenderStrategy())
    {
        case Narrate::NarrateProject::RenderStrategy::Scrolling:
            runningView.setRenderStrategy (std::make_unique<ScrollingRenderStrategy>());
            break;
        case Narrate::NarrateProject::RenderStrategy::Karaoke:
            runningView.setRenderStrategy (std::make_unique<KaraokeRenderStrategy>());
            break;
        case Narrate::NarrateProject::RenderStrategy::Teleprompter:
            runningView.setRenderStrategy (std::make_unique<TeleprompterRenderStrategy>());
            break;
    }

    // Hide editor, show running view
    editorView.setVisible (false);
    runningView.setVisible (true);
    showingEditor = false;

    // Start playing the project
    runningView.start (project);
}

void NarrateAudioProcessorEditor::switchToEditorView()
{
    // Stop the running view
    runningView.stop();

    // Hide running view, show editor
    runningView.setVisible (false);
    editorView.setVisible (true);
    showingEditor = true;
}

void NarrateAudioProcessorEditor::toggleTheme()
{
    narrateLookAndFeel.toggleTheme();
    saveTheme();
    repaint();
}

void NarrateAudioProcessorEditor::loadTheme()
{
    auto* settings = processorRef.getSettings();
    if (settings != nullptr)
    {
        // Default to dark theme (0), load saved preference
        auto themeValue = settings->getIntValue("theme", 0);
        auto theme = (themeValue == 0) ? NarrateLookAndFeel::Theme::Dark : NarrateLookAndFeel::Theme::Light;
        narrateLookAndFeel.setTheme(theme);
    }
}

void NarrateAudioProcessorEditor::saveTheme()
{
    auto* settings = processorRef.getSettings();
    if (settings != nullptr)
    {
        int themeValue = (narrateLookAndFeel.getTheme() == NarrateLookAndFeel::Theme::Dark) ? 0 : 1;
        settings->setValue("theme", themeValue);
        settings->saveIfNeeded();
    }
}
