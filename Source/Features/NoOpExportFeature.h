#pragma once

#include "ExportFeature.h"

/**
 * NoOpExportFeature
 *
 * No-op implementation for Plugin builds (DAW handles export).
 */
class NoOpExportFeature : public ExportFeature
{
public:
    NoOpExportFeature() = default;
    ~NoOpExportFeature() override = default;

    bool isAvailable() const override { return false; }

    bool exportSRT(const Narrate::NarrateProject&, const juce::File&) override { return false; }
    bool exportWebVTT(const Narrate::NarrateProject&, const juce::File&) override { return false; }
    bool exportPlainText(const Narrate::NarrateProject&, const juce::File&) override { return false; }
    bool exportJSON(const Narrate::NarrateProject&, const juce::File&) override { return false; }
    bool exportCSV(const Narrate::NarrateProject&, const juce::File&) override { return false; }

    bool supportsSRT() const override { return false; }
    bool supportsWebVTT() const override { return false; }
    bool supportsPlainText() const override { return false; }
    bool supportsJSON() const override { return false; }
    bool supportsCSV() const override { return false; }
};
