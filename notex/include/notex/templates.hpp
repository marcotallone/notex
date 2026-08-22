/**
 * @file templates.hpp
 * @brief templates: encoded skeletons used to scaffold new NoTeX projects.
 *
 * Mirrors the structure of the reference projects under examples/, but
 * generalised: no repository-specific paths or content, and always
 * beginning from a single introductory section. Kept in step by hand with
 * examples/mono/ and examples/multi/ whenever either changes.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace notex::templates {

/// Which project skeleton to scaffold: a single main.tex, or a main.tex
/// plus a `sections/` tree of subfiles.
enum class ProjectType {
    MONO,   ///< A single main.tex file.
    MULTI,  ///< main.tex plus a sections/ directory of subfiles.
};

/// @return "mono" or "multi".
std::string_view to_string(ProjectType type);

/**
 * @brief Parses a project type name.
 * @param name Candidate name, e.g. from a command-line argument.
 * @return The matching ProjectType, or std::nullopt if @p name is
 *         neither "mono" nor "multi".
 */
std::optional<ProjectType> project_type_from_string(std::string_view name);

/**
 * @brief Generates the entry file for a single-file project.
 * @param class_path Argument to `\documentclass`, e.g. "notex" or
 *                    "settings/notex".
 */
std::string mono_main(std::string_view class_path);

/**
 * @brief Generates the entry file for a multi-file project, referencing
 *        @p section_stems in order.
 * @param class_path See mono_main().
 * @param section_stems Filename stems (as returned by section_stem(),
 *                       no directory or extension) of the sections to
 *                       reference via `\subfile`, in order; defaults to
 *                       a single introductory section, matching a fresh
 *                       project's initial state.
 */
std::string multi_main(
    std::string_view class_path,
    const std::vector<std::string>& section_stems = {"1_introduction"});

/**
 * @brief Computes the filename stem (no directory, no extension) for a
 *        numbered section.
 * @param number Section number, prefixed onto the returned stem.
 * @param title Section title, slugified (lower-cased, non-alphanumeric
 *              runs collapsed to a single underscore) to form the rest
 *              of the stem.
 * @return e.g. section_stem(1, "Introduction") returns "1_introduction".
 */
std::string section_stem(int number, std::string_view title);

/**
 * @brief Generates a numbered section subfile.
 * @param number Section number; substituted only into a landmark comment,
 *               since the number itself lives in the filename rather than
 *               the file's content.
 * @param title Section title, substituted into the `\section{}` heading.
 */
std::string section(int number, std::string_view title);

/// @return A starter bibliography file with one example entry.
std::string bibliography_starter();

}  // namespace notex::templates
