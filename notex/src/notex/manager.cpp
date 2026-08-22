/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements the read side of Manager: project discovery and `notex.json`
 * loading/saving.
 */

#include "notex/manager.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <system_error>

#include "notex/assets.hpp"
#include "notex/document.hpp"
#include "notex/environment.hpp"
#include "notex/errors.hpp"
#include "notex/logging.hpp"
#include "notex/output.hpp"

namespace notex {

namespace {

std::filesystem::path config_file_for(const std::filesystem::path& root_dir) {
    return root_dir / ".notex" / "notex.json";
}

void write_text_file(const std::filesystem::path& path,
                     const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    if (!stream) {
        throw FilesystemError("could not write '" + path.string() + "'");
    }
    stream << content;
}

// Decides the \documentclass argument a freshly scaffolded main.tex
// should use. The local check is done directly against the filesystem,
// rather than through Environment, because Environment's constructor
// always tries to resolve TEXMFHOME (even just to check a local
// installation) and throws if it can't; a machine with no TeX
// installation at all should still be able to scaffold a project that
// will later use a local install.
std::string resolve_class_path(const std::filesystem::path& target_dir) {
    if (std::filesystem::exists(target_dir / "settings" / "notex.cls")) {
        return "settings/notex";
    }

    try {
        const Environment environment(target_dir);
        if (environment.installation_type() == InstallationType::NONE) {
            PLOG_WARNING << "No NoTeX installation found for '"
                         << target_dir.string()
                         << "'; scaffolding will reference "
                            "\\documentclass{notex}.";
            ui::warning(
                "No NoTeX installation found; the generated project will "
                "reference \\documentclass{notex}. Run 'notex install' "
                "before compiling.");
        }
    } catch (const EnvironmentError& e) {
        PLOG_WARNING << "Could not resolve the TeX environment for '"
                     << target_dir.string() << "': " << e.what();
        ui::warning(
            "Could not resolve the TeX environment; the generated project "
            "will reference \\documentclass{notex}. Run 'notex install' "
            "before compiling.");
    }
    return "notex";
}

// Filename suffixes of known LaTeX build artefacts. Matched with
// std::string::ends_with rather than std::filesystem::path::extension(),
// because several of these, such as ".synctex.gz" and "-blx.bib", are not
// true extensions and extension() would miss them entirely.
constexpr std::array<std::string_view, 20> kCleanFileSuffixes = {
    ".aux",
    ".log",
    ".out",
    ".toc",
    ".lof",
    ".lot",
    ".fls",
    ".blg",
    ".bbl",
    ".bcf",
    ".fdb_latexmk",
    ".synctex.gz",
    ".listing",
    "-blx.bib",
    ".run.xml",
    ".nav",
    ".snm",
    ".vrb",
    ".synctex.gz(busy)",
    ".minted",
};

// Name prefixes of known LaTeX build artefact directories. The `minted`
// package always names its cache directories `_minted-<jobname>`,
// regardless of what the project's own files are called.
constexpr std::array<std::string_view, 1> kCleanDirectoryPrefixes = {
    "_minted-",
};

bool is_clean_target_file(std::string_view filename) {
    for (const std::string_view suffix : kCleanFileSuffixes) {
        if (filename.ends_with(suffix)) return true;
    }
    return false;
}

bool is_clean_target_directory(std::string_view name) {
    for (const std::string_view prefix : kCleanDirectoryPrefixes) {
        if (name.starts_with(prefix)) return true;
    }
    return false;
}

std::string_view trim(std::string_view text) {
    const std::size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string_view::npos) return {};
    const std::size_t end = text.find_last_not_of(" \t");
    return text.substr(begin, end - begin + 1);
}

bool is_documentclass_line(const std::string& line) {
    return trim(line).starts_with("\\documentclass");
}

bool is_end_document_line(const std::string& line) {
    return trim(line) == "\\end{document}";
}

std::string subfile_line_for(std::string_view stem) {
    return "\\subfile{sections/" + std::string(stem) + "}";
}

bool is_subfile_line_for(const std::string& line, std::string_view stem) {
    return trim(line) == subfile_line_for(stem);
}

bool is_any_subfile_line(const std::string& line) {
    return trim(line).starts_with("\\subfile{sections/");
}

bool is_biblatex_usepackage_line(const std::string& line) {
    const std::string_view trimmed = trim(line);
    return trimmed.starts_with("\\usepackage") &&
          trimmed.find("{biblatex}") != std::string_view::npos;
}

bool is_addbibresource_line(const std::string& line) {
    return trim(line).starts_with("\\addbibresource");
}

bool is_printbibliography_line(const std::string& line) {
    return trim(line).starts_with("\\printbibliography");
}

bool is_bibliographystyle_line(const std::string& line) {
    return trim(line).starts_with("\\bibliographystyle");
}

bool is_bibliography_command_line(const std::string& line) {
    return trim(line).starts_with("\\bibliography{");
}

// Inserts @p text right after the main file's unique \documentclass line
// (for preamble commands) or right before its unique \end{document} line
// (for body commands), unless a line already matching @p already_present
// exists, in which case nothing changes: the bibliography commands must
// stay idempotent to re-running `add bib`.
void ensure_line(Document& doc, const std::string& text,
                 const Document::LinePredicate& already_present,
                 bool after_documentclass) {
    if (!doc.find_all(already_present).empty()) return;

    if (after_documentclass) {
        const std::size_t anchor =
            doc.find_unique(is_documentclass_line, "a unique \\documentclass line");
        doc.insert_line(anchor + 1, text);
    } else {
        const std::size_t anchor =
            doc.find_unique(is_end_document_line, "a unique \\end{document} line");
        doc.insert_line(anchor, text);
    }
}

// Removes every line matching @p predicate, back to front so that
// earlier indices stay valid as later ones are erased.
void remove_matching_lines(Document& doc,
                           const Document::LinePredicate& predicate) {
    const std::vector<std::size_t> matches = doc.find_all(predicate);
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
        doc.remove_line(*it);
    }
}

