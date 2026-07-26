# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CACHE{ENABLE_SANITIZER}
    TYPE STRING
    HELP "Enable Sanitizer [MaxSan|TSan]"
    VALUE ""
)
mark_as_advanced(ENABLE_SANITIZER)

function(target_enable_sanitizer target)
    if(ENABLE_SANITIZER)
        if(SANITIZER_C_FLAGS)
            target_compile_options(
                ${target}
                PRIVATE $<$<COMPILE_LANGUAGE:C>:${SANITIZER_C_FLAGS}>
            )
        endif()
        if(SANITIZER_CXX_FLAGS)
            target_compile_options(
                ${target}
                PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${SANITIZER_CXX_FLAGS}>
            )
        endif()
        if(SANITIZER_LINKER_FLAGS)
            target_link_options(${target} PUBLIC ${SANITIZER_LINKER_FLAGS})
        endif()
    endif()
endfunction()
