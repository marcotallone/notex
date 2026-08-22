/**
 * @file manager.hpp
 * @brief Manager: represents and operates on a single NoTeX project.
 *
 * Manager implements finding a project's root directory, loading and
 * saving its metadata, creating a new project, cleaning build artefacts,
 * and mutating an existing project (switching its theme, adding and
 * removing sections, adding and removing a bibliography). Every mutation
 * that edits an existing `.tex` file delegates that work to a Document
 * rather than touching text directly.
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "notex/templates.hpp"

namespace notex {

/**
 * @brief Persisted metadata for a single NoTeX project, stored as JSON in
 *        `.notex/notex.json`.
 *
 * Fields that are not yet populated by anything are left as empty
 * strings rather than std::optional, so that the schema itself does not
 * have to change shape once something starts writing to them. The list
 * of sections is deliberately not part of this schema: it is always
 * derived by scanning the filesystem and the project's `\subfile` lines,
 * so that sections added or removed by hand are always respected.
 */
struct ProjectConfig {
    int schema_version = 1;         ///< Version of this on-disk schema.
    std::string notex_version;      ///< notex version that created the
                                     ///< project.
    std::string project_type;       ///< "mono" or "multi"; set by `init`.
    std::string main_file = "main.tex";  ///< Project's entry `.tex` file.
    std::string installation_type;  ///< "local" or "global"; set by
                                     ///< `install`.
    std::string theme;              ///< Set by the `theme` command.
    std::string bibliography_file;  ///< Set once a bibliography exists.
};

/**
 * @brief Outcome of a Manager::clean() call.
 *
 * Lists every file and directory that was removed or, under a dry run,
 * that would have been, so that the command layer can report exactly
 * what happened rather than just a count.
 */
struct CleanReport {
    std::vector<std::filesystem::path> removed_files;
    std::vector<std::filesystem::path> removed_directories;

    /// @return The total number of files and directories removed (or, in
    ///         a dry run, that would have been).
    std::size_t total_removed() const noexcept {
        return removed_files.size() + removed_directories.size();
    }
};

/**
 * @brief Represents a single NoTeX project: its location and metadata.
 *
 * A Manager locates its project the way `git` locates a repository: it
 * starts from a given directory and walks upward until it finds the
 * hidden `.notex/` directory that marks the project root, then loads the
 * project's metadata from `.notex/notex.json`.
 */
class Manager {
public:
    /**
     * @brief Locates and loads the project that contains @p start_dir.
     * @param start_dir Directory to start the upward search from;
     *                   defaults to the current working directory.
     * @throws ProjectNotFoundError if no `.notex/` directory is found
     *         between @p start_dir and the filesystem root.
     * @throws ConfigError if `.notex/notex.json` exists but cannot be
     *         parsed.
     */
    explicit Manager(
        std::filesystem::path start_dir = std::filesystem::current_path());

    /**
     * @brief Searches upward from @p start_dir for a `.notex/` directory,
     *        without requiring one to exist.
     * @param start_dir Directory to start the search from.
     * @return The project root directory, or std::nullopt if none is
     *         found before reaching the filesystem root.
     */
    static std::optional<std::filesystem::path> find_project_root(
        const std::filesystem::path& start_dir);

    /// @return The project's root directory (the one containing
    ///         `.notex/`).
    const std::filesystem::path& root_dir() const noexcept {
        return root_dir_;
    }

    /// @return The project's current metadata.
    const ProjectConfig& config() const noexcept { return config_; }

    /// @return The project's current metadata, mutable so that later
    ///         phases can update it before calling save().
    ProjectConfig& config() noexcept { return config_; }

    /**
     * @brief Persists the current metadata back to `.notex/notex.json`.
     * @throws ConfigError if the file cannot be written.
     */
    void save() const;

