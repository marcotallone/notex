/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Environment: TEXMFHOME resolution and installation detection,
 * always against a fake TEXMFHOME so that no real TeX installation is
 * needed.
 */

#include "notex/environment.hpp"

#include <gtest/gtest.h>

#include <fstream>

#include "fixtures.hpp"

namespace {

using notex::testing::EnvVarGuard;
using notex::testing::TempDirFixture;

class Test_Environment : public TempDirFixture {};

void touch_file(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path).close();
}

TEST_F(Test_Environment, TexmfHomeFromEnvironmentVariable) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_);
    EXPECT_EQ(environment.texmf_home(), temp_dir_);
}

TEST_F(Test_Environment, DetectsGlobalInstallation) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    touch_file(temp_dir_ / "tex" / "latex" / "notex" / "notex.cls");

    const notex::Environment environment(temp_dir_ / "empty_project");
    EXPECT_EQ(environment.installation_type(),
              notex::InstallationType::GLOBAL);
}

TEST_F(Test_Environment, LocalOverridesGlobal) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    touch_file(temp_dir_ / "tex" / "latex" / "notex" / "notex.cls");
    const std::filesystem::path project_dir = temp_dir_ / "project";
    touch_file(project_dir / "settings" / "notex.cls");

    const notex::Environment environment(project_dir);
    EXPECT_EQ(environment.installation_type(),
              notex::InstallationType::LOCAL);
}

TEST_F(Test_Environment, NoneWhenNothingInstalled) {
    EnvVarGuard guard("TEXMFHOME", temp_dir_.string());
    const notex::Environment environment(temp_dir_);
    EXPECT_EQ(environment.installation_type(), notex::InstallationType::NONE);
}

TEST_F(Test_Environment, ToStringNamesEveryInstallationType) {
    EXPECT_EQ(notex::to_string(notex::InstallationType::NONE), "none");
    EXPECT_EQ(notex::to_string(notex::InstallationType::LOCAL), "local");
    EXPECT_EQ(notex::to_string(notex::InstallationType::GLOBAL), "global");
}

}  // namespace
