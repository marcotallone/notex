/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager's bibliography commands: creating the .bib file,
 * detecting biblatex, idempotency, and removal.
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

class Test_Bibliography : public TempDirFixture {};

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

std::size_t count_occurrences(const std::string& haystack,
                              const std::string& needle) {
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

// See test_sections.cpp: remove_bibliography() asks for confirmation
// before deleting the .bib file itself, honouring the global "assume
// yes" flag, which must be restored afterwards.
class AssumeYesGuard {
public:
    explicit AssumeYesGuard(bool value) { notex::ui::set_assume_yes(value); }
    ~AssumeYesGuard() { notex::ui::set_assume_yes(false); }
    AssumeYesGuard(const AssumeYesGuard&) = delete;
    AssumeYesGuard& operator=(const AssumeYesGuard&) = delete;
};

TEST_F(Test_Bibliography, AddCreatesBibAndCommands) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    manager.add_bibliography();

    EXPECT_TRUE(
        std::filesystem::exists(project_dir / "bibliography.bib"));
    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_NE(content.find("\\bibliographystyle{unsrturl}"), std::string::npos);
    EXPECT_NE(content.find("\\bibliography{bibliography}"), std::string::npos);
    EXPECT_EQ(notex::Manager(project_dir).config().bibliography_file,
             "bibliography.bib");
}

TEST_F(Test_Bibliography, AddIsIdempotent) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    manager.add_bibliography();
    manager.add_bibliography();

    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_EQ(count_occurrences(content, "\\bibliographystyle{unsrturl}"), 1u);
    EXPECT_EQ(count_occurrences(content, "\\bibliography{bibliography}"), 1u);
}

TEST_F(Test_Bibliography, AddDetectsBiblatex) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);

    std::string content = read_file(project_dir / "main.tex");
    const std::string anchor = "\\documentclass[light]{notex}";
    const std::size_t pos = content.find(anchor);
    ASSERT_NE(pos, std::string::npos);
    content.insert(pos + anchor.size(), "\n\\usepackage{biblatex}");
    write_file(project_dir / "main.tex", content);

    manager.add_bibliography();

    const std::string updated = read_file(project_dir / "main.tex");
    EXPECT_NE(updated.find("\\addbibresource{bibliography.bib}"),
             std::string::npos);
    EXPECT_NE(updated.find("\\printbibliography"), std::string::npos);
    EXPECT_EQ(updated.find("\\bibliographystyle"), std::string::npos);
}

TEST_F(Test_Bibliography, RemoveStripsCommands) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_bibliography();

    // remove_bibliography() strips the commands unconditionally, but
    // still asks before deleting the .bib file itself, so this still
    // needs the non-interactive fallback forced deterministically.
    NonInteractiveGuard guard;
    manager.remove_bibliography();

    const std::string content = read_file(project_dir / "main.tex");
    EXPECT_EQ(content.find("\\bibliographystyle"), std::string::npos);
    EXPECT_EQ(content.find("\\bibliography{bibliography}"), std::string::npos);
    EXPECT_EQ(notex::Manager(project_dir).config().bibliography_file, "");
}

TEST_F(Test_Bibliography, RemoveDeclinedKeepsBibFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_bibliography();

    // Non-interactive, assume-yes off: the .bib deletion prompt falls
    // back to its default answer (decline), but the commands are
    // stripped from main.tex unconditionally regardless.
    NonInteractiveGuard guard;
    manager.remove_bibliography();

    EXPECT_TRUE(
        std::filesystem::exists(project_dir / "bibliography.bib"));
}

TEST_F(Test_Bibliography, RemoveConfirmedDeletesBibFile) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager manager = notex::Manager::init(
        project_dir, notex::templates::ProjectType::MULTI);
    manager.add_bibliography();

    AssumeYesGuard guard(true);
    manager.remove_bibliography();

    EXPECT_FALSE(
        std::filesystem::exists(project_dir / "bibliography.bib"));
}

}  // namespace
