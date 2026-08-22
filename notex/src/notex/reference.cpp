/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements reference: the encoded snippet table served by `notex get`.
 */

#include "notex/reference.hpp"

#include <algorithm>

namespace notex::reference {

namespace {

/// One registered snippet, before alias resolution flattens it into the
/// public Snippet type. See notex/latex/notex-callouts.sty and
/// notex/latex/notex-code.sty for the environments these mirror; kept in step
/// by hand whenever either changes.
struct Entry {
    std::string_view name;
    std::vector<std::string_view> aliases;
    std::string_view content;
};

const std::vector<Entry>& entries() {
    static const std::vector<Entry> kEntries = {
        {"info",
         {},
         "\\begin{info}[OPTIONAL_COLOR]\n    WRITE_HERE\n\\end{info}\n"},
        {"warning",
         {},
         "\\begin{warning}[OPTIONAL_COLOR]\n    WRITE_HERE\n"
         "\\end{warning}\n"},
        {"note",
         {},
         "\\begin{note}[OPTIONAL_COLOR]\n    WRITE_HERE\n\\end{note}\n"},
        {"tip",
         {},
         "\\begin{tip}[OPTIONAL_COLOR]\n    WRITE_HERE\n\\end{tip}\n"},
        {"important",
         {},
         "\\begin{important}[OPTIONAL_COLOR]\n    WRITE_HERE\n"
         "\\end{important}\n"},
        {"caution",
         {},
         "\\begin{caution}[OPTIONAL_COLOR]\n    WRITE_HERE\n"
         "\\end{caution}\n"},
        {"cbox",
         {"box"},
         "\\begin{cbox}[OPTIONAL_COLOR]\n    WRITE_HERE\n\\end{cbox}\n"},
        {"ebox",
         {"box"},
         "\\begin{ebox}[OPTIONAL_COLOR]\n    WRITE_HERE\n\\end{ebox}\n"},
        {"example",
         {},
         "\\begin{example}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{example}\n"},
        {"definition",
         {},
         "\\begin{definition}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{definition}\n"},
        {"theorem",
         {},
         "\\begin{theorem}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{theorem}\n"},
        {"corollary",
         {},
         "\\begin{corollary}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{corollary}\n"},
        {"proposition",
         {},
         "\\begin{proposition}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{proposition}\n"},
        {"lemma",
         {},
         "\\begin{lemma}[OPTIONAL_TITLE][OPTIONAL_LABEL]\n"
         "    WRITE_HERE\n\\end{lemma}\n"},
        {"code",
         {},
         "\\begin{code}[caption = OPTIONAL_CAPTION, label = OPTIONAL_LABEL]"
         "{LANGUAGE}\n    WRITE_HERE\n\\end{code}\n"},
        {"python",
         {},
         "\\begin{python}[caption = OPTIONAL_CAPTION, label = OPTIONAL_LABEL]"
         "\n    WRITE_HERE\n\\end{python}\n"},
        {"cpp",
         {},
         "\\begin{cpp}[caption = OPTIONAL_CAPTION, label = OPTIONAL_LABEL]"
         "\n    WRITE_HERE\n\\end{cpp}\n"},
        {"bash",
         {},
         "\\begin{bash}[caption = OPTIONAL_CAPTION, label = OPTIONAL_LABEL]"
         "\n    WRITE_HERE\n\\end{bash}\n"},
        {"pseudocode",
         {},
         "\\begin{pseudocode}[caption = OPTIONAL_CAPTION, "
         "label = OPTIONAL_LABEL]\n    WRITE_HERE\n\\end{pseudocode}\n"},
        {"cc", {}, "\\cc{WRITE_HERE}\n"},
        {"inline", {}, "\\inline[LANGUAGE]{WRITE_HERE}\n"},
    };
    return kEntries;
}

}  // namespace

const std::vector<Snippet>& all_snippets() {
    static const std::vector<Snippet> kSnippets = [] {
        std::vector<Snippet> snippets;
        for (const Entry& entry : entries()) {
            snippets.push_back(Snippet{entry.name, entry.content});
        }
        return snippets;
    }();
    return kSnippets;
}

std::vector<std::string_view> snippet_names() {
    std::vector<std::string_view> names;
    for (const Entry& entry : entries()) {
        names.push_back(entry.name);
        for (const std::string_view alias : entry.aliases) {
            names.push_back(alias);
        }
    }
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    return names;
}

std::vector<Snippet> find_snippets(std::string_view name) {
    std::vector<Snippet> matches;
    for (const Entry& entry : entries()) {
        const bool is_match =
            entry.name == name ||
            std::find(entry.aliases.begin(), entry.aliases.end(), name) !=
                entry.aliases.end();
        if (is_match) { matches.push_back(Snippet{entry.name, entry.content}); }
    }
    return matches;
}

}  // namespace notex::reference
