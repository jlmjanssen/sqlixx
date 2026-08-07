// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:binder_context;

import std;
import :error.sqlite;
import :index;

namespace sqlixx {

export struct binder_context {
    enum class copy : bool { deep, shallow };

    explicit binder_context(::sqlite3_stmt* handle) noexcept
        : handle_(handle), index_(1, ::sqlite3_bind_parameter_count(handle) + 1) {
        set_strategy(copy::deep);
    }

    [[nodiscard]] constexpr auto get() const noexcept -> ::sqlite3_stmt* { return handle_; }

    [[nodiscard]] constexpr auto set_index(int index) noexcept -> std::expected<void, std::error_code> {
        return index_.set(index);
    }

    [[nodiscard]] constexpr auto get_and_advance_index() noexcept -> std::expected<int, std::error_code> {
        return index_.get_and_advance();
    }

    auto set_destructor(::sqlite3_destructor_type destructor) noexcept -> void { destructor_ = destructor; }

    auto set_strategy(copy strategy) noexcept -> void {
        if (strategy == copy::deep) {
            // NOLINTNEXTLINE(performance-*,cppcoreguidelines-*)
            destructor_ = reinterpret_cast<::sqlite3_destructor_type>(-1); // SQLITE_TRANSIENT
        } else {
            destructor_ = nullptr; // SQLITE_STATIC
        }
    }

    [[nodiscard]] auto get_destructor() noexcept -> ::sqlite3_destructor_type { return destructor_; }

private:
    ::sqlite3_stmt* handle_;
    checked_index<int> index_;
    ::sqlite3_destructor_type destructor_ = nullptr;
};

} // namespace sqlixx
