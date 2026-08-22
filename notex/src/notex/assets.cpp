/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Implements the notex::assets accessors declared in notex/assets.hpp.
 * This is the only translation unit that includes the generated,
 * build-time embedded-asset headers.
 */

#include "notex/assets.hpp"

#include "notex/embedded_fonts.hpp"
#include "notex/embedded_latex.hpp"

#include <string>

namespace notex::assets {

namespace {

/// Decodes a lowercase-hex string, as produced by CMake's
/// `file(READ ... HEX)`, back into raw bytes.
std::string decode_hex(std::string_view hex) {
    const auto nibble = [](char c) -> unsigned {
        return c <= '9' ? static_cast<unsigned>(c - '0')
                         : static_cast<unsigned>(c - 'a' + 10);
    };

    std::string decoded;
    decoded.reserve(hex.size() / 2);
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        decoded.push_back(
            static_cast<char>((nibble(hex[i]) << 4) | nibble(hex[i + 1])));
    }
    return decoded;
}

/// Decoded font bytes, kept alive for the lifetime of the program so that
/// the EmbeddedFile views font_files() returns stay valid. Built once,
/// in full, before any view into it is taken.
const std::vector<std::string>& decoded_font_bytes() {
    static const std::vector<std::string> storage = [] {
        std::vector<std::string> result;
        result.reserve(detail::embedded_fonts().size());
        for (const auto& raw : detail::embedded_fonts()) {
            result.push_back(decode_hex(raw.hex_content));
        }
        return result;
    }();
    return storage;
}

}  // namespace

const std::vector<EmbeddedFile>& latex_files() {
    static const std::vector<EmbeddedFile> files(
        detail::embedded_latex_files().begin(),
        detail::embedded_latex_files().end());
    return files;
}

std::optional<EmbeddedFile> find_latex_file(std::string_view name) {
    for (const auto& file : latex_files()) {
        if (file.name == name) return file;
    }
    return std::nullopt;
}

const std::vector<EmbeddedFile>& font_files() {
    static const std::vector<EmbeddedFile> files = [] {
        std::vector<EmbeddedFile> result;
        const auto& raw_files = detail::embedded_fonts();
        const auto& decoded = decoded_font_bytes();
        result.reserve(raw_files.size());
        for (std::size_t i = 0; i < raw_files.size(); ++i) {
            result.push_back(EmbeddedFile{raw_files[i].name, decoded[i]});
        }
        return result;
    }();
    return files;
}

std::optional<EmbeddedFile> find_font_file(std::string_view name) {
    for (const auto& file : font_files()) {
        if (file.name == name) return file;
    }
    return std::nullopt;
}

}  // namespace notex::assets
