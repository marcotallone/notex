/**
 * @file notex.hpp
 * @brief Umbrella header for the notex_core library.
 *
 * Re-exports every public header of notex_core so that external consumers,
 * and the test binary, can pull in the whole library with a single
 * `#include "notex/notex.hpp"`. This file grows one line at a time as new
 * public headers are added to the library.
 */

// clang-format off
// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃                                                                            ┃
// ┃                         ░███                 ░███                          ┃
// ┃                        ░██     ░███    ░██     ░██                         ┃
// ┃                        ░██     ░████   ░██     ░██                         ┃
// ┃                        ░██     ░██░██  ░██     ░██                         ┃
// ┃                      ░███      ░██ ░██ ░██      ░██                        ┃
// ┃                        ░██     ░██  ░██░██     ░██                         ┃
// ┃                        ░██     ░██   ░████     ░██                         ┃
// ┃                        ░██     ░██    ░███     ░██                         ┃
// ┃                         ░███                 ░███                          ┃
// ┃                                                                            ┃
// ┃                               ~ N O T E X ~                                ┃
// ┃                      Copyright (c) 2026 Marco Tallone                      ┃
// ┃                                MIT License                                 ┃
// ┃                                                                            ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
// clang-format on

#pragma once

// TODO: THis is here just to be documented here...
namespace notex {}

#include "notex/assets.hpp"
#include "notex/constants.hpp"
#include "notex/document.hpp"
#include "notex/environment.hpp"
#include "notex/errors.hpp"
#include "notex/installer.hpp"
#include "notex/logging.hpp"
#include "notex/manager.hpp"
#include "notex/output.hpp"
#include "notex/reference.hpp"
#include "notex/system.hpp"
#include "notex/templates.hpp"
