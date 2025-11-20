#include "StandaloneImportFeature.h"
#include <juce_data_structures/juce_data_structures.h>

// ============================================================================
// Helper function for parsing timecode (static method in base class)
// ============================================================================

double ImportFeature::parseTimecode(const juce::String& timecodeStr)
{
    // Supports formats:
    // HH:MM:SS,mmm (SRT format)
    // HH:MM:SS.mmm (WebVTT format)

    juce::String normalized = timecodeStr.trim();

    // Replace comma with period for consistent parsing
    normalized = normalized.replace(",", ".");

    // Split by colons
    juce::StringArray parts = juce::StringArray::fromTokens(normalized, ":", "");

    if (parts.size() != 3)
        return -1.0;

    int hours = parts[0].getIntValue();
    int minutes = parts[1].getIntValue();

    // The seconds part may contain milliseconds (e.g., "45.500")
    double seconds = parts[2].getDoubleValue();

    return hours * 3600.0 + minutes * 60.0 + seconds;
}

// ============================================================================
// Format Detection
// ============================================================================

bool StandaloneImportFeature::detectFormat(const juce::File& file, juce::String& outFormat) const
{
    if (!file.existsAsFile())
        return false;

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
        return false;

    // Check for WebVTT signature
    if (content.startsWith("WEBVTT"))
    {
        outFormat = "vtt";
        return true;
    }

    // Check for JSON format
    if (content.trimStart().startsWith("{") && content.contains("\"projectName\""))
    {
        outFormat = "json";
        return true;
    }

    // Check for SRT format (numeric index followed by timecode)
    juce::StringArray lines = juce::StringArray::fromLines(content);
    if (lines.size() >= 3)
    {
        // SRT starts with a numeric index
        if (lines[0].trim().containsOnly("0123456789"))
        {
            // Second line should contain the arrow separator
            if (lines[1].contains("-->"))
            {
                outFormat = "srt";
                return true;
            }
        }
    }

    // Default to plain text
    outFormat = "txt";
    return true;
}

// ============================================================================
// SRT Import
// ============================================================================

