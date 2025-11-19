#pragma once

#include "ExportFeature.h"
#include "../NarrateConfig.h"

#if NARRATE_ENABLE_SUBTITLE_EXPORT

/**
 * StandaloneExportFeature
 *
 * Full implementation of subtitle export for Standalone builds.
 * Supports SRT and WebVTT formats.
 */
class StandaloneExportFeature : public ExportFeature
{
public:
    StandaloneExportFeature() = default;
    ~StandaloneExportFeature() override = default;

    // Feature availability
    bool isAvailable() const override { return true; }

    // Export operations
    bool exportSRT(const Narrate::NarrateProject& project, const juce::File& outputFile) override;
    bool exportWebVTT(const Narrate::NarrateProject& project, const juce::File& outputFile) override;

    // Format support queries
    bool supportsSRT() const override { return true; }
    bool supportsWebVTT() const override { return true; }

private:
    // Helper functions
    juce::String formatSRTTimestamp(double seconds) const;
    juce::String formatWebVTTTimestamp(double seconds) const;
    juce::String escapeHTMLEntities(const juce::String& text) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneExportFeature)
};

#endif // NARRATE_ENABLE_SUBTITLE_EXPORT
