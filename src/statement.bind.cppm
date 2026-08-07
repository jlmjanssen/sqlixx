// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:statement.bind;

import std;
import :binders;
import :statement;

namespace sqlixx {

export template <typename... Ts>
[[nodiscard]] auto bind(binder_context& ctxt, Ts&&... values) noexcept -> std::expected<void, std::error_code> {
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = binder_t<Ts>{}(ctxt, std::forward<Ts>(values))) && ...);
    return result;
}

export template <typename... Ts>
[[nodiscard]] auto bind(statement_handle stmt, int index, Ts&&... values) noexcept
    -> std::expected<void, std::error_code> {
    binder_context ctxt{stmt.get()};
    auto result = ctxt.set_index(index);
    if (!result) {
        return result;
    }
    return bind(ctxt, std::forward<Ts>(values)...);
}

export template <typename... Ts>
[[nodiscard]] auto bind(statement_handle stmt, const char* name, Ts&&... values) noexcept
    -> std::expected<void, std::error_code> {
    int index = ::sqlite3_bind_parameter_index(stmt.get(), name);
    return bind(stmt, index, std::forward<Ts>(values)...);
}

} // namespace sqlixx
