#pragma once

#include "ExportFeature.h"
#include "../NarrateConfig.h"

#if NARRATE_ENABLE_SUBTITLE_EXPORT

/**
 * StandaloneExportFeature
 *
 * Full implementation of export functionality for Standalone builds.
 * Supports multiple formats:
 * - Subtitle formats: SRT, WebVTT
 * - Text formats: Plain Text, JSON, CSV
 */
class StandaloneExportFeature : public ExportFeature
{
public:
    StandaloneExportFeature() = default;
    ~StandaloneExportFeature() override = default;

    // Feature availability
    bool isAvailable() const override { return true; }

    // Subtitle export operations
    bool exportSRT(const Narrate::NarrateProject& project, const juce::File& outputFile) override;
    bool exportWebVTT(const Narrate::NarrateProject& project, const juce::File& outputFile) override;

    // Text export operations
    bool exportPlainText(const Narrate::NarrateProject& project, const juce::File& outputFile) override;
    bool exportJSON(const Narrate::NarrateProject& project, const juce::File& outputFile) override;
    bool exportCSV(const Narrate::NarrateProject& project, const juce::File& outputFile) override;

    // Format support queries
    bool supportsSRT() const override { return true; }
    bool supportsWebVTT() const override { return true; }
    bool supportsPlainText() const override { return true; }
    bool supportsJSON() const override { return true; }
    bool supportsCSV() const override { return true; }

private:
    // Helper functions for subtitle formats
    juce::String formatSRTTimestamp(double seconds) const;
    juce::String formatWebVTTTimestamp(double seconds) const;
    juce::String escapeHTMLEntities(const juce::String& text) const;

    // Helper functions for text formats
    juce::String escapeCSVField(const juce::String& text) const;
    juce::String escapeJSONString(const juce::String& text) const;
    juce::String colourToHex(const juce::Colour& colour) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneExportFeature)
};

#endif // NARRATE_ENABLE_SUBTITLE_EXPORT
