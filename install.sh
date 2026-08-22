#!/bin/bash
#
# Convenience wrapper around the CMake build described in README.md /
# AGENTS.md. Builds the notex C++ CLI in a common configuration and,
# for `global`, installs the resulting binary plus the LaTeX template
# and fonts it embeds onto this system.
#
# Usage:
#   ./install.sh [release|debug|tests|global]
#
#   release (default)  configure and build in Release mode
#   debug               configure and build in Debug mode (-DDEBUG=ON)
#   tests                configure and build in Debug mode with tests
#                        (-DDEBUG=ON -DTESTS=ON), then run them via ctest
#   global               build in Release mode, install the notex binary
#                        system-wide (`cmake --install`, retried with sudo
#                        if the install prefix isn't user-writable), then
#                        run it to install the LaTeX template and fonts
#                        globally (`notex install`)
#
# The binary always also lands at ./bin/notex regardless of mode.

set -eou pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="build"
MODE="${1:-release}"

usage() {
    sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

cpu_count() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu
    else
        echo 1
    fi
}

configure_and_build() {
    echo "Configuring ($*)..."
    cmake -B "$BUILD_DIR" "$@"
    echo "Building..."
    cmake --build "$BUILD_DIR" --parallel "$(cpu_count)"
}

case "$MODE" in
    release)
        configure_and_build -DCMAKE_BUILD_TYPE=Release
        echo "Done. Binary at ./bin/notex"
        ;;
    debug)
        configure_and_build -DDEBUG=ON
        echo "Done. Binary at ./bin/notex (debug build)"
        ;;
    tests)
        configure_and_build -DDEBUG=ON -DTESTS=ON
        echo "Running tests..."
        ctest --test-dir "$BUILD_DIR" --output-on-failure
        ;;
    global)
        configure_and_build -DCMAKE_BUILD_TYPE=Release

        echo "Installing the notex binary onto this system..."
        if ! cmake --install "$BUILD_DIR"; then
            echo "The install prefix isn't writable by this user; retrying with sudo..."
            sudo cmake --install "$BUILD_DIR"
        fi

        echo "Installing the LaTeX template and fonts globally..."
        ./bin/notex install --force

        echo "Done. 'notex' and the LaTeX template are now available system-wide."
        ;;
    -h | --help | help)
        usage
        exit 0
        ;;
    *)
        echo "Error: unknown mode '$MODE'." >&2
        usage >&2
        exit 1
        ;;
esac
