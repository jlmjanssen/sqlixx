# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

include_guard(GLOBAL)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_C_FLAGS_INIT "-D_FORTIFY_SOURCE=0")
set(CMAKE_CXX_FLAGS_INIT "-D_FORTIFY_SOURCE=0 -Wno-reserved-module-identifier")

set(CACHE{COVERAGE_C_FLAGS}
    TYPE INTERNAL
    VALUE "-fprofile-instr-generate;-fcoverage-mapping"
)
set(CACHE{COVERAGE_CXX_FLAGS}
    TYPE INTERNAL
    VALUE "-fprofile-instr-generate;-fcoverage-mapping"
)
set(CACHE{COVERAGE_LINKER_FLAGS}
    TYPE INTERNAL
    VALUE "-fprofile-instr-generate;-Wl,--build-id"
)

set(CACHE{COVERAGE_SCRIPT} TYPE INTERNAL VALUE "collect-llvm-data.sh")

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
