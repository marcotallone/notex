/**
 * @file orchestrator.hpp
 * @brief Declares Orchestrator, the application layer that turns
 *        command-line input into calls on the notex_core domain classes.
 *
 * This header is deliberately kept outside include/notex/: it is not part
 * of notex_core's public surface, and only the notex executable includes
 * it. It is the only header, besides its own translation unit, that
 * depends on the command-line parsing library.
 */

#pragma once

#include <CLI/CLI.hpp>

#include <string>

#include "notex/errors.hpp"

namespace notex {

/**
 * @brief Owns the command-line parser and coordinates the domain objects
 *        that carry out each command.
 *
 * The Orchestrator registers every planned subcommand up front, even
 * before it is implemented, so that the command surface described in the
 * design document is always visible through `notex --help`; a command
 * without an implementation yet simply reports that it is not available.
 * It holds the state shared across a single invocation, such as the
 * global `--yes` and `--verbose` flags, and is the only class in the
 * program that knows about command-line parsing.
 */
class Orchestrator {
public:
    Orchestrator();
    Orchestrator(Orchestrator&&) = delete;
    Orchestrator(const Orchestrator&) = delete;
    Orchestrator& operator=(Orchestrator&&) = delete;
    Orchestrator& operator=(const Orchestrator&) = delete;
    ~Orchestrator() = default;

    /**
     * @brief Parses the command line and dispatches to the requested
     *        command, catching every exception at this single point.
     * @param argc Argument count, as received by main.
     * @param argv Argument vector, as received by main.
     * @return The exit code the process should return.
     */
    ExitCode run(int argc, char** argv);

private:
    /// Registers the global flags and every planned subcommand.
    void register_commands();

    /// Implements `notex version`: prints the version string, identical
    /// to what the global `--version` flag prints.
    void run_version() const;

    /// Implements `notex help`: prints the full help text, identical to
    /// what the global `--help`/`-h` flag prints.
    void run_help() const;

    /// Implements the `info` command: prints the software's build
    /// metadata.
    void run_info() const;

    /// Implements the `clean` command: removes build artefacts from
    /// clean_path_, honouring clean_dry_run_.
    void run_clean() const;

    /// Implements `notex install` (no path): installs globally.
    void run_install_global() const;

    /// Implements `notex install <path>`: installs locally into
    /// install_path_.
    void run_install_local() const;

    /// Implements `notex init`: creates a new project, resolving
    /// init_type_arg_/init_path_arg_'s type-vs-path ambiguity first.
    void run_init() const;

    /// Implements `notex theme <name>`: switches the current project's
    /// theme to theme_name_.
    void run_theme() const;

    /// Implements `notex add section <title>`.
    void run_add_section() const;

    /// Implements `notex remove section <number>`.
    void run_remove_section() const;

    /// Implements `notex add bib`.
    void run_add_bib() const;

    /// Implements `notex remove bib`.
    void run_remove_bib() const;

    /// Implements `notex get [name]`: prints a snippet, lists every
    /// name when none is given, and suggests alternatives for an
    /// unknown one.
    void run_get() const;

    /// Implements `notex checkhealth`: runs environment and project
    /// checks, throwing (non-zero exit) only on a genuine failure.
    void run_checkhealth() const;

    /// Implements `notex set <key> <value>`.
    void run_set() const;

    /// Implements `notex ls [path]`.
    void run_ls() const;

    /// Implements `notex uninstall`/`notex prune` (no path): uninstalls
    /// globally.
    void run_uninstall_global() const;

    /// Implements `notex uninstall <path>`/`notex prune <path>`:
    /// uninstalls locally from uninstall_path_.
    void run_uninstall_local() const;

    /// Implements `notex reset`.
    void run_reset() const;

    /// Implements `notex delete`.
    void run_delete() const;

    CLI::App app_{std::string("notex - ") + PROJECT_DESCRIPTION};
    bool assume_yes_ = false;  ///< Backing storage for the `--yes` flag.
    bool verbose_ = false;     ///< Backing storage for the `--verbose` flag.
    std::string clean_path_ = ".";  ///< Backing storage for `clean`'s path.
    bool clean_dry_run_ = false;    ///< Backing storage for `--dry-run`.
    std::string install_path_;      ///< Backing storage for `install`'s
                                     ///< optional local path.
    bool install_force_ = false;    ///< Backing storage for `install
                                     ///< --force`.
    std::string init_type_arg_;     ///< Backing storage for `init`'s first
                                     ///< positional ('mono'/'multi' or, if
                                     ///< the second is empty, the path).
    std::string init_path_arg_;     ///< Backing storage for `init`'s
                                     ///< second positional (the path, when
                                     ///< the first was a type).
    bool init_force_ = false;       ///< Backing storage for `init --force`.
    std::string theme_name_;        ///< Backing storage for `theme`'s name
                                     ///< argument.
    std::string section_title_;     ///< Backing storage for `add
                                     ///< section`'s title argument.
    int section_number_ = 0;        ///< Backing storage for `remove
                                     ///< section`'s number argument.
    std::string get_name_;          ///< Backing storage for `get`'s
                                     ///< optional name argument.
    std::string set_key_;           ///< Backing storage for `set`'s key
                                     ///< argument.
    std::string set_value_;         ///< Backing storage for `set`'s
                                     ///< value argument.
    std::string ls_path_ = ".";     ///< Backing storage for `ls`'s
                                     ///< optional path argument.
    std::string uninstall_path_;    ///< Backing storage for
                                     ///< `uninstall`/`prune`'s optional
                                     ///< local path.
    bool uninstall_force_ = false;  ///< Backing storage for `uninstall
                                     ///< --force`.
    bool reset_force_ = false;      ///< Backing storage for `reset
                                     ///< --force`.
    bool delete_all_ = false;       ///< Backing storage for `delete
                                     ///< --all`.
    bool delete_force_ = false;     ///< Backing storage for `delete
                                     ///< --force`.
};

}  // namespace notex
