# SOURCE CMAKE FILE ────────────────────────────────────────────────────────────

message(STATUS "──────────────────────────────────────────────────────────────")
message(STATUS "Configuring notex_core library and notex executable...")

# The notex_core static library, its own translation units and its own
# CMakeLists.txt, lives one level down and knows nothing about CLI11.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/notex)

# Main executable: only main.cpp and orchestrator.cpp, plus CLI11, which
# nothing else in the project ever sees.
add_executable(notex ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/orchestrator.cpp)
target_include_directories(notex PRIVATE ${INCLUDE_DIR})
target_link_libraries(notex PRIVATE notex_core CLI11::CLI11)
add_compilation_flags(notex)
message(STATUS "  ✓ notex (executable)")

# Lets `cmake --install <build>` (or `sudo cmake --install <build>` for a
# system-owned prefix) place the binary onto CMAKE_INSTALL_PREFIX's bin/
# directory, honouring the standard -DCMAKE_INSTALL_PREFIX override. This
# is what install.sh's `global` mode calls under the hood.
install(TARGETS notex RUNTIME DESTINATION bin)
