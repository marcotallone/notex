/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Installer: global and local template deployment, always
 * against a fake TEXMFHOME so that no real TeX installation is touched.
 */

#include "notex/installer.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "fixtures.hpp"
#include "notex/assets.hpp"
#include "notex/manager.hpp"

namespace {

using notex::testing::EnvVarGuard;
using notex::testing::NonInteractiveGuard;
using notex::testing::TempDirFixture;

class Test_Installer : public TempDirFixture {};

bool file_matches(const std::filesystem::path& path,
                   std::string_view expected) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) return false;
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str() == expected;
}

TEST_F(Test_Installer, GlobalCreatesTdsTree) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_ / "unrelated");

    ASSERT_TRUE(notex::Installer::install_global(environment));

    EXPECT_TRUE(std::filesystem::is_directory(environment.global_latex_dir()));
    EXPECT_TRUE(
        std::filesystem::is_directory(environment.global_fonts_dir()));
    EXPECT_TRUE(std::filesystem::exists(environment.global_latex_dir() /
                                         "notex.cls"));
}

TEST_F(Test_Installer, GlobalWritesAllLatexFiles) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_ / "unrelated");

    ASSERT_TRUE(notex::Installer::install_global(environment));

    for (const auto& file : notex::assets::latex_files()) {
        const std::filesystem::path target =
            environment.global_latex_dir() / file.name;
        EXPECT_TRUE(file_matches(target, file.content))
            << "mismatch for " << file.name;
    }
    for (const auto& file : notex::assets::font_files()) {
        const std::filesystem::path target =
            environment.global_fonts_dir() / file.name;
        EXPECT_TRUE(file_matches(target, file.content))
            << "mismatch for " << file.name;
    }
}

TEST_F(Test_Installer, GlobalReinstallRequiresForceWhenModified) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_ / "unrelated");

    ASSERT_TRUE(notex::Installer::install_global(environment));

    const std::filesystem::path cls_file =
        environment.global_latex_dir() / "notex.cls";
    {
        std::ofstream stream(cls_file, std::ios::app);
        stream << "% tampered\n";
    }

    // Non-interactive and no --force: confirm() falls back to its
    // default answer (decline), so the modification must survive.
    NonInteractiveGuard non_interactive_guard;
    EXPECT_FALSE(notex::Installer::install_global(environment, false));
    EXPECT_FALSE(file_matches(cls_file, notex::assets::find_latex_file(
                                             "notex.cls")
                                             ->content));

    EXPECT_TRUE(notex::Installer::install_global(environment, true));
    EXPECT_TRUE(file_matches(
        cls_file, notex::assets::find_latex_file("notex.cls")->content));
}

TEST_F(Test_Installer, LocalCreatesSettingsAndFontsDirs) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    EXPECT_TRUE(
        std::filesystem::is_directory(project_dir / "settings"));
    EXPECT_TRUE(std::filesystem::is_directory(project_dir / "fonts"));
    EXPECT_TRUE(
        std::filesystem::exists(project_dir / "settings" / "notex.cls"));
}

TEST_F(Test_Installer, LocalWritesNotexJson) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    ASSERT_TRUE(std::filesystem::is_directory(project_dir / ".notex"));
    const notex::Manager manager(project_dir);
    EXPECT_EQ(manager.config().installation_type, "local");
}

TEST_F(Test_Installer, LocalIsIdempotentWithForce) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    ASSERT_TRUE(notex::Installer::install_local(project_dir, true));
    ASSERT_TRUE(notex::Installer::install_local(project_dir, true));

    for (const auto& file : notex::assets::latex_files()) {
        EXPECT_TRUE(file_matches(project_dir / "settings" / file.name,
                                  file.content));
    }
    const notex::Manager manager(project_dir);
    EXPECT_EQ(manager.config().installation_type, "local");
}

TEST_F(Test_Installer, LocalPreservesExistingProjectMetadata) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    std::filesystem::create_directories(project_dir / ".notex");
    notex::ProjectConfig config;
    config.project_type = "multi";
    config.theme = "dark";
    notex::Manager::write_config(project_dir, config);

    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    const notex::Manager manager(project_dir);
    EXPECT_EQ(manager.config().project_type, "multi");
    EXPECT_EQ(manager.config().theme, "dark");
    EXPECT_EQ(manager.config().installation_type, "local");
}

}  // namespace
