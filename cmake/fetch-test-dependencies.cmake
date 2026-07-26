# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CACHEP{CATCH_INSTALL_HELPERS} TYPE BOOL FORCE VALUE OFF)
mark_as_advanced(CATCH_INSTALL_HELPERS)

cpmaddpackage("gh:catchorg/Catch2@3.15.3")
cpmaddpackage("gh:rollbear/trompeloeil@49")

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
