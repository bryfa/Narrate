#pragma once

#include <juce_core/juce_core.h>
#include "../NarrateDataModel.h"

/**
 * ExportFeature
 *
 * Interface for subtitle export functionality (SRT, WebVTT, etc.).
 */
class ExportFeature
{
public:
    virtual ~ExportFeature() = default;

    // Feature availability
    virtual bool isAvailable() const = 0;

    // Export operations
    virtual bool exportSRT(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;
    virtual bool exportWebVTT(const Narrate::NarrateProject& project, const juce::File& outputFile) = 0;

    // Format support queries
    virtual bool supportsSRT() const = 0;
    virtual bool supportsWebVTT() const = 0;
};
