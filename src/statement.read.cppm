// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:statement.read;

import std;
import :readers;
import :statement;

namespace sqlixx {

export template <typename... Ts>
[[nodiscard]] auto read(reader_context& ctxt, Ts&&... values) noexcept -> std::expected<void, std::error_code> {
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = reader<std::remove_cvref_t<Ts>>{}(ctxt, std::forward<Ts>(values))) && ...);
    return result;
}

export template <typename... Ts>
[[nodiscard]] auto read(statement_handle stmt, int column, Ts&&... values) noexcept
    -> std::expected<void, std::error_code> {
    reader_context ctxt{stmt.get()};
    auto result = ctxt.set_column_index(column);
    if (!result) {
        return result;
    }
    return read(ctxt, std::forward<Ts>(values)...);
}

} // namespace sqlixx
