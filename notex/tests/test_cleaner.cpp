/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Manager::clean: recursive removal of LaTeX build artefacts.
 */

#include "notex/manager.hpp"

#include <gtest/gtest.h>

#include <fstream>

#include "fixtures.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Cleaner : public TempDirFixture {};

void write_file(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path).close();
}

TEST_F(Test_Cleaner, RemovesArtifactExtensions) {
    write_file(temp_dir_ / "main.aux");
    write_file(temp_dir_ / "main.log");
    write_file(temp_dir_ / "main.toc");

    const auto report = notex::Manager::clean(temp_dir_);

    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "main.aux"));
    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "main.log"));
    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "main.toc"));
    EXPECT_EQ(report.removed_files.size(), 3u);
}

TEST_F(Test_Cleaner, RemovesSuffixArtifactsLikeSynctexBusy) {
    write_file(temp_dir_ / "main.synctex.gz");
    write_file(temp_dir_ / "main.synctex.gz(busy)");
    write_file(temp_dir_ / "references-blx.bib");

    const auto report = notex::Manager::clean(temp_dir_);

    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "main.synctex.gz"));
    EXPECT_FALSE(
        std::filesystem::exists(temp_dir_ / "main.synctex.gz(busy)"));
    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "references-blx.bib"));
    EXPECT_EQ(report.removed_files.size(), 3u);
}

TEST_F(Test_Cleaner, RemovesMintedDirectories) {
    write_file(temp_dir_ / "_minted-main" / "default.pygstyle");
    write_file(temp_dir_ / "_minted-report" / "default.pygstyle");

    const auto report = notex::Manager::clean(temp_dir_);

    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "_minted-main"));
    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "_minted-report"));
    EXPECT_EQ(report.removed_directories.size(), 2u);
}

TEST_F(Test_Cleaner, PreservesSourceFiles) {
    write_file(temp_dir_ / "main.tex");
    write_file(temp_dir_ / "sections" / "intro.tex");
    write_file(temp_dir_ / "bibliography.bib");
    write_file(temp_dir_ / "main.aux");

    notex::Manager::clean(temp_dir_);

    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "main.tex"));
    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "sections" / "intro.tex"));
    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "bibliography.bib"));
    EXPECT_FALSE(std::filesystem::exists(temp_dir_ / "main.aux"));
}

TEST_F(Test_Cleaner, DryRunDeletesNothing) {
    write_file(temp_dir_ / "main.aux");
    write_file(temp_dir_ / "_minted-main" / "default.pygstyle");

    const auto report = notex::Manager::clean(temp_dir_, /*dry_run=*/true);

    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "main.aux"));
    EXPECT_TRUE(std::filesystem::exists(temp_dir_ / "_minted-main"));
    EXPECT_EQ(report.removed_files.size(), 1u);
    EXPECT_EQ(report.removed_directories.size(), 1u);
}

TEST_F(Test_Cleaner, ReportCountsAreAccurate) {
    write_file(temp_dir_ / "a.aux");
    write_file(temp_dir_ / "b.log");
    write_file(temp_dir_ / "c.toc");
    write_file(temp_dir_ / "_minted-main" / "x.pygstyle");
    write_file(temp_dir_ / "keep.tex");

    const auto report = notex::Manager::clean(temp_dir_);

    EXPECT_EQ(report.removed_files.size(), 3u);
    EXPECT_EQ(report.removed_directories.size(), 1u);
    EXPECT_EQ(report.total_removed(), 4u);
}

TEST_F(Test_Cleaner, NonexistentDirectoryYieldsEmptyReport) {
    const auto report =
        notex::Manager::clean(temp_dir_ / "does" / "not" / "exist");
    EXPECT_EQ(report.total_removed(), 0u);
}

}  // namespace
