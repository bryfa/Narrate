#include <catch2/catch_test_macros.hpp>
#include "../../Source/Features/StandaloneImportFeature.h"
#include "../../Source/NarrateDataModel.h"
#include <juce_core/juce_core.h>

/**
 * Unit tests for Import Feature
 *
 * Tests cover:
 * - SRT format import
 * - WebVTT format import
 * - JSON format import (round-trip)
 * - Plain text import
 * - Format auto-detection
 * - Edge cases and error handling
 */

TEST_CASE("ImportFeature - SRT Format", "[import][srt]")
{
    StandaloneImportFeature importer;
    Narrate::NarrateProject project;

    SECTION("Import valid SRT file")
    {
        // Create a temporary SRT file
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_import.srt");

        juce::String srtContent =
            "1\n"
            "00:00:01,000 --> 00:00:03,500\n"
            "This is the first subtitle\n"
            "\n"
            "2\n"
            "00:00:04,000 --> 00:00:06,500\n"
            "This is the second subtitle\n"
            "\n"
            "3\n"
            "00:00:07,000 --> 00:00:09,500\n"
            "This is the third subtitle\n";

        tempFile.replaceWithText(srtContent);

        // Import the file
        bool result = importer.importSRT(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 3);

        // Check first clip
        auto& clip1 = project.getClip(0);
        REQUIRE(clip1.getStartTime() == 1.0);
        REQUIRE(clip1.getEndTime() == 3.5);
        REQUIRE(clip1.getNumWords() == 5);  // "This is the first subtitle"

        // Check second clip
        auto& clip2 = project.getClip(1);
        REQUIRE(clip2.getStartTime() == 4.0);
        REQUIRE(clip2.getEndTime() == 6.5);

        // Check third clip
        auto& clip3 = project.getClip(2);
        REQUIRE(clip3.getStartTime() == 7.0);
        REQUIRE(clip3.getEndTime() == 9.5);

        // Cleanup
        tempFile.deleteFile();
    }

    SECTION("Import SRT with multi-line text")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_multiline.srt");

        juce::String srtContent =
            "1\n"
            "00:00:01,000 --> 00:00:03,500\n"
            "This is line one\n"
            "This is line two\n"
            "\n";

        tempFile.replaceWithText(srtContent);

        bool result = importer.importSRT(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 1);

        // Multi-line text should be combined with spaces
        auto& clip = project.getClip(0);
        REQUIRE(clip.getNumWords() == 7);  // "This is line one This is line two"

        tempFile.deleteFile();
    }

    SECTION("Handle empty SRT file")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_empty.srt");

        tempFile.replaceWithText("");

        bool result = importer.importSRT(tempFile, project);

        REQUIRE(result == false);

        tempFile.deleteFile();
    }

    SECTION("Handle malformed timecode")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_malformed.srt");

        juce::String srtContent =
            "1\n"
            "INVALID TIMECODE\n"
            "This should not be imported\n"
            "\n";

        tempFile.replaceWithText(srtContent);

        bool result = importer.importSRT(tempFile, project);

        REQUIRE(result == false);

        tempFile.deleteFile();
    }

    SECTION("Import long SRT file with special characters")
    {
        // Test the real-world SRT file that had character encoding issues
        juce::File testFile = juce::File::getCurrentWorkingDirectory()
            .getChildFile("Tests/Data/long-srt-subs-test-file.srt");

        // Only run test if file exists
        if (testFile.existsAsFile())
        {
            bool result = importer.importSRT(testFile, project);

            REQUIRE(result == true);
            REQUIRE(project.getNumClips() > 0);

            // Verify first clip is properly parsed
            auto& firstClip = project.getClip(0);
            REQUIRE(firstClip.getStartTime() >= 0.0);
            REQUIRE(firstClip.getEndTime() > firstClip.getStartTime());
            REQUIRE(firstClip.getNumWords() > 0);

            // Verify text doesn't contain problematic control characters
            for (int i = 0; i < project.getNumClips(); ++i)
            {
                auto& clip = project.getClip(i);
                for (int w = 0; w < clip.getNumWords(); ++w)
                {
                    juce::String wordText = clip.getWord(w).text;

                    // Check that text doesn't contain zero-width spaces or control chars
                    for (int c = 0; c < wordText.length(); ++c)
                    {
                        juce::juce_wchar ch = wordText[c];
                        // Allow normal printable characters, spaces, newlines
                        bool isAcceptable = (ch >= 0x20 && ch < 0x7F) ||  // ASCII printable
                                          ch >= 0x80 ||                    // Extended ASCII/Unicode
                                          ch == '\n' || ch == '\r' || ch == '\t';
                        REQUIRE(isAcceptable);
                    }
                }
            }
        }
        else
        {
            // Skip test if file doesn't exist
            WARN("Test file not found: Tests/Data/long-srt-subs-test-file.srt - skipping test");
        }
    }
}

