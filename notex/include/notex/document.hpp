/**
 * @file document.hpp
 * @brief Document: a small, safe line-oriented editor for a single .tex
 *        file.
 */

#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace notex {

/**
 * @brief Loads a `.tex` file as a sequence of lines and offers editing
 *        primitives anchored on recognisable landmarks.
 *
 * A Document deliberately refuses to guess: locating an anchor throws if
 * it is missing, or if it appears more than once so that the correct
 * location would be ambiguous, rather than picking one silently. This is
 * what makes it safe to run on documents a user has been editing by
 * hand. Changes are only visible on disk once save() is called, which
 * writes atomically (to a temporary file, then renamed over the
 * original) so that an interrupted write can never leave a half-written
 * file behind.
 */
class Document {
public:
    /// A predicate tested against one line (without its trailing
    /// newline) to decide whether it matches an anchor.
    using LinePredicate = std::function<bool(const std::string&)>;

    /**
     * @brief Loads @p path as a sequence of lines.
     * @throws FilesystemError if @p path cannot be opened for reading.
     */
    static Document load(const std::filesystem::path& path);

    /// @return Every line of the document, in order, without their
    ///         trailing newlines.
    const std::vector<std::string>& lines() const noexcept { return lines_; }

    /// @return The index of every line matching @p predicate, in order.
    std::vector<std::size_t> find_all(const LinePredicate& predicate) const;

    /**
     * @brief Locates the single line matching @p predicate.
     * @param predicate Test applied to each line.
     * @param description Human-readable description of what was being
     *                     looked for, used only to compose the error
     *                     message when this fails.
     * @throws DocumentError if no line matches, or if more than one
     *         does.
     * @return The index of the one matching line.
     */
    std::size_t find_unique(const LinePredicate& predicate,
                            std::string_view description) const;

    /// @return The index of the last line matching @p predicate, or
    ///         std::nullopt if none does. Unlike find_unique(), more
    ///         than one match is not an error.
    std::optional<std::size_t> find_last(const LinePredicate& predicate) const;

    /// Inserts @p text as a new line before the current line @p index.
    void insert_line(std::size_t index, std::string text);

    /// Inserts @p new_lines, in order, before the current line @p index.
    void insert_lines(std::size_t index, std::vector<std::string> new_lines);

    /// Removes the line at @p index.
    void remove_line(std::size_t index);

    /**
     * @brief Rewrites the document's unique `\documentclass` line so
     *        that its bracketed options contain @p option instead of
     *        whichever member of @p mutually_exclusive_group was there,
     *        preserving every other option untouched.
     * @param option Option to make sure is present, e.g. a theme name.
     * @param mutually_exclusive_group The full set @p option is drawn
     *                                  from, e.g. every known theme name;
     *                                  every option in this set is
     *                                  removed before @p option is added.
     * @throws DocumentError if there is not exactly one `\documentclass`
     *         line, or if it is malformed (missing its `{...}` class
     *         argument).
     */
    void set_documentclass_option(
        std::string_view option,
        const std::vector<std::string>& mutually_exclusive_group);

    /**
     * @brief Writes the current lines back to disk atomically.
     * @throws FilesystemError if the temporary file cannot be written or
     *         cannot be renamed over the original.
     */
    void save() const;

private:
    Document(std::filesystem::path path, std::vector<std::string> lines);

    std::filesystem::path path_;
    std::vector<std::string> lines_;
};

}  // namespace notex
