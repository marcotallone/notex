/**
 * @file installer.hpp
 * @brief Installer: deploys the NoTeX LaTeX template, globally into the
 *        TeX tree or locally into a project.
 */

#pragma once

#include <filesystem>

#include "notex/environment.hpp"

namespace notex {

/**
 * @brief Deploys the NoTeX LaTeX template and fonts onto the system.
 *
 * Every method is static: an installation is really about its target
 * directory, not about any state an Installer instance would hold
 * between calls, so instances of this class serve no purpose.
 */
class Installer {
public:
    Installer() = delete;

    /**
     * @brief Installs the template globally, using @p environment to
     *        resolve the target directories inside the TeX tree.
     *
     * Writes every embedded LaTeX and font file into the TeX tree,
     * creating intermediate directories as needed, and refreshes the TeX
     * filename database afterwards; a failure to do so is downgraded to
     * a warning rather than propagated, since some environments lack
     * `mktexlsr`. If a previous installation already exists there and
     * differs from what would be written, the user is asked to confirm
     * before overwriting, unless @p force is set.
     *
     * @param environment Resolved TeX environment to install into.
     * @param force Skip the confirmation prompt when overwriting a
     *              differing installation.
     * @return True if the installation was performed; false if the user
     *         declined the confirmation prompt.
     */
    static bool install_global(const Environment& environment,
                                bool force = false);

    /**
     * @brief Installs the template locally into @p target_dir.
     *
     * Writes every embedded LaTeX file into `<target_dir>/settings/` and
     * every font into `<target_dir>/fonts/`, then records the local
     * installation in `<target_dir>/.notex/notex.json`, creating that
     * directory (with an otherwise-empty configuration) if @p target_dir
     * is not already a NoTeX project, and preserving its existing
     * configuration if it is.
     *
     * @param target_dir Directory to install into.
     * @param force Skip the confirmation prompt when overwriting a
     *              differing installation.
     * @return True if the installation was performed; false if the user
     *         declined the confirmation prompt.
     */
    static bool install_local(const std::filesystem::path& target_dir,
                               bool force = false);

    /**
     * @brief Removes a global installation: every embedded LaTeX and
     *        font file's target directory inside @p environment's TeX
     *        tree, then refreshes the TeX filename database (a failure
     *        there is downgraded to a warning, as in install_global()).
     *        Never touches anything outside those directories.
     *
     * @param environment Resolved TeX environment to uninstall from.
     * @param force Skip the confirmation prompt.
     * @return True if the uninstall was performed; false if the user
     *         declined the confirmation prompt.
     */
    static bool uninstall_global(const Environment& environment,
                                  bool force = false);

    /**
     * @brief Removes a local installation: @p target_dir's `settings/`
     *        and `fonts/` directories, then clears
     *        `installation_type` in `.notex/notex.json` if present.
     *        Never touches the project's own documents.
     *
     * @param target_dir Directory to uninstall from.
     * @param force Skip the confirmation prompt.
     * @return True if the uninstall was performed; false if the user
     *         declined the confirmation prompt.
     */
    static bool uninstall_local(const std::filesystem::path& target_dir,
                                 bool force = false);

    /**
     * @brief Reports whether an installation at @p latex_dir/@p
     *        fonts_dir differs from what would be freshly installed.
     *
     * Used by `checkhealth` to note installation drift (e.g. a
     * hand-customised local install) without needing to actually
     * reinstall anything.
     *
     * @param latex_dir Directory to compare the embedded LaTeX files
     *                   against.
     * @param fonts_dir Directory to compare the embedded font files
     *                   against.
     * @return True if any embedded file is missing from, or differs
     *         from, its counterpart in @p latex_dir or @p fonts_dir.
     */
    static bool installation_differs(const std::filesystem::path& latex_dir,
                                      const std::filesystem::path& fonts_dir);
};

}  // namespace notex
