/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for reference: the snippet table served by `notex get`.
 */

#include "notex/reference.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

TEST(Test_Reference, GetInfoReturnsCalloutSnippet) {
    const auto snippets = notex::reference::find_snippets("info");
    ASSERT_EQ(snippets.size(), 1u);
    EXPECT_EQ(snippets.front().name, "info");
    EXPECT_NE(snippets.front().content.find("\\begin{info}"),
             std::string_view::npos);
}

TEST(Test_Reference, GetBoxReturnsMultipleSnippets) {
    const auto snippets = notex::reference::find_snippets("box");
    ASSERT_EQ(snippets.size(), 2u);

    std::vector<std::string_view> names;
    for (const auto& snippet : snippets) { names.push_back(snippet.name); }
    EXPECT_NE(std::find(names.begin(), names.end(), "cbox"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "ebox"), names.end());
}

TEST(Test_Reference, FindSnippetsReturnsEmptyForUnknownName) {
    EXPECT_TRUE(notex::reference::find_snippets("not-a-real-snippet").empty());
}

TEST(Test_Reference, SnippetNamesIsSortedAndDeduplicated) {
    const auto names = notex::reference::snippet_names();
    ASSERT_FALSE(names.empty());
    EXPECT_TRUE(std::is_sorted(names.begin(), names.end()));
    EXPECT_EQ(std::adjacent_find(names.begin(), names.end()), names.end());
}

TEST(Test_Reference, SnippetNamesIncludesAliases) {
    const auto names = notex::reference::snippet_names();
    EXPECT_NE(std::find(names.begin(), names.end(), "box"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "cbox"), names.end());
}

TEST(Test_Reference, AllSnippetsContentIsNonEmpty) {
    const auto& snippets = notex::reference::all_snippets();
    ASSERT_FALSE(snippets.empty());
    for (const auto& snippet : snippets) {
        EXPECT_FALSE(snippet.name.empty());
        EXPECT_FALSE(snippet.content.empty());
    }
}

}  // namespace
