/**
 * @file logging.hpp
 * @brief Logging utility with plog initialization.
 *
 * Provides simple logging initialization to a file in logs/ directory with
 * rotating file appender. Uses a custom single-line formatter that produces
 * records in the form:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [file.cpp:function:line] message
 *
 * Usage in main():
 *   init_logging("registration");  // Creates logs/registration.log
 *   PLOG_INFO << "Application started";
 *   PLOG_DEBUG << "Debug information";
 *   PLOG_ERROR << "An error occurred";
 */

#pragma once

// plog only records __FILE__ into each Record when PLOG_CAPTURE_FILE is
// defined before <plog/Log.h> is first included in a translation unit;
// otherwise Record::getFile() always returns "" and CustomFormatter's file
// field silently goes blank. Must stay defined ahead of the plog includes
// below, and every translation unit that uses PLOG_* must reach plog only
// through this header so the definition is always seen first.
#define PLOG_CAPTURE_FILE

#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Record.h>
#include <plog/Severity.h>
#include <plog/Util.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "constants.hpp"

namespace fs = std::filesystem;

/**
 * @brief Single-line plog formatter: "[time] [LEVEL] [file:func:line] msg".
 *
 * Replaces plog's default multi-field format with one self-contained,
 * newline-terminated line per record, matching plog::TxtFormatter's
 * trailing-newline convention so records never run together:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [file.cpp:function:line] message
 */
class CustomFormatter {
public:
    /**
     * @brief Header line prepended once when a log file is created.
     *
     * @return Empty string; no header is emitted.
     */
    static plog::util::nstring header() { return plog::util::nstring(); }

    /**
     * @brief Format a single log record as one newline-terminated line.
     *
     * @param record Log record to format.
     *
     * @return Formatted "[time] [LEVEL] [file:func:line] message\n" string.
     */
    static plog::util::nstring format(const plog::Record& record) {
        std::tm t{};
        plog::util::localtime_s(&t, &record.getTime().time);

        plog::util::nostringstream ss;
        ss << "[" << (t.tm_year + 1900) << "-" << std::setfill('0')
           << std::setw(2) << (t.tm_mon + 1) << "-" << std::setfill('0')
           << std::setw(2) << t.tm_mday << " " << std::setfill('0')
           << std::setw(2) << t.tm_hour << ":" << std::setfill('0')
           << std::setw(2) << t.tm_min << ":" << std::setfill('0')
           << std::setw(2) << t.tm_sec << "." << std::setfill('0')
           << std::setw(3) << static_cast<int>(record.getTime().millitm) << "]";

        ss << " [" << plog::severityToString(record.getSeverity()) << "]";

        ss << " [" << fs::path(record.getFile()).filename().string() << ":"
           << record.getFunc() << ":" << record.getLine() << "]";

        ss << " " << record.getMessage() << "\n";

        return ss.str();
    }
};

/**
 * @brief Initialize logging to file in logs/ directory.
 *
 * Creates logs/ directory if it doesn't exist and initializes plog with a
 * rolling file appender using CustomFormatter. Log file is named
 * <app_name>.log.
 *
 * @param app_name Base name for log file (without .log extension).
 *                 Example: "registration" → logs/registration.log
 * @param severity Minimum log level to record (default: plog::debug).
 *                 Levels: plog::verbose, plog::debug, plog::info,
 *                 plog::warning, plog::error, plog::fatal
 *
 * @throws std::runtime_error if directory creation or file initialization
 * fails.
 *
 * @details After calling this function, use plog logging macros:
 *   PLOG_VERBOSE << "Very detailed message";
 *   PLOG_DEBUG << "Debug info";
 *   PLOG_INFO << "Info message";
 *   PLOG_WARNING << "Warning";
 *   PLOG_ERROR << "Error";
 *   PLOG_FATAL << "Fatal error";
 *
 * Each log record is written on its own line, formatted by CustomFormatter as:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [FILE:FUNC:LINE] message
 */
inline void init_logging(const std::string& app_name,
                         plog::Severity severity = plog::debug) {
    try {
        // Create logs directory if it doesn't exist
        fs::path logs_dir = fs::current_path() / "logs";
        if (!fs::exists(logs_dir)) {
            if (!fs::create_directories(logs_dir)) {
                throw std::runtime_error("Failed to create logs directory: " +
                                         logs_dir.string());
            }
        }

        // Construct log file path
        fs::path log_file = logs_dir / (app_name + ".log");

        // Initialize plog with rolling file appender and CustomFormatter
        // Parameters: severity, appender, formatter, max_size, max_files
        static plog::RollingFileAppender<CustomFormatter> file_appender(
            log_file.string().c_str(), 5000000, 3);  // 5MB per file, 3 files

        plog::init(severity, &file_appender);

#ifdef DEBUG
        std::cout << ansi::text::dim << "Logging initialized to: " << log_file
                  << ansi::reset << std::endl;
#endif
    } catch (const std::exception& e) {
        std::cerr << ansi::color::red
                  << "Logging initialization error: " << e.what() << ansi::reset
                  << std::endl;
        throw;
    }
}