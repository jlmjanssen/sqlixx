# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CACHE{COVERAGE_C_FLAGS}
    TYPE INTERNAL
    VALUE "--coverage;-fkeep-inline-functions;-fkeep-static-functions"
)
set(CACHE{COVERAGE_CXX_FLAGS}
    TYPE INTERNAL
    VALUE "--coverage;-fkeep-inline-functions;-fkeep-static-functions"
)
set(CACHE{COVERAGE_LINKER_FLAGS} TYPE INTERNAL VALUE "--coverage")

set(CACHE{COVERAGE_SCRIPT} TYPE INTERNAL VALUE "collect-gcov-data.sh")

if(ENABLE_SANITIZER STREQUAL "MaxSan")
    set(CACHE{SANITIZER_C_FLAGS}
        TYPE INTERNAL
        VALUE
            "-fsanitize=address;-fsanitize=leak;-fsanitize=pointer-compare;-fsanitize=pointer-subtract;-fsanitize=undefined;-fsanitize-undefined-trap-on-error;-fno-omit-frame-pointer"
    )
    set(CACHE{SANITIZER_CXX_FLAGS}
        TYPE INTERNAL
        VALUE
            "-fsanitize=address;-fsanitize=leak;-fsanitize=pointer-compare;-fsanitize=pointer-subtract;-fsanitize=undefined;-fsanitize-undefined-trap-on-error;-fno-omit-frame-pointer"
    )
    set(CACHE{SANITIZER_LINKER_FLAGS}
        TYPE INTERNAL
        VALUE
            "-fsanitize=address;-fsanitize=leak;-fsanitize=pointer-compare;-fsanitize=pointer-subtract;-fsanitize=undefined;-fsanitize-undefined-trap-on-error"
    )
elseif(ENABLE_SANITIZER STREQUAL "TSan")
    set(CACHE{SANITIZER_C_FLAGS} TYPE INTERNAL VALUE "-fsanitize=thread")
    set(CACHE{SANITIZER_CXX_FLAGS} TYPE INTERNAL VALUE "-fsanitize=thread")
    set(CACHE{SANITIZER_LINKER_FLAGS} TYPE INTERNAL VALUE "-fsanitize=thread")
elseif(ENABLE_SANITIZER)
    message(AUTHOR_WARNING "Unknown sanitizer option: ${ENABLE_SANITIZER}")
    set(CACHE{ENABLE_SANITIZER} FORCE VALUE "OFF")
endif()
