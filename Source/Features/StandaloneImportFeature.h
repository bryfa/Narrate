#pragma once

#include "ImportFeature.h"

/**
 * StandaloneImportFeature - Full implementation of ImportFeature
 *
 * Provides complete import functionality for standalone and console builds.
 * Supports SRT, WebVTT, JSON, and plain text formats.
 */
class StandaloneImportFeature : public ImportFeature
{
public:
    StandaloneImportFeature() = default;
    ~StandaloneImportFeature() override = default;

    // Format detection
    bool detectFormat(const juce::File& file, juce::String& outFormat) const override;

    // Import operations
    bool importSRT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) override;
    bool importWebVTT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) override;
    bool importPlainText(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) override;
    bool importJSON(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) override;

    // Format support queries
    bool supportsSRT() const override { return true; }
    bool supportsWebVTT() const override { return true; }
    bool supportsPlainText() const override { return true; }
    bool supportsJSON() const override { return true; }

private:
    // Helper methods for parsing
    struct SubtitleEntry
    {
        int index;
        double startTime;
        double endTime;
        juce::String text;
    };

    /**
     * Parse a single SRT subtitle entry
     * Returns true if successfully parsed
     */
    bool parseSRTEntry(juce::StringArray& lines, int& lineIndex, SubtitleEntry& outEntry) const;

    /**
     * Parse a single WebVTT cue
     * Returns true if successfully parsed
     */
    bool parseWebVTTCue(juce::StringArray& lines, int& lineIndex, SubtitleEntry& outEntry) const;

    /**
     * Parse JSON value for color
     */
    juce::Colour parseColourFromHex(const juce::String& hexStr) const;

    /**
     * Estimate duration for plain text based on word count
     * Uses typical reading speed of ~2.5 words per second
     */
    double estimateDuration(const juce::String& text) const;
};
