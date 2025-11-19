#include "StandaloneExportFeature.h"

#if NARRATE_ENABLE_SUBTITLE_EXPORT

bool StandaloneExportFeature::exportSRT(const Narrate::NarrateProject& project, const juce::File& outputFile)
{
    if (project.getNumClips() == 0)
        return false;

    juce::String srtContent;
    int sequenceNumber = 1;

    // Iterate through all clips and words
    for (int clipIdx = 0; clipIdx < project.getNumClips(); ++clipIdx)
    {
        const auto& clip = project.getClip(clipIdx);

        for (int wordIdx = 0; wordIdx < clip.getNumWords(); ++wordIdx)
        {
            const auto& word = clip.getWord(wordIdx);

            // Calculate absolute start time
            double startTime = clip.getStartTime() + word.relativeTime;

            // Calculate end time (until next word or clip end)
            double endTime;
            if (wordIdx + 1 < clip.getNumWords())
            {
                const auto& nextWord = clip.getWord(wordIdx + 1);
                endTime = clip.getStartTime() + nextWord.relativeTime;
            }
            else
            {
                // Last word in clip - use clip end time or add default duration
                endTime = clip.getEndTime();
                if (endTime <= startTime)
                    endTime = startTime + 1.0; // Minimum 1 second
            }

            // Format SRT entry
            srtContent += juce::String(sequenceNumber) + "\n";
            srtContent += formatSRTTimestamp(startTime) + " --> " + formatSRTTimestamp(endTime) + "\n";
            srtContent += word.text + "\n";
            srtContent += "\n"; // Blank line between entries

            sequenceNumber++;
        }
    }

    // Write to file
    if (!outputFile.replaceWithText(srtContent))
        return false;

    return true;
}

bool StandaloneExportFeature::exportWebVTT(const Narrate::NarrateProject& project, const juce::File& outputFile)
{
    if (project.getNumClips() == 0)
        return false;

    juce::String vttContent = "WEBVTT\n\n"; // WebVTT header

    // Iterate through all clips and words
    for (int clipIdx = 0; clipIdx < project.getNumClips(); ++clipIdx)
    {
        const auto& clip = project.getClip(clipIdx);

        for (int wordIdx = 0; wordIdx < clip.getNumWords(); ++wordIdx)
        {
            const auto& word = clip.getWord(wordIdx);

            // Calculate absolute start time
            double startTime = clip.getStartTime() + word.relativeTime;

            // Calculate end time
            double endTime;
            if (wordIdx + 1 < clip.getNumWords())
            {
                const auto& nextWord = clip.getWord(wordIdx + 1);
                endTime = clip.getStartTime() + nextWord.relativeTime;
            }
            else
            {
                endTime = clip.getEndTime();
                if (endTime <= startTime)
                    endTime = startTime + 1.0;
            }

            // Format WebVTT cue
            vttContent += formatWebVTTTimestamp(startTime) + " --> " + formatWebVTTTimestamp(endTime) + "\n";

            // Apply formatting if present
            juce::String formattedText = escapeHTMLEntities(word.text);

            if (word.formatting.has_value())
            {
                const auto& fmt = word.formatting.value();

                if (fmt.bold)
                    formattedText = "<b>" + formattedText + "</b>";
                if (fmt.italic)
                    formattedText = "<i>" + formattedText + "</i>";

                // WebVTT supports color via CSS class or <c> tag
                if (fmt.colour != juce::Colours::white)
                {
                    // Convert JUCE color to hex
                    juce::String colorHex = fmt.colour.toString().substring(2); // Skip "ff" alpha
                    formattedText = "<c.color_" + colorHex + ">" + formattedText + "</c>";
                }
            }

            vttContent += formattedText + "\n";
            vttContent += "\n"; // Blank line between cues
        }
    }

    // Write to file
    if (!outputFile.replaceWithText(vttContent))
        return false;

    return true;
}