bool StandaloneImportFeature::parseSRTEntry(juce::StringArray& lines, int& lineIndex, SubtitleEntry& outEntry) const
{
    // SRT Format:
    // 1
    // 00:00:01,000 --> 00:00:03,500
    // This is the subtitle text
    // It can span multiple lines
    //
    // (blank line separator)

    if (lineIndex >= lines.size())
        return false;

    // Skip empty lines
    while (lineIndex < lines.size() && lines[lineIndex].trim().isEmpty())
        ++lineIndex;

    if (lineIndex >= lines.size())
        return false;

    // Parse index (line 1)
    outEntry.index = lines[lineIndex].trim().getIntValue();
    ++lineIndex;

    if (lineIndex >= lines.size())
        return false;

    // Parse timecode (line 2): "00:00:01,000 --> 00:00:03,500"
    juce::String timecodeLine = lines[lineIndex].trim();
    std::cerr << "DEBUG parseSRTEntry: Timecode line: '" << timecodeLine.toStdString() << "'\n";
    ++lineIndex;

    // Split on " --> " separator
    int separatorPos = timecodeLine.indexOf("-->");
    if (separatorPos < 0)
    {
        std::cerr << "DEBUG parseSRTEntry: No --> separator found\n";
        return false;
    }

    juce::String startTimeStr = timecodeLine.substring(0, separatorPos).trim();
    juce::String endTimeStr = timecodeLine.substring(separatorPos + 3).trim();

    std::cerr << "DEBUG parseSRTEntry: Start: '" << startTimeStr.toStdString()
              << "' End: '" << endTimeStr.toStdString() << "'\n";

    outEntry.startTime = parseTimecode(startTimeStr);
    outEntry.endTime = parseTimecode(endTimeStr);
    std::cerr << "DEBUG parseSRTEntry: Parsed times: " << outEntry.startTime << " -> " << outEntry.endTime << "\n";

    if (outEntry.startTime < 0.0 || outEntry.endTime < 0.0)
    {
        std::cerr << "DEBUG parseSRTEntry: Invalid times (negative)\n";
        return false;
    }

    // Parse text (can be multiple lines until blank line)
    juce::String textContent;
    while (lineIndex < lines.size() && !lines[lineIndex].trim().isEmpty())
    {
        juce::String line = lines[lineIndex].trim();

        // Skip lines with invalid characters or encoding issues
        if (line.isEmpty())
        {
            ++lineIndex;
            continue;
        }

        // Remove any zero-width or control characters that might cause issues
        line = line.replace(juce::CharPointer_UTF8("\xE2\x80\x8B"), "");  // Zero-width space
        line = line.replace(juce::CharPointer_UTF8("\xEF\xBB\xBF"), "");  // UTF-8 BOM
        line = line.removeCharacters("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x0B\x0C\x0E\x0F"
                                    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");  // Control chars

        if (!textContent.isEmpty())
            textContent += " ";
        textContent += line;
        ++lineIndex;
    }

    outEntry.text = textContent.trim();

    return !textContent.isEmpty();
}

bool StandaloneImportFeature::importSRT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback)
{
    if (!file.existsAsFile())
    {
        std::cerr << "DEBUG: File does not exist\n";
        return false;
    }

    // Report start
    if (progressCallback && !progressCallback(0.0, "Loading SRT file..."))
        return false;

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
    {
        std::cerr << "DEBUG: File content is empty\n";
        return false;
    }

    std::cerr << "DEBUG: File loaded, " << content.length() << " characters\n";

    // Clear existing project
    outProject = Narrate::NarrateProject();
    outProject.setProjectName(file.getFileNameWithoutExtension());

    // Parse SRT content
    juce::StringArray lines = juce::StringArray::fromLines(content);
    int lineIndex = 0;

    std::cerr << "DEBUG: Split into " << lines.size() << " lines\n";

    if (progressCallback && !progressCallback(0.1, "Parsing SRT entries..."))
        return false;

    int clipCount = 0;
    int totalLines = lines.size();

    while (lineIndex < lines.size())
    {
        // Skip empty lines before trying to parse
        int startLineIndex = lineIndex;
        while (lineIndex < lines.size() && lines[lineIndex].trim().isEmpty())
            ++lineIndex;

        // Check if we've reached the end
        if (lineIndex >= lines.size())
            break;

        SubtitleEntry entry;
        if (parseSRTEntry(lines, lineIndex, entry))
        {
            std::cerr << "DEBUG: Parsed entry " << entry.index
                      << " [" << entry.startTime << " -> " << entry.endTime << "]: "
                      << entry.text.toStdString() << "\n";

            // Create a clip from this entry
            Narrate::NarrateClip clip;
            clip.setStartTime(entry.startTime);
            clip.setEndTime(entry.endTime);

            // Split text into words
            juce::StringArray words = juce::StringArray::fromTokens(entry.text, " ", "");

            // Distribute words evenly across the clip duration
            double clipDuration = clip.getDuration();
            int wordCount = words.size();

            if (wordCount > 0)
            {
                double timePerWord = clipDuration / wordCount;

                for (int i = 0; i < wordCount; ++i)
                {
                    Narrate::NarrateWord word(words[i], i * timePerWord);
                    clip.addWord(word);
                }
            }

            outProject.addClip(clip);
            clipCount++;

            // Report progress periodically (every 10 clips)
            if (progressCallback && clipCount % 10 == 0)
            {
                double progress = 0.1 + (0.8 * lineIndex / totalLines);
                juce::String message = "Importing clip " + juce::String(clipCount) + "...";
                if (!progressCallback(progress, message))
                    return false;
            }
        }
        else
        {
            std::cerr << "DEBUG: Failed to parse entry at line " << startLineIndex
                     << " (skipping malformed entry)\n";
            // Skip to next line to avoid infinite loop
            ++lineIndex;
        }
    }

    std::cerr << "DEBUG: Total clips imported: " << clipCount << "\n";

    // Report completion
    if (progressCallback && !progressCallback(1.0, "Import complete!"))
        return false;

    return outProject.getNumClips() > 0;
}

