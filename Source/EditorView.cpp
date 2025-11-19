#include "EditorView.h"
#include "PluginProcessor.h"

EditorView::EditorView(NarrateAudioProcessor* processor)
    : audioProcessor(processor),
      audioPlaybackPanel(processor),
      exportPanel(processor),
      dawSyncPanel(processor)
{
    // Initialize with test project
    project = createTestProject();

    // Setup clip list
    clipListBox.setModel(this);
    clipListBox.setRowHeight(50);
    clipListBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::darkgrey);
    addAndMakeVisible(clipListBox);

    // Setup clip list buttons
    addClipButton.onClick = [this] { addClipClicked(); };
    addAndMakeVisible(addClipButton);

    removeClipButton.onClick = [this] { removeClipClicked(); };
    addAndMakeVisible(removeClipButton);

    recalculateButton.onClick = [this] { recalculateTimelineClicked(); };
    addAndMakeVisible(recalculateButton);

    // Setup clip editor labels
    startTimeLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(startTimeLabel);

    endTimeLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(endTimeLabel);

    textLabel.setJustificationType(juce::Justification::topRight);
    addAndMakeVisible(textLabel);

    // Setup editors
    startTimeEditor.setInputRestrictions(10, "0123456789.");
    startTimeEditor.onTextChange = [this] { textChanged(); };
    addAndMakeVisible(startTimeEditor);

    endTimeEditor.setInputRestrictions(10, "0123456789.");
    endTimeEditor.onTextChange = [this] { textChanged(); };
    addAndMakeVisible(endTimeEditor);

    clipTextEditor.setMultiLine(true);
    clipTextEditor.setReturnKeyStartsNewLine(true);
    clipTextEditor.setScrollbarsShown(true);
    clipTextEditor.onTextChange = [this] { textChanged(); };
    addAndMakeVisible(clipTextEditor);

    autoSpaceButton.onClick = [this] { autoSpaceWords(); };
    addAndMakeVisible(autoSpaceButton);

    wordsInfoLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(wordsInfoLabel);

    // Setup toolbar buttons
    newProjectButton.onClick = [this] { newProjectClicked(); };
    addAndMakeVisible(newProjectButton);

    loadProjectButton.onClick = [this] { loadProjectClicked(); };
    addAndMakeVisible(loadProjectButton);

    saveProjectButton.onClick = [this] { saveProjectClicked(); };
    addAndMakeVisible(saveProjectButton);

    // Setup render strategy combo box
    renderStrategyLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(renderStrategyLabel);

    renderStrategyCombo.addItem("Scrolling", 1);
    renderStrategyCombo.addItem("Karaoke", 2);
    renderStrategyCombo.addItem("Teleprompter", 3);
    renderStrategyCombo.setSelectedId(static_cast<int>(project.getRenderStrategy()) + 1, juce::dontSendNotification);
    renderStrategyCombo.onChange = [this]
    {
        int selectedId = renderStrategyCombo.getSelectedId();
        if (selectedId > 0)
            project.setRenderStrategy(static_cast<Narrate::NarrateProject::RenderStrategy>(selectedId - 1));
    };
    addAndMakeVisible(renderStrategyCombo);

    previewButton.onClick = [this]
    {
        if (onPreviewClicked)
            onPreviewClicked();
    };
    addAndMakeVisible(previewButton);

    // Add feature panels (NO conditional compilation!)
    // Panels manage their own visibility and layout
    addChildComponent(audioPlaybackPanel);
    addChildComponent(exportPanel);
    addChildComponent(dawSyncPanel);

    // Show panels based on feature availability
    audioPlaybackPanel.setVisible(audioProcessor->getAudioPlayback().isAvailable());
    exportPanel.setVisible(audioProcessor->getExportFeature().isAvailable());
    dawSyncPanel.setVisible(audioProcessor->getDawSync().isAvailable());

    // Select first clip
    if (project.getNumClips() > 0)
        clipListBox.selectRow(0);
}

EditorView::~EditorView()
{
    clipListBox.setModel(nullptr);
}

void EditorView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    // Draw toolbar background
    if (!toolbarBounds.isEmpty())
    {
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(toolbarBounds);

        g.setColour(juce::Colours::black);
        g.drawRect(toolbarBounds, 1);
    }

    // Draw separator line (below the feature panels and toolbar)
    g.setColour(juce::Colours::black);
    auto bounds = getLocalBounds();
    int separatorX = bounds.getWidth() / 3;

    // Calculate where separator should start
    float separatorStartY = 0.0f;

    if (audioPlaybackPanel.isVisible())
        separatorStartY += audioPlaybackPanel.getHeight();

    separatorStartY += toolbarBounds.getHeight() + 5;

    g.drawLine(static_cast<float>(separatorX), separatorStartY,
                static_cast<float>(separatorX), static_cast<float>(bounds.getHeight()), 2.0f);
}

