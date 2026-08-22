/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements Orchestrator: command-line parsing and dispatch to the
 * notex_core domain classes.
 */

#include "orchestrator.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "notex/environment.hpp"
#include "notex/installer.hpp"
#include "notex/logging.hpp"
#include "notex/manager.hpp"
#include "notex/output.hpp"
#include "notex/reference.hpp"
#include "notex/system.hpp"
#include "notex/templates.hpp"

namespace notex {

Orchestrator::Orchestrator() { register_commands(); }

void Orchestrator::run_version() const { ui::info(PROJECT_VERSION); }

void Orchestrator::run_help() const {
    // app_.help() itself won't do here: by the time this callback runs,
    // "help" has already been recorded as app_'s selected subcommand, and
    // App::help() delegates to a selected subcommand's own (much
    // shorter) help text rather than the top-level one --help shows.
    // Calling the formatter directly, exactly as App::exit()'s
    // CallForHelp branch does, bypasses that delegation.
    std::cout << app_.get_formatter()->make_help(
        &app_, app_.get_name(), CLI::AppFormatMode::Normal);
}

void Orchestrator::run_info() const {
    // OLD:
    // ui::step("NoTeX");
    // ui::info(std::string("Version:     ") + PROJECT_VERSION);
    // ui::info(std::string("Author:      ") + PROJECT_AUTHOR);
    // ui::info(std::string("License:     ") + PROJECT_LICENSE);
    // ui::info(std::string("Homepage:    ") + PROJECT_HOMEPAGE_URL);
    // ui::info(std::string("Description: ") + PROJECT_DESCRIPTION);

    // NEW:
    ui::info(
        std::string("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
                    "━━━━━━━━━━━━━━━━━━━┓"));
    ui::info(
        std::string("┃                                                         "
                    "                   ┃"));
    ui::info(
        std::string("┃                         ░███                 ░███       "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░███    ░██     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░████   ░██     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░██░██  ░██     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                      ░███      ░██ ░██ ░██      ░██     "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░██  ░██░██     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░██   ░████     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                        ░██     ░██    ░███     ░██      "
                    "                   ┃"));
    ui::info(
        std::string("┃                         ░███                 ░███       "
                    "                   ┃"));
    ui::info(
        std::string("┃                                                         "
                    "                   ┃"));
    ui::info(
        std::string("┃                               ~ N O T E X ~             "
                    "                   ┃"));
    ui::info(std::string("┃                     ") + PROJECT_DESCRIPTION +
             std::string("                     ┃"));
    ui::info(std::string("┃                               Version ") +
             PROJECT_VERSION +
             std::string("                                ┃"));
    ui::info(
        std::string("┃                                                         "
                    "                   ┃"));
    ui::info(std::string("┃                      Copyright (c) 2026 ") +
             PROJECT_AUTHOR + std::string("                      ┃"));
    ui::info(std::string("┃                                ") +
             PROJECT_LICENSE +
             std::string(" License                                 ┃"));
    ui::info(
        std::string("┃                                                         "
                    "                   ┃"));
    ui::info(std::string("┃                   ") + PROJECT_HOMEPAGE_URL +
             std::string("                    ┃"));
    ui::info(
        std::string("┃                                                         "
                    "                   ┃"));
    ui::info(
        std::string("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
                    "━━━━━━━━━━━━━━━━━━━┛"));

    ui::step("Environment");
    try {
        const Environment environment;
        ui::info(std::string("  TEXMFHOME:         ") +
                 environment.texmf_home().string());
        ui::info(std::string("  Global class dir:  ") +
                 environment.global_latex_dir().string());
        ui::info(std::string("  Local class dir:   ") +
                 environment.local_latex_dir().string());
        ui::info(std::string("  Installation:      ") +
                 std::string(to_string(environment.installation_type())));
    } catch (const EnvironmentError& e) {
        PLOG_WARNING << e.what();
        ui::warning(e.what());
    }

    ui::step("Project");
    try {
        const Manager manager;
        const ProjectConfig& config = manager.config();
        ui::info(std::string("  Root:              ") +
                 manager.root_dir().string());
        ui::info(
            std::string("  Project type:      ") +
            (config.project_type.empty() ? "(unknown)" : config.project_type));
        ui::info(std::string("  Main file:         ") + config.main_file);
        ui::info(std::string("  Installation type: ") +
                 (config.installation_type.empty() ? "(unknown)"
                                                   : config.installation_type));
        ui::info(std::string("  Theme:             ") +
                 (config.theme.empty() ? "(none)" : config.theme));
    } catch (const ProjectNotFoundError&) {
        ui::info("  Not inside a NoTeX project (.notex/ directory not found).");
    }
}

void Orchestrator::run_clean() const {
    const std::filesystem::path path(clean_path_);

    if (!Manager::find_project_root(path).has_value()) {
        PLOG_WARNING << "'clean' invoked outside a NoTeX project; cleaning '"
                     << path.string() << "' directly.";
        ui::warning("You're not inside a NoTeX project.");
        ui::info("Cleaning inside '" + path.string() + "' directly.");
    }

    const CleanReport report = Manager::clean(path, clean_dry_run_);

    const std::string verb = clean_dry_run_ ? "Would remove: " : "Removed: ";
    for (const auto& file : report.removed_files) {
        ui::step(verb + file.string());
    }
    for (const auto& directory : report.removed_directories) {
        ui::step(verb + directory.string());
    }

    if (report.total_removed() == 0) {
        PLOG_INFO << "Clean found nothing to remove in '" << path.string()
                  << "'.";
        ui::success("Nothing to clean.");
    } else {
        const std::string count = std::to_string(report.total_removed());
        PLOG_INFO << count << " item(s) "
                  << (clean_dry_run_ ? "would be removed from '"
                                     : "removed from '")
                  << path.string() << "'.";
        ui::success(count + " item(s) " +
                    (clean_dry_run_ ? "would be removed." : "removed."));
    }
}

void Orchestrator::run_install_global() const {
    const Environment environment;
    if (Installer::install_global(environment, install_force_)) {
        PLOG_INFO << "Installed NoTeX globally into '"
                  << environment.global_latex_dir().string() << "'.";
        ui::success("Installed NoTeX globally into '" +
                    environment.global_latex_dir().string() + "'.");
    }
}

void Orchestrator::run_install_local() const {
    const std::filesystem::path target_dir(install_path_);
    if (Installer::install_local(target_dir, install_force_)) {
        PLOG_INFO << "Installed NoTeX locally into '"
                  << (target_dir / "settings").string() << "'.";
        ui::success(
            "Installed NoTeX locally into '" +
            (target_dir / "settings").string() +
            "'. Documents should refer to \\documentclass{settings/notex}.");
    }
}

void Orchestrator::run_init() const {
    // `notex init .` and `notex init multi .` are equivalent (DESIGN.md,
    // "initialization"): the first positional is only a type when it
    // parses as one and a second positional was also given; a lone
    // positional is always the path, defaulting the type to multi-file.
    templates::ProjectType project_type = templates::ProjectType::MULTI;
    std::string path = ".";

    if (!init_path_arg_.empty()) {
        const auto parsed = templates::project_type_from_string(init_type_arg_);
        if (!parsed.has_value()) {
            throw UsageError("'" + init_type_arg_ +
                             "' is not a valid project type; expected "
                             "'mono' or 'multi'.");
        }
        project_type = *parsed;
        path = init_path_arg_;
    } else if (!init_type_arg_.empty()) {
        if (const auto parsed =
                templates::project_type_from_string(init_type_arg_);
            parsed.has_value()) {
            project_type = *parsed;
        } else {
            path = init_type_arg_;
        }
    }

    const Manager manager =
        Manager::init(std::filesystem::path(path), project_type, init_force_);
    PLOG_INFO << "Initialized a " << templates::to_string(project_type)
              << "-file NoTeX project in '" << manager.root_dir().string()
              << "'.";
    ui::success("Initialized a " +
               std::string(templates::to_string(project_type)) +
               "-file NoTeX project in '" + manager.root_dir().string() +
               "'.");
}

void Orchestrator::run_theme() const {
    Manager manager;
    manager.set_theme(theme_name_);
    PLOG_INFO << "Theme set to '" << theme_name_ << "' for project at '"
              << manager.root_dir().string() << "'.";
    ui::success("Theme set to '" + theme_name_ + "'.");
}

void Orchestrator::run_add_section() const {
    Manager manager;
    manager.add_section(section_title_);
    PLOG_INFO << "Added section '" << section_title_ << "' to project at '"
              << manager.root_dir().string() << "'.";
    ui::success("Added section '" + section_title_ + "'.");
}

void Orchestrator::run_remove_section() const {
    Manager manager;
    if (manager.remove_section(section_number_)) {
        PLOG_INFO << "Removed section " << section_number_
                  << " from project at '" << manager.root_dir().string()
                  << "'.";
        ui::success("Removed section " + std::to_string(section_number_) +
                    ".");
    }
}

void Orchestrator::run_add_bib() const {
    Manager manager;
    manager.add_bibliography();
    PLOG_INFO << "Bibliography ready: '" << manager.config().bibliography_file
              << "'.";
    ui::success("Bibliography ready: '" + manager.config().bibliography_file +
               "'.");
}

void Orchestrator::run_remove_bib() const {
    Manager manager;
    manager.remove_bibliography();
    PLOG_INFO << "Removed the bibliography from project at '"
              << manager.root_dir().string() << "'.";
    ui::success("Bibliography removed.");
}

void Orchestrator::run_get() const {
    const auto print_names = [] {
        for (const std::string_view name : reference::snippet_names()) {
            ui::info("  " + std::string(name));
        }
    };

    if (get_name_.empty()) {
        ui::step("Available snippets");
        print_names();
        return;
    }

    const std::vector<reference::Snippet> snippets =
        reference::find_snippets(get_name_);
    if (snippets.empty()) {
        PLOG_WARNING << "'" << get_name_ << "' is not a known snippet name.";
        ui::warning("'" + get_name_ + "' is not a known snippet name.");
        std::vector<std::string_view> suggestions;
        for (const std::string_view name : reference::snippet_names()) {
            if (name.find(get_name_) != std::string_view::npos) {
                suggestions.push_back(name);
            }
        }
        ui::step(suggestions.empty() ? "Available snippets" : "Did you mean");
        if (suggestions.empty()) {
            print_names();
        } else {
            for (const std::string_view name : suggestions) {
                ui::info("  " + std::string(name));
            }
        }
        return;
    }

    for (const reference::Snippet& snippet : snippets) {
        if (snippets.size() > 1) { ui::step(std::string(snippet.name)); }
        std::cout << snippet.content;
    }
}

void Orchestrator::run_checkhealth() const {
    bool healthy = true;
    const auto check = [&healthy](bool ok, const std::string& message) {
        if (ok) {
            PLOG_INFO << message;
            ui::success(message);
        } else {
            PLOG_ERROR << message;
            ui::error(message);
            healthy = false;
        }
    };

    ui::step("Environment");
    try {
        system::run_command("kpsewhich --version");
        check(true, "kpsewhich is available.");
    } catch (const NotexError&) {
        check(false, "kpsewhich is not available on PATH.");
    }

    try {
        const Environment environment;
        check(true, "TEXMFHOME resolved: " + environment.texmf_home().string());

        const InstallationType type = environment.installation_type();
        check(type != InstallationType::NONE,
             std::string("Installation detected: ") +
                 std::string(to_string(type)));

        if (type == InstallationType::LOCAL &&
            Installer::installation_differs(environment.local_latex_dir(),
                                            environment.local_fonts_dir())) {
            PLOG_WARNING << "The local installation differs from the "
                            "embedded template (customised install?).";
            ui::warning("The local installation differs from the embedded "
                        "template (customised install?).");
        } else if (type == InstallationType::GLOBAL &&
                  Installer::installation_differs(
                      environment.global_latex_dir(),
                      environment.global_fonts_dir())) {
            PLOG_WARNING << "The global installation differs from the "
                            "embedded template (customised install?).";
            ui::warning("The global installation differs from the embedded "
                        "template (customised install?).");
        }
    } catch (const EnvironmentError& e) { check(false, e.what()); }

    ui::step("Project");
    try {
        const Manager manager;
        check(true, "Project metadata parses.");

        const std::filesystem::path main_path =
            manager.root_dir() / manager.config().main_file;
        check(std::filesystem::exists(main_path),
             "Main file exists: " + manager.config().main_file);

        if (!manager.config().theme.empty()) {
            const std::vector<std::string> themes = Manager::available_themes();
            check(std::find(themes.begin(), themes.end(),
                            manager.config().theme) != themes.end(),
                 "Theme is valid: " + manager.config().theme);
        }

        const std::vector<std::string> orphans = manager.orphan_sections();
        if (orphans.empty()) {
            PLOG_INFO << "Every section file is referenced from the main "
                         "file.";
            ui::success("Every section file is referenced from the main "
                        "file.");
        } else {
            for (const std::string& stem : orphans) {
                PLOG_WARNING << "Section file '" << stem
                             << "' is not referenced by any \\subfile line.";
                ui::warning("Section file '" + stem +
                            "' is not referenced by any \\subfile line.");
            }
        }
    } catch (const ProjectNotFoundError&) {
        ui::info("Not inside a NoTeX project; skipping project checks.");
    } catch (const ConfigError& e) { check(false, e.what()); }

    if (!healthy) {
        throw NotexError("checkhealth found one or more problems.",
                         ExitCode::FAILURE);
    }
    PLOG_INFO << "checkhealth: everything looks healthy.";
    ui::success("Everything looks healthy.");
}

void Orchestrator::run_set() const {
    Manager manager;
    manager.set_config_value(set_key_, set_value_);
    PLOG_INFO << "Set '" << set_key_ << "' to '" << set_value_
              << "' for project at '" << manager.root_dir().string() << "'.";
    ui::success("Set '" + set_key_ + "' to '" + set_value_ + "'.");
}

void Orchestrator::run_ls() const {
    const Manager manager{std::filesystem::path(ls_path_)};
    const std::vector<std::filesystem::path> files = manager.project_files();

    if (files.empty()) {
        ui::info("No project files found.");
        return;
    }

    const std::filesystem::path main_path =
        manager.root_dir() / manager.config().main_file;
    for (const std::filesystem::path& file : files) {
        if (file == main_path) {
            ui::success(file.string() + " (main)");
        } else if (file.extension() == ".bib") {
            ui::info(file.string() + " (bibliography)");
        } else {
            ui::step(file.string());
        }
    }
}

void Orchestrator::run_uninstall_global() const {
    const Environment environment;
    if (Installer::uninstall_global(environment, uninstall_force_)) {
        PLOG_INFO << "Uninstalled the global NoTeX template.";
        ui::success("Uninstalled the global NoTeX template.");
    }
}

void Orchestrator::run_uninstall_local() const {
    const std::filesystem::path target_dir(uninstall_path_);
    if (Installer::uninstall_local(target_dir, uninstall_force_)) {
        PLOG_INFO << "Uninstalled the local NoTeX template from '"
                  << target_dir.string() << "'.";
        ui::success("Uninstalled the local NoTeX template from '" +
                    target_dir.string() + "'.");
    }
}

void Orchestrator::run_reset() const {
    Manager manager;
    const std::string main_file = manager.config().main_file;
    if (manager.reset(reset_force_)) {
        PLOG_INFO << "Regenerated '" << main_file
                  << "' from the template for project at '"
                  << manager.root_dir().string() << "'.";
        ui::success("Regenerated '" + main_file + "' from the template.");
    }
}

void Orchestrator::run_delete() const {
    Manager manager;
    const std::filesystem::path root = manager.root_dir();
    if (manager.delete_scaffolding(delete_all_, delete_force_)) {
        PLOG_INFO << (delete_all_ ? "Deleted the project at '"
                                  : "Removed NoTeX's scaffolding from '")
                  << root.string() << "'.";
        ui::success((delete_all_ ? "Deleted the project at '"
                                : "Removed NoTeX's scaffolding from '") +
                    root.string() + "'.");
    }
}

void Orchestrator::register_commands() {
    // Without this, a token that matches another top-level subcommand's
    // name (e.g. "info") is parsed as chaining into that sibling
    // subcommand rather than as consumed by the current one's own
    // positional argument — so e.g. `notex get info` would silently run
    // both `get` (with no name) and `info`, rather than passing "info"
    // as get's snippet name. Capping at one subcommand per invocation
    // keeps a token that matches a subcommand name from matching as a
    // second, chained subcommand once the first has already claimed its
    // positionals (see CLI11's docs on require_subcommand: "limiting
    // the maximum number allows you to keep arguments that match a
    // previous subcommand name from matching").
    app_.require_subcommand(0, 1);

    app_.set_version_flag("--version", std::string(PROJECT_VERSION));
    // Propagates to notex::ui immediately as the flag is parsed, via
    // CLI11's per-token ->each() callback, rather than through App's own
    // deferred callback() mechanism: a subcommand's callback can run
    // ui::confirm() while argv is still being parsed, so waiting for a
    // callback that fires only once parsing finishes would be too late.
    app_.add_flag("-y,--yes", assume_yes_,
                  "Assume 'yes' to every confirmation prompt")
        ->each([](const std::string&) { ui::set_assume_yes(true); });
    app_.add_flag("-v,--verbose", verbose_, "Enable verbose output");

    // Subcommand equivalents of the --version/--help flags above, for
    // users who reach for a subcommand out of habit; both print exactly
    // what their flag counterpart does.
    app_.add_subcommand("version", "Show version information and exit")
        ->callback([this] { run_version(); });
    app_.add_subcommand("help", "Show this help message and exit")
        ->callback([this] { run_help(); });

    app_.add_subcommand("info",
                        "Show information about the project, the "
                        "installation, and the template")
        ->callback([this] { run_info(); });

    auto* clean_command = app_.add_subcommand(
        "clean", "Remove build artefacts recursively from a directory");
    clean_command->add_option("path", clean_path_,
                              "Directory to clean (default: current "
                              "directory)");
    clean_command->add_flag("--dry-run", clean_dry_run_,
                            "Report what would be removed without "
                            "deleting anything");
    clean_command->callback([this] { run_clean(); });

    auto* install_command = app_.add_subcommand(
        "install", "Install the NoTeX template globally, or locally when "
                    "a path is given");
    auto* install_path_option = install_command->add_option(
        "path", install_path_,
        "Install locally into this directory instead of globally");
    install_command->add_flag(
        "--force", install_force_,
        "Overwrite a differing existing installation without asking");
    install_command->callback([this, install_path_option] {
        if (install_path_option->count() > 0) {
            run_install_local();
        } else {
            run_install_global();
        }
    });

    auto* init_command = app_.add_subcommand(
        "init", "Initialize a new NoTeX project (defaults to multi-file "
                 "in the current directory)");
    init_command->add_option(
        "type", init_type_arg_,
        "'mono' or 'multi' (default: multi); may be omitted, in which "
        "case a lone positional argument is treated as the path instead");
    init_command->add_option(
        "path", init_path_arg_,
        "Directory to initialize the project in (default: current "
        "directory)");
    init_command->add_flag(
        "--force", init_force_,
        "Overwrite an existing project instead of refusing to");
    init_command->callback([this] { run_init(); });

    auto* theme_command =
        app_.add_subcommand("theme", "Switch the project's theme");
    theme_command
        ->add_option("name", theme_name_,
                    "Theme to switch to, e.g. 'light', 'dark', 'tokyo', "
                    "or 'bw'")
        ->required();
    theme_command->callback([this] { run_theme(); });

    app_.add_subcommand("checkhealth",
                       "Check the installation and project state")
        ->callback([this] { run_checkhealth(); });

    auto* set_command = app_.add_subcommand(
        "set", "Change a project configuration value");
    set_command
        ->add_option("key", set_key_,
                    "Configuration key to change: 'main_file', 'theme', "
                    "or 'bibliography_file'")
        ->required();
    set_command->add_option("value", set_value_, "New value")->required();
    set_command->callback([this] { run_set(); });

    auto* ls_command = app_.add_subcommand(
        "ls", "List the files that belong to a project");
    ls_command->add_option("path", ls_path_,
                           "Directory to list the project files of "
                           "(default: current directory)");
    ls_command->callback([this] { run_ls(); });

    auto* get_command = app_.add_subcommand(
        "get", "Print a ready-to-paste template snippet");
    get_command->add_option("name", get_name_,
                            "Snippet name (omit to list every available "
                            "name)");
    get_command->callback([this] { run_get(); });

    // `prune` is a plain alias for `uninstall`: both subcommands share
    // the same backing storage and dispatch logic, since only one of
    // them is ever actually invoked in a given run.
    for (const char* name : {"uninstall", "prune"}) {
        auto* command = app_.add_subcommand(
            name, "Remove the NoTeX template installation, globally or "
                   "(given a path) locally");
        auto* path_option = command->add_option(
            "path", uninstall_path_,
            "Uninstall from this directory instead of globally");
        command->add_flag("--force", uninstall_force_,
                          "Skip the confirmation prompt");
        command->callback([this, path_option] {
            if (path_option->count() > 0) {
                run_uninstall_local();
            } else {
                run_uninstall_global();
            }
        });
    }

    auto* reset_command = app_.add_subcommand(
        "reset", "Regenerate the project's main file from the template");
    reset_command->add_flag("--force", reset_force_,
                            "Skip the confirmation prompt");
    reset_command->callback([this] { run_reset(); });

    auto* delete_command = app_.add_subcommand(
        "delete", "Remove NoTeX's own scaffolding from a project");
    delete_command->add_flag(
        "--all", delete_all_,
        "Also remove every user-authored file (the whole project)");
    delete_command->add_flag("--force", delete_force_,
                             "Skip the confirmation prompt");
    delete_command->callback([this] { run_delete(); });

    auto* add_command = app_.add_subcommand(
        "add", "Add a section or a bibliography to the project");
    auto* add_section_command = add_command->add_subcommand(
        "section", "Add a section to the project");
    add_section_command->add_option("title", section_title_, "Section title")
        ->required();
    add_section_command->callback([this] { run_add_section(); });
    add_command->add_subcommand("bib", "Add a bibliography to the project")
        ->callback([this] { run_add_bib(); });

    auto* remove_command = app_.add_subcommand(
        "remove", "Remove a section or the bibliography from the project");
    auto* remove_section_command = remove_command->add_subcommand(
        "section", "Remove a section from the project");
    remove_section_command
        ->add_option("number", section_number_,
                    "Number of the section to remove, as scanned from "
                    "sections/")
        ->required();
    remove_section_command->callback([this] { run_remove_section(); });
    remove_command
        ->add_subcommand("bib", "Remove the bibliography from the project")
        ->callback([this] { run_remove_bib(); });
}

ExitCode Orchestrator::run(int argc, char** argv) {
    try {
        app_.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        // Covers both real usage errors and CLI11's internal use of
        // ParseError to unwind for --help/--version, so this isn't
        // necessarily a failure; logged at DEBUG rather than ERROR.
        const int code = app_.exit(e);
        PLOG_DEBUG << "CLI parsing exited with code " << code << ": "
                   << e.what();
        return code == 0 ? ExitCode::SUCCESS : ExitCode::USAGE_ERROR;
    } catch (const NotexError& e) {
        PLOG_ERROR << e.what();
        ui::error(e.what());
        return e.exit_code();
    } catch (const std::exception& e) {
        PLOG_ERROR << e.what();
        ui::error(e.what());
        return ExitCode::FAILURE;
    }

    if (app_.get_subcommands().empty()) {
        std::cout << app_.help() << std::flush;
    }

    return ExitCode::SUCCESS;
}

}  // namespace notex