// ============================================================================
// WebVTT Import
// ============================================================================

bool StandaloneImportFeature::parseWebVTTCue(juce::StringArray& lines, int& lineIndex, SubtitleEntry& outEntry) const
{
    // WebVTT Format:
    // WEBVTT
    //
    // 00:00:01.000 --> 00:00:03.500
    // This is the subtitle text
    //
    // Optional cue identifiers:
    // cue1
    // 00:00:01.000 --> 00:00:03.500
    // Text

    if (lineIndex >= lines.size())
        return false;

    // Skip empty lines
    while (lineIndex < lines.size() && lines[lineIndex].trim().isEmpty())
        ++lineIndex;

    if (lineIndex >= lines.size())
        return false;

    // Check if this line is a cue identifier or timecode
    juce::String currentLine = lines[lineIndex].trim();

    // If it doesn't contain -->, it might be a cue identifier
    if (!currentLine.contains("-->"))
    {
        // Skip cue identifier
        ++lineIndex;
        if (lineIndex >= lines.size())
            return false;
        currentLine = lines[lineIndex].trim();
    }

    // Parse timecode line
    juce::StringArray timecodes = juce::StringArray::fromTokens(currentLine, "-->", "");
    if (timecodes.size() != 2)
        return false;

    outEntry.startTime = parseTimecode(timecodes[0]);
    outEntry.endTime = parseTimecode(timecodes[1]);
    ++lineIndex;

    if (outEntry.startTime < 0.0 || outEntry.endTime < 0.0)
        return false;

    // Parse text (can be multiple lines until blank line)
    juce::String textContent;
    while (lineIndex < lines.size() && !lines[lineIndex].trim().isEmpty())
    {
        if (!textContent.isEmpty())
            textContent += " ";
        textContent += lines[lineIndex].trim();
        ++lineIndex;
    }

    outEntry.text = textContent;

    return !textContent.isEmpty();
}

bool StandaloneImportFeature::importWebVTT(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback)
{
    if (!file.existsAsFile())
        return false;

    // Report start
    if (progressCallback && !progressCallback(0.0, "Loading WebVTT file..."))
        return false;

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
        return false;

    // WebVTT must start with "WEBVTT"
    if (!content.startsWith("WEBVTT"))
        return false;

    // Clear existing project
    outProject = Narrate::NarrateProject();
    outProject.setProjectName(file.getFileNameWithoutExtension());

    // Parse WebVTT content
    juce::StringArray lines = juce::StringArray::fromLines(content);
    int lineIndex = 1; // Skip "WEBVTT" header

    if (progressCallback && !progressCallback(0.1, "Parsing WebVTT cues..."))
        return false;

    int clipCount = 0;
    int totalLines = lines.size();

    while (lineIndex < lines.size())
    {
        SubtitleEntry entry;
        if (parseWebVTTCue(lines, lineIndex, entry))
        {
            // Create a clip from this entry
            Narrate::NarrateClip clip;
            clip.setStartTime(entry.startTime);
            clip.setEndTime(entry.endTime);

            // Split text into words
            juce::StringArray words = juce::StringArray::fromTokens(entry.text, " ", "");

            // Distribute words evenly across the clip duration
            double clipDuration = clip.getDuration();
            int wordCount = words.size();

            if (wordCount > 0)
            {
                double timePerWord = clipDuration / wordCount;

                for (int i = 0; i < wordCount; ++i)
                {
                    Narrate::NarrateWord word(words[i], i * timePerWord);
                    clip.addWord(word);
                }
            }

            outProject.addClip(clip);
            clipCount++;

            // Report progress periodically (every 10 clips)
            if (progressCallback && clipCount % 10 == 0)
            {
                double progress = 0.1 + (0.8 * lineIndex / totalLines);
                juce::String message = "Importing cue " + juce::String(clipCount) + "...";
                if (!progressCallback(progress, message))
                    return false;
            }
        }
    }

    // Report completion
    if (progressCallback && !progressCallback(1.0, "Import complete!"))
        return false;

    return outProject.getNumClips() > 0;
}

