/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for the read side of Manager: project discovery and `notex.json`
 * round-tripping.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "fixtures.hpp"
#include "notex/errors.hpp"
#include "notex/templates.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Manager : public TempDirFixture {};

void write_file(const std::filesystem::path& path,
                 const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << content;
}

void make_project(const std::filesystem::path& root_dir,
                   const std::string& config_json) {
    write_file(root_dir / ".notex" / "notex.json", config_json);
}

TEST_F(Test_Manager, FindWalksUpToNotexDir) {
    make_project(temp_dir_, R"({"schema_version": 1})");
    const std::filesystem::path nested = temp_dir_ / "a" / "b" / "c";
    std::filesystem::create_directories(nested);

    const auto found = notex::Manager::find_project_root(nested);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(std::filesystem::weakly_canonical(*found),
              std::filesystem::weakly_canonical(temp_dir_));
}

TEST_F(Test_Manager, FindStopsAtFilesystemRoot) {
    const std::filesystem::path nested = temp_dir_ / "no" / "project" / "here";
    std::filesystem::create_directories(nested);

    EXPECT_FALSE(notex::Manager::find_project_root(nested).has_value());
}

TEST_F(Test_Manager, RequireThrowsOutsideProject) {
    EXPECT_THROW(notex::Manager manager(temp_dir_),
                 notex::ProjectNotFoundError);
}

TEST_F(Test_Manager, ConfigRoundTripSaveLoad) {
    make_project(temp_dir_, R"({"schema_version": 1})");

    notex::Manager manager(temp_dir_);
    manager.config().notex_version = "1.0.0";
    manager.config().project_type = "multi";
    manager.config().main_file = "main.tex";
    manager.config().installation_type = "local";
    manager.config().theme = "dark";
    manager.config().bibliography_file = "bibliography.bib";
    manager.save();

    const notex::Manager reloaded(temp_dir_);
    EXPECT_EQ(reloaded.config().notex_version, "1.0.0");
    EXPECT_EQ(reloaded.config().project_type, "multi");
    EXPECT_EQ(reloaded.config().main_file, "main.tex");
    EXPECT_EQ(reloaded.config().installation_type, "local");
    EXPECT_EQ(reloaded.config().theme, "dark");
    EXPECT_EQ(reloaded.config().bibliography_file, "bibliography.bib");
}

TEST_F(Test_Manager, ThrowsOnMalformedConfig) {
    make_project(temp_dir_, "{not valid json");
    EXPECT_THROW(notex::Manager manager(temp_dir_), notex::ConfigError);
}

TEST_F(Test_Manager, DefaultsMainFileWhenMissingFromConfig) {
    make_project(temp_dir_, R"({"schema_version": 1})");
    const notex::Manager manager(temp_dir_);
    EXPECT_EQ(manager.config().main_file, "main.tex");
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

TEST_F(Test_Manager, InitMonoCreatesMainAndNotexDir) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    const notex::Manager manager =
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    EXPECT_TRUE(std::filesystem::exists(project_dir / "main.tex"));
    EXPECT_TRUE(std::filesystem::is_directory(project_dir / ".notex"));
    EXPECT_FALSE(std::filesystem::exists(project_dir / "sections"));
    EXPECT_EQ(manager.config().project_type, "mono");
}

TEST_F(Test_Manager, InitMultiCreatesSectionsTree) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    notex::Manager::init(project_dir, notex::templates::ProjectType::MULTI);

    EXPECT_TRUE(std::filesystem::exists(project_dir / "main.tex"));
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "1_introduction.tex"));
    const std::string main_content = read_file(project_dir / "main.tex");
    EXPECT_NE(main_content.find("\\subfile{sections/1_introduction}"),
             std::string::npos);
}

TEST_F(Test_Manager, InitDefaultsToMulti) {
    const std::filesystem::path project_dir = temp_dir_ / "project";

    const notex::Manager manager = notex::Manager::init(project_dir);

    EXPECT_EQ(manager.config().project_type, "multi");
    EXPECT_TRUE(std::filesystem::exists(project_dir / "sections" /
                                        "1_introduction.tex"));
}

TEST_F(Test_Manager, InitRefusesExistingProjectWithoutForce) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    notex::Manager::init(project_dir);

    EXPECT_THROW(notex::Manager::init(project_dir), notex::FilesystemError);

    EXPECT_NO_THROW(
        notex::Manager::init(project_dir, notex::templates::ProjectType::MONO,
                             /*force=*/true));
}

TEST_F(Test_Manager, InitUsesSettingsClassPathWhenLocalInstall) {
    const std::filesystem::path project_dir = temp_dir_ / "project";
    write_file(project_dir / "settings" / "notex.cls", "% fake class file");

    notex::Manager::init(project_dir, notex::templates::ProjectType::MONO);

    const std::string main_content = read_file(project_dir / "main.tex");
    EXPECT_NE(main_content.find("\\documentclass[light]{settings/notex}"),
             std::string::npos);
}

TEST_F(Test_Manager, InitConfigRecordsProjectType) {
    const std::filesystem::path mono_dir = temp_dir_ / "mono-project";
    const std::filesystem::path multi_dir = temp_dir_ / "multi-project";

    notex::Manager::init(mono_dir, notex::templates::ProjectType::MONO);
    notex::Manager::init(multi_dir, notex::templates::ProjectType::MULTI);

    EXPECT_EQ(notex::Manager(mono_dir).config().project_type, "mono");
    EXPECT_EQ(notex::Manager(multi_dir).config().project_type, "multi");
}

}  // namespace