// Scans <sections_dir> for files named "<N>_....tex" and returns their
// (number, stem) pairs, sorted by number. Deliberately reads the
// filesystem directly rather than any tracked state, so that sections
// added or removed by hand are always respected (see ProjectConfig's
// docs); every other section helper below is built on this one.
std::vector<std::pair<int, std::string>> scan_sections(
    const std::filesystem::path& sections_dir) {
    std::vector<std::pair<int, std::string>> numbered;
    std::error_code ec;
    if (!std::filesystem::is_directory(sections_dir, ec)) return numbered;

    for (const auto& entry :
        std::filesystem::directory_iterator(sections_dir, ec)) {
        if (ec || !entry.is_regular_file()) continue;
        const std::string stem = entry.path().stem().string();
        const std::size_t underscore = stem.find('_');
        if (underscore == std::string::npos) continue;

        int number = 0;
        const auto result = std::from_chars(
            stem.data(), stem.data() + underscore, number);
        if (result.ec != std::errc()) continue;
        numbered.emplace_back(number, stem);
    }
    std::sort(numbered.begin(), numbered.end());
    return numbered;
}

int highest_section_number(const std::filesystem::path& sections_dir) {
    const auto numbered = scan_sections(sections_dir);
    return numbered.empty() ? 0 : numbered.back().first;
}

std::vector<std::string> section_stems_sorted(
    const std::filesystem::path& sections_dir) {
    std::vector<std::string> stems;
    for (auto& [number, stem] : scan_sections(sections_dir)) {
        stems.push_back(std::move(stem));
    }
    return stems;
}

// Extracts "sections/NAME" from a "\subfile{sections/NAME}" line's NAME
// part; returns std::nullopt if @p line isn't such a line.
std::optional<std::string> subfile_target(const std::string& line) {
    const std::string_view trimmed = trim(line);
    constexpr std::string_view kPrefix = "\\subfile{sections/";
    if (!trimmed.starts_with(kPrefix)) return std::nullopt;
    const std::size_t end = trimmed.find('}', kPrefix.size());
    if (end == std::string_view::npos) return std::nullopt;
    return std::string(trimmed.substr(kPrefix.size(), end - kPrefix.size()));
}

