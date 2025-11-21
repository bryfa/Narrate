#pragma once

#include "ImportFeature.h"

/**
 * NoOpImportFeature - Stub implementation for plugin builds
 *
 * Import functionality is not available in plugin builds (VST3, AU, etc.)
 * All methods return false to indicate failure.
 */
class NoOpImportFeature : public ImportFeature
{
public:
    NoOpImportFeature() = default;
    ~NoOpImportFeature() override = default;

    // Format detection - always fails
    bool detectFormat(const juce::File&, juce::String&) const override { return false; }

    // Import operations - all return false (not supported)
    bool importSRT(const juce::File&, Narrate::NarrateProject&, ProgressCallback = nullptr) override { return false; }
    bool importWebVTT(const juce::File&, Narrate::NarrateProject&, ProgressCallback = nullptr) override { return false; }
    bool importPlainText(const juce::File&, Narrate::NarrateProject&, ProgressCallback = nullptr) override { return false; }
    bool importJSON(const juce::File&, Narrate::NarrateProject&, ProgressCallback = nullptr) override { return false; }

    // Format support queries - none supported
    bool supportsSRT() const override { return false; }
    bool supportsWebVTT() const override { return false; }
    bool supportsPlainText() const override { return false; }
    bool supportsJSON() const override { return false; }
};
