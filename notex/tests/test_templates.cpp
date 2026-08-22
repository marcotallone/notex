/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for templates: the encoded project skeletons used by
 * Manager::init.
 */

#include "notex/templates.hpp"

#include <gtest/gtest.h>

namespace {

using notex::templates::ProjectType;

TEST(Test_Templates, MonoContainsDocumentclassAndEndDocument) {
    const std::string content = notex::templates::mono_main("notex");
    EXPECT_NE(content.find("\\documentclass[light]{notex}"),
             std::string::npos);
    EXPECT_NE(content.find("\\begin{document}"), std::string::npos);
    EXPECT_NE(content.find("\\end{document}"), std::string::npos);
}

TEST(Test_Templates, MonoSubstitutesClassPath) {
    const std::string content = notex::templates::mono_main("settings/notex");
    EXPECT_NE(content.find("\\documentclass[light]{settings/notex}"),
             std::string::npos);
    EXPECT_EQ(content.find("@CLASS@"), std::string::npos);
}

TEST(Test_Templates, MultiReferencesFirstSection) {
    const std::string content = notex::templates::multi_main("notex");
    EXPECT_NE(content.find("\\usepackage{subfiles}"), std::string::npos);
    EXPECT_NE(content.find("\\subfile{sections/1_introduction}"),
             std::string::npos);
}

TEST(Test_Templates, SectionPlaceholderSubstitution) {
    const std::string content = notex::templates::section(3, "My Title");
    EXPECT_NE(content.find("\\section{My Title}"), std::string::npos);
    EXPECT_NE(content.find("% Section 3"), std::string::npos);
    EXPECT_EQ(content.find("@TITLE@"), std::string::npos);
    EXPECT_EQ(content.find("@NUMBER@"), std::string::npos);
}

TEST(Test_Templates, SectionStemSlugifiesTitle) {
    EXPECT_EQ(notex::templates::section_stem(1, "Introduction"),
             "1_introduction");
    EXPECT_EQ(notex::templates::section_stem(3, "My Title"), "3_my_title");
    EXPECT_EQ(notex::templates::section_stem(2, "  Weird   Spacing! "),
             "2_weird_spacing");
}

TEST(Test_Templates, BibliographyStarterContainsAnEntry) {
    const std::string content = notex::templates::bibliography_starter();
    EXPECT_NE(content.find('@'), std::string::npos);
}

TEST(Test_Templates, ProjectTypeToStringRoundTrips) {
    EXPECT_EQ(notex::templates::to_string(ProjectType::MONO), "mono");
    EXPECT_EQ(notex::templates::to_string(ProjectType::MULTI), "multi");
    EXPECT_EQ(notex::templates::project_type_from_string("mono"),
             ProjectType::MONO);
    EXPECT_EQ(notex::templates::project_type_from_string("multi"),
             ProjectType::MULTI);
    EXPECT_FALSE(
        notex::templates::project_type_from_string("bogus").has_value());
}

}  // namespace