void EditorView::resized()
{
    auto area = getLocalBounds();

    // Feature panels at the top (self-managing, only visible if feature available)
    if (audioPlaybackPanel.isVisible())
    {
        audioPlaybackPanel.setBounds(area.removeFromTop(155)); // Audio panel height
    }

    // Add spacing after audio panel
    if (audioPlaybackPanel.isVisible())
        area.removeFromTop(5);

    // Top toolbar with buttons and render strategy selector
    auto toolbarArea = area.removeFromTop(50);
    toolbarBounds = toolbarArea;

    toolbarArea.reduce(10, 5);
    auto toolbar = toolbarArea;

    int buttonWidth = 70;
    newProjectButton.setBounds(toolbar.removeFromLeft(buttonWidth).reduced(2));
    toolbar.removeFromLeft(5);
    loadProjectButton.setBounds(toolbar.removeFromLeft(buttonWidth).reduced(2));
    toolbar.removeFromLeft(5);
    saveProjectButton.setBounds(toolbar.removeFromLeft(buttonWidth).reduced(2));
    toolbar.removeFromLeft(20);

    // Render strategy selector in middle
    renderStrategyLabel.setBounds(toolbar.removeFromLeft(60).reduced(2));
    toolbar.removeFromLeft(5);
    renderStrategyCombo.setBounds(toolbar.removeFromLeft(120).reduced(2));
    toolbar.removeFromLeft(10);

    // DAW sync panel (if visible)
    if (dawSyncPanel.isVisible())
    {
        toolbar.removeFromLeft(10);
        dawSyncPanel.setBounds(toolbar.removeFromLeft(250));
    }

    // Preview button on the right
    previewButton.setBounds(toolbar.removeFromRight(100).reduced(2));

    // Export panel (if visible, on the right side)
    if (exportPanel.isVisible())
    {
        toolbar.removeFromRight(5);
        exportPanel.setBounds(toolbar.removeFromRight(200));
    }

    // Add spacing after toolbar
    area.removeFromTop(5);
    area.reduce(10, 10);

    // Left panel - Clip list
    auto leftPanel = area.removeFromLeft(area.getWidth() / 3).reduced(5);

    // Button area - two rows
    auto buttonArea = leftPanel.removeFromBottom(85);

    // First row: Add and Remove buttons
    auto firstRow = buttonArea.removeFromTop(40);
    addClipButton.setBounds(firstRow.removeFromLeft(firstRow.getWidth() / 2).reduced(2));
    removeClipButton.setBounds(firstRow.reduced(2));

    buttonArea.removeFromTop(5);

    // Second row: Recalculate button (full width)
    auto secondRow = buttonArea.removeFromTop(40);
    recalculateButton.setBounds(secondRow.reduced(2));

    leftPanel.removeFromBottom(5);
    clipListBox.setBounds(leftPanel);

    // Right panel - Clip editor
    auto rightPanel = area.reduced(10);

    // Time inputs
    auto timeRow = rightPanel.removeFromTop(30);
    startTimeLabel.setBounds(timeRow.removeFromLeft(100));
    timeRow.removeFromLeft(5);
    startTimeEditor.setBounds(timeRow.removeFromLeft(80));
    timeRow.removeFromLeft(20);
    endTimeLabel.setBounds(timeRow.removeFromLeft(100));
    timeRow.removeFromLeft(5);
    endTimeEditor.setBounds(timeRow.removeFromLeft(80));

    rightPanel.removeFromTop(10);

    // Text label and editor
    auto textLabelRow = rightPanel.removeFromTop(25);
    textLabel.setBounds(textLabelRow);

    rightPanel.removeFromTop(5);

    // Auto-space button and info at bottom
    auto autoSpaceRow = rightPanel.removeFromBottom(30);
    autoSpaceButton.setBounds(autoSpaceRow.removeFromLeft(150).reduced(2));
    autoSpaceRow.removeFromLeft(10);
    wordsInfoLabel.setBounds(autoSpaceRow);

    rightPanel.removeFromBottom(10);

    // Text editor takes remaining space
    auto textArea = rightPanel;
    if (textArea.getHeight() < 100)
        textArea.setHeight(100);
    clipTextEditor.setBounds(textArea);
}

// ListBoxModel overrides
int EditorView::getNumRows()
{
    return project.getNumClips();
}

