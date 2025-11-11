#include "PluginProcessor.h"
#include "PluginEditor.h"

NarrateAudioProcessorEditor::NarrateAudioProcessorEditor (NarrateAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    // Apply custom LookAndFeel
    setLookAndFeel (&narrateLookAndFeel);

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
    editorView.onRunClicked = [this] { switchToRunningView(); };
    runningView.onStopClicked = [this] { switchToEditorView(); };

    // Start with editor view
    addAndMakeVisible (editorView);
    runningView.setVisible (false);
    addChildComponent (runningView);

    // Enable keyboard focus so we can receive keyboard events
    setWantsKeyboardFocus (true);

    // Set the initial size of the plugin window
    setSize (800, 600);
}

NarrateAudioProcessorEditor::~NarrateAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void NarrateAudioProcessorEditor::paint (juce::Graphics& g)
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
    // Get the text from editor and start the running view
    auto text = editorView.getText();

    if (text.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                 "No Text",
                                                 "Please enter some text before running!");
        return;
    }

    // Hide editor, show running view
    editorView.setVisible (false);
    runningView.setVisible (true);
    showingEditor = false;

    // Start the word highlighting
    runningView.start (text);
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
    repaint();
}
