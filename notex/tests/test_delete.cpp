/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager::delete_scaffolding(): removing NoTeX's own files
 * while preserving user-authored ones, unless --all is given.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>

#include "fixtures.hpp"
#include "notex/installer.hpp"
#include "notex/output.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::NonInteractiveGuard;
using notex::testing::TempDirFixture;

class Test_Delete : public TempDirFixture {};

class AssumeYesGuard {
public:
    explicit AssumeYesGuard(bool value) { notex::ui::set_assume_yes(value); }
    ~AssumeYesGuard() { notex::ui::set_assume_yes(false); }
    AssumeYesGuard(const AssumeYesGuard&) = delete;
    AssumeYesGuard& operator=(const AssumeYesGuard&) = delete;
};

TEST_F(Test_Delete, PreservesUserFilesWithoutAllFlag) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_bibliography();
    ASSERT_TRUE(notex::Installer::install_local(project_dir, true));

    AssumeYesGuard guard(true);
    EXPECT_TRUE(manager.delete_scaffolding(/*remove_all=*/false));

    EXPECT_FALSE(std::filesystem::exists(project_dir / ".notex"));
    EXPECT_FALSE(std::filesystem::exists(project_dir / "settings"));
    EXPECT_FALSE(std::filesystem::exists(project_dir / "fonts"));

    EXPECT_TRUE(std::filesystem::exists(project_dir / "main.tex"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "1_introduction.tex"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "bibliography.bib"));
}

TEST_F(Test_Delete, AllFlagRemovesEntireProject) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    AssumeYesGuard guard(true);
    EXPECT_TRUE(manager.delete_scaffolding(/*remove_all=*/true));

    EXPECT_FALSE(std::filesystem::exists(project_dir));
}

TEST_F(Test_Delete, DeclinedLeavesProjectUntouched) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    // Non-interactive and assume-yes off: confirm() falls back to its
    // default answer (decline).
    NonInteractiveGuard guard;
    EXPECT_FALSE(manager.delete_scaffolding());
    EXPECT_TRUE(std::filesystem::exists(project_dir / "main.tex"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / ".notex"));
}

TEST_F(Test_Delete, RemovesBuildArtefacts) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);
    { std::ofstream(project_dir / "main.aux") << "junk"; }

    AssumeYesGuard guard(true);
    ASSERT_TRUE(manager.delete_scaffolding());

    EXPECT_FALSE(std::filesystem::exists(project_dir / "main.aux"));
}

}  // namespace