juce::String StandaloneExportFeature::formatSRTTimestamp(double seconds) const
{
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    return juce::String::formatted("%02d:%02d:%02d,%03d", hours, minutes, secs, millis);
}

juce::String StandaloneExportFeature::formatWebVTTTimestamp(double seconds) const
{
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    return juce::String::formatted("%02d:%02d:%02d.%03d", hours, minutes, secs, millis);
}

juce::String StandaloneExportFeature::escapeHTMLEntities(const juce::String& text) const
{
    juce::String escaped = text;
    escaped = escaped.replace("&", "&amp;");
    escaped = escaped.replace("<", "&lt;");
    escaped = escaped.replace(">", "&gt;");
    return escaped;
}

bool StandaloneExportFeature::exportPlainText(const Narrate::NarrateProject& project, const juce::File& outputFile)
{
    if (project.getNumClips() == 0)
        return false;

    juce::String plainText;

    // Iterate through all clips and words
    for (int clipIdx = 0; clipIdx < project.getNumClips(); ++clipIdx)
    {
        const auto& clip = project.getClip(clipIdx);

        // Add all words from the clip
        for (int wordIdx = 0; wordIdx < clip.getNumWords(); ++wordIdx)
        {
            const auto& word = clip.getWord(wordIdx);
            plainText += word.text + " ";
        }
    }

    plainText = plainText.trim();

    // Write to file
    if (!outputFile.replaceWithText(plainText))
        return false;

    return true;
}

bool StandaloneExportFeature::exportJSON(const Narrate::NarrateProject& project, const juce::File& outputFile)
{
    if (project.getNumClips() == 0)
        return false;

    juce::String jsonContent = "{\n";
    jsonContent += "  \"projectName\": \"" + escapeJSONString(project.getProjectName()) + "\",\n";
    jsonContent += "  \"totalDuration\": " + juce::String(project.getTotalDuration()) + ",\n";
    jsonContent += "  \"clips\": [\n";

    // Iterate through all clips
    for (int clipIdx = 0; clipIdx < project.getNumClips(); ++clipIdx)
    {
        const auto& clip = project.getClip(clipIdx);

        jsonContent += "    {\n";
        jsonContent += "      \"id\": \"" + escapeJSONString(clip.getId()) + "\",\n";
        jsonContent += "      \"startTime\": " + juce::String(clip.getStartTime()) + ",\n";
        jsonContent += "      \"endTime\": " + juce::String(clip.getEndTime()) + ",\n";
        jsonContent += "      \"duration\": " + juce::String(clip.getDuration()) + ",\n";

        // Default formatting
        const auto& fmt = clip.getDefaultFormatting();
        jsonContent += "      \"defaultFormatting\": {\n";
        jsonContent += "        \"colour\": \"" + colourToHex(fmt.colour) + "\",\n";
        jsonContent += "        \"bold\": " + juce::String(fmt.bold ? "true" : "false") + ",\n";
        jsonContent += "        \"italic\": " + juce::String(fmt.italic ? "true" : "false") + ",\n";
        jsonContent += "        \"fontSizeMultiplier\": " + juce::String(fmt.fontSizeMultiplier) + "\n";
        jsonContent += "      },\n";

        jsonContent += "      \"words\": [\n";

        // Iterate through all words
        for (int wordIdx = 0; wordIdx < clip.getNumWords(); ++wordIdx)
        {
            const auto& word = clip.getWord(wordIdx);

            jsonContent += "        {\n";
            jsonContent += "          \"text\": \"" + escapeJSONString(word.text) + "\",\n";
            jsonContent += "          \"relativeTime\": " + juce::String(word.relativeTime) + ",\n";
            jsonContent += "          \"absoluteTime\": " + juce::String(clip.getStartTime() + word.relativeTime);

            if (word.formatting.has_value())
            {
                const auto& wordFmt = word.formatting.value();
                jsonContent += ",\n";
                jsonContent += "          \"formatting\": {\n";
                jsonContent += "            \"colour\": \"" + colourToHex(wordFmt.colour) + "\",\n";
                jsonContent += "            \"bold\": " + juce::String(wordFmt.bold ? "true" : "false") + ",\n";
                jsonContent += "            \"italic\": " + juce::String(wordFmt.italic ? "true" : "false") + ",\n";
                jsonContent += "            \"fontSizeMultiplier\": " + juce::String(wordFmt.fontSizeMultiplier) + "\n";
                jsonContent += "          }\n";
            }
            else
            {
                jsonContent += "\n";
            }

            jsonContent += "        }";
            if (wordIdx < clip.getNumWords() - 1)
                jsonContent += ",";
            jsonContent += "\n";
        }

        jsonContent += "      ]\n";
        jsonContent += "    }";
        if (clipIdx < project.getNumClips() - 1)
            jsonContent += ",";
        jsonContent += "\n";
    }

    jsonContent += "  ]\n";
    jsonContent += "}\n";

    // Write to file
    if (!outputFile.replaceWithText(jsonContent))
        return false;

    return true;
}

