/**
 * @file fixtures.hpp
 * @brief Shared GoogleTest fixtures for temporary directories and faked
 *        environment variables.
 *
 * Every test that touches the filesystem or depends on the environment
 * uses these, so that the suite never needs a real TeX distribution and
 * never leaves anything behind on disk.
 */

#pragma once

#include <gtest/gtest.h>

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#include "notex/output.hpp"

namespace notex::testing {

/// Creates a fresh, empty temporary directory before each test and
/// removes it (recursively) afterwards.
class TempDirFixture : public ::testing::Test {
protected:
    void SetUp() override {
        static std::atomic<int> counter{0};
#if defined(__unix__) || defined(__APPLE__)
        const long pid = static_cast<long>(::getpid());
#else
        const long pid = 0;
#endif
        temp_dir_ = std::filesystem::temp_directory_path() /
                    ("notex_test_" + std::to_string(pid) + "_" +
                     std::to_string(counter.fetch_add(1)));
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override {
        std::error_code ignored;
        std::filesystem::remove_all(temp_dir_, ignored);
    }

    std::filesystem::path temp_dir_;
};

/// Temporarily sets (or unsets) an environment variable for the lifetime
/// of the object, restoring its previous value when it goes out of scope.
class EnvVarGuard {
public:
    EnvVarGuard(std::string name, const std::string& value)
        : name_(std::move(name)) {
        if (const char* previous = std::getenv(name_.c_str());
            previous != nullptr) {
            had_previous_value_ = true;
            previous_value_ = previous;
        }
        ::setenv(name_.c_str(), value.c_str(), 1);
    }

    ~EnvVarGuard() {
        if (had_previous_value_) {
            ::setenv(name_.c_str(), previous_value_.c_str(), 1);
        } else {
            ::unsetenv(name_.c_str());
        }
    }

    EnvVarGuard(const EnvVarGuard&) = delete;
    EnvVarGuard& operator=(const EnvVarGuard&) = delete;

private:
    std::string name_;
    bool had_previous_value_ = false;
    std::string previous_value_;
};

/**
 * @brief RAII guard that deterministically forces notex::ui::confirm()'s
 *        non-interactive fallback path for the lifetime of the object.
 *
 * confirm()'s fallback is normally driven by the real ::isatty() state of
 * the process, which makes a test that expects the fallback (the "decline
 * by default" answer) depend on how the test binary itself was invoked:
 * it passes when stdin is piped (as in CI), but blocks forever waiting on
 * input that will never arrive when a developer runs `ctest` directly
 * from an interactive shell. Any test that exercises a confirm()-gated
 * operation without setting notex::ui::set_assume_yes(true) first must
 * hold one of these for the call to be deterministic and hang-free.
 */
class NonInteractiveGuard {
public:
    NonInteractiveGuard() {
        notex::ui::set_stdin_interactive_override_for_testing(false);
    }
    ~NonInteractiveGuard() {
        notex::ui::set_stdin_interactive_override_for_testing(std::nullopt);
    }

    NonInteractiveGuard(const NonInteractiveGuard&) = delete;
    NonInteractiveGuard& operator=(const NonInteractiveGuard&) = delete;
};

}  // namespace notex::testing
