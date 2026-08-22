/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager::project_files(), the listing `ls` is built on.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>

#include "fixtures.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Ls : public TempDirFixture {};

bool contains(const std::vector<std::filesystem::path>& files,
             const std::filesystem::path& target) {
    return std::find(files.begin(), files.end(), target) != files.end();
}

TEST_F(Test_Ls, ListsOnlyRelevantFiles) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_bibliography();

    // Irrelevant files that don't belong to the project's tracked
    // content: a build artefact and an unrelated document.
    { std::ofstream(project_dir / "main.aux") << "junk"; }
    { std::ofstream(project_dir / "notes.txt") << "unrelated"; }

    const auto files = manager.project_files();

    EXPECT_TRUE(contains(files, project_dir / "main.tex"));
    EXPECT_TRUE(contains(files, project_dir / "sections" /
                                    "1_introduction.tex"));
    EXPECT_TRUE(contains(files, project_dir / "bibliography.bib"));
    EXPECT_FALSE(contains(files, project_dir / "main.aux"));
    EXPECT_FALSE(contains(files, project_dir / "notes.txt"));
}

TEST_F(Test_Ls, MonoProjectHasNoSectionsListed) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    const auto files = manager.project_files();
    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files.front(), project_dir / "main.tex");
}

TEST_F(Test_Ls, MissingMainFileIsOmitted) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);
    std::filesystem::remove(project_dir / "main.tex");

    EXPECT_TRUE(manager.project_files().empty());
}

}  // namespace
