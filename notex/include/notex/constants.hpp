/**
 * @file constants.hpp
 * @brief Compile-time constants.
 *
 * This header provides a comprehensive collection of compile-time ANSI escape
 * sequence constants organized in logical namespaces for terminal text styling,
 * colors, cursor movement, and screen operations. All constants are available
 * at compile-time for efficient and zero-cost terminal output formatting.
 */

#pragma once

#include <string_view>

/**
 * @brief Namespace containing ANSI escape codes for terminal formatting.
 *
 * Provides a structured collection of ANSI/VT100 escape sequences for
 * controlling terminal appearance, including text styling, foreground colors,
 * background colors, cursor movement, and screen operations. All constants
 * are `constexpr std::string_view` for compile-time evaluation and zero
 * runtime overhead.
 *
 * @note ANSI escape sequences work on most modern terminals but may not be
 *       supported on all platforms or terminal emulators.
 */
namespace ansi {

/// Reset all text formatting and colors to terminal default.
constexpr std::string_view reset = "\033[0m";

/**
 * @brief Namespace containing text styling escape sequences.
 *
 * Provides ANSI escape codes for text formatting options such as bold, dim,
 * italic, underline, and other text decorations.
 */
namespace text {
/// Make text appear bold/bright.
constexpr std::string_view bold = "\033[1m";
/// Reduce text intensity (dim/faint).
constexpr std::string_view dim = "\033[2m";
/// Display text in italic style.
constexpr std::string_view italic = "\033[3m";
/// Underline text.
constexpr std::string_view underline = "\033[4m";
/// Make text blink (if supported by terminal).
constexpr std::string_view blink = "\033[5m";
/// Reverse video: swap foreground and background colors.
constexpr std::string_view reverse = "\033[7m";
/// Conceal text (useful for passwords).
constexpr std::string_view hidden = "\033[8m";
/// Cross out / strike through text.
constexpr std::string_view strikethrough = "\033[9m";
}  // namespace text

/**
 * @brief Namespace containing text foreground color escape sequences.
 *
 * Provides ANSI escape codes for standard and bright/intense text colors.
 * Standard colors are available in the root namespace, bright variants
 * are prefixed with `bright_`.
 */
namespace color {

/// Text foreground color: black.
constexpr std::string_view black = "\033[30m";
/// Text foreground color: red.
constexpr std::string_view red = "\033[31m";
/// Text foreground color: green.
constexpr std::string_view green = "\033[32m";
/// Text foreground color: yellow.
constexpr std::string_view yellow = "\033[33m";
/// Text foreground color: blue.
constexpr std::string_view blue = "\033[34m";
/// Text foreground color: magenta.
constexpr std::string_view magenta = "\033[35m";
/// Text foreground color: cyan.
constexpr std::string_view cyan = "\033[36m";
/// Text foreground color: white.
constexpr std::string_view white = "\033[37m";

/// Text foreground color: bright black (gray).
constexpr std::string_view bright_black = "\033[90m";
/// Text foreground color: bright red.
constexpr std::string_view bright_red = "\033[91m";
/// Text foreground color: bright green.
constexpr std::string_view bright_green = "\033[92m";
/// Text foreground color: bright yellow.
constexpr std::string_view bright_yellow = "\033[93m";
/// Text foreground color: bright blue.
constexpr std::string_view bright_blue = "\033[94m";
/// Text foreground color: bright magenta.
constexpr std::string_view bright_magenta = "\033[95m";
/// Text foreground color: bright cyan.
constexpr std::string_view bright_cyan = "\033[96m";
/// Text foreground color: bright white.
constexpr std::string_view bright_white = "\033[97m";
}  // namespace color

/**
 * @brief Namespace containing background color escape sequences.
 *
 * Provides ANSI escape codes for filling the background of text with colors.
 * Standard background colors are available in the root namespace, bright
 * variants are prefixed with `bright_`.
 */
namespace bg {

/// Background color: black.
constexpr std::string_view black = "\033[40m";
/// Background color: red.
constexpr std::string_view red = "\033[41m";
/// Background color: green.
constexpr std::string_view green = "\033[42m";
/// Background color: yellow.
constexpr std::string_view yellow = "\033[43m";
/// Background color: blue.
constexpr std::string_view blue = "\033[44m";
/// Background color: magenta.
constexpr std::string_view magenta = "\033[45m";
/// Background color: cyan.
constexpr std::string_view cyan = "\033[46m";
/// Background color: white.
constexpr std::string_view white = "\033[47m";

/// Background color: bright black.
constexpr std::string_view bright_black = "\033[100m";
/// Background color: bright red.
constexpr std::string_view bright_red = "\033[101m";
/// Background color: bright green.
constexpr std::string_view bright_green = "\033[102m";
/// Background color: bright yellow.
constexpr std::string_view bright_yellow = "\033[103m";
/// Background color: bright blue.
constexpr std::string_view bright_blue = "\033[104m";
/// Background color: bright magenta.
constexpr std::string_view bright_magenta = "\033[105m";
/// Background color: bright cyan.
constexpr std::string_view bright_cyan = "\033[106m";
/// Background color: bright white.
constexpr std::string_view bright_white = "\033[107m";
}  // namespace bg

/**
 * @brief Namespace containing cursor movement escape sequences.
 *
 * Provides ANSI escape codes for controlling cursor position and movement
 * within the terminal. Note that not all terminals support all cursor
 * operations.
 */
namespace cursor {
/// Move cursor to home position (top-left corner).
constexpr std::string_view home = "\033[H";
/// Move cursor up one line.
constexpr std::string_view up = "\033[A";
/// Move cursor down one line.
constexpr std::string_view down = "\033[B";
/// Move cursor right one column.
constexpr std::string_view right = "\033[C";
/// Move cursor left one column.
constexpr std::string_view left = "\033[D";
/// Save current cursor position.
constexpr std::string_view save = "\033[s";
/// Restore previously saved cursor position.
constexpr std::string_view restore = "\033[u";
}  // namespace cursor

/**
 * @brief Namespace containing screen clearing escape sequences.
 *
 * Provides ANSI escape codes for clearing portions of the terminal screen
 * or lines.
 */
namespace clear {
/// Clear entire screen and move cursor to home position.
constexpr std::string_view screen = "\033[2J";
/// Clear from cursor to end of current line.
constexpr std::string_view line = "\033[K";
/// Clear from cursor to end of screen.
constexpr std::string_view to_end = "\033[0J";
}  // namespace clear

}  // namespace ansi