    /**
     * @brief Creates a new NoTeX project inside @p target_dir.
     *
     * Writes a fresh `main.tex` — a single file for
     * templates::ProjectType::MONO, or a `main.tex` plus a `sections/`
     * directory holding one introductory section for
     * templates::ProjectType::MULTI — and records the project's metadata
     * in a newly created `.notex/notex.json`. The generated `main.tex`
     * refers to `\documentclass{settings/notex}` when @p target_dir
     * already holds a local installation, and to plain `notex` otherwise,
     * warning when no installation at all can be found.
     *
     * @param target_dir Directory to create the project in; created if
     *                    it does not already exist.
     * @param project_type Whether to scaffold a single-file or a
     *                      multi-file project.
     * @param force Overwrite an existing `main.tex` or `.notex/`
     *              directory instead of refusing to.
     * @throws FilesystemError if @p target_dir already looks like a
     *         project and @p force is not set, or if a file cannot be
     *         written.
     * @return A Manager for the newly created project.
     */
    static Manager init(const std::filesystem::path& target_dir,
                         templates::ProjectType project_type =
                             templates::ProjectType::MULTI,
                         bool force = false);

    /**
     * @brief Writes @p config as `<root_dir>/.notex/notex.json`, creating
     *        `<root_dir>/.notex/` first if it does not already exist.
     *
     * Unlike the constructor, this does not require @p root_dir to
     * already be a NoTeX project: it is the mechanical write operation
     * that both save() and a fresh install use, the latter to bootstrap
     * `.notex/` for a directory that is not a project yet.
     *
     * @param root_dir Directory whose `.notex/` should hold the config.
     * @param config Metadata to write.
     * @throws ConfigError if the file cannot be written.
     */
    static void write_config(const std::filesystem::path& root_dir,
                              const ProjectConfig& config);

    /**
     * @brief Recursively removes known LaTeX build artefacts from
     *        @p start_dir.
     *
     * Operates on an arbitrary path and needs no project, which is why
     * this is a static operation rather than an instance method. Files
     * are matched by filename suffix rather than by
     * std::filesystem::path::extension(), because several artefacts,
     * such as `.synctex.gz` and `-blx.bib`, are not true extensions;
     * directories generated by the `minted` package are matched by the
     * `_minted-` prefix that package always uses, regardless of the
     * project's file names. A @p start_dir that does not exist, or is
     * not a directory, yields an empty report rather than an error, so
     * that the command works the same way whether or not it happens to
     * be run inside a project.
     *
     * @param start_dir Directory to clean, recursively.
     * @param dry_run When true, nothing is deleted; the report describes
     *                what would have been removed.
     * @return A report of every file and directory removed (or that
     *         would have been, under a dry run).
     */
    static CleanReport clean(const std::filesystem::path& start_dir,
                              bool dry_run = false);

    /// @return The theme names available, derived from the embedded
    ///         `notex-theme-*.tex` files (e.g. "bw", "dark", "light",
    ///         "tokyo").
    static std::vector<std::string> available_themes();

    /**
     * @brief Switches the project's theme.
     *
     * Rewrites the `\documentclass` options of the project's main file
     * to select @p theme instead of whichever other theme was there, and
     * records it in the project's metadata.
     *
     * @param theme Theme name; must be one of available_themes().
     * @throws UsageError if @p theme is not a known theme.
     * @throws DocumentError if the main file's `\documentclass` line is
     *         missing, ambiguous, or malformed.
     */
    void set_theme(std::string_view theme);

    /**
     * @brief Adds a new section titled @p title.
     *
     * In a single-file project, inserts a `\section{}` heading before
     * the unique `\end{document}`. In a multi-file project, determines
     * the next section number by scanning `sections/` (never tracked in
     * the metadata, so sections added or removed by hand are always
     * respected), writes a new numbered subfile from
     * templates::section(), and inserts the corresponding `\subfile`
     * line after the last one already in the main file.
     *
     * @param title Section title.
     * @throws DocumentError if the expected anchor (`\end{document}` for
     *         a single-file project, the last `\subfile{sections/...}`
     *         line for a multi-file one) is missing or ambiguous.
     */
    void add_section(std::string_view title);

