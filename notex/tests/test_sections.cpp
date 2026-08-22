/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager's section commands: adding and removing sections in
 * both mono and multi projects.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "fixtures.hpp"
#include "notex/errors.hpp"
#include "notex/output.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::NonInteractiveGuard;
using notex::testing::TempDirFixture;

class Test_Sections : public TempDirFixture {};

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << content;
}

// confirm() honours the global "assume yes" flag; remove_section() asks
// for confirmation, so tests exercising the success path set this for
// their duration and restore it afterwards, since it is process-global
// state shared with every other test in the suite.
class AssumeYesGuard {
public:
    explicit AssumeYesGuard(bool value) { notex::ui::set_assume_yes(value); }
    ~AssumeYesGuard() { notex::ui::set_assume_yes(false); }
    AssumeYesGuard(const AssumeYesGuard&) = delete;
    AssumeYesGuard& operator=(const AssumeYesGuard&) = delete;
};

TEST_F(Test_Sections, AddMonoInsertsBeforeEndDocument) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    manager.add_section("My New Section");

    const std::string content = read_file(project_dir / "main.tex");
    const std::size_t section_pos = content.find("\\section{My New Section}");
    const std::size_t end_pos = content.find("\\end{document}");
    ASSERT_NE(section_pos, std::string::npos);
    ASSERT_NE(end_pos, std::string::npos);
    EXPECT_LT(section_pos, end_pos);
}

TEST_F(Test_Sections, AddMultiCreatesNumberedFileAndSubfileLine) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    manager.add_section("Usage Guide");

    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "2_usage_guide.tex"));
    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_NE(content.find("\\subfile{sections/2_usage_guide}"),
             std::string::npos);
}

TEST_F(Test_Sections, AddMultiNumbersAfterUserCreatedSections) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    // The user created this section by hand, without going through
    // `notex add section`, and without updating main.tex.
    write_file(project_dir / "sections" / "5_manual.tex",
              "\\documentclass[../main.tex]{subfiles}\n"
              "\\begin{document}\n\\section{Manual}\n\\end{document}\n");

    manager.add_section("New Section");

    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "6_new_section.tex"));
    const std::string content = read_file(project_dir / "main.tex");
    // The insertion anchor is the last \subfile line already in
    // main.tex (still "1_introduction", since the hand-added section 5
    // was never referenced there), independent of the numbering, which
    // comes from scanning sections/ instead.
    EXPECT_NE(content.find("\\subfile{sections/6_new_section}"),
             std::string::npos);
}

TEST_F(Test_Sections, AddFailsGracefullyWithoutEndDocument) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    const std::string original = read_file(project_dir / "main.tex");
    std::string mangled = original;
    const std::size_t pos = mangled.find("\\end{document}");
    ASSERT_NE(pos, std::string::npos);
    mangled.erase(pos, std::string("\\end{document}").size());
    write_file(project_dir / "main.tex", mangled);

    EXPECT_THROW(manager.add_section("Doomed"), notex::DocumentError);
    EXPECT_EQ(read_file(project_dir / "main.tex"), mangled);
}

TEST_F(Test_Sections, RemoveDeletesFileAndSubfileLine) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_section("Usage Guide");
    ASSERT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "2_usage_guide.tex"));

    AssumeYesGuard guard(true);
    EXPECT_TRUE(manager.remove_section(2));

    EXPECT_FALSE(std::filesystem::exists(project_dir / "sections" /
                                         "2_usage_guide.tex"));
    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_EQ(content.find("\\subfile{sections/2_usage_guide}"),
             std::string::npos);
}

TEST_F(Test_Sections, RemoveDeclinedLeavesFileUntouched) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    // Non-interactive and assume-yes off: confirm() falls back to its
    // default answer (decline).
    NonInteractiveGuard guard;
    EXPECT_FALSE(manager.remove_section(1));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "1_introduction.tex"));
}

TEST_F(Test_Sections, RemoveRejectsMonoProject) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    EXPECT_THROW(manager.remove_section(1), notex::UsageError);
}

TEST_F(Test_Sections, RemoveRejectsUnknownNumber) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    AssumeYesGuard guard(true);
    EXPECT_THROW(manager.remove_section(99), notex::UsageError);
}

}  // namespace
