#include "EditorView.h"
#include "PluginProcessor.h"

EditorView::EditorView(NarrateAudioProcessor* processor)
    : audioProcessor(processor)
{
    // Setup text editor
    textEditor.setMultiLine (true);
    textEditor.setReturnKeyStartsNewLine (true);
    textEditor.setScrollbarsShown (true);
    textEditor.setCaretVisible (true);
    textEditor.setPopupMenuEnabled (true);
    textEditor.addListener(this);  // Listen for text changes
    addAndMakeVisible (textEditor);

    // Setup run button
    runButton.setButtonText ("Run");
    runButton.onClick = [this]
    {
        if (onRunClicked)
            onRunClicked();
    };
    addAndMakeVisible (runButton);

    // Load saved text or use default
    loadText();
}

EditorView::~EditorView()
{
    textEditor.removeListener(this);
    saveText();  // Save when destroyed
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

void EditorView::setText(const juce::String& text)
{
    textEditor.setText(text, false);
}

void EditorView::loadText()
{
    if (audioProcessor == nullptr)
    {
        textEditor.setText(defaultText);
        return;
    }

    // Load text from processor state (per-instance, saved in DAW projects)
    auto savedText = audioProcessor->getEditorText();
    if (savedText.isEmpty())
        textEditor.setText(defaultText);
    else
        textEditor.setText(savedText);
}

void EditorView::saveText()
{
    if (audioProcessor == nullptr)
        return;

    // Save text to processor state (will be saved in DAW projects)
    audioProcessor->setEditorText(textEditor.getText());
}

void EditorView::textEditorTextChanged(juce::TextEditor&)
{
    // Auto-save text when it changes
    saveText();
}
