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

#endif // NARRATE_ENABLE_SUBTITLE_EXPORT