    /**
     * @brief Removes the section numbered @p number.
     *
     * Only supported in a multi-file project, since a single-file
     * project has no single section body that could be removed
     * unambiguously. Asks for confirmation before deleting anything.
     *
     * @param number Section number, as scanned from `sections/`.
     * @throws UsageError if this is a single-file project, or if no
     *         section numbered @p number exists.
     * @throws DocumentError if more than one `\subfile` line references
     *         the section being removed.
     * @return True if the section was removed; false if the user
     *         declined the confirmation prompt.
     */
    bool remove_section(int number);

    /**
     * @brief Adds a bibliography.
     *
     * Creates `bibliography_file` (defaulting to "bibliography.bib")
     * from templates::bibliography_starter() if it does not already
     * exist, then inserts the bibliography commands appropriate to the
     * document: `\addbibresource`/`\printbibliography` if a `biblatex`
     * `\usepackage` line is detected in the main file, or the classic
     * `\bibliographystyle`/`\bibliography` pair otherwise. Idempotent:
     * a command already present is left untouched.
     *
     * @throws DocumentError if the main file has no unique
     *         `\documentclass` or `\end{document}` line to anchor the
     *         insertion on.
     */
    void add_bibliography();

    /**
     * @brief Removes the bibliography commands added by
     *        add_bibliography() from the main file.
     *
     * Always strips the commands themselves, which is non-destructive
     * to any content; separately asks for confirmation before deleting
     * the `.bib` file itself, since that step is not reversible.
     */
    void remove_bibliography();

    /// @return Every file this project is considered to genuinely own:
    ///         the main file, every file under `sections/` (for a
    ///         multi-file project), and the bibliography file — each
    ///         only if it actually exists on disk. Used by `ls`.
    std::vector<std::filesystem::path> project_files() const;

    /// @return The filename stems (see templates::section_stem()) of
    ///         every section file present under `sections/` that is not
    ///         referenced by any `\subfile` line in the main file.
    ///         Always empty for a single-file project, or if the main
    ///         file does not exist. Used by `checkhealth`.
    std::vector<std::string> orphan_sections() const;

    /**
     * @brief Regenerates the main file from the template, after backing
     *        up the previous one to `<main_file>.bak`.
     *
     * A multi-file project's regenerated main file references every
     * section currently present under `sections/`, in number order, not
     * just the first — sections and the bibliography file themselves
     * are left untouched. Asks for confirmation first.
     *
     * @param force Skip the confirmation prompt.
     * @return True if the reset was performed; false if the user
     *         declined the confirmation prompt.
     */
    bool reset(bool force = false);

    /**
     * @brief Removes NoTeX's own scaffolding from the project.
     *
     * Removes `.notex/`, `settings/`, `fonts/`, and any build artefacts
     * (via clean()), but preserves every user-authored file — the main
     * file, sections, the bibliography, and anything else — unless
     * @p remove_all is set, in which case the entire project directory
     * is removed. Asks for confirmation first.
     *
     * @param remove_all Also remove every user-authored file (the whole
     *                    project directory), not just NoTeX's own.
     * @param force Skip the confirmation prompt.
     * @return True if the deletion was performed; false if the user
     *         declined the confirmation prompt.
     */
    bool delete_scaffolding(bool remove_all = false, bool force = false);

    /**
     * @brief Sets a single whitelisted metadata key directly.
     *
     * A low-level escape hatch for the fields the higher-level commands
     * (theme, add/remove section/bib) don't cover the raw editing of;
     * unlike set_theme(), this does not touch the main file's content
     * at all, only the recorded metadata.
     *
     * @param key One of "main_file", "theme", "bibliography_file".
     * @param value New value for @p key.
     * @throws UsageError if @p key is not one of the whitelisted keys.
     */
    void set_config_value(std::string_view key, std::string_view value);

private:
    static ProjectConfig load_config(
        const std::filesystem::path& config_file);

    std::filesystem::path root_dir_;
    ProjectConfig config_;
};

}  // namespace notex
