# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CACHE{ENABLE_CLANG_TIDY}
    TYPE BOOL
    HELP "Enable static analysis with clang-tidy"
    VALUE OFF
)
mark_as_advanced(ENABLE_CLANG_TIDY)

function(target_enable_clang_tidy target)
    if(ENABLE_CLANG_TIDY)
        set(warning_flags "-Wall;-Wextra;-Wpedantic")
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY clang-tidy)
        target_compile_options(${target} PRIVATE ${warning_flags})
    endif()
endfunction()
