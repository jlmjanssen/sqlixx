// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:statement.coro;
import std;
import :error.sqlite;
import :statement;
import :statement.step;
import :readers;

namespace sqlixx {

export using step_error_handler = std::move_only_function<void(std::error_code) noexcept>;

export [[nodiscard]] auto execute(statement_handle stmt, step_error_handler on_error = nullptr) noexcept
    -> std::generator<row_context> {
    for (;;) {
        if (auto step_res = sqlixx::step(stmt); !step_res) [[unlikely]] {
            if (on_error) {
                std::invoke(on_error, step_res.error());
            }
            co_return;
        } else if (*step_res == row_state::empty) {
            co_return;
        } else {
            co_yield row_context{stmt};
        }
    }
}

} // namespace sqlixx
