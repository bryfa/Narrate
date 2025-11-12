#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"
#include <functional>

class NarrateAudioProcessor;

class EditorView : public juce::Component,
                   private juce::ListBoxModel
{
public:
    EditorView(NarrateAudioProcessor* processor);
    ~EditorView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Get the current project
    const Narrate::NarrateProject& getProject() const { return project; }

    // Create a test project with sample lyrics
    Narrate::NarrateProject createTestProject();

    // Set a callback for when the Preview button is clicked
    std::function<void()> onPreviewClicked;

private:
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged (int lastRowSelected) override;

    // UI event handlers
    void addClipClicked();
    void removeClipClicked();
    void newProjectClicked();
    void loadProjectClicked();
    void saveProjectClicked();
    void updateClipFromUI();
    void updateUIFromClip();
    void textChanged();
    void autoSpaceWords();

private:
    NarrateAudioProcessor* audioProcessor;
    Narrate::NarrateProject project;
    int selectedClipIndex = -1;

    // Left panel - Clip List
    juce::ListBox clipListBox;
    juce::TextButton addClipButton {"Add Clip"};
    juce::TextButton removeClipButton {"Remove Clip"};

    // Right panel - Clip Editor
    juce::Label startTimeLabel {"", "Start Time (s):"};
    juce::TextEditor startTimeEditor;
    juce::Label endTimeLabel {"", "End Time (s):"};
    juce::TextEditor endTimeEditor;
    juce::Label textLabel {"", "Text:"};
    juce::TextEditor clipTextEditor;
    juce::TextButton autoSpaceButton {"Auto-Space Words"};
    juce::Label wordsInfoLabel {"", "Words will be evenly spaced"};

    // Bottom toolbar
    juce::TextButton newProjectButton {"New Project"};
    juce::TextButton loadProjectButton {"Load..."};
    juce::TextButton saveProjectButton {"Save..."};
    juce::TextButton previewButton {"Preview"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorView)
};
