/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements the notex::ui styled terminal output and the confirmation
 * prompt declared in notex/output.hpp.
 */

#include "notex/output.hpp"

#include <cstdio>
#include <iostream>
#include <string>

#include "notex/constants.hpp"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

namespace notex::ui {

namespace {

bool g_assume_yes = false;  // NOLINT: deliberate translation-unit-local state
std::optional<bool> g_interactive_override;  // NOLINT: same as above

/// @return Whether standard input is attached to an interactive terminal.
bool stdin_is_interactive() {
    if (g_interactive_override.has_value()) return *g_interactive_override;
#if defined(__unix__) || defined(__APPLE__)
    return ::isatty(fileno(stdin)) != 0;
#else
    return false;
#endif
}

}  // namespace

void set_assume_yes(bool assume_yes) { g_assume_yes = assume_yes; }

bool assume_yes() { return g_assume_yes; }

void set_stdin_interactive_override_for_testing(
    std::optional<bool> is_interactive) {
    g_interactive_override = is_interactive;
}

void success(std::string_view message) {
    std::cout << ansi::color::green << "✓ " << ansi::reset << message << "\n";
}

void warning(std::string_view message) {
    std::cout << ansi::color::yellow << "! " << ansi::reset << message << "\n";
}

void error(std::string_view message) {
    std::cerr << ansi::color::red << "✗ " << ansi::reset << message << "\n";
}

void step(std::string_view message) {
    std::cout << ansi::color::cyan << "➤ " << ansi::reset << message << "\n";
}

void info(std::string_view message) { std::cout << message << "\n"; }

bool confirm(std::string_view prompt, bool default_answer) {
    if (g_assume_yes) return true;
    if (!stdin_is_interactive()) return default_answer;

    std::cout << prompt << " [" << (default_answer ? "Y/n" : "y/N") << "] ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer.empty()) return default_answer;
    return answer.front() == 'y' || answer.front() == 'Y';
}

}  // namespace notex::ui