bool StandaloneExportFeature::exportCSV(const Narrate::NarrateProject& project, const juce::File& outputFile)
{
    if (project.getNumClips() == 0)
        return false;

    juce::String csvContent;

    // CSV Header
    csvContent += "ClipID,StartTime,EndTime,Duration,WordIndex,Word,RelativeTime,AbsoluteTime,Bold,Italic,Colour,FontSizeMultiplier\n";

    // Iterate through all clips and words
    for (int clipIdx = 0; clipIdx < project.getNumClips(); ++clipIdx)
    {
        const auto& clip = project.getClip(clipIdx);

        for (int wordIdx = 0; wordIdx < clip.getNumWords(); ++wordIdx)
        {
            const auto& word = clip.getWord(wordIdx);
            const auto effectiveFmt = word.getEffectiveFormatting(clip.getDefaultFormatting());
            double absoluteTime = clip.getStartTime() + word.relativeTime;

            csvContent += escapeCSVField(clip.getId()) + ",";
            csvContent += juce::String(clip.getStartTime()) + ",";
            csvContent += juce::String(clip.getEndTime()) + ",";
            csvContent += juce::String(clip.getDuration()) + ",";
            csvContent += juce::String(wordIdx) + ",";
            csvContent += escapeCSVField(word.text) + ",";
            csvContent += juce::String(word.relativeTime) + ",";
            csvContent += juce::String(absoluteTime) + ",";
            csvContent += juce::String(effectiveFmt.bold ? "true" : "false") + ",";
            csvContent += juce::String(effectiveFmt.italic ? "true" : "false") + ",";
            csvContent += escapeCSVField(colourToHex(effectiveFmt.colour)) + ",";
            csvContent += juce::String(effectiveFmt.fontSizeMultiplier) + "\n";
        }
    }

    // Write to file
    if (!outputFile.replaceWithText(csvContent))
        return false;

    return true;
}

juce::String StandaloneExportFeature::escapeCSVField(const juce::String& text) const
{
    // If field contains comma, quote, or newline, wrap in quotes and escape quotes
    if (text.contains(",") || text.contains("\"") || text.contains("\n"))
    {
        juce::String escaped = text.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return text;
}

juce::String StandaloneExportFeature::escapeJSONString(const juce::String& text) const
{
    juce::String escaped = text;
    escaped = escaped.replace("\\", "\\\\");
    escaped = escaped.replace("\"", "\\\"");
    escaped = escaped.replace("\n", "\\n");
    escaped = escaped.replace("\r", "\\r");
    escaped = escaped.replace("\t", "\\t");
    return escaped;
}

juce::String StandaloneExportFeature::colourToHex(const juce::Colour& colour) const
{
    return colour.toString().substring(2); // Skip alpha channel (first 2 chars)
}

#endif // NARRATE_ENABLE_SUBTITLE_EXPORT
