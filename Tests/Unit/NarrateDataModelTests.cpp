#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../Source/NarrateDataModel.h"

using namespace Narrate;

TEST_CASE("TextFormatting", "[data-model][formatting]")
{
    SECTION("Default construction")
    {
        TextFormatting formatting;
        REQUIRE(formatting.colour == juce::Colours::white);
        REQUIRE(formatting.bold == false);
        REQUIRE(formatting.italic == false);
        REQUIRE_THAT(formatting.fontSizeMultiplier, Catch::Matchers::WithinRel(1.0f, 0.001f));
    }

    SECTION("XML serialization round-trip")
    {
        TextFormatting original;
        original.colour = juce::Colours::red;
        original.bold = true;
        original.italic = true;
        original.fontSizeMultiplier = 1.5f;

        auto xml = original.toXml("formatting");
        REQUIRE(xml != nullptr);

        auto restored = TextFormatting::fromXml(*xml);
        REQUIRE(restored.colour == original.colour);
        REQUIRE(restored.bold == original.bold);
        REQUIRE(restored.italic == original.italic);
        REQUIRE_THAT(restored.fontSizeMultiplier,
                     Catch::Matchers::WithinRel(original.fontSizeMultiplier, 0.001f));
    }
}

TEST_CASE("NarrateWord", "[data-model][word]")
{
    SECTION("Construction")
    {
        NarrateWord word("Hello", 1.5);
        REQUIRE(word.text == "Hello");
        REQUIRE_THAT(word.relativeTime, Catch::Matchers::WithinRel(1.5, 0.001));
        REQUIRE(!word.formatting.has_value());
    }

    SECTION("Effective formatting - uses clip default when no word formatting")
    {
        NarrateWord word("Test", 0.0);
        TextFormatting clipDefault;
        clipDefault.colour = juce::Colours::blue;

        auto effective = word.getEffectiveFormatting(clipDefault);
        REQUIRE(effective.colour == juce::Colours::blue);
    }

    SECTION("Effective formatting - uses word formatting when set")
    {
        NarrateWord word("Test", 0.0);
        word.formatting = TextFormatting();
        word.formatting->colour = juce::Colours::red;

        TextFormatting clipDefault;
        clipDefault.colour = juce::Colours::blue;

        auto effective = word.getEffectiveFormatting(clipDefault);
        REQUIRE(effective.colour == juce::Colours::red);
    }

    SECTION("XML serialization round-trip")
    {
        NarrateWord original("World", 2.5);
        original.formatting = TextFormatting();
        original.formatting->bold = true;

        auto xml = original.toXml();
        REQUIRE(xml != nullptr);

        auto restored = NarrateWord::fromXml(*xml);
        REQUIRE(restored.text == original.text);
        REQUIRE_THAT(restored.relativeTime,
                     Catch::Matchers::WithinRel(original.relativeTime, 0.001));
        REQUIRE(restored.formatting.has_value());
        REQUIRE(restored.formatting->bold == true);
    }
}

TEST_CASE("NarrateClip", "[data-model][clip]")
{
    SECTION("Construction and basic properties")
    {
        NarrateClip clip("clip1", 0.0, 5.0);
        REQUIRE(clip.getId() == "clip1");
        REQUIRE_THAT(clip.getStartTime(), Catch::Matchers::WithinRel(0.0, 0.001));
        REQUIRE_THAT(clip.getEndTime(), Catch::Matchers::WithinRel(5.0, 0.001));
        REQUIRE_THAT(clip.getDuration(), Catch::Matchers::WithinRel(5.0, 0.001));
        REQUIRE(clip.getNumWords() == 0);
    }

    SECTION("Word management")
    {
        NarrateClip clip("clip1", 0.0, 10.0);

        clip.addWord(NarrateWord("Hello", 0.5));
        clip.addWord(NarrateWord("World", 1.5));

        REQUIRE(clip.getNumWords() == 2);
        REQUIRE(clip.getWord(0).text == "Hello");
        REQUIRE(clip.getWord(1).text == "World");

        clip.removeWord(0);
        REQUIRE(clip.getNumWords() == 1);
        REQUIRE(clip.getWord(0).text == "World");

        clip.clearWords();
        REQUIRE(clip.getNumWords() == 0);
    }

    SECTION("setText - auto-creates words")
    {
        NarrateClip clip("clip1", 0.0, 10.0);
        clip.setText("Hello World Test");

        REQUIRE(clip.getNumWords() == 3);
        REQUIRE(clip.getWord(0).text == "Hello");
        REQUIRE(clip.getWord(1).text == "World");
        REQUIRE(clip.getWord(2).text == "Test");
    }

    SECTION("getFullText - concatenates all words")
    {
        NarrateClip clip("clip1", 0.0, 10.0);
        clip.addWord(NarrateWord("Hello", 0.0));
        clip.addWord(NarrateWord("World", 1.0));

        REQUIRE(clip.getFullText() == "Hello World");
    }

    SECTION("getWordAbsoluteTime")
    {
        NarrateClip clip("clip1", 10.0, 20.0);
        clip.addWord(NarrateWord("Test", 2.5));

        auto absoluteTime = clip.getWordAbsoluteTime(0);
        REQUIRE_THAT(absoluteTime, Catch::Matchers::WithinRel(12.5, 0.001));
    }

    SECTION("XML serialization round-trip")
    {
        NarrateClip original("clip1", 1.0, 5.0);
        original.addWord(NarrateWord("Hello", 0.5));
        original.addWord(NarrateWord("World", 1.5));

        TextFormatting formatting;
        formatting.bold = true;
        original.setDefaultFormatting(formatting);

        auto xml = original.toXml();
        REQUIRE(xml != nullptr);

        auto restored = NarrateClip::fromXml(*xml);
        REQUIRE(restored.getId() == original.getId());
        REQUIRE_THAT(restored.getStartTime(),
                     Catch::Matchers::WithinRel(original.getStartTime(), 0.001));
        REQUIRE_THAT(restored.getEndTime(),
                     Catch::Matchers::WithinRel(original.getEndTime(), 0.001));
        REQUIRE(restored.getNumWords() == original.getNumWords());
        REQUIRE(restored.getDefaultFormatting().bold == true);
    }
}

