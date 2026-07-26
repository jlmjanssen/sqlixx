# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CACHE{ENABLE_COVERAGE} TYPE BOOL HELP "Enable Coverage" VALUE OFF)
mark_as_advanced(ENABLE_COVERAGE)

function(target_enable_coverage target)
    if(ENABLE_COVERAGE)
        if(COVERAGE_C_FLAGS)
            target_compile_options(
                ${target}
                PRIVATE $<$<COMPILE_LANGUAGE:C>:${COVERAGE_C_FLAGS}>
            )
        endif()
        if(COVERAGE_CXX_FLAGS)
            target_compile_options(
                ${target}
                PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${COVERAGE_CXX_FLAGS}>
            )
        endif()
        if(COVERAGE_LINKER_FLAGS)
            target_link_options(${target} PUBLIC ${COVERAGE_LINKER_FLAGS})
        endif()
    endif()
endfunction()

if(ENABLE_COVERAGE AND COVERAGE_SCRIPT)
    set(script_path "${CMAKE_CURRENT_LIST_DIR}/${COVERAGE_SCRIPT}")
    if(EXISTS "${script_path}")
        add_custom_target(
            coverage-summary
            COMMAND "${script_path}" "summary" "${CMAKE_BINARY_DIR}"
            VERBATIM
        )
        add_custom_target(
            coverage-report
            COMMAND "${script_path}" "report" "${CMAKE_BINARY_DIR}"
            VERBATIM
        )
        add_custom_target(
            coverage-data
            COMMAND "${script_path}" "data" "${CMAKE_BINARY_DIR}"
            VERBATIM
        )
    endif()
endif()
