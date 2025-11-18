#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../NarrateConfig.h"

class NarrateAudioProcessor;

/**
 * ExportPanel
 *
 * UI panel for export functionality (SRT, WebVTT, etc.).
 * Full implementation in Standalone, no-op in Plugin.
 */
class ExportPanel : public juce::Component
{
public:
    ExportPanel(NarrateAudioProcessor* processor);
    ~ExportPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NarrateAudioProcessor* audioProcessor;

#if NARRATE_SHOW_EXPORT_MENU
    // Standalone-only members
    juce::TextButton exportSrtButton {"Export SRT"};
    juce::TextButton exportVttButton {"Export WebVTT"};

    void exportSrtClicked();
    void exportVttClicked();
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportPanel)
};