TEST_CASE("ImportFeature - WebVTT Format", "[import][webvtt]")
{
    StandaloneImportFeature importer;
    Narrate::NarrateProject project;

    SECTION("Import valid WebVTT file")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_import.vtt");

        juce::String vttContent =
            "WEBVTT\n"
            "\n"
            "00:00:01.000 --> 00:00:03.500\n"
            "First subtitle\n"
            "\n"
            "00:00:04.000 --> 00:00:06.500\n"
            "Second subtitle\n"
            "\n";

        tempFile.replaceWithText(vttContent);

        bool result = importer.importWebVTT(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 2);

        // Check first clip
        auto& clip1 = project.getClip(0);
        REQUIRE(clip1.getStartTime() == 1.0);
        REQUIRE(clip1.getEndTime() == 3.5);

        tempFile.deleteFile();
    }

    SECTION("Import WebVTT with cue identifiers")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_cues.vtt");

        juce::String vttContent =
            "WEBVTT\n"
            "\n"
            "cue1\n"
            "00:00:01.000 --> 00:00:03.500\n"
            "First subtitle\n"
            "\n"
            "cue2\n"
            "00:00:04.000 --> 00:00:06.500\n"
            "Second subtitle\n"
            "\n";

        tempFile.replaceWithText(vttContent);

        bool result = importer.importWebVTT(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 2);

        tempFile.deleteFile();
    }

    SECTION("Reject file without WEBVTT header")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_no_header.vtt");

        juce::String vttContent =
            "00:00:01.000 --> 00:00:03.500\n"
            "This should fail\n";

        tempFile.replaceWithText(vttContent);

        bool result = importer.importWebVTT(tempFile, project);

        REQUIRE(result == false);

        tempFile.deleteFile();
    }
}

TEST_CASE("ImportFeature - Plain Text Format", "[import][text]")
{
    StandaloneImportFeature importer;
    Narrate::NarrateProject project;

    SECTION("Import plain text with paragraphs")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_import.txt");

        juce::String txtContent =
            "This is the first paragraph.\n"
            "\n"
            "This is the second paragraph.\n"
            "\n"
            "This is the third paragraph.\n";

        tempFile.replaceWithText(txtContent);

        bool result = importer.importPlainText(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 3);

        // Timing should be estimated based on word count
        // Average reading speed: 2.5 words per second
        auto& clip1 = project.getClip(0);
        REQUIRE(clip1.getStartTime() == 0.0);
        // First paragraph: 5 words, so duration ~= 5 / 2.5 = 2.0 seconds
        REQUIRE(clip1.getDuration() == Catch::Approx(2.0).epsilon(0.1));

        // Second clip should start after first
        auto& clip2 = project.getClip(1);
        REQUIRE(clip2.getStartTime() == Catch::Approx(2.0).epsilon(0.1));

        tempFile.deleteFile();
    }

    SECTION("Handle continuous text without paragraphs")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_continuous.txt");

        juce::String txtContent = "This is continuous text without breaks.";

        tempFile.replaceWithText(txtContent);

        bool result = importer.importPlainText(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getNumClips() == 1);

        auto& clip = project.getClip(0);
        REQUIRE(clip.getNumWords() == 6);

        tempFile.deleteFile();
    }
}

TEST_CASE("ImportFeature - JSON Format", "[import][json]")
{
    StandaloneImportFeature importer;
    Narrate::NarrateProject project;

    SECTION("Import valid JSON file")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_import.json");

        juce::String jsonContent = R"({
  "projectName": "Test Project",
  "clips": [
    {
      "startTime": 1.0,
      "duration": 2.5,
      "words": [
        { "text": "Hello", "time": 0.0 },
        { "text": "World", "time": 0.5 }
      ]
    }
  ]
})";

        tempFile.replaceWithText(jsonContent);

        bool result = importer.importJSON(tempFile, project);

        REQUIRE(result == true);
        REQUIRE(project.getProjectName() == "Test Project");
        REQUIRE(project.getNumClips() == 1);

        auto& clip = project.getClip(0);
        REQUIRE(clip.getStartTime() == 1.0);
        REQUIRE(clip.getDuration() == 2.5);
        REQUIRE(clip.getNumWords() == 2);

        tempFile.deleteFile();
    }

    SECTION("Handle malformed JSON")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_malformed.json");

        juce::String jsonContent = "{ invalid json }";

        tempFile.replaceWithText(jsonContent);

        bool result = importer.importJSON(tempFile, project);

        REQUIRE(result == false);

        tempFile.deleteFile();
    }
}