void EditorView::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);

    if (rowNumber < 0 || rowNumber >= project.getNumClips())
        return;

    const auto& clip = project.getClip(rowNumber);

    g.setColour(rowIsSelected ? juce::Colours::black : juce::Colours::white);
    g.setFont(14.0f);

    auto text = juce::String::formatted("Clip %d: %.1f - %.1fs",
                                         rowNumber + 1,
                                         clip.getStartTime(),
                                         clip.getEndTime());

    g.drawText(text, 5, 2, width - 10, height / 2 - 2, juce::Justification::centredLeft, true);

    // Draw preview of text
    g.setColour(juce::Colours::grey);
    g.setFont(11.0f);
    auto preview = clip.getFullText();
    if (preview.length() > 30)
        preview = preview.substring(0, 27) + "...";
    g.drawText(preview, 5, height / 2 + 2, width - 10, height / 2 - 4, juce::Justification::centredLeft, true);
}

void EditorView::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < project.getNumClips())
    {
        // Save previous clip before switching
        if (selectedClipIndex >= 0 && selectedClipIndex < project.getNumClips())
            updateClipFromUI();

        selectedClipIndex = lastRowSelected;
        updateUIFromClip();
    }
}

// UI event handlers
void EditorView::addClipClicked()
{
    // Create new clip after the last one
    double startTime = 0.0;
    if (project.getNumClips() > 0)
    {
        const auto& lastClip = project.getClip(project.getNumClips() - 1);
        startTime = lastClip.getEndTime() + 0.5;
    }

    Narrate::NarrateClip newClip(juce::Uuid().toString(), startTime, startTime + 3.0);
    newClip.setText("New clip text here");

    // Auto-space words
    double duration = newClip.getDuration();
    int numWords = newClip.getNumWords();
    if (numWords > 0)
    {
        double timePerWord = duration / numWords;
        for (int i = 0; i < numWords; ++i)
            newClip.getWord(i).relativeTime = i * timePerWord;
    }

    project.addClip(newClip);
    clipListBox.updateContent();
    clipListBox.selectRow(project.getNumClips() - 1);
}

void EditorView::removeClipClicked()
{
    if (selectedClipIndex >= 0 && selectedClipIndex < project.getNumClips())
    {
        project.removeClip(selectedClipIndex);
        clipListBox.updateContent();

        // Select previous clip or first clip
        if (project.getNumClips() > 0)
        {
            int newSelection = juce::jmin(selectedClipIndex, project.getNumClips() - 1);
            clipListBox.selectRow(newSelection);
        }
        else
        {
            selectedClipIndex = -1;
            startTimeEditor.clear();
            endTimeEditor.clear();
            clipTextEditor.clear();
        }
    }
}

void EditorView::recalculateTimelineClicked()
{
    if (project.getNumClips() == 0)
        return;

    // Save current clip before recalculating
    if (selectedClipIndex >= 0 && selectedClipIndex < project.getNumClips())
        updateClipFromUI();

    // Recalculate the timeline to remove gaps
    project.recalculateTimeline();

    // Refresh UI
    clipListBox.updateContent();
    if (selectedClipIndex >= 0 && selectedClipIndex < project.getNumClips())
        updateUIFromClip();
}

void EditorView::newProjectClicked()
{
    project = Narrate::NarrateProject();
    project.setProjectName("New Project");
    selectedClipIndex = -1;
    clipListBox.updateContent();
    startTimeEditor.clear();
    endTimeEditor.clear();
    clipTextEditor.clear();
    renderStrategyCombo.setSelectedId(static_cast<int>(project.getRenderStrategy()) + 1, juce::dontSendNotification);
}

void EditorView::loadProjectClicked()
{
    auto chooser = std::make_shared<juce::FileChooser>("Load Narrate Project", juce::File(), "*.narrate");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File())
            return;

        if (project.loadFromFile(file))
        {
            clipListBox.updateContent();
            if (project.getNumClips() > 0)
                clipListBox.selectRow(0);
            renderStrategyCombo.setSelectedId(static_cast<int>(project.getRenderStrategy()) + 1, juce::dontSendNotification);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                     "Load Failed",
                                                     "Could not load project file.");
        }
    });
}

void EditorView::saveProjectClicked()
{
    // Update current clip before saving
    if (selectedClipIndex >= 0 && selectedClipIndex < project.getNumClips())
        updateClipFromUI();

    auto chooser = std::make_shared<juce::FileChooser>("Save Narrate Project", juce::File(), "*.narrate");

    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File())
            return;

        file = file.withFileExtension(".narrate");
        if (!project.saveToFile(file))
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                     "Save Failed",
                                                     "Could not save project file.");
        }
    });
}

