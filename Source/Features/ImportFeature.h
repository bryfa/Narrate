#pragma once

#include <juce_core/juce_core.h>
#include "../NarrateDataModel.h"
#include <functional>

/**
 * ImportFeature - Abstract interface for importing subtitle/transcript files
 *
 * Provides methods to parse various subtitle formats (SRT, WebVTT, JSON, etc.)
 * into the internal NarrateProject data model.
 *
 * Implementations:
 * - StandaloneImportFeature: Full implementation for standalone/console builds
 * - NoOpImportFeature: Stub implementation for plugin builds where import is not available
 */
class ImportFeature
{
public:
    /**
     * Progress callback function type
     * Parameters: (progress 0.0-1.0, status message)
     * Returns: true to continue, false to cancel
     */
    using ProgressCallback = std::function<bool(double, const juce::String&)>;

    virtual ~ImportFeature() = default;

    // Format detection
    /**
     * Attempts to detect the format of a subtitle file by examining its content.
     *
     * @param file The file to analyze
     * @param outFormat If detection succeeds, this will contain the detected format ("srt", "vtt", "json", etc.)
     * @return true if format was successfully detected, false otherwise
     */
    virtual bool detectFormat(const juce::File& file, juce::String& outFormat) const = 0;

    // Import operations - each returns true on success, false on failure
    /**
     * Import from SubRip (.srt) format
     *
     * SRT Format Example:
     * 1
     * 00:00:01,000 --> 00:00:03,500
     * This is the first subtitle
     *
     * @param file The SRT file to import
     * @param outProject The project to populate with imported data (will be cleared first)
     * @param progressCallback Optional callback for progress updates (progress, message) -> continue?
     * @return true if import succeeded, false otherwise (including cancellation)
     */
    virtual bool importSRT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Import from WebVTT (.vtt) format
     *
     * @param file The VTT file to import
     * @param outProject The project to populate with imported data (will be cleared first)
     * @param progressCallback Optional callback for progress updates (progress, message) -> continue?
     * @return true if import succeeded, false otherwise (including cancellation)
     */
    virtual bool importWebVTT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Import from plain text with basic timing
     * Attempts to guess reasonable timing based on text length
     *
     * @param file The text file to import
     * @param outProject The project to populate with imported data (will be cleared first)
     * @param progressCallback Optional callback for progress updates (progress, message) -> continue?
     * @return true if import succeeded, false otherwise (including cancellation)
     */
    virtual bool importPlainText(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Import from JSON format (matches the JSON export format)
     *
     * @param file The JSON file to import
     * @param outProject The project to populate with imported data (will be cleared first)
     * @param progressCallback Optional callback for progress updates (progress, message) -> continue?
     * @return true if import succeeded, false otherwise (including cancellation)
     */
    virtual bool importJSON(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback = nullptr) = 0;

    // Format support queries
    virtual bool supportsSRT() const = 0;
    virtual bool supportsWebVTT() const = 0;
    virtual bool supportsPlainText() const = 0;
    virtual bool supportsJSON() const = 0;

protected:
    /**
     * Helper to parse timecode in various formats
     * Supports: HH:MM:SS,mmm (SRT) and HH:MM:SS.mmm (WebVTT)
     *
     * @param timecodeStr The timecode string to parse
     * @return Time in seconds as double, or -1.0 if parsing failed
     */
    static double parseTimecode(const juce::String& timecodeStr);
};
