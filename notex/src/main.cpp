/**
 * NoTeX
 * Copyright (c) 2026 Marco Tallone
 * Licensed under the MIT License. See LICENCE for details.
 *
 * Entry point of the `notex` CLI binary. Construction and running of the
 * Orchestrator is the whole of main's job; every real decision is made
 * inside notex_core and the Orchestrator itself.
 */

#include "orchestrator.hpp"

#if defined(LOGGING)
#include "notex/logging.hpp"
#endif

int main(int argc, char** argv) {
#if defined(LOGGING)
    // A debug build wants full internal detail; a release build with
    // logging explicitly enabled only wants user-facing operations and
    // above, to keep logs/notex.log from filling up with anchor-lookup
    // and filesystem-scan noise.
#if defined(DEBUG)
    init_logging("notex", plog::debug);
#else
    init_logging("notex", plog::info);
#endif
#endif

    notex::Orchestrator orchestrator;
    return static_cast<int>(orchestrator.run(argc, argv));
}