// ============================================================================
// Plain Text Import
// ============================================================================

double StandaloneImportFeature::estimateDuration(const juce::String& text) const
{
    // Estimate reading time based on word count
    // Average reading speed: ~2.5 words per second
    int wordCount = juce::StringArray::fromTokens(text, " ", "").size();
    return std::max(1.0, wordCount / 2.5);
}

bool StandaloneImportFeature::importPlainText(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback)
{
    if (!file.existsAsFile())
        return false;

    // Report start
    if (progressCallback && !progressCallback(0.0, "Loading text file..."))
        return false;

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
        return false;

    // Clear existing project
    outProject = Narrate::NarrateProject();
    outProject.setProjectName(file.getFileNameWithoutExtension());

    // Split into paragraphs (separated by blank lines)
    juce::StringArray lines = juce::StringArray::fromLines(content);

    if (progressCallback && !progressCallback(0.1, "Processing paragraphs..."))
        return false;

    double currentTime = 0.0;
    juce::String currentParagraph;
    int totalLines = lines.size();
    int lineIndex = 0;
    int paragraphCount = 0;

    for (const auto& line : lines)
    {
        juce::String trimmedLine = line.trim();

        if (trimmedLine.isEmpty())
        {
            // End of paragraph
            if (!currentParagraph.isEmpty())
            {
                Narrate::NarrateClip clip;
                clip.setStartTime(currentTime);
                double duration = estimateDuration(currentParagraph);
                clip.setEndTime(currentTime + duration);

                // Split into words
                juce::StringArray words = juce::StringArray::fromTokens(currentParagraph, " ", "");
                double timePerWord = duration / words.size();

                for (int i = 0; i < words.size(); ++i)
                {
                    Narrate::NarrateWord word(words[i], i * timePerWord);
                    clip.addWord(word);
                }

                outProject.addClip(clip);
                currentTime += duration;
                currentParagraph.clear();
                paragraphCount++;

                // Report progress periodically (every 5 paragraphs)
                if (progressCallback && paragraphCount % 5 == 0)
                {
                    double progress = 0.1 + (0.8 * lineIndex / totalLines);
                    juce::String message = "Processing paragraph " + juce::String(paragraphCount) + "...";
                    if (!progressCallback(progress, message))
                        return false;
                }
            }
        }
        else
        {
            // Continue building paragraph
            if (!currentParagraph.isEmpty())
                currentParagraph += " ";
            currentParagraph += trimmedLine;
        }

        lineIndex++;
    }

    // Add final paragraph if any
    if (!currentParagraph.isEmpty())
    {
        Narrate::NarrateClip clip;
        clip.setStartTime(currentTime);
        double duration = estimateDuration(currentParagraph);
        clip.setEndTime(currentTime + duration);

        juce::StringArray words = juce::StringArray::fromTokens(currentParagraph, " ", "");
        double timePerWord = duration / words.size();

        for (int i = 0; i < words.size(); ++i)
        {
            Narrate::NarrateWord word(words[i], i * timePerWord);
            clip.addWord(word);
        }

        outProject.addClip(clip);
    }

    // Report completion
    if (progressCallback && !progressCallback(1.0, "Import complete!"))
        return false;

    return outProject.getNumClips() > 0;
}

// ============================================================================
// JSON Import
// ============================================================================