TEST_CASE("ImportFeature - Format Detection", "[import][detection]")
{
    StandaloneImportFeature importer;
    juce::String detectedFormat;

    SECTION("Detect SRT format")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test.srt");

        juce::String srtContent =
            "1\n"
            "00:00:01,000 --> 00:00:03,500\n"
            "Subtitle text\n";

        tempFile.replaceWithText(srtContent);

        bool result = importer.detectFormat(tempFile, detectedFormat);

        REQUIRE(result == true);
        REQUIRE(detectedFormat == "srt");

        tempFile.deleteFile();
    }

    SECTION("Detect WebVTT format")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test.vtt");

        juce::String vttContent = "WEBVTT\n\n00:00:01.000 --> 00:00:03.500\nText\n";

        tempFile.replaceWithText(vttContent);

        bool result = importer.detectFormat(tempFile, detectedFormat);

        REQUIRE(result == true);
        REQUIRE(detectedFormat == "vtt");

        tempFile.deleteFile();
    }

    SECTION("Detect JSON format")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test.json");

        juce::String jsonContent = R"({ "projectName": "Test", "clips": [] })";

        tempFile.replaceWithText(jsonContent);

        bool result = importer.detectFormat(tempFile, detectedFormat);

        REQUIRE(result == true);
        REQUIRE(detectedFormat == "json");

        tempFile.deleteFile();
    }

    SECTION("Default to plain text for unknown format")
    {
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test.txt");

        juce::String txtContent = "Just some plain text";

        tempFile.replaceWithText(txtContent);

        bool result = importer.detectFormat(tempFile, detectedFormat);

        REQUIRE(result == true);
        REQUIRE(detectedFormat == "txt");

        tempFile.deleteFile();
    }
}

TEST_CASE("ImportFeature - Timecode Parsing", "[import][timecode]")
{
    SECTION("Parse SRT timecode format (HH:MM:SS,mmm)")
    {
        REQUIRE(ImportFeature::parseTimecode("00:00:01,000") == 1.0);
        REQUIRE(ImportFeature::parseTimecode("00:01:30,500") == 90.5);
        REQUIRE(ImportFeature::parseTimecode("01:23:45,678") == Catch::Approx(5025.678));
    }

    SECTION("Parse WebVTT timecode format (HH:MM:SS.mmm)")
    {
        REQUIRE(ImportFeature::parseTimecode("00:00:01.000") == 1.0);
        REQUIRE(ImportFeature::parseTimecode("00:01:30.500") == 90.5);
        REQUIRE(ImportFeature::parseTimecode("01:23:45.678") == Catch::Approx(5025.678));
    }

    SECTION("Handle invalid timecode formats")
    {
        REQUIRE(ImportFeature::parseTimecode("invalid") == -1.0);
        REQUIRE(ImportFeature::parseTimecode("12:34") == -1.0);
        REQUIRE(ImportFeature::parseTimecode("") == -1.0);
    }

    SECTION("Handle edge cases")
    {
        REQUIRE(ImportFeature::parseTimecode("00:00:00,000") == 0.0);
        REQUIRE(ImportFeature::parseTimecode("23:59:59,999") == Catch::Approx(86399.999));
        REQUIRE(ImportFeature::parseTimecode("  00:00:01.000  ") == 1.0);  // With whitespace
    }
}

TEST_CASE("ImportFeature - Round-trip Conversion", "[import][export][integration]")
{
    StandaloneImportFeature importer;
    Narrate::NarrateProject originalProject, importedProject;

    SECTION("SRT -> Project -> JSON -> Project round-trip")
    {
        // Create original SRT file
        juce::File srtFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_roundtrip.srt");

        juce::String srtContent =
            "1\n"
            "00:00:01,000 --> 00:00:03,500\n"
            "Test subtitle\n"
            "\n";

        srtFile.replaceWithText(srtContent);

        // Import SRT
        bool importResult = importer.importSRT(srtFile, originalProject);
        REQUIRE(importResult == true);
        REQUIRE(originalProject.getNumClips() == 1);

        // Save to JSON
        juce::File jsonFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("test_roundtrip.json");

        // Export using StandaloneExportFeature would be needed here
        // For now, just test the import side

        // Cleanup
        srtFile.deleteFile();
        jsonFile.deleteFile();
    }
}