std::optional<std::filesystem::path> find_section_file(
    const std::filesystem::path& sections_dir, int number) {
    const std::string prefix = std::to_string(number) + "_";
    std::error_code ec;
    if (!std::filesystem::is_directory(sections_dir, ec)) return std::nullopt;

    for (const auto& entry :
        std::filesystem::directory_iterator(sections_dir, ec)) {
        if (ec || !entry.is_regular_file()) continue;
        if (entry.path().filename().string().starts_with(prefix)) {
            return entry.path();
        }
    }
    return std::nullopt;
}

}  // namespace

std::optional<std::filesystem::path> Manager::find_project_root(
    const std::filesystem::path& start_dir) {
    std::filesystem::path current =
        std::filesystem::absolute(start_dir).lexically_normal();

    PLOG_DEBUG << "Scanning upward for .notex/ starting at '"
               << current.string() << "'.";
    for (;;) {
        if (std::filesystem::is_directory(current / ".notex")) {
            PLOG_DEBUG << "Found project root at '" << current.string()
                       << "'.";
            return current;
        }
        const std::filesystem::path parent = current.parent_path();
        if (parent == current) {
            PLOG_DEBUG << "No .notex/ directory found above '"
                       << start_dir.string() << "'.";
            return std::nullopt;
        }
        current = parent;
    }
}

ProjectConfig Manager::load_config(const std::filesystem::path& config_file) {
    std::ifstream stream(config_file);
    if (!stream) {
        throw ConfigError("could not open '" + config_file.string() + "'");
    }

    nlohmann::json parsed;
    try {
        stream >> parsed;
    } catch (const nlohmann::json::exception& e) {
        throw ConfigError("malformed '" + config_file.string() +
                          "': " + e.what());
    }

    ProjectConfig config;
    try {
        config.schema_version = parsed.value("schema_version", 1);
        config.notex_version = parsed.value("notex_version", std::string());
        config.project_type = parsed.value("project_type", std::string());
        config.main_file = parsed.value("main_file", std::string("main.tex"));
        config.installation_type =
            parsed.value("installation_type", std::string());
        config.theme = parsed.value("theme", std::string());
        config.bibliography_file =
            parsed.value("bibliography_file", std::string());
    } catch (const nlohmann::json::exception& e) {
        throw ConfigError("malformed '" + config_file.string() +
                          "': " + e.what());
    }

    PLOG_DEBUG << "Loaded project config from '" << config_file.string()
               << "'.";
    return config;
}

Manager::Manager(std::filesystem::path start_dir) {
    const auto root = find_project_root(start_dir);
    if (!root.has_value()) {
        throw ProjectNotFoundError(
            "no NoTeX project found in '" + start_dir.string() +
            "' or any parent directory (missing .notex/); run 'notex "
            "init' first.");
    }

    root_dir_ = *root;
    config_ = load_config(config_file_for(root_dir_));
}

void Manager::save() const { write_config(root_dir_, config_); }

void Manager::write_config(const std::filesystem::path& root_dir,
                            const ProjectConfig& config) {
    nlohmann::json json;
    json["schema_version"] = config.schema_version;
    json["notex_version"] = config.notex_version;
    json["project_type"] = config.project_type;
    json["main_file"] = config.main_file;
    json["installation_type"] = config.installation_type;
    json["theme"] = config.theme;
    json["bibliography_file"] = config.bibliography_file;

    const std::filesystem::path config_file = config_file_for(root_dir);
    std::filesystem::create_directories(config_file.parent_path());
    std::ofstream stream(config_file);
    if (!stream) {
        throw ConfigError("could not write '" + config_file.string() + "'");
    }
    stream << json.dump(2) << '\n';
    PLOG_DEBUG << "Wrote project config to '" << config_file.string()
               << "'.";
}

