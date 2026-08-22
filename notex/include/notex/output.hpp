/**
 * @file output.hpp
 * @brief Styled terminal output and interactive confirmation prompts.
 *
 * The `ui` namespace is the only part of the codebase, besides constants.hpp
 * itself, that touches the ANSI escape sequences in constants.hpp: every
 * other component reports outcomes through these helpers instead of writing
 * to std::cout or std::cerr directly, so that the styling stays consistent
 * everywhere.
 */

#pragma once

#include <optional>
#include <string_view>

namespace notex::ui {

/**
 * @brief Sets whether confirm() should answer "yes" without prompting.
 *
 * Mirrors the global `--yes` command-line flag: once set, every subsequent
 * call to confirm() succeeds immediately, which is what allows destructive
 * commands to be scripted non-interactively.
 *
 * @param assume_yes Whether prompts should be assumed answered "yes".
 */
void set_assume_yes(bool assume_yes);

/// @return Whether confirm() is currently set to assume "yes".
bool assume_yes();

/// Prints a success message, prefixed with a green checkmark, to stdout.
void success(std::string_view message);

/// Prints a warning message, prefixed with a yellow exclamation mark, to
/// stdout.
void warning(std::string_view message);

/// Prints an error message, prefixed with a red cross, to stderr.
void error(std::string_view message);

/// Prints a step or progress message, prefixed with an arrow, to stdout.
void step(std::string_view message);

/// Prints a plain informational message to stdout.
void info(std::string_view message);

/**
 * @brief Asks the user to confirm an action.
 *
 * When the global "assume yes" flag is set, the confirmation succeeds
 * immediately. Otherwise, if standard input is not attached to an
 * interactive terminal, the prompt is skipped and @p default_answer is
 * returned, so that a command run non-interactively (for example from a
 * script or from the test suite) never blocks waiting on input that will
 * never arrive.
 *
 * @param prompt Question to show the user, without a trailing "?".
 * @param default_answer Answer used when the prompt cannot be shown, and
 *                        the answer suggested to the user otherwise.
 * @return True if the user confirmed, or if @p default_answer applied.
 */
bool confirm(std::string_view prompt, bool default_answer = false);

/**
 * @brief Testing seam: overrides whether confirm() treats standard input
 *        as an interactive terminal.
 *
 * confirm()'s non-interactive fallback is normally driven by the real
 * ::isatty() state of the process, which makes it depend on how the test
 * binary itself happens to be invoked (piped in CI, but still attached to
 * a real terminal when a developer runs `ctest` directly) — the latter
 * would otherwise block forever waiting on input that never arrives. This
 * override lets the non-interactive path be exercised deterministically,
 * regardless of the real terminal state. Not meant to be called outside
 * of tests.
 *
 * @param is_interactive Value to force; std::nullopt restores the real
 *                        ::isatty() check.
 */
void set_stdin_interactive_override_for_testing(
    std::optional<bool> is_interactive);

}  // namespace notex::ui
