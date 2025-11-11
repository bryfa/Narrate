#include "EditorView.h"

EditorView::EditorView()
{
    // Setup text editor
    textEditor.setMultiLine (true);
    textEditor.setReturnKeyStartsNewLine (true);
    textEditor.setScrollbarsShown (true);
    textEditor.setCaretVisible (true);
    textEditor.setPopupMenuEnabled (true);
    textEditor.setText ("Enter your text here...\n\nEach word will be highlighted for one second when you click Run.");
    addAndMakeVisible (textEditor);

    // Setup run button
    runButton.setButtonText ("Run");
    runButton.onClick = [this]
    {
        if (onRunClicked)
            onRunClicked();
    };
    addAndMakeVisible (runButton);
}

EditorView::~EditorView()
{
}

void EditorView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void EditorView::resized()
{
    auto area = getLocalBounds().reduced (10);

    // Run button at the bottom
    runButton.setBounds (area.removeFromBottom (40));

    area.removeFromBottom (10); // spacing

    // Text editor takes the rest
    textEditor.setBounds (area);
}

juce::String EditorView::getText() const
{
    return textEditor.getText();
}
