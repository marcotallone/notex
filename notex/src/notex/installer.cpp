/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements Installer: writing the embedded template and fonts to a
 * global or local target, and refreshing the TeX filename database.
 */

#include "notex/installer.hpp"

#include "notex/assets.hpp"
#include "notex/errors.hpp"
#include "notex/logging.hpp"
#include "notex/manager.hpp"
#include "notex/output.hpp"
#include "notex/system.hpp"

#include <fstream>
#include <sstream>
#include <system_error>
#include <vector>

namespace notex {

namespace {

void write_file(const std::filesystem::path& path, std::string_view content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    if (!stream) {
        throw FilesystemError("could not write '" + path.string() + "'");
    }
    stream.write(content.data(), static_cast<std::streamsize>(content.size()));
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

/// @return Whether @p dir already holds a different (missing or
///         modified) copy of any of @p files, meaning a fresh write
///         would change something a user might not expect.
bool differs_from_embedded(const std::filesystem::path& dir,
                            const std::vector<assets::EmbeddedFile>& files) {
    if (!std::filesystem::exists(dir)) return false;
    for (const auto& file : files) {
        const std::filesystem::path target = dir / file.name;
        if (!std::filesystem::exists(target)) return true;
        if (read_file(target) != file.content) return true;
    }
    return false;
}

void write_all(const std::filesystem::path& dir,
               const std::vector<assets::EmbeddedFile>& files) {
    std::filesystem::create_directories(dir);
    for (const auto& file : files) {
        write_file(dir / file.name, file.content);
    }
    PLOG_DEBUG << "Wrote " << files.size() << " embedded file(s) into '"
               << dir.string() << "'.";
}

/// @return False (and prints a warning) if the write should be skipped
///         because @p dir already holds a differing copy of @p files and
///         the user declined to overwrite it; true otherwise.
bool confirm_overwrite_if_needed(const std::filesystem::path& dir,
                                  const std::vector<assets::EmbeddedFile>& files,
                                  bool force) {
    if (!differs_from_embedded(dir, files) || force) return true;

    const bool confirmed = ui::confirm(
        "A different NoTeX installation already exists at '" + dir.string() +
            "'. Overwrite it?",
        false);
    if (!confirmed) {
        PLOG_WARNING << "Installation cancelled by the user for '"
                     << dir.string() << "'.";
        ui::warning("Installation cancelled.");
        return false;
    }
    return true;
}

}  // namespace

bool Installer::install_global(const Environment& environment, bool force) {
    if (!confirm_overwrite_if_needed(environment.global_latex_dir(),
                                      assets::latex_files(), force) ||
        !confirm_overwrite_if_needed(environment.global_fonts_dir(),
                                      assets::font_files(), force)) {
        return false;
    }

    write_all(environment.global_latex_dir(), assets::latex_files());
    write_all(environment.global_fonts_dir(), assets::font_files());

    try {
        system::run_command("mktexlsr \"" + environment.texmf_home().string() +
                             "\"");
    } catch (const NotexError& e) {
        PLOG_WARNING << "Could not refresh the TeX filename database after "
                        "a global install: "
                     << e.what();
        ui::warning(std::string("could not refresh the TeX filename "
                                 "database: ") +
                    e.what());
    }

    return true;
}

bool Installer::install_local(const std::filesystem::path& target_dir,
                               bool force) {
    const std::filesystem::path settings_dir = target_dir / "settings";
    const std::filesystem::path fonts_dir = target_dir / "fonts";

    if (!confirm_overwrite_if_needed(settings_dir, assets::latex_files(),
                                      force) ||
        !confirm_overwrite_if_needed(fonts_dir, assets::font_files(),
                                      force)) {
        return false;
    }

    write_all(settings_dir, assets::latex_files());
    write_all(fonts_dir, assets::font_files());

    const bool already_project =
        std::filesystem::is_directory(target_dir / ".notex");
    ProjectConfig config;
    if (already_project) {
        config = Manager(target_dir).config();
    }
    config.installation_type = "local";
    Manager::write_config(target_dir, config);

    return true;
}

bool Installer::uninstall_global(const Environment& environment, bool force) {
    if (!force &&
        !ui::confirm("Remove the NoTeX template from '" +
                         environment.global_latex_dir().string() + "' and '" +
                         environment.global_fonts_dir().string() + "'?",
                     false)) {
        PLOG_WARNING << "Global uninstall cancelled by the user.";
        ui::warning("Uninstall cancelled.");
        return false;
    }

    std::error_code ec;
    std::filesystem::remove_all(environment.global_latex_dir(), ec);
    std::filesystem::remove_all(environment.global_fonts_dir(), ec);

    try {
        system::run_command("mktexlsr \"" + environment.texmf_home().string() +
                             "\"");
    } catch (const NotexError& e) {
        PLOG_WARNING << "Could not refresh the TeX filename database after "
                        "a global uninstall: "
                     << e.what();
        ui::warning(std::string("could not refresh the TeX filename "
                                 "database: ") +
                    e.what());
    }

    return true;
}

bool Installer::uninstall_local(const std::filesystem::path& target_dir,
                                 bool force) {
    if (!force &&
        !ui::confirm("Remove the local NoTeX installation from '" +
                         (target_dir / "settings").string() + "' and '" +
                         (target_dir / "fonts").string() + "'?",
                     false)) {
        PLOG_WARNING << "Local uninstall cancelled by the user for '"
                     << target_dir.string() << "'.";
        ui::warning("Uninstall cancelled.");
        return false;
    }

    std::error_code ec;
    std::filesystem::remove_all(target_dir / "settings", ec);
    std::filesystem::remove_all(target_dir / "fonts", ec);

    if (std::filesystem::is_directory(target_dir / ".notex")) {
        Manager manager(target_dir);
        manager.config().installation_type.clear();
        manager.save();
    }

    return true;
}

bool Installer::installation_differs(const std::filesystem::path& latex_dir,
                                      const std::filesystem::path& fonts_dir) {
    return differs_from_embedded(latex_dir, assets::latex_files()) ||
          differs_from_embedded(fonts_dir, assets::font_files());
}

}  // namespace notex
