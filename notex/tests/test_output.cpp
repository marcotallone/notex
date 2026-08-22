/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for the notex::ui terminal output helpers and confirmation prompt.
 */

#include <gtest/gtest.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "notex/output.hpp"

namespace {

/// Redirects std::cout into an in-memory buffer for the lifetime of the
/// object, so a test can inspect exactly what a ui:: helper printed.
class CoutCapture {
public:
    CoutCapture() : old_buffer_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(old_buffer_); }

    CoutCapture(const CoutCapture&) = delete;
    CoutCapture& operator=(const CoutCapture&) = delete;

    std::string str() const { return buffer_.str(); }

private:
    std::ostringstream buffer_;
    std::streambuf* old_buffer_;
};

TEST(Test_Output, SuccessFormatsWithAnsiPrefix) {
    CoutCapture capture;
    notex::ui::success("done");
    const std::string output = capture.str();

    EXPECT_NE(output.find("\033[32m"), std::string::npos)
        << "expected the green ANSI color code, got: " << output;
    EXPECT_NE(output.find("done"), std::string::npos);
}

TEST(Test_Output, ConfirmDefaultsNoWhenNotInteractive) {
    // Whether stdin is a real terminal depends on how the test binary
    // itself was invoked (piped in CI, but possibly a real tty when a
    // developer runs `ctest` directly), so the non-interactive branch is
    // forced deterministically here instead of relying on the process's
    // actual ::isatty() state — otherwise this would block forever
    // waiting on input that never arrives.
    notex::ui::set_assume_yes(false);
    notex::ui::set_stdin_interactive_override_for_testing(false);
    EXPECT_FALSE(notex::ui::confirm("Proceed?", false));
    EXPECT_TRUE(notex::ui::confirm("Proceed?", true));
    notex::ui::set_stdin_interactive_override_for_testing(std::nullopt);
}

TEST(Test_Output, ConfirmAlwaysSucceedsWhenAssumeYesIsSet) {
    notex::ui::set_assume_yes(true);
    EXPECT_TRUE(notex::ui::confirm("Proceed?", false));
    notex::ui::set_assume_yes(false);
}

}  // namespace
