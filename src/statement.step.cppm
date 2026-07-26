// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:statement.step;
import std;
import :error.sqlite;
import :statement;

namespace sqlixx {

export enum class row_state : bool { available, empty };

export [[nodiscard]] constexpr auto step(statement_handle stmt) noexcept -> std::expected<row_state, std::error_code> {
    switch (auto result_code = ::sqlite3_step(stmt.get())) {
    case SQLITE_ROW:
        return row_state::available;
    case SQLITE_DONE:
        return row_state::empty;
    default:
        return std::unexpected(make_sqlite_error_code(result_code));
    }
}

export [[nodiscard]] auto reset(statement_handle stmt) noexcept -> std::expected<void, std::error_code> {
    if (auto result_code = ::sqlite3_reset(stmt.get()); result_code != SQLITE_OK) [[unlikely]] {
        return std::unexpected(make_sqlite_error_code(result_code));
    }
    return {};
}

export [[nodiscard]] auto clear_bindings(statement_handle stmt) noexcept -> std::expected<void, std::error_code> {
    if (auto result_code = ::sqlite3_clear_bindings(stmt.get()); result_code != SQLITE_OK) [[unlikely]] {
        return std::unexpected(make_sqlite_error_code(result_code));
    }
    return {};
}

} // namespace sqlixx