Manager Manager::init(const std::filesystem::path& target_dir,
                      templates::ProjectType project_type, bool force) {
    const std::filesystem::path main_file = target_dir / "main.tex";
    const bool already_project =
        std::filesystem::is_directory(target_dir / ".notex") ||
        std::filesystem::exists(main_file);

    if (already_project && !force) {
        throw FilesystemError(
            "'" + target_dir.string() +
            "' already looks like a NoTeX project (main.tex or .notex/ "
            "already exists); pass --force to overwrite it.");
    }

    const std::string class_path = resolve_class_path(target_dir);

    if (project_type == templates::ProjectType::MONO) {
        write_text_file(main_file, templates::mono_main(class_path));
    } else {
        write_text_file(main_file, templates::multi_main(class_path));
        const std::string stem = templates::section_stem(1, "Introduction");
        write_text_file(target_dir / "sections" / (stem + ".tex"),
                        templates::section(1, "Introduction"));
    }

    ProjectConfig config;
    config.notex_version = PROJECT_VERSION;
    config.project_type = std::string(templates::to_string(project_type));
    config.main_file = "main.tex";
    write_config(target_dir, config);

    PLOG_DEBUG << "Scaffolded a " << templates::to_string(project_type)
               << "-file project at '" << target_dir.string() << "'.";
    return Manager(target_dir);
}

CleanReport Manager::clean(const std::filesystem::path& start_dir,
                           bool dry_run) {
    CleanReport report;

    std::error_code ec;
    if (!std::filesystem::is_directory(start_dir, ec)) { return report; }

    PLOG_DEBUG << "Scanning '" << start_dir.string()
               << "' for build artefacts" << (dry_run ? " (dry-run)" : "")
               << ".";

    const auto options =
        std::filesystem::directory_options::skip_permission_denied;
    auto it =
        std::filesystem::recursive_directory_iterator(start_dir, options, ec);
    const auto end = std::filesystem::recursive_directory_iterator();

    while (!ec && it != end) {
        const std::filesystem::path path = it->path();
        const std::string filename = path.filename().string();

        if (it->is_directory(ec) && is_clean_target_directory(filename)) {
            PLOG_DEBUG << "Matched artefact directory '" << path.string()
                       << "'.";
            report.removed_directories.push_back(path);
            if (!dry_run) { std::filesystem::remove_all(path, ec); }
            it.disable_recursion_pending();
        } else if (it->is_regular_file(ec) && is_clean_target_file(filename)) {
            PLOG_DEBUG << "Matched artefact file '" << path.string() << "'.";
            report.removed_files.push_back(path);
            if (!dry_run) { std::filesystem::remove(path, ec); }
        }

        it.increment(ec);
    }

    PLOG_DEBUG << "Clean scan of '" << start_dir.string() << "' found "
               << report.total_removed() << " artefact(s).";
    return report;
}

std::vector<std::string> Manager::available_themes() {
    constexpr std::string_view kPrefix = "notex-theme-";
    constexpr std::string_view kSuffix = ".tex";

    std::vector<std::string> themes;
    for (const auto& file : assets::latex_files()) {
        if (file.name.starts_with(kPrefix) && file.name.ends_with(kSuffix)) {
            themes.emplace_back(file.name.substr(
                kPrefix.size(), file.name.size() - kPrefix.size() - kSuffix.size()));
        }
    }
    std::sort(themes.begin(), themes.end());
    return themes;
}

void Manager::set_theme(std::string_view theme) {
    const std::vector<std::string> themes = available_themes();
    if (std::find(themes.begin(), themes.end(), std::string(theme)) ==
        themes.end()) {
        std::string available;
        for (std::size_t i = 0; i < themes.size(); ++i) {
            if (i != 0) available += ", ";
            available += themes[i];
        }
        throw UsageError("'" + std::string(theme) +
                         "' is not a valid theme; available themes: " +
                         available + ".");
    }

    Document doc = Document::load(root_dir_ / config_.main_file);
    doc.set_documentclass_option(theme, themes);
    doc.save();

    config_.theme = std::string(theme);
    save();
}

void Manager::add_section(std::string_view title) {
    Document doc = Document::load(root_dir_ / config_.main_file);

    if (config_.project_type == "mono") {
        const std::size_t anchor = doc.find_unique(
            is_end_document_line, "a unique \\end{document} line");
        doc.insert_lines(
            anchor, {"\\section{" + std::string(title) + "}", "", ""});
        doc.save();
        return;
    }

    const std::filesystem::path sections_dir = root_dir_ / "sections";
    const int number = highest_section_number(sections_dir) + 1;
    const std::string stem = templates::section_stem(number, title);
    write_text_file(sections_dir / (stem + ".tex"),
                    templates::section(number, title));

    const std::optional<std::size_t> last_subfile =
        doc.find_last(is_any_subfile_line);
    if (!last_subfile.has_value()) {
        throw DocumentError(
            "could not find an existing \\subfile{sections/...} line in '" +
            (root_dir_ / config_.main_file).string() +
            "' to insert the new section after; the document may have "
            "been restructured by hand.");
    }
    doc.insert_line(*last_subfile + 1, subfile_line_for(stem));
    doc.save();
}

