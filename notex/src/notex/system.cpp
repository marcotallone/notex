/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements notex::system, wrapping ::popen/::pclose and std::getenv.
 */

#include "notex/system.hpp"

#include "notex/errors.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>

namespace notex::system {

std::string run_command(const std::string& command) {
    // Merge stderr into stdout so that a failing command's diagnostics end
    // up in the error message thrown below, instead of being discarded.
    const std::string full_command = command + " 2>&1";

    FILE* pipe = ::popen(full_command.c_str(), "r");
    if (pipe == nullptr) {
        throw NotexError("failed to run command: " + command,
                          ExitCode::FAILURE);
    }

    std::array<char, 256> buffer{};
    std::string output;
    size_t bytes_read = 0;
    while ((bytes_read = std::fread(buffer.data(), 1, buffer.size(), pipe)) >
           0) {
        output.append(buffer.data(), bytes_read);
    }

    const int status = ::pclose(pipe);
    if (status != 0) {
        throw NotexError("command failed (" + command + "): " + output,
                          ExitCode::FAILURE);
    }

    while (!output.empty() &&
           (output.back() == '\n' || output.back() == '\r')) {
        output.pop_back();
    }

    return output;
}

std::optional<std::string> get_env(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) return std::nullopt;
    return std::string(value);
}

}  // namespace notex::system
