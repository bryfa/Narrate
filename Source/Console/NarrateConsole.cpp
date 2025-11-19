#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include "../NarrateDataModel.h"
#include "../Features/StandaloneExportFeature.h"
#include "../NarrateConfig.h"

#include <iostream>
#include <string>

/**
 * NarrateConsole
 *
 * Command-line application for exporting Narrate projects to various formats.
 *
 * Usage:
 *   narrate-console <input.narrate> <output> --format <format>
 *
 * Formats:
 *   - srt       : SubRip subtitle format
 *   - vtt       : WebVTT subtitle format
 *   - txt       : Plain text
 *   - json      : JSON format with full metadata
 *   - csv       : CSV format with word-level timing
 */

void printUsage(const juce::String& programName)
{
    std::cout << "Narrate Console Exporter v0.1.0\n";
    std::cout << "================================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName.toStdString() << " <input.narrate> <output> --format <format>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --format <format>   Output format (required)\n";
    std::cout << "                      Available formats: srt, vtt, txt, json, csv\n";
    std::cout << "  --help, -h          Show this help message\n";
    std::cout << "  --version, -v       Show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName.toStdString() << " project.narrate output.srt --format srt\n";
    std::cout << "  " << programName.toStdString() << " project.narrate output.json --format json\n";
    std::cout << "  " << programName.toStdString() << " project.narrate transcript.txt --format txt\n\n";
}

void printVersion()
{
    std::cout << "Narrate Console Exporter v0.1.0\n";
    std::cout << "Copyright (c) 2025 MulhacenLabs\n";
}

struct CommandLineArgs
{
    juce::File inputFile;
    juce::File outputFile;
    juce::String format;
    bool valid = false;
};

CommandLineArgs parseArguments(int argc, char* argv[])
{
    CommandLineArgs args;

    if (argc < 2)
        return args;

    juce::String programName = juce::File(argv[0]).getFileName();

    // Check for help or version flags
    for (int i = 1; i < argc; ++i)
    {
        juce::String arg(argv[i]);

        if (arg == "--help" || arg == "-h")
        {
            printUsage(programName);
            return args;
        }

        if (arg == "--version" || arg == "-v")
        {
            printVersion();
            return args;
        }
    }

    // Parse required arguments
    if (argc < 5)
    {
        std::cerr << "Error: Not enough arguments\n\n";
        printUsage(programName);
        return args;
    }

    args.inputFile = juce::File(argv[1]);
    args.outputFile = juce::File(argv[2]);

    // Parse optional arguments
    for (int i = 3; i < argc; ++i)
    {
        juce::String arg(argv[i]);

        if (arg == "--format" && i + 1 < argc)
        {
            args.format = juce::String(argv[i + 1]).toLowerCase();
            ++i;
        }
    }

    // Validate arguments
    if (!args.inputFile.existsAsFile())
    {
        std::cerr << "Error: Input file does not exist: " << args.inputFile.getFullPathName().toStdString() << "\n";
        return args;
    }

    if (args.format.isEmpty())
    {
        std::cerr << "Error: Output format not specified. Use --format <format>\n";
        return args;
    }

    if (args.format != "srt" && args.format != "vtt" &&
        args.format != "txt" && args.format != "json" && args.format != "csv")
    {
        std::cerr << "Error: Unknown format '" << args.format.toStdString() << "'\n";
        std::cerr << "Available formats: srt, vtt, txt, json, csv\n";
        return args;
    }

    args.valid = true;
    return args;
}

int main(int argc, char* argv[])
{
    // Initialize JUCE
    juce::initialiseJuce_GUI();

    // Parse command line arguments
    CommandLineArgs args = parseArguments(argc, argv);

    if (!args.valid)
    {
        juce::shutdownJuce_GUI();
        return args.format.isEmpty() && args.inputFile == juce::File() ? 0 : 1;
    }

    // Load the project
    std::cout << "Loading project: " << args.inputFile.getFullPathName().toStdString() << "\n";

    Narrate::NarrateProject project;
    if (!project.loadFromFile(args.inputFile))
    {
        std::cerr << "Error: Failed to load project file\n";
        juce::shutdownJuce_GUI();
        return 1;
    }

    std::cout << "Project loaded successfully\n";
    std::cout << "  - Clips: " << project.getNumClips() << "\n";
    std::cout << "  - Duration: " << project.getTotalDuration() << " seconds\n\n";

    // Check if project is empty
    if (project.getNumClips() == 0)
    {
        std::cerr << "Warning: Project has no clips. Output will be empty.\n";
    }

    // Create exporter
#if NARRATE_ENABLE_SUBTITLE_EXPORT
    StandaloneExportFeature exporter;
#else
    std::cerr << "Error: Export functionality is not available in this build\n";
    juce::shutdownJuce_GUI();
    return 1;
#endif

    // Export based on format
    std::cout << "Exporting to " << args.format.toStdString() << " format...\n";

    bool success = false;

    if (args.format == "srt")
    {
        success = exporter.exportSRT(project, args.outputFile);
    }
    else if (args.format == "vtt")
    {
        success = exporter.exportWebVTT(project, args.outputFile);
    }
    else if (args.format == "txt")
    {
        success = exporter.exportPlainText(project, args.outputFile);
    }
    else if (args.format == "json")
    {
        success = exporter.exportJSON(project, args.outputFile);
    }
    else if (args.format == "csv")
    {
        success = exporter.exportCSV(project, args.outputFile);
    }

    if (success)
    {
        std::cout << "Export successful!\n";
        std::cout << "Output saved to: " << args.outputFile.getFullPathName().toStdString() << "\n";
    }
    else
    {
        std::cerr << "Error: Export failed\n";
        juce::shutdownJuce_GUI();
        return 1;
    }

    // Cleanup
    juce::shutdownJuce_GUI();
    return 0;
}