void EditorView::updateClipFromUI()
{
    if (selectedClipIndex < 0 || selectedClipIndex >= project.getNumClips())
        return;

    auto& clip = project.getClip(selectedClipIndex);

    // Update times
    double startTime = startTimeEditor.getText().getDoubleValue();
    double endTime = endTimeEditor.getText().getDoubleValue();

    if (endTime <= startTime)
        endTime = startTime + 1.0;

    clip.setStartTime(startTime);
    clip.setEndTime(endTime);

    // Update text
    clip.setText(clipTextEditor.getText());

    // Auto-space words after text update
    double duration = clip.getDuration();
    int numWords = clip.getNumWords();
    if (numWords > 0)
    {
        double timePerWord = duration / numWords;
        for (int i = 0; i < numWords; ++i)
            clip.getWord(i).relativeTime = i * timePerWord;
    }

    clipListBox.repaintRow(selectedClipIndex);
}

void EditorView::updateUIFromClip()
{
    if (selectedClipIndex < 0 || selectedClipIndex >= project.getNumClips())
        return;

    const auto& clip = project.getClip(selectedClipIndex);

    startTimeEditor.setText(juce::String(clip.getStartTime(), 2), false);
    endTimeEditor.setText(juce::String(clip.getEndTime(), 2), false);
    clipTextEditor.setText(clip.getFullText(), false);
}

void EditorView::textChanged()
{
    // Auto-update the clip as user types (could add a timer for debouncing)
}

void EditorView::autoSpaceWords()
{
    if (selectedClipIndex < 0 || selectedClipIndex >= project.getNumClips())
        return;

    // Update clip from UI first
    updateClipFromUI();

    auto& clip = project.getClip(selectedClipIndex);

    // Evenly space words across the clip duration
    double duration = clip.getDuration();
    int numWords = clip.getNumWords();

    if (numWords > 0)
    {
        double timePerWord = duration / numWords;
        for (int i = 0; i < numWords; ++i)
            clip.getWord(i).relativeTime = i * timePerWord;
    }

    clipListBox.repaintRow(selectedClipIndex);
}

Narrate::NarrateProject EditorView::createTestProject()
{
    using namespace Narrate;

    NarrateProject testProject;
    testProject.setProjectName("Test Lyrics - Amazing Grace");

    // Create first clip: "Amazing grace, how sweet the sound"
    NarrateClip clip1("clip1", 0.0, 4.0);

    NarrateWord word1("Amazing", 0.0);
    word1.formatting = TextFormatting();
    word1.formatting->bold = true;
    word1.formatting->colour = juce::Colours::cyan;
    clip1.addWord(word1);

    clip1.addWord(NarrateWord("grace,", 0.8));
    clip1.addWord(NarrateWord("how", 1.5));
    clip1.addWord(NarrateWord("sweet", 2.0));
    clip1.addWord(NarrateWord("the", 2.7));

    NarrateWord word6("sound", 3.2);
    word6.formatting = TextFormatting();
    word6.formatting->fontSizeMultiplier = 1.5f;
    word6.formatting->colour = juce::Colours::yellow;
    clip1.addWord(word6);

    testProject.addClip(clip1);

    // Create second clip: "That saved a wretch like me"
    NarrateClip clip2("clip2", 4.5, 8.0);

    clip2.addWord(NarrateWord("That", 0.0));
    clip2.addWord(NarrateWord("saved", 0.5));

    NarrateWord wretch("a", 1.0);
    clip2.addWord(wretch);

    NarrateWord wretch2("wretch", 1.2);
    wretch2.formatting = TextFormatting();
    wretch2.formatting->italic = true;
    wretch2.formatting->colour = juce::Colours::orange;
    clip2.addWord(wretch2);

    clip2.addWord(NarrateWord("like", 2.0));
    clip2.addWord(NarrateWord("me", 2.7));

    testProject.addClip(clip2);

    // Create third clip: "I once was lost, but now am found"
    NarrateClip clip3("clip3", 8.5, 12.5);

    clip3.addWord(NarrateWord("I", 0.0));
    clip3.addWord(NarrateWord("once", 0.5));
    clip3.addWord(NarrateWord("was", 1.0));

    NarrateWord lost("lost,", 1.5);
    lost.formatting = TextFormatting();
    lost.formatting->colour = juce::Colours::red;
    lost.formatting->bold = true;
    clip3.addWord(lost);

    clip3.addWord(NarrateWord("but", 2.2));
    clip3.addWord(NarrateWord("now", 2.7));
    clip3.addWord(NarrateWord("am", 3.2));

    NarrateWord found("found", 3.7);
    found.formatting = TextFormatting();
    found.formatting->colour = juce::Colours::green;
    found.formatting->bold = true;
    clip3.addWord(found);

    testProject.addClip(clip3);

    return testProject;
}
