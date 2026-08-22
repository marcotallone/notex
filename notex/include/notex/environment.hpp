/**
 * @file environment.hpp
 * @brief Environment: discovers where TeX lives and whether NoTeX is
 *        installed.
 */

#pragma once

#include <filesystem>
#include <string_view>

namespace notex {

/// Whether, and how, the NoTeX template is installed relative to the
/// directory an Environment was asked about.
enum class InstallationType {
    NONE,    ///< Neither a local nor a global installation was found.
    LOCAL,   ///< A local installation exists (takes precedence over global).
    GLOBAL,  ///< Only a global installation exists.
};

/// @return A human-readable name for @p type ("none", "local", "global").
std::string_view to_string(InstallationType type);

/**
 * @brief Represents the surrounding system as NoTeX perceives it: where
 *        TeX expects user files to live, and whether NoTeX is installed.
 *
 * An Environment holds no project state and never modifies anything on
 * disk; it is a read-only snapshot that the rest of the program consults.
 * It resolves `TEXMFHOME` by honouring the `TEXMFHOME` environment
 * variable first and, only if that is absent, falling back to
 * `kpsewhich -var-value=TEXMFHOME`.
 */
class Environment {
public:
    /**
     * @brief Resolves `TEXMFHOME` and checks @p start_dir for a local
     *        installation.
     * @param start_dir Directory checked for a local installation
     *                   (`<start_dir>/settings/notex.cls`); defaults to
     *                   the current working directory.
     * @throws EnvironmentError if `TEXMFHOME` cannot be resolved from
     *         either the environment variable or `kpsewhich`.
     */
    explicit Environment(
        std::filesystem::path start_dir = std::filesystem::current_path());

    /// @return The resolved `TEXMFHOME` directory.
    const std::filesystem::path& texmf_home() const noexcept {
        return texmf_home_;
    }

    /// @return Where a global installation's class and style files live:
    ///         `<texmf_home>/tex/latex/notex`.
    const std::filesystem::path& global_latex_dir() const noexcept {
        return global_latex_dir_;
    }

    /// @return Where a global installation's fonts live:
    ///         `<texmf_home>/fonts/truetype/notex`.
    const std::filesystem::path& global_fonts_dir() const noexcept {
        return global_fonts_dir_;
    }

    /// @return Where a local installation's class and style files would
    ///         live: `<start_dir>/settings`.
    const std::filesystem::path& local_latex_dir() const noexcept {
        return local_latex_dir_;
    }

    /// @return Where a local installation's fonts would live:
    ///         `<start_dir>/fonts`.
    const std::filesystem::path& local_fonts_dir() const noexcept {
        return local_fonts_dir_;
    }

    /// @return Whether a local, global, or no installation was detected
    ///         relative to @p start_dir. A local installation takes
    ///         precedence over a global one when both are present.
    InstallationType installation_type() const noexcept {
        return installation_type_;
    }

private:
    /// Resolves TEXMFHOME: the environment variable first, `kpsewhich` as
    /// a fallback.
    static std::filesystem::path resolve_texmf_home();

    std::filesystem::path texmf_home_;
    std::filesystem::path global_latex_dir_;
    std::filesystem::path global_fonts_dir_;
    std::filesystem::path local_latex_dir_;
    std::filesystem::path local_fonts_dir_;
    InstallationType installation_type_;
};

}  // namespace notex
