#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NarrateDataModel.h"
#include "UI/AudioPlaybackPanel.h"
#include "UI/ExportPanel.h"
#include "UI/DawSyncPanel.h"
#include <functional>

class NarrateAudioProcessor;

/**
 * EditorView
 *
 * Refactored to use UI panels instead of conditional compilation.
 * Feature-specific UI is now handled by dedicated panel components.
 */
class EditorView : public juce::Component,
                   private juce::ListBoxModel
{
public:
    EditorView(NarrateAudioProcessor* processor);
    ~EditorView() override;

    void paint(juce::Graphics&) override;
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
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

    // UI event handlers
    void addClipClicked();
    void removeClipClicked();
    void selectAllClicked();
    void recalculateTimelineClicked();
    void newProjectClicked();
    void loadProjectClicked();
    void importProjectClicked();
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
    juce::TextButton selectAllButton {"Select All"};
    juce::TextButton recalculateButton {"Recalculate Timeline"};

    // Right panel - Clip Editor
    juce::Label startTimeLabel {"", "Start Time (s):"};
    juce::TextEditor startTimeEditor;
    juce::Label endTimeLabel {"", "End Time (s):"};
    juce::TextEditor endTimeEditor;
    juce::Label textLabel {"", "Text:"};
    juce::TextEditor clipTextEditor;
    juce::TextButton autoSpaceButton {"Auto-Space Words"};
    juce::Label wordsInfoLabel {"", "Words will be evenly spaced"};

    // Top toolbar
    juce::TextButton newProjectButton {"New"};
    juce::TextButton loadProjectButton {"Load"};
    juce::TextButton importProjectButton {"Import"};
    juce::TextButton saveProjectButton {"Save"};
    juce::Label renderStrategyLabel {"", "Render:"};
    juce::ComboBox renderStrategyCombo;
    juce::TextButton previewButton {"Preview"};

    // Store toolbar bounds for painting background
    juce::Rectangle<int> toolbarBounds;

    // Feature panels (NO conditional compilation!)
    // These are always present; visibility managed by panels themselves
    AudioPlaybackPanel audioPlaybackPanel;
    ExportPanel exportPanel;
    DawSyncPanel dawSyncPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorView)
};
