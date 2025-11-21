#pragma once

#include <juce_core/juce_core.h>
#include <vector>

/**
 * OperationResult
 *
 * Rich result object returned by all long-running operations.
 * Provides detailed feedback including success/failure status, statistics,
 * warnings, and errors for user feedback.
 *
 * Used by ImportFeature, ExportFeature, AudioPlaybackFeature, and other
 * operations that need to provide detailed feedback to users.
 */

namespace Narrate
{
    /**
     * Severity level for operation messages (warnings/errors)
     */
    enum class MessageSeverity
    {
        Info,       // Informational message
        Warning,    // Non-critical issue, operation continued
        Error       // Critical error that may have caused failure
    };

    /**
     * Individual message from an operation (warning/error/info)
     */
    struct OperationMessage
    {
        MessageSeverity severity;
        juce::String message;
        juce::String context;  // Optional context (e.g., "Line 42", "File: foo.srt")

        OperationMessage(MessageSeverity sev, const juce::String& msg, const juce::String& ctx = juce::String())
            : severity(sev), message(msg), context(ctx)
        {
        }

        // Helper constructors
        static OperationMessage info(const juce::String& msg, const juce::String& ctx = juce::String())
        {
            return OperationMessage(MessageSeverity::Info, msg, ctx);
        }

        static OperationMessage warning(const juce::String& msg, const juce::String& ctx = juce::String())
        {
            return OperationMessage(MessageSeverity::Warning, msg, ctx);
        }

        static OperationMessage error(const juce::String& msg, const juce::String& ctx = juce::String())
        {
            return OperationMessage(MessageSeverity::Error, msg, ctx);
        }
    };

    /**
     * Result of a long-running operation
     * Contains all information needed for user feedback
     */
    struct OperationResult
    {
        // Core status
        bool success = false;
        juce::String operationName;     // E.g., "Import SRT", "Export Project", "Load Audio"
        juce::String operationDetail;   // E.g., filename, target format

        // Statistics
        int itemsProcessed = 0;         // E.g., clips imported, files exported
        int itemsSuccessful = 0;        // Successfully processed items
        int itemsSkipped = 0;           // Skipped items (due to errors)
        double timeElapsedSeconds = 0.0;

        // Additional metrics (operation-specific)
        juce::StringPairArray metadata;  // Key-value pairs for custom data

        // Messages
        std::vector<OperationMessage> messages;

        // Constructors
        OperationResult() = default;

        OperationResult(bool succeeded, const juce::String& opName)
            : success(succeeded), operationName(opName)
        {
        }

        // Helper methods
        void addInfo(const juce::String& msg, const juce::String& context = juce::String())
        {
            messages.push_back(OperationMessage::info(msg, context));
        }

        void addWarning(const juce::String& msg, const juce::String& context = juce::String())
        {
            messages.push_back(OperationMessage::warning(msg, context));
        }

        void addError(const juce::String& msg, const juce::String& context = juce::String())
        {
            messages.push_back(OperationMessage::error(msg, context));
        }

        // Get counts by severity
        int getWarningCount() const
        {
            return static_cast<int>(std::count_if(messages.begin(), messages.end(),
                [](const OperationMessage& msg) { return msg.severity == MessageSeverity::Warning; }));
        }

        int getErrorCount() const
        {
            return static_cast<int>(std::count_if(messages.begin(), messages.end(),
                [](const OperationMessage& msg) { return msg.severity == MessageSeverity::Error; }));
        }

        int getInfoCount() const
        {
            return static_cast<int>(std::count_if(messages.begin(), messages.end(),
                [](const OperationMessage& msg) { return msg.severity == MessageSeverity::Info; }));
        }

        // Get messages by severity
        std::vector<OperationMessage> getWarnings() const
        {
            std::vector<OperationMessage> warnings;
            for (const auto& msg : messages)
                if (msg.severity == MessageSeverity::Warning)
                    warnings.push_back(msg);
            return warnings;
        }

        std::vector<OperationMessage> getErrors() const
        {
            std::vector<OperationMessage> errors;
            for (const auto& msg : messages)
                if (msg.severity == MessageSeverity::Error)
                    errors.push_back(msg);
            return errors;
        }

        std::vector<OperationMessage> getInfos() const
        {
            std::vector<OperationMessage> infos;
            for (const auto& msg : messages)
                if (msg.severity == MessageSeverity::Info)
                    infos.push_back(msg);
            return infos;
        }

        // Helper to create a simple success result
        static OperationResult createSuccess(const juce::String& opName, int itemCount = 0)
        {
            OperationResult result(true, opName);
            result.itemsProcessed = itemCount;
            result.itemsSuccessful = itemCount;
            return result;
        }

        // Helper to create a simple failure result
        static OperationResult createFailure(const juce::String& opName, const juce::String& errorMsg)
        {
            OperationResult result(false, opName);
            result.addError(errorMsg);
            return result;
        }

        // Get a human-readable summary
        juce::String getSummary() const
        {
            juce::String summary;

            if (success)
            {
                summary << operationName << " completed successfully";
                if (itemsProcessed > 0)
                    summary << " (" << itemsProcessed << " items)";
            }
            else
            {
                summary << operationName << " failed";
            }

            int warningCount = getWarningCount();
            int errorCount = getErrorCount();

            if (warningCount > 0 || errorCount > 0)
            {
                summary << " - ";
                if (errorCount > 0)
                    summary << errorCount << " error" << (errorCount > 1 ? "s" : "");
                if (warningCount > 0 && errorCount > 0)
                    summary << ", ";
                if (warningCount > 0)
                    summary << warningCount << " warning" << (warningCount > 1 ? "s" : "");
            }

            return summary;
        }
    };

} // namespace Narrate