juce::Colour StandaloneImportFeature::parseColourFromHex(const juce::String& hexStr) const
{
    // Parse hex color like "FF5733"
    if (hexStr.length() != 6)
        return juce::Colours::white;

    return juce::Colour::fromString("FF" + hexStr);  // Add alpha channel
}

bool StandaloneImportFeature::importJSON(const juce::File& file, Narrate::NarrateProject& outProject, ProgressCallback progressCallback)
{
    if (!file.existsAsFile())
        return false;

    // Report start
    if (progressCallback && !progressCallback(0.0, "Loading JSON file..."))
        return false;

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
        return false;

    // Parse JSON
    juce::var jsonData;
    auto result = juce::JSON::parse(content, jsonData);

    if (result.failed() || !jsonData.isObject())
        return false;

    if (progressCallback && !progressCallback(0.2, "Parsing JSON data..."))
        return false;

    // Clear existing project
    outProject = Narrate::NarrateProject();

    // Extract project metadata
    auto projectObj = jsonData.getDynamicObject();
    if (projectObj == nullptr)
        return false;

    outProject.setProjectName(projectObj->getProperty("projectName").toString());

    // Parse clips array
    auto clipsArray = projectObj->getProperty("clips");
    if (!clipsArray.isArray())
        return false;

    int clipCount = 0;
    int totalClips = clipsArray.getArray()->size();

    for (const auto& clipVar : *clipsArray.getArray())
    {
        if (!clipVar.isObject())
            continue;

        auto clipObj = clipVar.getDynamicObject();

        Narrate::NarrateClip clip;
        double startTime = clipObj->getProperty("startTime");
        double duration = clipObj->getProperty("duration");
        clip.setStartTime(startTime);
        clip.setEndTime(startTime + duration);

        // Parse default formatting if present
        if (clipObj->hasProperty("defaultFormatting"))
        {
            auto fmtObj = clipObj->getProperty("defaultFormatting").getDynamicObject();
            if (fmtObj != nullptr)
            {
                Narrate::TextFormatting fmt;
                fmt.bold = fmtObj->getProperty("bold");
                fmt.italic = fmtObj->getProperty("italic");
                fmt.colour = parseColourFromHex(fmtObj->getProperty("colour").toString());
                fmt.fontSizeMultiplier = fmtObj->getProperty("fontSizeMultiplier");
                clip.setDefaultFormatting(fmt);
            }
        }

        // Parse words
        auto wordsArray = clipObj->getProperty("words");
        if (wordsArray.isArray())
        {
            for (const auto& wordVar : *wordsArray.getArray())
            {
                if (!wordVar.isObject())
                    continue;

                auto wordObj = wordVar.getDynamicObject();

                Narrate::NarrateWord word(wordObj->getProperty("text").toString(),
                                          wordObj->getProperty("time"));

                // Parse word-specific formatting if present
                if (wordObj->hasProperty("formatting"))
                {
                    auto fmtObj = wordObj->getProperty("formatting").getDynamicObject();
                    if (fmtObj != nullptr)
                    {
                        Narrate::TextFormatting fmt;
                        fmt.bold = fmtObj->getProperty("bold");
                        fmt.italic = fmtObj->getProperty("italic");
                        fmt.colour = parseColourFromHex(fmtObj->getProperty("colour").toString());
                        fmt.fontSizeMultiplier = fmtObj->getProperty("fontSizeMultiplier");
                        word.formatting = fmt;
                    }
                }

                clip.addWord(word);
            }
        }

        outProject.addClip(clip);
        clipCount++;

        // Report progress periodically (every 10 clips)
        if (progressCallback && clipCount % 10 == 0)
        {
            double progress = 0.2 + (0.7 * clipCount / totalClips);
            juce::String message = "Importing clip " + juce::String(clipCount) + "/" + juce::String(totalClips) + "...";
            if (!progressCallback(progress, message))
                return false;
        }
    }

    // Report completion
    if (progressCallback && !progressCallback(1.0, "Import complete!"))
        return false;

    return outProject.getNumClips() > 0;
}
