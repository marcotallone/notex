/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for Document: anchor lookup, editing primitives, and atomic
 * writing.
 */

#include "notex/document.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "fixtures.hpp"
#include "notex/errors.hpp"

namespace {

using notex::testing::TempDirFixture;

class Test_Document : public TempDirFixture {};

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << content;
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

TEST_F(Test_Document, LoadThrowsOnMissingFile) {
    EXPECT_THROW(notex::Document::load(temp_dir_ / "nope.tex"),
                notex::FilesystemError);
}

TEST_F(Test_Document, LoadSplitsIntoLines) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "line one\nline two\nline three\n");

    const notex::Document doc = notex::Document::load(path);
    ASSERT_EQ(doc.lines().size(), 3u);
    EXPECT_EQ(doc.lines()[0], "line one");
    EXPECT_EQ(doc.lines()[2], "line three");
}

TEST_F(Test_Document, FindUniqueAnchorThrowsOnDuplicate) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "\\end{document}\nsomething\n\\end{document}\n");

    const notex::Document doc = notex::Document::load(path);
    EXPECT_THROW(doc.find_unique(
                     [](const std::string& l) { return l == "\\end{document}"; },
                     "a unique \\end{document} line"),
                notex::DocumentError);
}

TEST_F(Test_Document, FindUniqueAnchorThrowsWhenMissing) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "nothing to see here\n");

    const notex::Document doc = notex::Document::load(path);
    EXPECT_THROW(doc.find_unique(
                     [](const std::string& l) { return l == "\\end{document}"; },
                     "a unique \\end{document} line"),
                notex::DocumentError);
}

TEST_F(Test_Document, FindUniqueAnchorReturnsIndex) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "one\ntwo\n\\end{document}\n");

    const notex::Document doc = notex::Document::load(path);
    const std::size_t index = doc.find_unique(
        [](const std::string& l) { return l == "\\end{document}"; },
        "a unique \\end{document} line");
    EXPECT_EQ(index, 2u);
}

TEST_F(Test_Document, FindLastReturnsNulloptWhenMissing) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "one\ntwo\n");

    const notex::Document doc = notex::Document::load(path);
    EXPECT_FALSE(doc.find_last([](const std::string& l) { return l == "x"; })
                    .has_value());
}

TEST_F(Test_Document, FindLastReturnsLastMatch) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "match\nother\nmatch\n");

    const notex::Document doc = notex::Document::load(path);
    const auto index =
        doc.find_last([](const std::string& l) { return l == "match"; });
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(*index, 2u);
}

TEST_F(Test_Document, InsertAndRemoveLines) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "one\ntwo\n");

    notex::Document doc = notex::Document::load(path);
    doc.insert_line(1, "inserted");
    ASSERT_EQ(doc.lines().size(), 3u);
    EXPECT_EQ(doc.lines()[1], "inserted");

    doc.remove_line(0);
    ASSERT_EQ(doc.lines().size(), 2u);
    EXPECT_EQ(doc.lines()[0], "inserted");
}

TEST_F(Test_Document, InsertLinesInsertsInOrder) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "start\nend\n");

    notex::Document doc = notex::Document::load(path);
    doc.insert_lines(1, {"a", "b", "c"});
    ASSERT_EQ(doc.lines().size(), 5u);
    EXPECT_EQ(doc.lines()[1], "a");
    EXPECT_EQ(doc.lines()[2], "b");
    EXPECT_EQ(doc.lines()[3], "c");
}

TEST_F(Test_Document, SaveWritesLinesAndPersists) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "one\ntwo\n");

    notex::Document doc = notex::Document::load(path);
    doc.insert_line(1, "middle");
    doc.save();

    EXPECT_EQ(read_file(path), "one\nmiddle\ntwo\n");
}

TEST_F(Test_Document, SetDocumentclassOptionReplacesTheme) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path, "\\documentclass[light]{notex}\n\\end{document}\n");

    notex::Document doc = notex::Document::load(path);
    doc.set_documentclass_option("dark", {"light", "dark", "tokyo", "bw"});

    EXPECT_EQ(doc.lines()[0], "\\documentclass[dark]{notex}");
}

TEST_F(Test_Document, SetDocumentclassOptionPreservesOtherOptions) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path,
              "\\documentclass[light, nocallouts]{notex}\n\\end{document}\n");

    notex::Document doc = notex::Document::load(path);
    doc.set_documentclass_option("dark", {"light", "dark", "tokyo", "bw"});

    EXPECT_EQ(doc.lines()[0], "\\documentclass[dark, nocallouts]{notex}");
}

TEST_F(Test_Document, SetDocumentclassOptionThrowsWhenAmbiguous) {
    const std::filesystem::path path = temp_dir_ / "doc.tex";
    write_file(path,
              "\\documentclass[light]{notex}\n"
              "\\documentclass[light]{notex}\n");

    notex::Document doc = notex::Document::load(path);
    EXPECT_THROW(
        doc.set_documentclass_option("dark", {"light", "dark", "tokyo", "bw"}),
        notex::DocumentError);
}

TEST_F(Test_Document, AtomicWritePreservesFileOnFailure) {
    const std::filesystem::path dir = temp_dir_ / "readonly";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "doc.tex";
    write_file(path, "original content\n");

    notex::Document doc = notex::Document::load(path);
    doc.insert_line(0, "this should never be saved");

    ::chmod(dir.c_str(), 0555);  // Read+execute only: no new files inside.
    EXPECT_THROW(doc.save(), notex::FilesystemError);
    ::chmod(dir.c_str(), 0755);  // Restore so TearDown can remove it.

    EXPECT_EQ(read_file(path), "original content\n");
}

}  // namespace
