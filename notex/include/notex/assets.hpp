/**
 * @file assets.hpp
 * @brief Accessor over the LaTeX template and font files embedded into
 *        the binary.
 *
 * The contents of notex/latex/ and notex/fonts/ are turned into C++ source at
 * build time by cmake/generate_embedded_assets.cmake; this header
 * declares the small, stable interface that the rest of the codebase
 * uses to reach them, without needing to know anything about how the
 * embedding works. Only assets.cpp includes the generated headers
 * themselves.
 */

#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace notex::assets {

/// One file embedded into the binary: its original name and its full
/// contents, both borrowed from static storage for the lifetime of the
/// program.
struct EmbeddedFile {
    std::string_view name;     ///< File name, relative to its source
                                ///< directory (e.g. "notex.cls").
    std::string_view content;  ///< Full contents of the file.
};

/**
 * @brief One binary file embedded into the binary as hexadecimal text.
 *
 * Used only as the intermediate representation the generated font header
 * produces: font files are binary, so cmake/generate_embedded_assets.cmake
 * reads them as a hex-encoded byte stream (binary-safe, unlike the raw
 * string literals used for text assets) rather than as an EmbeddedFile
 * directly. font_files() decodes these into ordinary EmbeddedFile values
 * the first time it is called.
 */
struct EmbeddedHexFile {
    std::string_view name;         ///< File name (e.g. "NotoSans.ttf").
    std::string_view hex_content;  ///< Contents, encoded as lowercase hex.
};

/// @return Every LaTeX template file embedded from notex/latex/ at build
///         time.
const std::vector<EmbeddedFile>& latex_files();

/**
 * @brief Looks up a single embedded LaTeX template file by name.
 * @param name File name to look for, e.g. "notex-theme-dark.tex".
 * @return The matching file, or std::nullopt if no embedded file has that
 *         name.
 */
std::optional<EmbeddedFile> find_latex_file(std::string_view name);

/// @return Every static font file embedded from notex/fonts/ at build
///         time, decoded back into their raw bytes.
const std::vector<EmbeddedFile>& font_files();

/**
 * @brief Looks up a single embedded font file by name.
 * @param name File name to look for, e.g. "JetBrainsMono-Regular.ttf".
 * @return The matching file, or std::nullopt if no embedded font has
 *         that name.
 */
std::optional<EmbeddedFile> find_font_file(std::string_view name);

}  // namespace notex::assets
