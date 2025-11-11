#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class EditorView : public juce::Component
{
public:
    EditorView();
    ~EditorView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Get the text entered by the user
    juce::String getText() const;

    // Set a callback for when the Run button is clicked
    std::function<void()> onRunClicked;

private:
    juce::TextEditor textEditor;
    juce::TextButton runButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorView)
};
