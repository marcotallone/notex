/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Installer::uninstall_global()/uninstall_local(): removing
 * only what was installed, and never a project's own documents.
 */

#include "notex/installer.hpp"

#include <gtest/gtest.h>

#include <fstream>

#include "fixtures.hpp"
#include "notex/manager.hpp"

namespace {

using notex::testing::EnvVarGuard;
using notex::testing::NonInteractiveGuard;
using notex::testing::TempDirFixture;

class Test_Uninstall : public TempDirFixture {};

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << content;
}

TEST_F(Test_Uninstall, GlobalRemovesTdsTreeOnly) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_ / "unrelated");
    ASSERT_TRUE(notex::Installer::install_global(environment));
    ASSERT_TRUE(std::filesystem::exists(environment.global_latex_dir() /
                                        "notex.cls"));

    // Something outside the TDS tree Installer manages, to confirm
    // uninstall doesn't reach beyond it.
    write_file(temp_dir_ / "untouched.txt", "keep me");

    ASSERT_TRUE(notex::Installer::uninstall_global(environment, true));

    EXPECT_FALSE(std::filesystem::exists(environment.global_latex_dir()));
    EXPECT_FALSE(std::filesystem::exists(environment.global_fonts_dir()));
    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "untouched.txt"));
}

TEST_F(Test_Uninstall, LocalPreservesUserTexFiles) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    write_file(project_dir / "main.tex", "\\documentclass{settings/notex}");
    write_file(project_dir / "sections" / "1_introduction.tex",
              "user content");

    ASSERT_TRUE(notex::Installer::uninstall_local(project_dir, true));

    EXPECT_FALSE(std::filesystem::exists(project_dir / "settings"));
    EXPECT_FALSE(std::filesystem::exists(project_dir / "fonts"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "main.tex"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "1_introduction.tex"));
}

TEST_F(Test_Uninstall, LocalClearsInstallationTypeInMetadata) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    ASSERT_TRUE(notex::Installer::install_local(project_dir));
    ASSERT_EQ(notex::Manager(project_dir).config().installation_type,
             "local");

    ASSERT_TRUE(notex::Installer::uninstall_local(project_dir, true));

    EXPECT_EQ(notex::Manager(project_dir).config().installation_type, "");
}

TEST_F(Test_Uninstall, DeclinedLeavesInstallationIntact) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    // Non-interactive, no --force: confirm() falls back to its default
    // answer (decline).
    NonInteractiveGuard guard;
    EXPECT_FALSE(notex::Installer::uninstall_local(project_dir, false));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "settings"));
}

}  // namespace
