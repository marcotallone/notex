# ROOT CMAKE FILE ──────────────────────────────────────────────────────────────
cmake_minimum_required(VERSION 3.20)
project(
  NoTeX
  VERSION 1.0.0
  DESCRIPTION "A modern noteworthy LaTeX template"
  HOMEPAGE_URL "https://github.com/marcotallone/notex"
  LANGUAGES CXX)

# Custom metadata variables
set(PROJECT_AUTHOR "Marco Tallone")
set(PROJECT_LICENSE "MIT")
set(PROJECT_LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")


# OPTIONS ──────────────────────────────────────────────────────────────────────
option(DEBUG "Enable debug mode" OFF) 
option(VERBOSE "Enable verbose mode" OFF)
option(LOGGING "Enable file logging" OFF)
option(TESTS "Build unit tests" OFF)
option(COVERAGE "Enable code coverage reporting" OFF)
option(DOCS "Enable documentation generation" OFF)

# Validate option dependencies
if(COVERAGE AND NOT TESTS)
  message(FATAL_ERROR "COVERAGE requires TESTS=ON")
endif()

# A debug build should always produce a log file, at least at debug
# severity, without the user needing to pass -DLOGGING=ON separately.
if(DEBUG AND NOT LOGGING)
  set(LOGGING ON)
endif()

# SETTINGS ─────────────────────────────────────────────────────────────────────

# Set project root directory
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Set notex C++ directory
set(NOTEX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/notex)

# Set include directory
set(INCLUDE_DIR ${NOTEX_DIR}/include)

# Set executables directory
# This is where the freshly-built binary lands (./bin/notex); making it
# available system-wide is a separate concern, handled by the notex
# target's install() rule (see notex/src/CMakeLists.txt) and CMake's own
# `cmake --install` step, which respects CMAKE_INSTALL_PREFIX as usual.
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) # -> ./bin/
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin) # -> ./build/bin/

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export the compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set CMake flags for debug and release versions
if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  function(add_compilation_flags target)
    target_compile_options(
      ${target}
      PRIVATE -O0
              -g
              -Wall
              -Werror
              -Wextra
              -Wpedantic
              -Wshadow
              -Wunused
              -Woverloaded-virtual
              -Wconversion
              -Wsign-conversion
              -Wnull-dereference)
    if(COVERAGE)
      target_compile_options(${target} PRIVATE --coverage -fno-inline)
      target_link_options(${target} PRIVATE --coverage)
    endif()
    target_compile_definitions(${target} PRIVATE DEBUG)
    if(VERBOSE)
      target_compile_definitions(${target} PRIVATE VERBOSE)
    endif()
    if(LOGGING)
      target_compile_definitions(${target} PRIVATE LOGGING)
    endif()
    if(TESTS)
      target_compile_definitions(${target} PRIVATE TESTS)
    endif()
    if(COVERAGE)
      target_compile_definitions(${target} PRIVATE COVERAGE)
    endif()
    target_compile_definitions(${target} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    PROJECT_AUTHOR="${PROJECT_AUTHOR}"
    PROJECT_AUTHOR_EMAIL="${PROJECT_AUTHOR_EMAIL}"
    PROJECT_LICENSE="${PROJECT_LICENSE}"
    PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
    )
  endfunction()
else()
  set(CMAKE_BUILD_TYPE Release)
  function(add_compilation_flags target)
    if(COVERAGE)
      # Coverage instrumentation needs unoptimized, non-inlined code even
      # outside of DEBUG builds, so it overrides the usual release flags.
      target_compile_options(${target} PRIVATE -O0 -g --coverage -fno-inline)
      target_link_options(${target} PRIVATE --coverage)
    else()
      target_compile_options(${target} PRIVATE -O3)
    endif()
    if(VERBOSE)
      target_compile_definitions(${target} PRIVATE VERBOSE)
    endif()
    if(LOGGING)
      target_compile_definitions(${target} PRIVATE LOGGING)
    endif()
    if(TESTS)
      target_compile_definitions(${target} PRIVATE TESTS)
    endif()
    if(COVERAGE)
      target_compile_definitions(${target} PRIVATE COVERAGE)
    endif()
    target_compile_definitions(${target} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    PROJECT_AUTHOR="${PROJECT_AUTHOR}"
    PROJECT_AUTHOR_EMAIL="${PROJECT_AUTHOR_EMAIL}"
    PROJECT_LICENSE="${PROJECT_LICENSE}"
    PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
    )
  endfunction()
