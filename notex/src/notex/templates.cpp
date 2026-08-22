/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements templates: the encoded project skeletons used by
 * Manager::init.
 */

#include "notex/templates.hpp"

#include <cctype>
#include <cstddef>

namespace notex::templates {

namespace {

std::string replace_all(std::string text, std::string_view token,
                         std::string_view replacement) {
    std::size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::string::npos) {
        text.replace(pos, token.size(), replacement);
        pos += replacement.size();
    }
    return text;
}

std::string slugify(std::string_view title) {
    std::string result;
    result.reserve(title.size());
    bool last_was_separator = true;  // Avoids a leading underscore.
    for (const char raw : title) {
        const auto c = static_cast<unsigned char>(raw);
        if (std::isalnum(c)) {
            result.push_back(static_cast<char>(std::tolower(c)));
            last_was_separator = false;
        } else if (!last_was_separator) {
            result.push_back('_');
            last_was_separator = true;
        }
    }
    while (!result.empty() && result.back() == '_') { result.pop_back(); }
    return result;
}

constexpr std::string_view kMonoMainTemplate =
    R"NOTEX(\documentclass[light]{@CLASS@}

% PREAMBLE ────────────────────────────────────────────────────────────────

% Additional packages here

% Title, Author and Date
\title{My Notes}
\author{}
\date{\today}

% DOCUMENT ────────────────────────────────────────────────────────────────
\begin{document}

% Title page
\maketitle
\tableofcontents

% Sections

\section{Introduction}



\end{document}
)NOTEX";

constexpr std::string_view kMultiMainTemplate =
    R"NOTEX(\documentclass[light]{@CLASS@}

% PREAMBLE ────────────────────────────────────────────────────────────────

% Additional packages here
\usepackage{subfiles} % Modular document structure (best loaded last)

% Title, Author and Date
\title{My Notes}
\author{}
\date{\today}

% DOCUMENT ────────────────────────────────────────────────────────────────
\begin{document}

% Title page
\maketitle
\tableofcontents

% Sections
\pagebreak
@SECTIONS@
\end{document}
)NOTEX";

constexpr std::string_view kSectionTemplate =
    R"NOTEX(\documentclass[../main.tex]{subfiles}

\begin{document}

% Section @NUMBER@
\section{@TITLE@}



\end{document}
)NOTEX";

constexpr std::string_view kBibliographyStarter =
    R"NOTEX(@misc{key,
    author = "",
    title  = "",
    url    = "",
}
)NOTEX";

}  // namespace

std::string_view to_string(ProjectType type) {
    switch (type) {
        case ProjectType::MONO:
            return "mono";
        case ProjectType::MULTI:
            return "multi";
    }
    return "multi";
}

std::optional<ProjectType> project_type_from_string(std::string_view name) {
    if (name == "mono") return ProjectType::MONO;
    if (name == "multi") return ProjectType::MULTI;
    return std::nullopt;
}

std::string mono_main(std::string_view class_path) {
    return replace_all(std::string(kMonoMainTemplate), "@CLASS@", class_path);
}

std::string multi_main(std::string_view class_path,
                       const std::vector<std::string>& section_stems) {
    std::string sections_block;
    for (std::size_t i = 0; i < section_stems.size(); ++i) {
        if (i != 0) sections_block += "\\pagebreak\n";
        sections_block += "\\subfile{sections/" + section_stems[i] + "}\n";
    }

    std::string result =
        replace_all(std::string(kMultiMainTemplate), "@CLASS@", class_path);
    return replace_all(std::move(result), "@SECTIONS@", sections_block);
}

std::string section_stem(int number, std::string_view title) {
    return std::to_string(number) + "_" + slugify(title);
}

std::string section(int number, std::string_view title) {
    std::string result =
        replace_all(std::string(kSectionTemplate), "@TITLE@", title);
    return replace_all(std::move(result), "@NUMBER@", std::to_string(number));
}

std::string bibliography_starter() {
    return std::string(kBibliographyStarter);
}

}  // namespace notex::templates
