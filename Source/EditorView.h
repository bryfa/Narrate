#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class NarrateAudioProcessor;

class EditorView : public juce::Component,
                   private juce::TextEditor::Listener
{
public:
    EditorView(NarrateAudioProcessor* processor);
    ~EditorView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Get the text entered by the user
    juce::String getText() const;

    // Set text (for restoring from settings)
    void setText(const juce::String& text);

    // Load and save text from/to settings
    void loadText();
    void saveText();

    // Set a callback for when the Run button is clicked
    std::function<void()> onRunClicked;

private:
    // TextEditor::Listener override
    void textEditorTextChanged(juce::TextEditor&) override;

private:
    NarrateAudioProcessor* audioProcessor;
    juce::TextEditor textEditor;
    juce::TextButton runButton;

    static constexpr const char* defaultText =
        "Enter your text here...\n\n"
        "Each word will be highlighted for one second when you click Run.";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorView)
};
