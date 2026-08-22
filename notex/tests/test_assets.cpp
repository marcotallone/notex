/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests that the assets embedded into the binary at build time faithfully
 * reproduce the real files under latex/, which is what guards the
 * asset-embedding pipeline against silent drift.
 */

#include "notex/assets.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

namespace {

namespace fs = std::filesystem;

std::string read_file(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

TEST(Test_Assets, ClsContentMatchesRepoFile) {
    const fs::path cls_path = fs::path(NOTEX_LATEX_DIR) / "notex.cls";
    const std::string expected = read_file(cls_path);

    const auto embedded = notex::assets::find_latex_file("notex.cls");
    ASSERT_TRUE(embedded.has_value());
    EXPECT_EQ(std::string(embedded->content), expected);
}

TEST(Test_Assets, AllRepoLatexFilesEmbedded) {
    std::set<std::string> repo_files;
    for (const auto& entry : fs::directory_iterator(NOTEX_LATEX_DIR)) {
        if (entry.is_regular_file()) {
            repo_files.insert(entry.path().filename().string());
        }
    }

    std::set<std::string> embedded_files;
    for (const auto& file : notex::assets::latex_files()) {
        embedded_files.insert(std::string(file.name));
    }

    EXPECT_EQ(embedded_files, repo_files);
}

TEST(Test_Assets, FindLatexFileReturnsNulloptForUnknownName) {
    EXPECT_FALSE(
        notex::assets::find_latex_file("does-not-exist.tex").has_value());
}

TEST(Test_Assets, FontFilesMatchRepoBytes) {
    std::set<std::string> repo_files;
    for (const auto& entry : fs::directory_iterator(NOTEX_FONTS_DIR)) {
        if (entry.is_regular_file()) {
            repo_files.insert(entry.path().filename().string());
        }
    }

    std::set<std::string> embedded_files;
    for (const auto& file : notex::assets::font_files()) {
        embedded_files.insert(std::string(file.name));

        const fs::path repo_path = fs::path(NOTEX_FONTS_DIR) / file.name;
        EXPECT_EQ(std::string(file.content), read_file(repo_path))
            << "font content mismatch for " << file.name;
    }

    EXPECT_EQ(embedded_files, repo_files);
}

TEST(Test_Assets, FindFontFileReturnsNulloptForUnknownName) {
    EXPECT_FALSE(
        notex::assets::find_font_file("does-not-exist.ttf").has_value());
}

}  // namespace
