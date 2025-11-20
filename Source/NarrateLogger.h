#pragma once

#include <juce_core/juce_core.h>
#include <iostream>

/**
 * NarrateLogger
 *
 * Centralized logging system for the Narrate application.
 * Provides different log levels and conditional compilation for debug vs release builds.
 *
 * Usage:
 *   NARRATE_LOG_DEBUG("Parsing entry at line " << lineIndex);
 *   NARRATE_LOG_INFO("Loaded " << clipCount << " clips");
 *   NARRATE_LOG_WARNING("File format not recognized: " << filename);
 *   NARRATE_LOG_ERROR("Failed to parse timecode: " << timeStr);
 */

namespace NarrateLogger
{
    // Log levels
    enum class Level
    {
        Debug,    // Detailed debugging information (only in debug builds)
        Info,     // General informational messages
        Warning,  // Warning messages for potential issues
        Error     // Error messages for failures
    };

    // Check if logging is enabled for a given level
    inline bool isLevelEnabled(Level level)
    {
        #if JUCE_DEBUG
            return true;  // All levels enabled in debug builds
        #else
            // In release builds, only show warnings and errors
            return level >= Level::Warning;
        #endif
    }

    // Format a log message with timestamp and level
    inline juce::String formatMessage(Level level, const juce::String& message)
    {
        juce::String levelStr;
        switch (level)
        {
            case Level::Debug:   levelStr = "[DEBUG]"; break;
            case Level::Info:    levelStr = "[INFO]"; break;
            case Level::Warning: levelStr = "[WARN]"; break;
            case Level::Error:   levelStr = "[ERROR]"; break;
        }

        auto time = juce::Time::getCurrentTime();
        auto timeStr = time.formatted("%H:%M:%S");

        return timeStr + " " + levelStr + " " + message;
    }

    // Core logging function
    inline void log(Level level, const juce::String& message)
    {
        if (!isLevelEnabled(level))
            return;

        auto formattedMsg = formatMessage(level, message);

        // In debug builds, use DBG() which goes to debugger output
        // In release builds, use standard error for warnings/errors
        #if JUCE_DEBUG
            DBG(formattedMsg);
        #else
            if (level >= Level::Warning)
            {
                std::cerr << formattedMsg.toStdString() << std::endl;
            }
        #endif

        // Optionally write to a file logger if one is set
        if (auto* logger = juce::Logger::getCurrentLogger())
        {
            logger->writeToLog(formattedMsg);
        }
    }

    // Convenience functions for each log level
    inline void debug(const juce::String& message)
    {
        log(Level::Debug, message);
    }

    inline void info(const juce::String& message)
    {
        log(Level::Info, message);
    }

    inline void warning(const juce::String& message)
    {
        log(Level::Warning, message);
    }

    inline void error(const juce::String& message)
    {
        log(Level::Error, message);
    }
}

// Convenience macros for logging with stream-like syntax
// These macros build a juce::String using the << operator

#define NARRATE_LOG_DEBUG(message) \
    do { \
        if (NarrateLogger::isLevelEnabled(NarrateLogger::Level::Debug)) { \
            juce::String logMsg; \
            logMsg << message; \
            NarrateLogger::debug(logMsg); \
        } \
    } while (false)

#define NARRATE_LOG_INFO(message) \
    do { \
        if (NarrateLogger::isLevelEnabled(NarrateLogger::Level::Info)) { \
            juce::String logMsg; \
            logMsg << message; \
            NarrateLogger::info(logMsg); \
        } \
    } while (false)

#define NARRATE_LOG_WARNING(message) \
    do { \
        if (NarrateLogger::isLevelEnabled(NarrateLogger::Level::Warning)) { \
            juce::String logMsg; \
            logMsg << message; \
            NarrateLogger::warning(logMsg); \
        } \
    } while (false)

#define NARRATE_LOG_ERROR(message) \
    do { \
        if (NarrateLogger::isLevelEnabled(NarrateLogger::Level::Error)) { \
            juce::String logMsg; \
            logMsg << message; \
            NarrateLogger::error(logMsg); \
        } \
    } while (false)
