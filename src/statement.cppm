// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:statement;
import std;
import :handles;

namespace sqlixx {

constexpr auto free_sqlite3_stmt = [](::sqlite3_stmt* handle) noexcept -> void {
    std::ignore = ::sqlite3_finalize(handle);
};

export using statement = owning_handle<::sqlite3_stmt*, free_sqlite3_stmt>;
export using statement_handle = statement::shallow_handle_type;

} // namespace sqlixx
