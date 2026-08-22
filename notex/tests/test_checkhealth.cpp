/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for the domain checks `notex checkhealth` composes (its own
 * assembly of these into a report lives in the Orchestrator, which is
 * frontend-only and untested here — see AGENTS.md/CLAUDE.md): the
 * missing-main-file check, install-drift detection, and orphan-section
 * discovery.
 */

#include "notex/installer.hpp"
#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>

#include "fixtures.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Checkhealth : public TempDirFixture {};

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << content;
}

TEST_F(Test_Checkhealth, ReportsMissingMainFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);
    std::filesystem::remove(project_dir / "main.tex");

    // The metadata itself must still parse (checkhealth reports this
    // separately from the main file's existence).
    const notex::Manager reloaded(project_dir);
    EXPECT_FALSE(std::filesystem::exists(reloaded.root_dir() /
                                         reloaded.config().main_file));
}

TEST_F(Test_Checkhealth, DetectsModifiedLocalSettings) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    ASSERT_TRUE(notex::Installer::install_local(project_dir));

    EXPECT_FALSE(notex::Installer::installation_differs(
        project_dir / "settings", project_dir / "fonts"));

    std::ofstream(project_dir / "settings" / "notex.cls", std::ios::app)
        << "% tampered\n";

    EXPECT_TRUE(notex::Installer::installation_differs(
        project_dir / "settings", project_dir / "fonts"));
}

TEST_F(Test_Checkhealth, ReportsOrphanSectionFiles) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    // Added by hand, without going through `add section`, and without a
    // corresponding \subfile line in main.tex.
    write_file(project_dir / "sections" / "5_orphan.tex",
              "\\documentclass[../main.tex]{subfiles}\n"
              "\\begin{document}\n\\section{Orphan}\n\\end{document}\n");

    const auto orphans = manager.orphan_sections();
    ASSERT_EQ(orphans.size(), 1u);
    EXPECT_EQ(orphans.front(), "5_orphan");
}

TEST_F(Test_Checkhealth, NoOrphansWhenEverySectionIsReferenced) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_section("Second Section");

    EXPECT_TRUE(manager.orphan_sections().empty());
}

TEST_F(Test_Checkhealth, MonoProjectHasNoOrphans) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    EXPECT_TRUE(manager.orphan_sections().empty());
}

}  // namespace
