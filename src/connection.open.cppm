// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:connection.open;
import std;
import :error.sqlite;
import :connection;

namespace sqlixx {

namespace open {
struct flag_t {};

template <int flags>
struct flags_t : flag_t {
    [[nodiscard]] constexpr auto get() const noexcept -> int { return flags; }
};

export struct dyn_flags : flag_t {
    explicit constexpr dyn_flags(int flags) noexcept : flags_(flags) {}
    [[nodiscard]] constexpr auto get() const noexcept -> int { return flags_; }

private:
    int flags_;
};

export constexpr flags_t<SQLITE_OPEN_READONLY> readonly;
export constexpr flags_t<SQLITE_OPEN_READWRITE> readwrite;
export constexpr flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE> readwrite_create;
export constexpr flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY> memory;
export constexpr flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI> uri;

export constexpr flags_t<SQLITE_OPEN_NOMUTEX> nomutex;
export constexpr flags_t<SQLITE_OPEN_FULLMUTEX> fullmutex;
export constexpr flags_t<SQLITE_OPEN_SHAREDCACHE> sharedcache;
export constexpr flags_t<SQLITE_OPEN_PRIVATECACHE> privatecache;
export constexpr flags_t<SQLITE_OPEN_NOFOLLOW> nofollow;

template <typename Flag>
concept is_flag = std::is_base_of_v<flag_t, std::decay_t<Flag>> && requires(std::decay_t<Flag> flags) {
    { flags.get() } noexcept -> std::convertible_to<int>;
};

template <typename... Flags>
concept all_flags = (is_flag<Flags> && ...);

template <typename... Flags>
    requires all_flags<Flags...>

[[nodiscard]] constexpr auto make_flags(Flags... flags) noexcept -> int {
    if constexpr (sizeof...(Flags) == 0) {
        return (SQLITE_OPEN_EXRESCODE | readwrite_create.get());
    } else {
        return (SQLITE_OPEN_EXRESCODE | ... | flags.get());
    }
}

using error_handler = std::move_only_function<void(std::error_code, std::string_view) noexcept>;
} // namespace open

[[nodiscard]] constexpr auto
open_connection_impl(const char* filename, int flags, const char* vfs, open::error_handler on_error = nullptr) noexcept
    -> std::expected<connection, std::error_code> {
    ::sqlite3* handle = nullptr;
    int result = ::sqlite3_open_v2(filename, &handle, flags, vfs);

    if (result != SQLITE_OK) {
        const auto errcode = make_sqlite_error_code(result);
        if (on_error) {
            const std::string_view errmsg = (handle != nullptr ? ::sqlite3_errmsg(handle) : "No active connection");
            std::invoke(std::forward<open::error_handler>(on_error), errcode, errmsg);
        }
        if (handle != nullptr) {
            ::sqlite3_close_v2(handle);
        }
        return std::unexpected(errcode);
    }

    return connection(handle);
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto open_connection(const char* filename, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), nullptr);
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto open_connection(const char* filename, const char* vfs, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), vfs);
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto open_connection(const char* filename,
                                             open::error_handler&& on_error,
                                             Flags... flags) noexcept -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), nullptr, std::move(on_error));
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto
open_connection(const char* filename, const char* vfs, open::error_handler&& on_error, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), vfs, std::move(on_error));
}

} // namespace sqlixx
