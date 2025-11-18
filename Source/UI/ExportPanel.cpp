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
    // TODO: Implement SRT export
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Export SRT",
        "SRT export functionality will be implemented in Phase 1.",
        "OK");
}

void ExportPanel::exportVttClicked()
{
    // TODO: Implement WebVTT export
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Export WebVTT",
        "WebVTT export functionality will be implemented in Phase 1.",
        "OK");
}

#endif // NARRATE_SHOW_EXPORT_MENU
