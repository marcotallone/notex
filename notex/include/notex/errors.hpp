/**
 * @file errors.hpp
 * @brief Exception hierarchy and process exit codes used throughout NoTeX.
 *
 * Every error that the core library raises derives from NotexError and
 * carries the ExitCode that the executable should return once the error
 * reaches its single top-level handler. The core library never calls
 * std::exit or terminates the process itself; translating an exception
 * into a process exit code is strictly the executable's responsibility.
 */

#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace notex {

/// Process exit codes returned by the `notex` executable.
enum class ExitCode : int {
    SUCCESS = 0,                  ///< The command completed successfully.
    FAILURE = 1,                  ///< An unexpected, unclassified error.
    USAGE_ERROR = 2,              ///< The command line was invalid.
    ENVIRONMENT_ERROR = 3,        ///< The TeX environment could not be
                                  ///< resolved.
    PROJECT_NOT_FOUND_ERROR = 4,  ///< No `.notex/` directory could be found.
    FILESYSTEM_ERROR = 5,         ///< A filesystem operation failed.
    CONFIG_ERROR = 6,             ///< `notex.json` could not be read or
                                  ///< written.
    DOCUMENT_ERROR = 7,           ///< A `.tex` file could not be edited
                                  ///< safely: an expected anchor is
                                  ///< missing or ambiguous.
};

/**
 * @brief Base class for every exception raised by the NoTeX core library.
 *
 * Carries both a human-readable message, inherited from std::runtime_error,
 * and the ExitCode that the exception should map onto once it reaches
 * the Orchestrator's single catch block.
 */
class NotexError : public std::runtime_error {
public:
    /**
     * @brief Constructs an error with its message and exit code.
     * @param message Human-readable description of what went wrong.
     * @param exit_code Exit code the executable should return.
     */
    NotexError(std::string message, ExitCode exit_code)
        : std::runtime_error(std::move(message)), exit_code_(exit_code) {}

    /// @return The exit code associated with this error.
    ExitCode exit_code() const noexcept { return exit_code_; }

private:
    ExitCode exit_code_;  ///< Exit code returned by the executable.
};

/// Raised when the command line supplied by the user is invalid.
class UsageError : public NotexError {
public:
    explicit UsageError(std::string message)
        : NotexError(std::move(message), ExitCode::USAGE_ERROR) {}
};

/// Raised when the surrounding TeX environment cannot be resolved.
class EnvironmentError : public NotexError {
public:
    explicit EnvironmentError(std::string message)
        : NotexError(std::move(message), ExitCode::ENVIRONMENT_ERROR) {}
};

/// Raised when a command that requires a project is run outside one.
class ProjectNotFoundError : public NotexError {
public:
    explicit ProjectNotFoundError(std::string message)
        : NotexError(std::move(message), ExitCode::PROJECT_NOT_FOUND_ERROR) {}
};

/// Raised when a filesystem operation (copy, write, remove) fails.
class FilesystemError : public NotexError {
public:
    explicit FilesystemError(std::string message)
        : NotexError(std::move(message), ExitCode::FILESYSTEM_ERROR) {}
};

/// Raised when `notex.json` is missing, malformed, or fails to write.
class ConfigError : public NotexError {
public:
    explicit ConfigError(std::string message)
        : NotexError(std::move(message), ExitCode::CONFIG_ERROR) {}
};

/// Raised when a `.tex` file cannot be edited safely: an expected anchor
/// (e.g. a unique `\documentclass` or `\end{document}` line) is missing
/// or ambiguous.
class DocumentError : public NotexError {
public:
    explicit DocumentError(std::string message)
        : NotexError(std::move(message), ExitCode::DOCUMENT_ERROR) {}
};

}  // namespace notex