endif()

# Set verbose Makefile and toggle messages
if(VERBOSE)
  set(CMAKE_RULE_MESSAGES
      ON
      CACHE INTERNAL "")
  set(CMAKE_VERBOSE_MAKEFILE
      ON
      CACHE INTERNAL "")
  set(CMAKE_VERBOSE_MAKEFILE ON)
else()
  set(CMAKE_RULE_MESSAGES
      OFF
      CACHE INTERNAL "")
  set(CMAKE_VERBOSE_MAKEFILE
      OFF
      CACHE INTERNAL "")
  set(CMAKE_VERBOSE_MAKEFILE OFF)
endif()

# EXTERNAL LIBRARIES ───────────────────────────────────────────────────────────
include(FetchContent)

message(STATUS "──────────────────────────────────────────────────────────────")
message(STATUS "Fetching third-party dependencies...")

# CLI11 (Header-only command line parser)
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.6.2
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(CLI11)
message(STATUS "  ✓ CLI11 (fetched)")

# Plog (logger)
FetchContent_Declare(
    plog
    GIT_REPOSITORY https://github.com/SergiusTheBest/plog
    GIT_TAG        1.1.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(plog)
message(STATUS "  ✓ plog (fetched)")

# nlohmann/json (JSON parsing for .notex/notex.json)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)
message(STATUS "  ✓ nlohmann/json (fetched)")

# SOURCE FILES ─────────────────────────────────────────────────────────────────
add_subdirectory(${NOTEX_DIR}/src)

# TESTS ────────────────────────────────────────────────────────────────────────
if(TESTS)
  enable_testing()
  add_subdirectory(${NOTEX_DIR}/tests)
endif()

# COVERAGE REPORT ──────────────────────────────────────────────────────────────
if(COVERAGE)
  find_program(GCOVR_EXECUTABLE gcovr)
  if(GCOVR_EXECUTABLE)
    # COVERAGE already requires TESTS=ON (checked at the top of this file),
    # so notex_tests is always available here. Running it is folded into
    # the target itself, so a single `cmake --build build --target
    # coverage` builds, exercises, and reports on the code in one step.
    # The filter/exclude pair restricts gcovr to the sources notex_core
    # and notex are actually made of: everything under notex/, minus the
    # tests themselves (which are never a meaningful thing to report
    # "coverage" on) and anything generated into the build directory.
    # Third-party dependencies (CLI11, plog, nlohmann/json, GoogleTest),
    # which all live under ${CMAKE_BINARY_DIR}/_deps/, are excluded by
    # construction: the filter never matches their paths in the first
    # place.
    add_custom_target(
      coverage
      COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_BINARY_DIR}/coverage
      COMMAND $<TARGET_FILE:notex_tests>
      COMMAND
        ${GCOVR_EXECUTABLE} --root ${PROJECT_ROOT_DIR} --filter
        "${NOTEX_DIR}/.*" --exclude "${NOTEX_DIR}/tests/.*" --exclude
        "${CMAKE_BINARY_DIR}/.*" --print-summary --html-details
        ${CMAKE_BINARY_DIR}/coverage/index.html
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      DEPENDS notex_tests
      COMMENT "Running notex_tests and generating a coverage report with gcovr"
      VERBATIM)
    message(STATUS "  ✓ coverage target available (run: cmake --build build --target coverage)")
  else()
    message(WARNING "COVERAGE is ON but gcovr was not found; the 'coverage' "
                     "target will not be available. Install gcovr to "
                     "generate a report.")
  endif()
endif()

