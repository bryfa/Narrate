#pragma once

#include <juce_core/juce_core.h>
#include "../NarrateDataModel.h"

/**
 * ExportFeature
 *
 * Interface for export functionality supporting multiple formats:
 * - Subtitle formats: SRT, WebVTT
 * - Text formats: Plain Text, JSON, CSV
 */
class ExportFeature
{
public:
    virtual ~ExportFeature() = default;

    // Feature availability
    virtual bool isAvailable() const = 0;

    // Subtitle export operations
    virtual bool exportSRT(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;
    virtual bool exportWebVTT(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;

    // Text export operations
    virtual bool exportPlainText(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;
    virtual bool exportJSON(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;
    virtual bool exportCSV(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;

    // Format support queries
    virtual bool supportsSRT() const = 0;
    virtual bool supportsWebVTT() const = 0;
    virtual bool supportsPlainText() const = 0;
    virtual bool supportsJSON() const = 0;
    virtual bool supportsCSV() const = 0;
};
