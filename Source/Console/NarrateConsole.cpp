#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include "../NarrateDataModel.h"
#include "../Features/StandaloneExportFeature.h"
#include "../Features/StandaloneImportFeature.h"
#include "../NarrateConfig.h"

#include <iostream>
#include <string>

/**
 * NarrateConsole
 *
 * Command-line application for converting between subtitle/transcript formats.
 *
 * Usage:
 *   narrate-console <input> <output> --format <format>
 *   narrate-console convert <input> <output> [--format <format>]
 *
 * Supported Formats:
 *   - srt       : SubRip subtitle format
 *   - vtt       : WebVTT subtitle format
 *   - txt       : Plain text
 *   - json      : JSON format with full metadata
 *   - csv       : CSV format with word-level timing
 *   - narrate   : Native Narrate project format
 */

void printUsage(const juce::String& programName)
{
    std::cout << "Narrate Console Converter v0.2.0\n";
    std::cout << "=================================\n\n";
    std::cout << "Convert between subtitle and transcript formats\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName.toStdString() << " <input> <output> --format <format>\n";
    std::cout << "  " << programName.toStdString() << " convert <input> <output> [--format <format>]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --format <format>   Output format (auto-detected if not specified)\n";
    std::cout << "                      Available: srt, vtt, txt, json, csv, narrate\n";
    std::cout << "  --help, -h          Show this help message\n";
    std::cout << "  --version, -v       Show version information\n\n";
    std::cout << "Supported Input Formats:\n";
    std::cout << "  .srt       SubRip subtitle files\n";
    std::cout << "  .vtt       WebVTT subtitle files\n";
    std::cout << "  .txt       Plain text (timing estimated)\n";
    std::cout << "  .json      JSON export from Narrate\n";
    std::cout << "  .narrate   Native Narrate project files\n\n";
    std::cout << "Examples:\n";
    std::cout << "  # Convert SRT to WebVTT\n";
    std::cout << "  " << programName.toStdString() << " input.srt output.vtt\n\n";
    std::cout << "  # Convert WebVTT to JSON with explicit format\n";
    std::cout << "  " << programName.toStdString() << " input.vtt output.json --format json\n\n";
    std::cout << "  # Import SRT and save as Narrate project\n";
    std::cout << "  " << programName.toStdString() << " subtitles.srt project.narrate\n\n";
    std::cout << "  # Export Narrate project to CSV\n";
    std::cout << "  " << programName.toStdString() << " project.narrate data.csv --format csv\n\n";
}

void printVersion()
{
    std::cout << "Narrate Console Converter v0.2.0\n";
    std::cout << "Copyright (c) 2025 MulhacenLabs\n";
}

struct CommandLineArgs
{
    juce::File inputFile;
    juce::File outputFile;
    juce::String format;  // Output format (can be empty for auto-detect)
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

    // Parse command (optional "convert" keyword)
    int argIndex = 1;
    if (argc > 1 && juce::String(argv[1]) == "convert")
    {
        argIndex = 2;  // Skip "convert" keyword
    }

    // Need at least input and output files
    if (argc < argIndex + 2)
    {
        std::cerr << "Error: Missing required arguments\n";
        printUsage(programName);
        return args;
    }

    // Parse input file
    args.inputFile = juce::File::getCurrentWorkingDirectory().getChildFile(argv[argIndex]);
    ++argIndex;

    // Parse output file
    args.outputFile = juce::File::getCurrentWorkingDirectory().getChildFile(argv[argIndex]);
    ++argIndex;

    // Parse optional format flag
    while (argIndex < argc)
    {
        juce::String arg(argv[argIndex]);

        if (arg == "--format" && argIndex + 1 < argc)
        {
            args.format = juce::String(argv[argIndex + 1]).toLowerCase();
            argIndex += 2;
        }
        else
        {
            std::cerr << "Error: Unknown argument '" << arg.toStdString() << "'\n";
            return args;
        }
    }