bool Manager::remove_section(int number) {
    if (config_.project_type != "multi") {
        throw UsageError(
            "removing a section is only supported in multi-file projects.");
    }

    const std::filesystem::path sections_dir = root_dir_ / "sections";
    const std::optional<std::filesystem::path> section_file =
        find_section_file(sections_dir, number);
    if (!section_file.has_value()) {
        throw UsageError("no section numbered " + std::to_string(number) +
                         " was found in '" + sections_dir.string() + "'.");
    }

    const bool confirmed = ui::confirm(
        "Remove section '" + section_file->filename().string() +
            "' and its \\subfile line? This cannot be undone.",
        false);
    if (!confirmed) {
        PLOG_WARNING << "Section removal cancelled by the user for '"
                     << section_file->string() << "'.";
        ui::warning("Section removal cancelled.");
        return false;
    }

    const std::string stem = section_file->stem().string();
    Document doc = Document::load(root_dir_ / config_.main_file);
    const std::vector<std::size_t> matches = doc.find_all(
        [&](const std::string& line) { return is_subfile_line_for(line, stem); });
    if (matches.size() > 1) {
        throw DocumentError(
            "found " + std::to_string(matches.size()) +
            " \\subfile lines referencing '" + stem + "' in '" +
            (root_dir_ / config_.main_file).string() +
            "', expected at most one; please resolve the ambiguity by "
            "hand.");
    }
    if (matches.empty()) {
        PLOG_WARNING << "No \\subfile line referencing '" << stem
                     << "' was found; only the section file will be "
                        "removed.";
        ui::warning("No \\subfile line referencing '" + stem +
                    "' was found; only the section file will be removed.");
    } else {
        doc.remove_line(matches.front());
        doc.save();
    }

    std::filesystem::remove(*section_file);
    return true;
}

void Manager::add_bibliography() {
    const std::string bib_filename = config_.bibliography_file.empty()
                                         ? "bibliography.bib"
                                         : config_.bibliography_file;
    const std::filesystem::path bib_path = root_dir_ / bib_filename;
    if (!std::filesystem::exists(bib_path)) {
        write_text_file(bib_path, templates::bibliography_starter());
    }

    Document doc = Document::load(root_dir_ / config_.main_file);
    const bool uses_biblatex = !doc.find_all(is_biblatex_usepackage_line).empty();
    const std::string bib_stem =
        std::filesystem::path(bib_filename).stem().string();

    if (uses_biblatex) {
        ensure_line(doc, "\\addbibresource{" + bib_filename + "}",
                   is_addbibresource_line, /*after_documentclass=*/true);
        ensure_line(doc, "\\printbibliography", is_printbibliography_line,
                   /*after_documentclass=*/false);
    } else {
        ensure_line(doc, "\\bibliographystyle{unsrturl}",
                   is_bibliographystyle_line, /*after_documentclass=*/false);
        ensure_line(doc, "\\bibliography{" + bib_stem + "}",
                   is_bibliography_command_line, /*after_documentclass=*/false);
    }
    doc.save();

    config_.bibliography_file = bib_filename;
    save();
}

void Manager::remove_bibliography() {
    Document doc = Document::load(root_dir_ / config_.main_file);
    remove_matching_lines(doc, is_addbibresource_line);
    remove_matching_lines(doc, is_printbibliography_line);
    remove_matching_lines(doc, is_bibliographystyle_line);
    remove_matching_lines(doc, is_bibliography_command_line);
    doc.save();

    if (!config_.bibliography_file.empty()) {
        const std::filesystem::path bib_path =
            root_dir_ / config_.bibliography_file;
        if (std::filesystem::exists(bib_path) &&
            ui::confirm("Also delete '" + bib_path.string() + "'?", false)) {
            std::filesystem::remove(bib_path);
        }
    }

    config_.bibliography_file.clear();
    save();
}

