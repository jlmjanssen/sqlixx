// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <sqlite3.h>
#include <trompeloeil.hpp>

class sqlite_mock {
public:
    inline static sqlite_mock* mock_ = nullptr;

    sqlite_mock() noexcept { mock_ = this; }
    ~sqlite_mock() noexcept { mock_ = nullptr; }

    MAKE_MOCK(sqlite3_errstr, auto(int)->const char*);
    MAKE_MOCK(sqlite3_errmsg, auto(::sqlite3*)->const char*);
    MAKE_MOCK(sqlite3_extended_errcode, auto(::sqlite3*)->int);

    MAKE_MOCK(sqlite3_open_v2, auto(const char*, sqlite3**, int, const char*)->int);
    MAKE_MOCK(sqlite3_close_v2, auto(sqlite3*)->int);

    MAKE_MOCK(sqlite3_prepare_v3, auto(sqlite3*, const char*, int, unsigned int, sqlite3_stmt**, const char**)->int);
    MAKE_MOCK(sqlite3_finalize, auto(sqlite3_stmt*)->int);
    MAKE_MOCK(sqlite3_step, auto(sqlite3_stmt*)->int);
    MAKE_MOCK(sqlite3_reset, auto(sqlite3_stmt*)->int);
    MAKE_MOCK(sqlite3_clear_bindings, auto(sqlite3_stmt*)->int);

    MAKE_MOCK(sqlite3_bind_int, auto(sqlite3_stmt*, int, int)->int);
    MAKE_MOCK(sqlite3_bind_int64, auto(sqlite3_stmt*, int, sqlite3_int64)->int);
    MAKE_MOCK(sqlite3_bind_double, auto(sqlite3_stmt*, int, double)->int);
    MAKE_MOCK(sqlite3_bind_text, auto(sqlite3_stmt*, int, const char*, int, void (*)(void*))->int);
    MAKE_MOCK(sqlite3_bind_blob64, auto(sqlite3_stmt*, int, const void*, sqlite3_uint64, void (*)(void*))->int);
    MAKE_MOCK(sqlite3_bind_zeroblob64, auto(sqlite3_stmt*, int, sqlite3_uint64)->int);
    MAKE_MOCK(sqlite3_bind_null, auto(sqlite3_stmt*, int)->int);
    MAKE_MOCK(sqlite3_bind_parameter_index, auto(sqlite3_stmt*, const char*)->int);

    MAKE_MOCK(sqlite3_data_count, auto(sqlite3_stmt*)->int);
    MAKE_MOCK(sqlite3_column_int, auto(sqlite3_stmt*, int)->int);
    MAKE_MOCK(sqlite3_column_int64, auto(sqlite3_stmt*, int)->sqlite3_int64);
    MAKE_MOCK(sqlite3_column_double, auto(sqlite3_stmt*, int)->double);
    MAKE_MOCK(sqlite3_column_text, auto(sqlite3_stmt*, int)->const unsigned char*);
    MAKE_MOCK(sqlite3_column_blob, auto(sqlite3_stmt*, int)->const void*);
    MAKE_MOCK(sqlite3_column_bytes, auto(sqlite3_stmt*, int)->int);
};
