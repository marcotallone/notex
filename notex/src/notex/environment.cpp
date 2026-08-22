/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements Environment: TEXMFHOME resolution and installation
 * detection.
 */

#include "notex/environment.hpp"

#include "notex/errors.hpp"
#include "notex/logging.hpp"
#include "notex/system.hpp"

namespace notex {

std::string_view to_string(InstallationType type) {
    switch (type) {
        case InstallationType::NONE:
            return "none";
        case InstallationType::LOCAL:
            return "local";
        case InstallationType::GLOBAL:
            return "global";
    }
    return "none";
}

std::filesystem::path Environment::resolve_texmf_home() {
    if (const auto env_value = system::get_env("TEXMFHOME");
        env_value.has_value() && !env_value->empty()) {
        PLOG_DEBUG << "Resolved TEXMFHOME from the environment variable: "
                   << *env_value;
        return std::filesystem::path(*env_value);
    }

    PLOG_DEBUG << "TEXMFHOME is not set; falling back to "
                  "'kpsewhich -var-value=TEXMFHOME'.";
    try {
        const std::string kpsewhich_output =
            system::run_command("kpsewhich -var-value=TEXMFHOME");
        if (!kpsewhich_output.empty()) {
            PLOG_DEBUG << "Resolved TEXMFHOME via kpsewhich: "
                       << kpsewhich_output;
            return std::filesystem::path(kpsewhich_output);
        }
    } catch (const NotexError& e) {
        // kpsewhich is absent or failed; fall through to the error below,
        // which reports both resolution paths having failed.
        PLOG_DEBUG << "kpsewhich fallback failed: " << e.what();
    }

    PLOG_WARNING << "Could not resolve TEXMFHOME from either the "
                    "environment variable or kpsewhich.";
    throw EnvironmentError(
        "could not resolve TEXMFHOME: set the TEXMFHOME environment "
        "variable, or make sure 'kpsewhich' is available on PATH.");
}

Environment::Environment(std::filesystem::path start_dir)
    : texmf_home_(resolve_texmf_home()),
      global_latex_dir_(texmf_home_ / "tex" / "latex" / "notex"),
      global_fonts_dir_(texmf_home_ / "fonts" / "truetype" / "notex"),
      local_latex_dir_(start_dir / "settings"),
      local_fonts_dir_(start_dir / "fonts") {
    const bool local_present =
        std::filesystem::exists(local_latex_dir_ / "notex.cls");
    const bool global_present =
        std::filesystem::exists(global_latex_dir_ / "notex.cls");

    if (local_present) {
        installation_type_ = InstallationType::LOCAL;
    } else if (global_present) {
        installation_type_ = InstallationType::GLOBAL;
    } else {
        installation_type_ = InstallationType::NONE;
    }
    PLOG_DEBUG << "Installation type detected: "
               << to_string(installation_type_);
}

}  // namespace notex
