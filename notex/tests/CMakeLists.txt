# TESTS CMAKE FILE ─────────────────────────────────────────────────────────────

message(STATUS "──────────────────────────────────────────────────────────────")
message(STATUS "Configuring notex_tests...")

# Find or fetch Google Test
include(GoogleTest)
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
  message(STATUS "")
  message(STATUS "Fetching Google Test v1.15.0...")
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip)
  set(gtest_disable_pthreads
      OFF
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

message(STATUS "")
message(STATUS "Setting up tests...")

# Every suite is collected into a single notex_tests binary, one file per
# component, following the Test_<Component>_<Functionality> naming
# convention (see docs/guidelines).
set(NOTEX_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_errors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_assets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_environment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cleaner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_installer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_templates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_document.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_theme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sections.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_bibliography.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_reference.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_checkhealth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_set.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_uninstall.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_reset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_delete.cpp)

add_executable(notex_tests ${NOTEX_TEST_SOURCES})
target_include_directories(notex_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(notex_tests PRIVATE notex_core GTest::gtest
                                           GTest::gtest_main)
target_compile_definitions(
  notex_tests PRIVATE NOTEX_REPO_DIR="${PROJECT_ROOT_DIR}"
                       NOTEX_LATEX_DIR="${PROJECT_ROOT_DIR}/notex/latex"
                       NOTEX_FONTS_DIR="${PROJECT_ROOT_DIR}/notex/fonts")
add_compilation_flags(notex_tests)

gtest_discover_tests(notex_tests)

message(STATUS "✓ notex_tests configured")
