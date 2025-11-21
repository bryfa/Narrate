#include "ExportPanel.h"
#include "../PluginProcessor.h"

ExportPanel::ExportPanel(NarrateAudioProcessor* processor)
    : audioProcessor(processor)
{
#if NARRATE_SHOW_EXPORT_MENU
    exportSrtButton.onClick = [this] { exportSrtClicked(); };
    addAndMakeVisible(exportSrtButton);

    exportVttButton.onClick = [this] { exportVttClicked(); };
    addAndMakeVisible(exportVttButton);
#endif
}

ExportPanel::~ExportPanel()
{
}

void ExportPanel::paint(juce::Graphics& g)
{
#if !NARRATE_SHOW_EXPORT_MENU
    juce::ignoreUnused(g);
#endif
}

void ExportPanel::resized()
{
#if NARRATE_SHOW_EXPORT_MENU
    auto area = getLocalBounds();

    // Layout buttons horizontally
    exportSrtButton.setBounds(area.removeFromLeft(100));
    area.removeFromLeft(5);
    exportVttButton.setBounds(area.removeFromLeft(100));
#endif
}

#if NARRATE_SHOW_EXPORT_MENU

void ExportPanel::exportSrtClicked()
{
    // TODO: Get project from processor/editor
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Export SRT",
        "Export functionality is ready!\n\n"
        "This will be wired up when EditorView is refactored.\n"
        "Implementation: StandaloneExportFeature::exportSRT()",
        "OK");
}

void ExportPanel::exportVttClicked()
{
    // TODO: Get project from processor/editor
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Export WebVTT",
        "Export functionality is ready!\n\n"
        "This will be wired up when EditorView is refactored.\n"
        "Implementation: StandaloneExportFeature::exportWebVTT()",
        "OK");
}

#endif // NARRATE_SHOW_EXPORT_MENU
