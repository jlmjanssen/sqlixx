// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:statement.prepare;

import std;
import :error.sqlite;
import :connection;
import :statement;

namespace sqlixx {

export struct prep_result {
    statement stmt;
    std::string_view tail;
};

namespace prep {
struct flag_t {};

template <unsigned int flags>
struct flags_t : flag_t {
    [[nodiscard]] constexpr auto get() const noexcept -> unsigned int { return flags; }
};

export struct dyn_flags : flag_t {
    explicit constexpr dyn_flags(unsigned int flags) noexcept : flags_(flags) {}
    [[nodiscard]] constexpr auto get() const noexcept -> unsigned int { return flags_; }

private:
    unsigned int flags_;
};

export constexpr flags_t<SQLITE_PREPARE_PERSISTENT> persistent;
export constexpr flags_t<SQLITE_PREPARE_NORMALIZE> normalize;
export constexpr flags_t<SQLITE_PREPARE_NO_VTAB> no_vtab;
export constexpr flags_t<SQLITE_PREPARE_DONT_LOG> dont_log;
export constexpr flags_t<SQLITE_PREPARE_FROM_DDL> from_ddl;

template <typename Flag>
concept is_flag = std::is_base_of_v<flag_t, std::decay_t<Flag>> && requires(std::decay_t<Flag> flags) {
    { flags.get() } noexcept -> std::convertible_to<unsigned int>;
};

template <typename... Flags>
concept all_flags = (is_flag<Flags> && ...);

export template <typename... Flags>
    requires all_flags<Flags...>
constexpr auto make_flags(Flags... flags) noexcept -> unsigned int {
    return (0U | ... | flags.get());
}
} // namespace prep

[[nodiscard]] constexpr auto
prepare_statement_impl(::sqlite3* db_handle, const char* sql_ptr, int byte_count, unsigned int prep_flags) noexcept
    -> std::expected<prep_result, std::error_code> {
    ::sqlite3_stmt* stmt_handle = nullptr;
    const char* tail_ptr = nullptr;

    const int result = ::sqlite3_prepare_v3(db_handle, sql_ptr, byte_count, prep_flags, &stmt_handle, &tail_ptr);

    if (result != SQLITE_OK) {
        return std::unexpected(make_sqlite_error_code(result));
    }

    std::string_view tail{};

    if (byte_count < 0) {
        tail = std::string_view(tail_ptr);
    } else {
        auto consumed = tail_ptr - sql_ptr;
        tail = std::string_view(tail_ptr, static_cast<std::size_t>(byte_count - consumed));
    }

    return prep_result{.stmt = statement(stmt_handle), .tail = tail};
}

export template <prep::is_flag... Flags>
[[nodiscard]] constexpr auto prepare_statement(connection_handle conn, const char* sql, Flags... flags) noexcept
    -> std::expected<prep_result, std::error_code> {
    return prepare_statement_impl(conn.get(), sql, -1, prep::make_flags(flags...));
}

export template <prep::is_flag... Flags>
[[nodiscard]] constexpr auto prepare_statement(connection_handle conn, std::string_view sql, Flags... flags) noexcept
    -> std::expected<prep_result, std::error_code> {
    return prepare_statement_impl(conn.get(), sql.data(), static_cast<int>(sql.size()), prep::make_flags(flags...));
}

} // namespace sqlixx
