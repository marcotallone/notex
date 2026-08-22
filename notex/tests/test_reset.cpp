/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager::reset(): regenerating the main file from the
 * template after backing up the previous one.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "fixtures.hpp"
#include "notex/output.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::NonInteractiveGuard;
using notex::testing::TempDirFixture;

class Test_Reset : public TempDirFixture {};

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

class AssumeYesGuard {
public:
    explicit AssumeYesGuard(bool value) { notex::ui::set_assume_yes(value); }
    ~AssumeYesGuard() { notex::ui::set_assume_yes(false); }
    AssumeYesGuard(const AssumeYesGuard&) = delete;
    AssumeYesGuard& operator=(const AssumeYesGuard&) = delete;
};

TEST_F(Test_Reset, BacksUpMainTex) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    const std::string original = read_file(project_dir / "main.tex");
    {
        std::ofstream stream(project_dir / "main.tex", std::ios::app);
        stream << "% hand-edited\n";
    }
    const std::string edited = read_file(project_dir / "main.tex");

    AssumeYesGuard guard(true);
    ASSERT_TRUE(manager.reset());

    EXPECT_EQ(read_file(project_dir / "main.tex.bak"), edited);
    EXPECT_NE(read_file(project_dir / "main.tex"), edited);
    EXPECT_NE(original.find("\\documentclass"), std::string::npos);
}

TEST_F(Test_Reset, DeclinedLeavesMainFileUntouched) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);
    const std::string original = read_file(project_dir / "main.tex");

    // Non-interactive and assume-yes off: confirm() falls back to its
    // default answer (decline).
    NonInteractiveGuard guard;
    EXPECT_FALSE(manager.reset());
    EXPECT_EQ(read_file(project_dir / "main.tex"), original);
    EXPECT_FALSE(std::filesystem::exists(project_dir / "main.tex.bak"));
}

TEST_F(Test_Reset, MultiFileReferencesEveryExistingSection) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_section("Second Section");
    manager.add_section("Third Section");

    AssumeYesGuard guard(true);
    ASSERT_TRUE(manager.reset());

    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_NE(content.find("\\subfile{sections/1_introduction}"),
             std::string::npos);
    EXPECT_NE(content.find("\\subfile{sections/2_second_section}"),
             std::string::npos);
    EXPECT_NE(content.find("\\subfile{sections/3_third_section}"),
             std::string::npos);

    // The sections themselves are untouched by reset().
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "2_second_section.tex"));
}

}  // namespace
