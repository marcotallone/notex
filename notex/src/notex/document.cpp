/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements Document: line-oriented reading, anchor lookup, and atomic
 * writing for a single .tex file.
 */

#include "notex/document.hpp"

#include <algorithm>
#include <fstream>
#include <system_error>

#include "notex/errors.hpp"
#include "notex/logging.hpp"

namespace notex {

namespace {

std::string_view trim(std::string_view text) {
    const std::size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string_view::npos) return {};
    const std::size_t end = text.find_last_not_of(" \t");
    return text.substr(begin, end - begin + 1);
}

}  // namespace

Document::Document(std::filesystem::path path, std::vector<std::string> lines)
    : path_(std::move(path)), lines_(std::move(lines)) {}

Document Document::load(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream) {
        throw FilesystemError("could not open '" + path.string() + "'");
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line)) { lines.push_back(line); }

    return Document(path, std::move(lines));
}

std::vector<std::size_t> Document::find_all(
    const LinePredicate& predicate) const {
    std::vector<std::size_t> matches;
    for (std::size_t i = 0; i < lines_.size(); ++i) {
        if (predicate(lines_[i])) matches.push_back(i);
    }
    return matches;
}

std::size_t Document::find_unique(const LinePredicate& predicate,
                                  std::string_view description) const {
    const std::vector<std::size_t> matches = find_all(predicate);
    PLOG_DEBUG << "find_unique(" << description << ") in '" << path_.string()
               << "': " << matches.size() << " match(es).";

    if (matches.empty()) {
        throw DocumentError("could not find " + std::string(description) +
                            " in '" + path_.string() +
                            "'; the document may have been restructured by "
                            "hand.");
    }
    if (matches.size() > 1) {
        throw DocumentError(
            "found " + std::to_string(matches.size()) + " lines matching " +
            std::string(description) + " in '" + path_.string() +
            "', expected exactly one; please resolve the ambiguity by "
            "hand.");
    }

    return matches.front();
}

std::optional<std::size_t> Document::find_last(
    const LinePredicate& predicate) const {
    const std::vector<std::size_t> matches = find_all(predicate);
    if (matches.empty()) return std::nullopt;
    return matches.back();
}

void Document::insert_line(std::size_t index, std::string text) {
    lines_.insert(lines_.begin() + static_cast<std::ptrdiff_t>(index),
                 std::move(text));
}

void Document::insert_lines(std::size_t index,
                            std::vector<std::string> new_lines) {
    lines_.insert(lines_.begin() + static_cast<std::ptrdiff_t>(index),
                 std::make_move_iterator(new_lines.begin()),
                 std::make_move_iterator(new_lines.end()));
}

void Document::remove_line(std::size_t index) {
    lines_.erase(lines_.begin() + static_cast<std::ptrdiff_t>(index));
}

void Document::set_documentclass_option(
    std::string_view option,
    const std::vector<std::string>& mutually_exclusive_group) {
    const std::size_t index = find_unique(
        [](const std::string& line) {
            return trim(line).starts_with("\\documentclass");
        },
        "a unique \\documentclass line");

    const std::string& line = lines_[index];
    const std::size_t brace_pos = line.find('{');
    if (brace_pos == std::string::npos) {
        throw DocumentError("malformed \\documentclass line in '" +
                            path_.string() + "': missing '{'.");
    }

    const std::size_t bracket_start = line.find('[');
    std::string prefix;
    std::string suffix;
    std::vector<std::string> options;

    if (bracket_start != std::string::npos && bracket_start < brace_pos) {
        const std::size_t bracket_end = line.find(']', bracket_start);
        if (bracket_end == std::string::npos || bracket_end > brace_pos) {
            throw DocumentError("malformed \\documentclass line in '" +
                                path_.string() + "': unmatched '['.");
        }
        prefix = line.substr(0, bracket_start);
        suffix = line.substr(bracket_end + 1);

        std::string_view remaining =
            std::string_view(line).substr(bracket_start + 1,
                                          bracket_end - bracket_start - 1);
        while (!remaining.empty()) {
            const std::size_t comma = remaining.find(',');
            const std::string_view token =
                trim(remaining.substr(0, comma));
            if (!token.empty()) options.emplace_back(token);
            if (comma == std::string_view::npos) break;
            remaining = remaining.substr(comma + 1);
        }
    } else {
        prefix = line.substr(0, brace_pos);
        suffix = line.substr(brace_pos);
    }

    std::vector<std::string> rewritten;
    rewritten.emplace_back(option);
    for (const std::string& existing : options) {
        const bool excluded =
            existing == option ||
            std::find(mutually_exclusive_group.begin(),
                     mutually_exclusive_group.end(),
                     existing) != mutually_exclusive_group.end();
        if (!excluded) rewritten.push_back(existing);
    }

    std::string joined;
    for (std::size_t i = 0; i < rewritten.size(); ++i) {
        if (i != 0) joined += ", ";
        joined += rewritten[i];
    }

    lines_[index] = prefix + "[" + joined + "]" + suffix;
}

void Document::save() const {
    const std::filesystem::path temp_path =
        path_.string() + ".notex.tmp";

    PLOG_DEBUG << "Atomically writing '" << path_.string() << "' via '"
               << temp_path.string() << "'.";
    {
        std::ofstream stream(temp_path, std::ios::binary);
        if (!stream) {
            throw FilesystemError("could not write '" + temp_path.string() +
                                  "'");
        }
        for (const std::string& line : lines_) { stream << line << '\n'; }
    }

    std::error_code ec;
    std::filesystem::rename(temp_path, path_, ec);
    if (ec) {
        std::error_code ignored;
        std::filesystem::remove(temp_path, ignored);
        throw FilesystemError("could not save '" + path_.string() +
                              "': " + ec.message());
    }
    PLOG_DEBUG << "Saved '" << path_.string() << "'.";
}

}  // namespace notex