    // Validate input file exists
    if (!args.inputFile.existsAsFile())
    {
        std::cerr << "Error: Input file does not exist: " << args.inputFile.getFullPathName().toStdString() << "\n";
        return args;
    }

    // Auto-detect output format from file extension if not specified
    if (args.format.isEmpty())
    {
        args.format = args.outputFile.getFileExtension().substring(1).toLowerCase();
    }

    // Validate format
    if (args.format != "srt" && args.format != "vtt" &&
        args.format != "txt" && args.format != "json" &&
        args.format != "csv" && args.format != "narrate")
    {
        std::cerr << "Error: Unknown format '" << args.format.toStdString() << "'\n";
        std::cerr << "Supported formats: srt, vtt, txt, json, csv, narrate\n";
        return args;
    }

    args.valid = true;
    return args;
}

bool loadProject(const juce::File& inputFile, Narrate::NarrateProject& project)
{
    // Try to load as native Narrate project first
    if (inputFile.hasFileExtension(".narrate"))
    {
        if (project.loadFromFile(inputFile))
        {
            std::cout << "Loaded Narrate project: " << inputFile.getFileNameWithoutExtension().toStdString() << "\n";
            return true;
        }
        std::cerr << "Error: Failed to load Narrate project\n";
        return false;
    }

    // Otherwise, try to import from subtitle format
    StandaloneImportFeature importer;

    // Auto-detect format
    juce::String detectedFormat;
    if (!importer.detectFormat(inputFile, detectedFormat))
    {
        std::cerr << "Error: Could not detect input file format\n";
        return false;
    }

    std::cout << "Detected format: " << detectedFormat.toStdString() << "\n";

    // Import based on detected format
    bool success = false;

    if (detectedFormat == "srt")
        success = importer.importSRT(inputFile, project);
    else if (detectedFormat == "vtt")
        success = importer.importWebVTT(inputFile, project);
    else if (detectedFormat == "json")
        success = importer.importJSON(inputFile, project);
    else if (detectedFormat == "txt")
        success = importer.importPlainText(inputFile, project);

    if (success)
    {
        std::cout << "Imported " << project.getNumClips() << " clips from " << detectedFormat.toStdString() << " file\n";
        return true;
    }

    std::cerr << "Error: Failed to import from " << detectedFormat.toStdString() << " format\n";
    return false;
}

bool exportProject(Narrate::NarrateProject& project, const juce::File& outputFile, const juce::String& format)
{
    // Handle native Narrate format
    if (format == "narrate")
    {
        if (project.saveToFile(outputFile))
        {
            std::cout << "Saved Narrate project to: " << outputFile.getFullPathName().toStdString() << "\n";
            return true;
        }
        std::cerr << "Error: Failed to save Narrate project\n";
        return false;
    }

    // Export to subtitle format
    StandaloneExportFeature exporter;
    bool success = false;

    if (format == "srt")
        success = exporter.exportSRT(project, outputFile);
    else if (format == "vtt")
        success = exporter.exportWebVTT(project, outputFile);
    else if (format == "txt")
        success = exporter.exportPlainText(project, outputFile);
    else if (format == "json")
        success = exporter.exportJSON(project, outputFile);
    else if (format == "csv")
        success = exporter.exportCSV(project, outputFile);

    if (success)
    {
        std::cout << "Exported to " << format.toStdString() << ": " << outputFile.getFullPathName().toStdString() << "\n";
        return true;
    }

    std::cerr << "Error: Export to " << format.toStdString() << " format failed\n";
    return false;
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
        return 1;
    }

    // Load/import project
    Narrate::NarrateProject project;
    if (!loadProject(args.inputFile, project))
    {
        juce::shutdownJuce_GUI();
        return 1;
    }

    // Export project
    if (!exportProject(project, args.outputFile, args.format))
    {
        juce::shutdownJuce_GUI();
        return 1;
    }

    std::cout << "\nConversion successful!\n";

    // Cleanup
    juce::shutdownJuce_GUI();
    return 0;
}
