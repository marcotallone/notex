/**
 * @file system.hpp
 * @brief Process execution and environment-variable access.
 *
 * Every place in notex_core that needs to shell out to an external tool
 * (kpsewhich, mktexlsr, ...) or read an environment variable goes through
 * this one choke point, so that the rest of the library never touches
 * ::popen or std::getenv directly.
 */

#pragma once

#include <optional>
#include <string>

namespace notex::system {

/**
 * @brief Runs @p command through the shell and returns everything it
 *        wrote to standard output (and standard error, since the two are
 *        merged so that a failing command's diagnostics end up in the
 *        error message below).
 *
 * @param command Shell command line to execute.
 * @return The command's captured output, with any trailing newline
 *         removed.
 * @throws NotexError if the command could not be started, or exited with
 *         a non-zero status. Callers that expect failure to be routine
 *         (for example, a database-refresh tool that may be absent) are
 *         expected to catch this and downgrade it rather than propagate
 *         it.
 */
std::string run_command(const std::string& command);

/**
 * @brief Reads an environment variable.
 * @param name Name of the environment variable to read.
 * @return Its value, or std::nullopt if it is not set.
 */
std::optional<std::string> get_env(const std::string& name);

}  // namespace notex::system