TEST_CASE("NarrateProject", "[data-model][project]")
{
    SECTION("Default construction")
    {
        NarrateProject project;
        REQUIRE(project.getNumClips() == 0);
        REQUIRE_THAT(project.getDefaultFontSize(), Catch::Matchers::WithinRel(24.0f, 0.001f));
        REQUIRE(project.getDefaultTextColour() == juce::Colours::white);
        REQUIRE(project.getHighlightColour() == juce::Colours::yellow);
        REQUIRE(project.getRenderStrategy() == NarrateProject::RenderStrategy::Scrolling);
    }

    SECTION("Clip management")
    {
        NarrateProject project;

        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip2", 5.0, 10.0));

        REQUIRE(project.getNumClips() == 2);
        REQUIRE(project.getClip(0).getId() == "clip1");
        REQUIRE(project.getClip(1).getId() == "clip2");

        project.removeClip(0);
        REQUIRE(project.getNumClips() == 1);
        REQUIRE(project.getClip(0).getId() == "clip2");

        project.clearClips();
        REQUIRE(project.getNumClips() == 0);
    }

    SECTION("Clips are automatically sorted by start time")
    {
        NarrateProject project;

        project.addClip(NarrateClip("clip2", 5.0, 10.0));
        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip3", 10.0, 15.0));

        REQUIRE(project.getClip(0).getId() == "clip1");
        REQUIRE(project.getClip(1).getId() == "clip2");
        REQUIRE(project.getClip(2).getId() == "clip3");
    }

    SECTION("getClipIndexAtTime")
    {
        NarrateProject project;
        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip2", 5.0, 10.0));
        project.addClip(NarrateClip("clip3", 10.0, 15.0));

        REQUIRE(project.getClipIndexAtTime(2.5) == 0);
        REQUIRE(project.getClipIndexAtTime(7.0) == 1);
        REQUIRE(project.getClipIndexAtTime(12.0) == 2);
        REQUIRE(project.getClipIndexAtTime(20.0) == -1);  // Beyond all clips
    }

    SECTION("getTotalDuration")
    {
        NarrateProject project;
        REQUIRE_THAT(project.getTotalDuration(), Catch::Matchers::WithinRel(0.0, 0.001));

        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip2", 5.0, 12.0));

        REQUIRE_THAT(project.getTotalDuration(), Catch::Matchers::WithinRel(12.0, 0.001));
    }

    SECTION("hasOverlappingClips - detects overlaps")
    {
        NarrateProject project;
        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip2", 6.0, 10.0));

        REQUIRE(!project.hasOverlappingClips());

        project.addClip(NarrateClip("clip3", 4.0, 8.0));  // Overlaps with clip1
        REQUIRE(project.hasOverlappingClips());
    }

    SECTION("Project settings")
    {
        NarrateProject project;

        project.setProjectName("Test Project");
        REQUIRE(project.getProjectName() == "Test Project");

        project.setDefaultFontSize(32.0f);
        REQUIRE_THAT(project.getDefaultFontSize(), Catch::Matchers::WithinRel(32.0f, 0.001f));

        project.setDefaultTextColour(juce::Colours::blue);
        REQUIRE(project.getDefaultTextColour() == juce::Colours::blue);

        project.setHighlightColour(juce::Colours::red);
        REQUIRE(project.getHighlightColour() == juce::Colours::red);

        project.setRenderStrategy(NarrateProject::RenderStrategy::Karaoke);
        REQUIRE(project.getRenderStrategy() == NarrateProject::RenderStrategy::Karaoke);
    }

    SECTION("recalculateTimeline - removes gaps")
    {
        NarrateProject project;
        project.addClip(NarrateClip("clip1", 0.0, 5.0));
        project.addClip(NarrateClip("clip2", 10.0, 15.0));  // 5 second gap
        project.addClip(NarrateClip("clip3", 20.0, 25.0));  // Another gap

        project.recalculateTimeline();

        // After recalculation, clips should be consecutive
        REQUIRE_THAT(project.getClip(0).getStartTime(), Catch::Matchers::WithinRel(0.0, 0.001));
        REQUIRE_THAT(project.getClip(0).getEndTime(), Catch::Matchers::WithinRel(5.0, 0.001));
        REQUIRE_THAT(project.getClip(1).getStartTime(), Catch::Matchers::WithinRel(5.0, 0.001));
        REQUIRE_THAT(project.getClip(1).getEndTime(), Catch::Matchers::WithinRel(10.0, 0.001));
        REQUIRE_THAT(project.getClip(2).getStartTime(), Catch::Matchers::WithinRel(10.0, 0.001));
        REQUIRE_THAT(project.getClip(2).getEndTime(), Catch::Matchers::WithinRel(15.0, 0.001));
    }
}