# DOCUMENTATION ────────────────────────────────────────────────────────────────
if(DOCS)
  message(STATUS "──────────────────────────────────────────────────────────────")
  find_package(Doxygen REQUIRED)

  if(DOXYGEN_FOUND)
    # Doxygen Awesome CSS: a modern GitHub-esque theme (collapsible sidebar,
    # dark/light toggle, tabs, cleaner typography). Fetched here rather than
    # alongside CLI11/plog/nlohmann_json above so it's only ever downloaded
    # when -DDOCS=ON is actually passed, matching the theme's own documented
    # FetchContent recipe: declare, make available, then read back its
    # source dir to point HTML_EXTRA_STYLESHEET/HTML_HEADER at it.
    FetchContent_Declare(
      doxygen-awesome-css
      GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
      GIT_TAG        v2.4.2
      GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css
                                SOURCE_DIR DOXYGEN_AWESOME_CSS_DIR)
    message(STATUS "  ✓ doxygen-awesome-css (fetched)")

    # Single-file JS assets the docs pull in from outside both Doxygen's own
    # bundled resources and the doxygen-awesome-css checkout above, fetched
    # (once per build directory) the same way rather than loaded from a CDN
    # at view time. Always the classic global-exposing bundle, never an ES
    # module: a hand-rolled `<script type="module">` here previously broke
    # outright once the generated page was opened directly via file://,
    # which browsers block cross-origin ES module imports from.
    function(notex_download_docs_asset out_var_name display_name url filename)
      set(asset_path "${CMAKE_CURRENT_BINARY_DIR}/${filename}")
      if(NOT EXISTS "${asset_path}")
        file(DOWNLOAD "${url}" "${asset_path}" STATUS download_status)
        list(GET download_status 0 download_code)
        if(NOT download_code EQUAL 0)
          file(REMOVE "${asset_path}")
          message(WARNING "Failed to download ${filename}; ${display_name} "
                           "will not work correctly in the generated docs.")
        endif()
      endif()
      message(STATUS "  ✓ ${display_name} (downloaded)")
      set(${out_var_name} "${asset_path}" PARENT_SCOPE)
    endfunction()

    # Doxygen's own $mermaidjs placeholder only offers Mermaid's stock
    # 'default'/'dark' themes with no way to ask for anything closer to
    # GitHub's own flat, grayscale rendering, so header.html calls
    # mermaid.initialize() itself instead and needs the library file
    # locally rather than through that placeholder.
    notex_download_docs_asset(MERMAID_JS_PATH "mermaid.js v11.16.0"
      "https://cdn.jsdelivr.net/npm/mermaid@11.16.0/dist/mermaid.min.js"
      "mermaid.min.js")

    # jQuery: doxygen-awesome-css v2.4.2 (the pinned, and current latest,
    # release) documents itself as tested only against "Doxygen 1.9.1-1.9.4
    # and 1.9.6-1.16.1" — its own darkmode-toggle/fragment-copy-button/etc.
    # extensions are written against jQuery's `$`, which Doxygen itself
    # bundled and referenced from HTML_HEADER as $relpath^jquery.js right
    # up through that range. Doxygen 1.17 dropped jQuery entirely (confirmed
    # against its actual shipped template resource list), so on 1.17+ that
    # `$` reference 404s, throws ReferenceError on the very first extension
    # .init() call in header.html, and silently aborts every statement
    # after it in the same <script> block, including the mermaid.initialize()
    # call above. Rather than wait on upstream to catch up, jQuery is
    # self-hosted here to satisfy the theme's dependency regardless of
    # what any given Doxygen version happens to bundle.
    notex_download_docs_asset(JQUERY_JS_PATH "jQuery 3.7.1"
      "https://cdn.jsdelivr.net/npm/jquery@3.7.1/dist/jquery.min.js"
      "jquery.min.js")

    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")
    set(DOXYGEN_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md")
    set(DOXYGEN_INPUT_PATHS
        "${NOTEX_DIR}/include"
        "${NOTEX_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/implementation"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/installation"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/usage"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen"
    )
    string(REPLACE ";" " " DOXYGEN_INPUT_PATHS "${DOXYGEN_INPUT_PATHS}")

    set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/Doxyfile")
    set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)

    add_custom_target(
      docs
      COMMAND ${DOXYGEN_EXECUTABLE} "${DOXYFILE_OUT}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)

    message(
      STATUS "  ✓ docs target configured: ${DOXYGEN_OUTPUT_DIRECTORY}/html")
  else()
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()
endif()

# SUMMARY ──────────────────────────────────────────────────────────────────────
message(STATUS "──────────────────────────────────────────────────────────────")
message(STATUS "Build Summary")
message(STATUS "  Debug:    ${DEBUG}")
message(STATUS "  Verbose:  ${VERBOSE}")
message(STATUS "  Logging:  ${LOGGING}")
message(STATUS "  Tests:    ${TESTS}")
message(STATUS "  Coverage: ${COVERAGE}")
message(STATUS "  Notex:    ${NOTEX_DIR}")
message(STATUS "  Includes: ${INCLUDE_DIR}")
message(STATUS "  Binaries: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "──────────────────────────────────────────────────────────────")
message(STATUS "Compile with:")
message(STATUS "  cmake --build build --parallel $(nproc)")
message(STATUS "Install with:")
message(STATUS "  (sudo) cmake --install build")
message(STATUS "──────────────────────────────────────────────────────────────")
