// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:connection;

import std;
import :handles;

namespace sqlixx {

constexpr auto free_sqlite3 = [](::sqlite3* handle) noexcept -> void { std::ignore = ::sqlite3_close_v2(handle); };

export using connection = owning_handle<::sqlite3*, free_sqlite3>;
export using connection_handle = connection::shallow_handle_type;

} // namespace sqlixx
