#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace Narrate
{

//==============================================================================
/** Formatting options that can be applied to text */
struct TextFormatting
{
    juce::Colour colour = juce::Colours::white;
    bool bold = false;
    bool italic = false;
    float fontSizeMultiplier = 1.0f;  // Relative to base font size

    TextFormatting() = default;

    // Create from XML
    static TextFormatting fromXml (const juce::XmlElement& xml);

    // Convert to XML
    std::unique_ptr<juce::XmlElement> toXml (const juce::String& elementName) const;
};

//==============================================================================
/** Individual word with timing and formatting */
struct NarrateWord
{
    juce::String text;
    double relativeTime = 0.0;  // Time in seconds relative to clip start

    // Optional word-specific formatting (overrides clip formatting if set)
    std::optional<TextFormatting> formatting;

    NarrateWord() = default;
    NarrateWord (const juce::String& wordText, double time = 0.0)
        : text (wordText), relativeTime (time) {}

    // Get effective formatting (uses word formatting if set, otherwise clip default)
    TextFormatting getEffectiveFormatting (const TextFormatting& clipDefault) const
    {
        return formatting.has_value() ? formatting.value() : clipDefault;
    }

    // Create from XML
    static NarrateWord fromXml (const juce::XmlElement& xml);

    // Convert to XML
    std::unique_ptr<juce::XmlElement> toXml() const;
};

//==============================================================================
/** A clip/line containing multiple words with timing */
class NarrateClip
{
public:
    NarrateClip() = default;
    NarrateClip (const juce::String& clipId, double start, double end)
        : id (clipId), startTime (start), endTime (end) {}

    // Getters
    const juce::String& getId() const { return id; }
    double getStartTime() const { return startTime; }
    double getEndTime() const { return endTime; }
    double getDuration() const { return endTime - startTime; }
    const juce::Array<NarrateWord>& getWords() const { return words; }
    const TextFormatting& getDefaultFormatting() const { return defaultFormatting; }

    // Setters
    void setStartTime (double time) { startTime = time; }
    void setEndTime (double time) { endTime = time; }
    void setDefaultFormatting (const TextFormatting& formatting) { defaultFormatting = formatting; }

    // Word management
    void addWord (const NarrateWord& word) { words.add (word); }
    void insertWord (int index, const NarrateWord& word) { words.insert (index, word); }
    void removeWord (int index) { words.remove (index); }
    void clearWords() { words.clear(); }
    NarrateWord& getWord (int index) { return words.getReference (index); }
    int getNumWords() const { return words.size(); }

    // Get full text of all words
    juce::String getFullText() const
    {
        juce::String result;
        for (const auto& word : words)
            result += word.text + " ";
        return result.trim();
    }

    // Set text and auto-create words (without timing)
    void setText (const juce::String& text)
    {
        words.clear();
        auto tokens = juce::StringArray::fromTokens (text, " \t\n", "");
        for (const auto& token : tokens)
            words.add (NarrateWord (token, 0.0));
    }

    // Get absolute time for a word
    double getWordAbsoluteTime (int wordIndex) const
    {
        if (wordIndex >= 0 && wordIndex < words.size())
            return startTime + words[wordIndex].relativeTime;
        return startTime;
    }

    // Create from XML
    static NarrateClip fromXml (const juce::XmlElement& xml);

    // Convert to XML
    std::unique_ptr<juce::XmlElement> toXml() const;

private:
    juce::String id;
    double startTime = 0.0;
    double endTime = 0.0;
    juce::Array<NarrateWord> words;
    TextFormatting defaultFormatting;  // Default formatting for all words in this clip

    JUCE_LEAK_DETECTOR (NarrateClip)
};

//==============================================================================
/** The complete project containing all clips and settings */
class NarrateProject
{
public:
    NarrateProject() = default;

    // Project metadata
    const juce::String& getProjectName() const { return projectName; }
    void setProjectName (const juce::String& name) { projectName = name; }

    // Background audio
    const juce::File& getBackgroundAudioFile() const { return backgroundAudioFile; }
    void setBackgroundAudioFile (const juce::File& file) { backgroundAudioFile = file; }
    bool hasBackgroundAudio() const { return backgroundAudioFile.existsAsFile(); }

    // Global settings
    float getDefaultFontSize() const { return defaultFontSize; }
    void setDefaultFontSize (float size) { defaultFontSize = size; }

    const juce::Colour& getDefaultTextColour() const { return defaultTextColour; }
    void setDefaultTextColour (const juce::Colour& colour) { defaultTextColour = colour; }

    const juce::Colour& getHighlightColour() const { return highlightColour; }
    void setHighlightColour (const juce::Colour& colour) { highlightColour = colour; }

    // Render strategy selection
    enum class RenderStrategy
    {
        Scrolling = 0,
        Karaoke = 1,
        Teleprompter = 2
    };

    RenderStrategy getRenderStrategy() const { return renderStrategy; }
    void setRenderStrategy (RenderStrategy strategy) { renderStrategy = strategy; }

    // Clip management
    void addClip (const NarrateClip& clip) { clips.add (clip); sortClips(); }
    void insertClip (int index, const NarrateClip& clip) { clips.insert (index, clip); sortClips(); }
    void removeClip (int index) { clips.remove (index); }
    void clearClips() { clips.clear(); }

    // Recalculate timeline to remove gaps between clips
    void recalculateTimeline();

    NarrateClip& getClip (int index) { return clips.getReference (index); }
    const NarrateClip& getClip (int index) const { return clips.getReference (index); }
    int getNumClips() const { return clips.size(); }

    // Get clip at a specific time
    int getClipIndexAtTime (double time) const
    {
        for (int i = 0; i < clips.size(); ++i)
        {
            const auto& clip = clips.getReference (i);
            if (time >= clip.getStartTime() && time < clip.getEndTime())
                return i;
        }
        return -1;
    }

    // Get total duration (end time of last clip)
    double getTotalDuration() const
    {
        if (clips.isEmpty())
            return 0.0;

        double maxEnd = 0.0;
        for (const auto& clip : clips)
            maxEnd = juce::jmax (maxEnd, clip.getEndTime());

        return maxEnd;
    }

    // Validation - check for overlapping clips
    bool hasOverlappingClips() const
    {
        for (int i = 0; i < clips.size() - 1; ++i)
        {
            const auto& current = clips.getReference (i);
            const auto& next = clips.getReference (i + 1);
            if (current.getEndTime() > next.getStartTime())
                return true;
        }
        return false;
    }

    // Serialization
    bool saveToFile (const juce::File& file);
    bool loadFromFile (const juce::File& file);

    std::unique_ptr<juce::XmlElement> toXml() const;
    static NarrateProject fromXml (const juce::XmlElement& xml);

private:
    juce::String projectName;
    juce::Array<NarrateClip> clips;
    juce::File backgroundAudioFile;

    // Global settings
    float defaultFontSize = 24.0f;
    juce::Colour defaultTextColour = juce::Colours::white;
    juce::Colour highlightColour = juce::Colours::yellow;
    RenderStrategy renderStrategy = RenderStrategy::Scrolling;

    // Keep clips sorted by start time
    void sortClips()
    {
        std::sort (clips.begin(), clips.end(),
                   [] (const NarrateClip& a, const NarrateClip& b)
                   { return a.getStartTime() < b.getStartTime(); });
    }

    JUCE_LEAK_DETECTOR (NarrateProject)
};

} // namespace Narrate
