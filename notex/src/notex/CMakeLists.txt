# NOTEX CORE LIBRARY CMAKE FILE ─────────────────────────────────────────────

message(STATUS "  Configuring notex_core static library...")

# BUILD-TIME ASSET EMBEDDING ────────────────────────────────────────────────
# Reads every file under notex/latex/ and notex/fonts/ and turns them into
# generated C++ headers that assets.cpp exposes to the rest of the
# program. See DESIGN.md, "A self-contained binary".

set(NOTEX_GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/generated/include)
set(NOTEX_EMBEDDED_LATEX_HEADER
    ${NOTEX_GENERATED_INCLUDE_DIR}/notex/embedded_latex.hpp)

file(GLOB NOTEX_LATEX_ASSET_FILES CONFIGURE_DEPENDS
     ${PROJECT_ROOT_DIR}/notex/latex/*)

add_custom_command(
  OUTPUT ${NOTEX_EMBEDDED_LATEX_HEADER}
  COMMAND
    ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_ROOT_DIR}/notex/latex
    -DOUTPUT_FILE=${NOTEX_EMBEDDED_LATEX_HEADER}
    -DNAMESPACE=notex::assets::detail
    -DACCESSOR_NAME=embedded_latex_files -P
    ${PROJECT_ROOT_DIR}/cmake/generate_embedded_assets.cmake
  DEPENDS ${NOTEX_LATEX_ASSET_FILES}
          ${PROJECT_ROOT_DIR}/cmake/generate_embedded_assets.cmake
  COMMENT "Embedding latex template files into notex_core"
  VERBATIM)

set(NOTEX_EMBEDDED_FONTS_HEADER
    ${NOTEX_GENERATED_INCLUDE_DIR}/notex/embedded_fonts.hpp)

file(GLOB NOTEX_FONT_ASSET_FILES CONFIGURE_DEPENDS
     ${PROJECT_ROOT_DIR}/notex/fonts/*)

add_custom_command(
  OUTPUT ${NOTEX_EMBEDDED_FONTS_HEADER}
  COMMAND
    ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_ROOT_DIR}/notex/fonts
    -DOUTPUT_FILE=${NOTEX_EMBEDDED_FONTS_HEADER}
    -DNAMESPACE=notex::assets::detail
    -DACCESSOR_NAME=embedded_fonts
    -DENCODING=HEX -P
    ${PROJECT_ROOT_DIR}/cmake/generate_embedded_assets.cmake
  DEPENDS ${NOTEX_FONT_ASSET_FILES}
          ${PROJECT_ROOT_DIR}/cmake/generate_embedded_assets.cmake
  COMMENT "Embedding fonts files into notex_core"
  VERBATIM)

add_custom_target(
  notex_generate_assets
  DEPENDS ${NOTEX_EMBEDDED_LATEX_HEADER} ${NOTEX_EMBEDDED_FONTS_HEADER})

# LIBRARY TARGET ─────────────────────────────────────────────────────────────

set(NOTEX_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/assets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/templates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/reference.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/environment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/document.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/installer.cpp)

add_library(notex_core STATIC ${NOTEX_CORE_SOURCES})
add_dependencies(notex_core notex_generate_assets)

target_include_directories(notex_core PUBLIC ${INCLUDE_DIR})
target_include_directories(notex_core SYSTEM
                            PUBLIC ${NOTEX_GENERATED_INCLUDE_DIR})
target_link_libraries(notex_core PUBLIC plog::plog nlohmann_json::nlohmann_json)

add_compilation_flags(notex_core)

message(STATUS "  ✓ notex_core (static library)")
