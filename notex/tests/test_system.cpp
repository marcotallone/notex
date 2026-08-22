/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for notex::system's command execution and environment-variable
 * access, using only trivial, portable POSIX commands (no TeX required).
 */

#include "notex/system.hpp"

#include <gtest/gtest.h>

#include <cstdlib>

#include "fixtures.hpp"
#include "notex/errors.hpp"

namespace {

TEST(Test_System, RunCommandCapturesStdout) {
    const std::string output = notex::system::run_command("echo hello");
    EXPECT_EQ(output, "hello");
}

TEST(Test_System, RunCommandThrowsOnFailure) {
    EXPECT_THROW(notex::system::run_command("false"), notex::NotexError);
}

TEST(Test_System, GetEnvReturnsSetValue) {
    notex::testing::EnvVarGuard guard("NOTEX_TEST_VAR", "some-value");
    const auto value = notex::system::get_env("NOTEX_TEST_VAR");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "some-value");
}

TEST(Test_System, GetEnvReturnsNulloptWhenUnset) {
    ::unsetenv("NOTEX_TEST_VAR_UNSET");
    EXPECT_FALSE(notex::system::get_env("NOTEX_TEST_VAR_UNSET").has_value());
}

}  // namespace
