// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:error.sqlite;
import std;

namespace sqlixx {

// NOLINTNEXTLINE(performance-*)
export enum class sqlite_errc : int {
    // Unused: SQLITE_OK
    error = SQLITE_ERROR,
    // Unused: SQLITE_INTERNAL
    perm = SQLITE_PERM,
    abort = SQLITE_ABORT,
    busy = SQLITE_BUSY,
    locked = SQLITE_LOCKED,
    nomem = SQLITE_NOMEM,
    readonly = SQLITE_READONLY,
    interrupt = SQLITE_INTERRUPT,
    ioerr = SQLITE_IOERR,
    corrupt = SQLITE_CORRUPT,
    notfound = SQLITE_NOTFOUND,
    full = SQLITE_FULL,
    cantopen = SQLITE_CANTOPEN,
    protocol = SQLITE_PROTOCOL,
    // Unused: SQLITE_EMPTY
    schema = SQLITE_SCHEMA,
    toobig = SQLITE_TOOBIG,
    constraint = SQLITE_CONSTRAINT,
    mismatch = SQLITE_MISMATCH,
    misuse = SQLITE_MISUSE,
    nolfs = SQLITE_NOLFS,
    auth = SQLITE_AUTH,
    // Unused: SQLITE_FORMAT
    range = SQLITE_RANGE,
    notadb = SQLITE_NOTADB
    // Unused: SQLITE_NOTICE
    // Unused: SQLITE_WARNING
    // Unused: SQLITE_ROW
    // Unused: SQLITE_DONE
};

class sqlite_error_category final : public std::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override { return "sqlite"; }

    [[nodiscard]] auto message(int code) const -> std::string override { return ::sqlite3_errstr(code); }

    [[nodiscard]] auto default_error_condition(int code) const noexcept -> std::error_condition override {
        // Optimized expression to exclude SQLITE_OK, SQLITE_OK_*, SQLITE_ROW, and SQLITE_DONE.
        // NOLINTNEXTLINE(cppcoreguidelines-*,readability-*)
        const int primary_code = code & 0xff;
        if (static_cast<unsigned>(primary_code - 1) < static_cast<unsigned>(SQLITE_ROW - 1)) {
            return {primary_code, *this};
        }
        return {};
    }
};

export [[nodiscard]] auto sqlite_category() noexcept -> const std::error_category& {
    static sqlite_error_category instance;
    return instance;
}

export [[nodiscard]] auto make_error_condition(sqlite_errc cond) noexcept -> std::error_condition {
    return {std::to_underlying(cond), sqlite_category()};
}

export [[nodiscard]] auto make_sqlite_error_code(int code) noexcept -> std::error_code {
    return {code, sqlite_category()};
}

export [[nodiscard]] auto make_checked_sqlite_error_code(int code) noexcept -> std::error_code {
    // Optimized expression to exclude SQLITE_OK, SQLITE_OK_*, SQLITE_ROW, and SQLITE_DONE.
    // NOLINTNEXTLINE(cppcoreguidelines-*,readability-*)
    const int primary_code = code & 0xff;
    if (static_cast<unsigned>(primary_code - 1) < static_cast<unsigned>(SQLITE_ROW - 1)) {
        return {code, sqlite_category()};
    }
    return {};
}

} // namespace sqlixx

namespace std {
template <>
struct is_error_condition_enum<sqlixx::sqlite_errc> : std::true_type {};
} // namespace std
