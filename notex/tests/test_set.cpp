/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager::set_config_value(): the whitelisted metadata
 * setter behind `notex set`.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <string>

#include "fixtures.hpp"
#include "notex/errors.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Set : public TempDirFixture {};

TEST_F(Test_Set, RejectsUnknownKey) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    EXPECT_THROW(manager.set_config_value("bogus", "value"),
                notex::UsageError);
}

TEST_F(Test_Set, SetsMainFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    manager.set_config_value("main_file", "custom.tex");

    EXPECT_EQ(manager.config().main_file, "custom.tex");
    EXPECT_EQ(notex::Manager(project_dir).config().main_file, "custom.tex");
}

TEST_F(Test_Set, SetsThemeWithoutTouchingMainFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    manager.set_config_value("theme", "dark");

    EXPECT_EQ(manager.config().theme, "dark");
    // Unlike set_theme(), this is a raw metadata write: the
    // \documentclass line is untouched.
    std::ifstream stream(project_dir / "main.tex");
    std::string content((std::istreambuf_iterator<char>(stream)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("\\documentclass[light]{notex}"), std::string::npos);
}

TEST_F(Test_Set, SetsBibliographyFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    manager.set_config_value("bibliography_file", "refs.bib");

    EXPECT_EQ(manager.config().bibliography_file, "refs.bib");
}

}  // namespace
