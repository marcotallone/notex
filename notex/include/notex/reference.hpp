/**
 * @file reference.hpp
 * @brief reference: the table of ready-to-paste snippets served by
 *        `notex get`.
 */

#pragma once

#include <string_view>
#include <vector>

namespace notex::reference {

/// One ready-to-paste snippet served by `notex get`.
struct Snippet {
    std::string_view name;     ///< Canonical name, e.g. "cbox".
    std::string_view content;  ///< Ready-to-paste LaTeX.
};

/// @return Every snippet known to `notex get`, one entry per canonical
///         name, in a stable order.
const std::vector<Snippet>& all_snippets();

/// @return Every distinct name a snippet can be looked up by — both
///         canonical names and aliases, such as "box" for "cbox" and
///         "ebox" — sorted alphabetically with duplicates removed.
std::vector<std::string_view> snippet_names();

/**
 * @brief Looks up every snippet registered under @p name, whether as
 *        its canonical name or as an alias.
 *
 * A name can resolve to more than one snippet: `notex get box` returns
 * both `cbox` and `ebox`, since a user asking for "a box" may not
 * remember which specific name they want.
 *
 * @param name Name to look up.
 * @return Every matching snippet, in registration order; empty if
 *         @p name matches nothing.
 */
std::vector<Snippet> find_snippets(std::string_view name);

}  // namespace notex::reference
