// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:statement.coro;

import std;
import :error.sqlite;
import :readers;
import :statement;
import :statement.step;

namespace sqlixx {

class scope_guard {
public:
    explicit scope_guard(::sqlite3_stmt* stmt_handle) : stmt_handle_(stmt_handle) {}
    scope_guard(const scope_guard&) = delete;
    scope_guard(scope_guard&&) = delete;
    auto operator=(const scope_guard&) -> scope_guard& = delete;
    auto operator=(scope_guard&&) -> scope_guard& = delete;
    ~scope_guard() noexcept { std::ignore = ::sqlite3_reset(stmt_handle_); }

private:
    ::sqlite3_stmt* stmt_handle_;
};

export [[nodiscard]] auto execute(statement_handle stmt) noexcept
    -> std::generator<std::expected<reader_context, std::error_code>> {
    const scope_guard reset_guard{stmt.get()};

    for (;;) {
        switch (const int result_code = ::sqlite3_step(stmt.get())) {
        case SQLITE_ROW:
            co_yield reader_context(stmt.get());
            continue;

        case SQLITE_DONE:
            co_return;

        case SQLITE_BUSY:
        case SQLITE_BUSY_RECOVERY:
        case SQLITE_LOCKED_SHAREDCACHE:
            co_yield std::unexpected(make_sqlite_error_code(result_code));
            continue;

        default:
            co_yield std::unexpected(make_sqlite_error_code(result_code));
            co_return;
        }
    }
}

} // namespace sqlixx
