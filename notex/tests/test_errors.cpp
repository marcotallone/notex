/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Tests for notex::ExitCode and the NotexError exception hierarchy.
 */

#include "notex/errors.hpp"

#include <gtest/gtest.h>

namespace {

// The numeric value of each ExitCode is part of the program's external
// contract (it becomes the process's return value), so it must never
// change silently.
TEST(Test_Errors, ExitCodesAreStable) {
    EXPECT_EQ(static_cast<int>(notex::ExitCode::SUCCESS), 0);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::FAILURE), 1);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::USAGE_ERROR), 2);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::ENVIRONMENT_ERROR), 3);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::PROJECT_NOT_FOUND_ERROR), 4);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::FILESYSTEM_ERROR), 5);
    EXPECT_EQ(static_cast<int>(notex::ExitCode::CONFIG_ERROR), 6);
}

TEST(Test_Errors, DerivedErrorsCarryTheirExitCode) {
    EXPECT_EQ(notex::UsageError("bad").exit_code(),
              notex::ExitCode::USAGE_ERROR);
    EXPECT_EQ(notex::EnvironmentError("bad").exit_code(),
              notex::ExitCode::ENVIRONMENT_ERROR);
    EXPECT_EQ(notex::ProjectNotFoundError("bad").exit_code(),
              notex::ExitCode::PROJECT_NOT_FOUND_ERROR);
    EXPECT_EQ(notex::FilesystemError("bad").exit_code(),
              notex::ExitCode::FILESYSTEM_ERROR);
    EXPECT_EQ(notex::ConfigError("bad").exit_code(),
              notex::ExitCode::CONFIG_ERROR);
}

TEST(Test_Errors, WhatReturnsTheOriginalMessage) {
    const notex::UsageError error("missing required argument");
    EXPECT_STREQ(error.what(), "missing required argument");
}

}  // namespace
