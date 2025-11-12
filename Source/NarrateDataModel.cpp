#include "NarrateDataModel.h"

namespace Narrate
{

//==============================================================================
// TextFormatting
//==============================================================================

TextFormatting TextFormatting::fromXml (const juce::XmlElement& xml)
{
    TextFormatting formatting;

    formatting.colour = juce::Colour::fromString (xml.getStringAttribute ("colour", "FFFFFFFF"));
    formatting.bold = xml.getBoolAttribute ("bold", false);
    formatting.italic = xml.getBoolAttribute ("italic", false);
    formatting.fontSizeMultiplier = (float) xml.getDoubleAttribute ("fontSizeMultiplier", 1.0);

    return formatting;
}

std::unique_ptr<juce::XmlElement> TextFormatting::toXml (const juce::String& elementName) const
{
    auto xml = std::make_unique<juce::XmlElement> (elementName);

    xml->setAttribute ("colour", colour.toString());
    xml->setAttribute ("bold", bold);
    xml->setAttribute ("italic", italic);
    xml->setAttribute ("fontSizeMultiplier", fontSizeMultiplier);

    return xml;
}

//==============================================================================
// NarrateWord
//==============================================================================

NarrateWord NarrateWord::fromXml (const juce::XmlElement& xml)
{
    NarrateWord word;

    word.text = xml.getStringAttribute ("text");
    word.relativeTime = xml.getDoubleAttribute ("relativeTime", 0.0);

    // Check for optional formatting
    if (auto* formattingXml = xml.getChildByName ("Formatting"))
        word.formatting = TextFormatting::fromXml (*formattingXml);

    return word;
}

std::unique_ptr<juce::XmlElement> NarrateWord::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("Word");

    xml->setAttribute ("text", text);
    xml->setAttribute ("relativeTime", relativeTime);

    // Add formatting if it's set
    if (formatting.has_value())
        xml->addChildElement (formatting->toXml ("Formatting").release());

    return xml;
}

//==============================================================================
// NarrateClip
//==============================================================================

NarrateClip NarrateClip::fromXml (const juce::XmlElement& xml)
{
    NarrateClip clip;

    clip.id = xml.getStringAttribute ("id");
    clip.startTime = xml.getDoubleAttribute ("startTime", 0.0);
    clip.endTime = xml.getDoubleAttribute ("endTime", 0.0);

    // Load default formatting
    if (auto* formattingXml = xml.getChildByName ("DefaultFormatting"))
        clip.defaultFormatting = TextFormatting::fromXml (*formattingXml);

    // Load words
    if (auto* wordsXml = xml.getChildByName ("Words"))
    {
        for (auto* wordXml : wordsXml->getChildIterator())
        {
            if (wordXml->hasTagName ("Word"))
                clip.words.add (NarrateWord::fromXml (*wordXml));
        }
    }

    return clip;
}

std::unique_ptr<juce::XmlElement> NarrateClip::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("Clip");

    xml->setAttribute ("id", id);
    xml->setAttribute ("startTime", startTime);
    xml->setAttribute ("endTime", endTime);

    // Add default formatting
    xml->addChildElement (defaultFormatting.toXml ("DefaultFormatting").release());

    // Add words
    auto wordsXml = xml->createNewChildElement ("Words");
    for (const auto& word : words)
        wordsXml->addChildElement (word.toXml().release());

    return xml;
}

//==============================================================================
// NarrateProject
//==============================================================================

bool NarrateProject::saveToFile (const juce::File& file)
{
    auto xml = toXml();
    if (xml == nullptr)
        return false;

    return xml->writeTo (file, juce::XmlElement::TextFormat().singleLine());
}

bool NarrateProject::loadFromFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return false;

    *this = fromXml (*xml);
    return true;
}

std::unique_ptr<juce::XmlElement> NarrateProject::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("NarrateProject");

    xml->setAttribute ("version", "1.0");
    xml->setAttribute ("projectName", projectName);

    // Global settings
    auto settingsXml = xml->createNewChildElement ("Settings");
    settingsXml->setAttribute ("defaultFontSize", defaultFontSize);
    settingsXml->setAttribute ("defaultTextColour", defaultTextColour.toString());
    settingsXml->setAttribute ("highlightColour", highlightColour.toString());

    // Background audio
    if (backgroundAudioFile.existsAsFile())
    {
        auto audioXml = xml->createNewChildElement ("BackgroundAudio");
        audioXml->setAttribute ("path", backgroundAudioFile.getFullPathName());
    }

    // Clips
    auto clipsXml = xml->createNewChildElement ("Clips");
    for (const auto& clip : clips)
        clipsXml->addChildElement (clip.toXml().release());

    return xml;
}

NarrateProject NarrateProject::fromXml (const juce::XmlElement& xml)
{
    NarrateProject project;

    if (!xml.hasTagName ("NarrateProject"))
        return project;

    project.projectName = xml.getStringAttribute ("projectName");

    // Load settings
    if (auto* settingsXml = xml.getChildByName ("Settings"))
    {
        project.defaultFontSize = (float) settingsXml->getDoubleAttribute ("defaultFontSize", 24.0f);
        project.defaultTextColour = juce::Colour::fromString (settingsXml->getStringAttribute ("defaultTextColour", "FFFFFFFF"));
        project.highlightColour = juce::Colour::fromString (settingsXml->getStringAttribute ("highlightColour", "FFFFFF00"));
    }

    // Load background audio
    if (auto* audioXml = xml.getChildByName ("BackgroundAudio"))
    {
        auto path = audioXml->getStringAttribute ("path");
        project.backgroundAudioFile = juce::File (path);
    }

    // Load clips
    if (auto* clipsXml = xml.getChildByName ("Clips"))
    {
        for (auto* clipXml : clipsXml->getChildIterator())
        {
            if (clipXml->hasTagName ("Clip"))
                project.clips.add (NarrateClip::fromXml (*clipXml));
        }
    }

    project.sortClips();

    return project;
}

} // namespace Narrate