std::vector<std::filesystem::path> Manager::project_files() const {
    std::vector<std::filesystem::path> files;

    const std::filesystem::path main_path = root_dir_ / config_.main_file;
    if (std::filesystem::exists(main_path)) { files.push_back(main_path); }

    if (config_.project_type == "multi") {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(
                 root_dir_ / "sections", ec)) {
            std::error_code file_ec;
            if (entry.is_regular_file(file_ec) && !file_ec) {
                files.push_back(entry.path());
            }
        }
    }

    if (!config_.bibliography_file.empty()) {
        const std::filesystem::path bib_path =
            root_dir_ / config_.bibliography_file;
        if (std::filesystem::exists(bib_path)) { files.push_back(bib_path); }
    }

    return files;
}

std::vector<std::string> Manager::orphan_sections() const {
    if (config_.project_type != "multi") return {};

    const std::filesystem::path main_path = root_dir_ / config_.main_file;
    if (!std::filesystem::exists(main_path)) return {};

    const Document doc = Document::load(main_path);
    std::vector<std::string> referenced;
    for (const std::string& line : doc.lines()) {
        if (const auto target = subfile_target(line); target.has_value()) {
            referenced.push_back(*target);
        }
    }

    std::vector<std::string> orphans;
    for (const std::string& stem :
        section_stems_sorted(root_dir_ / "sections")) {
        if (std::find(referenced.begin(), referenced.end(), stem) ==
            referenced.end()) {
            orphans.push_back(stem);
        }
    }
    return orphans;
}

bool Manager::reset(bool force) {
    if (!force &&
        !ui::confirm("Regenerate '" + config_.main_file +
                         "' from the template? The current version will "
                         "be backed up to '" +
                         config_.main_file + ".bak'.",
                     false)) {
        PLOG_WARNING << "Reset cancelled by the user for '"
                     << (root_dir_ / config_.main_file).string() << "'.";
        ui::warning("Reset cancelled.");
        return false;
    }

    const std::filesystem::path main_path = root_dir_ / config_.main_file;
    if (std::filesystem::exists(main_path)) {
        std::filesystem::copy_file(
            main_path, root_dir_ / (config_.main_file + ".bak"),
            std::filesystem::copy_options::overwrite_existing);
    }

    const std::string class_path = resolve_class_path(root_dir_);
    if (config_.project_type == "mono") {
        write_text_file(main_path, templates::mono_main(class_path));
    } else {
        std::vector<std::string> stems =
            section_stems_sorted(root_dir_ / "sections");
        if (stems.empty()) { stems.push_back("1_introduction"); }
        write_text_file(main_path, templates::multi_main(class_path, stems));
    }

    return true;
}

bool Manager::delete_scaffolding(bool remove_all, bool force) {
    const std::string prompt =
        remove_all
            ? "Delete the entire project at '" + root_dir_.string() +
                  "', including every file? This cannot be undone."
            : "Remove NoTeX's scaffolding (.notex/, settings/, fonts/, "
              "build artefacts) from '" +
                  root_dir_.string() +
                  "'? User files (the main file, sections/, the "
                  "bibliography) are kept.";
    if (!force && !ui::confirm(prompt, false)) {
        PLOG_WARNING << "Deletion cancelled by the user for '"
                     << root_dir_.string() << "'.";
        ui::warning("Deletion cancelled.");
        return false;
    }

    if (remove_all) {
        std::filesystem::remove_all(root_dir_);
        return true;
    }

    clean(root_dir_);
    std::error_code ec;
    std::filesystem::remove_all(root_dir_ / ".notex", ec);
    std::filesystem::remove_all(root_dir_ / "settings", ec);
    std::filesystem::remove_all(root_dir_ / "fonts", ec);
    return true;
}

void Manager::set_config_value(std::string_view key, std::string_view value) {
    if (key == "main_file") {
        config_.main_file = std::string(value);
    } else if (key == "theme") {
        config_.theme = std::string(value);
    } else if (key == "bibliography_file") {
        config_.bibliography_file = std::string(value);
    } else {
        throw UsageError("'" + std::string(key) +
                         "' is not a settable key; expected one of "
                         "'main_file', 'theme', 'bibliography_file'.");
    }
    save();
}

}  // namespace notex
