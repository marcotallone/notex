/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager's theme-switching: validation against the embedded
 * theme files and rewriting main.tex's \documentclass options.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "fixtures.hpp"
#include "notex/assets.hpp"
#include "notex/errors.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Theme : public TempDirFixture {};

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

TEST_F(Test_Theme, AvailableThemesListsAllFour) {
    const auto themes = notex::Manager::available_themes();
    for (const char* expected : {"light", "dark", "tokyo", "bw"}) {
        EXPECT_NE(std::find(themes.begin(), themes.end(), expected),
                 themes.end())
            << "missing theme: " << expected;
    }
}

TEST_F(Test_Theme, RewritesDocumentclassOption) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    manager.set_theme("dark");

    const std::string main_content = read_file(project_dir / "main.tex");
    EXPECT_NE(main_content.find("\\documentclass[dark]{notex}"),
             std::string::npos);
    EXPECT_EQ(notex::Manager(project_dir).config().theme, "dark");
}

TEST_F(Test_Theme, RejectsUnknownTheme) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    EXPECT_THROW(manager.set_theme("nonexistent-theme"), notex::UsageError);
}

TEST_F(Test_Theme, BwWorksAfterClsFix) {
    const auto cls = notex::assets::find_latex_file("notex.cls");
    ASSERT_TRUE(cls.has_value());
    EXPECT_NE(cls->content.find("\\DeclareOption{bw}"), std::string_view::npos);

    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);
    manager.set_theme("bw");

    const std::string main_content = read_file(project_dir / "main.tex");
    EXPECT_NE(main_content.find("\\documentclass[bw]{notex}"),
             std::string::npos);
}

}  // namespace
