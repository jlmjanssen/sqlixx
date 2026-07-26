// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:binders;
import std;
import :error.sqlite;
import :statement;

namespace sqlixx {

template <typename T>
struct binder;

export struct bind_context {
    enum class copy_strategy : bool { deep, shallow };

    explicit bind_context(statement_handle stmt, int index = 1) noexcept : handle_(stmt.get()), index_(index) {
        set_copy_strategy(copy_strategy::deep);
    }
    bind_context(statement_handle stmt, copy_strategy strategy) noexcept : handle_(stmt.get()), index_(1) {
        set_copy_strategy(strategy);
    }
    bind_context(statement_handle stmt, ::sqlite3_destructor_type destructor) noexcept
        : handle_(stmt.get()), index_(1) {
        set_destructor(destructor);
    }
    bind_context(statement_handle stmt, int index, copy_strategy strategy) noexcept
        : handle_(stmt.get()), index_(index) {
        set_copy_strategy(strategy);
    }
    bind_context(statement_handle stmt, int index, ::sqlite3_destructor_type destructor) noexcept
        : handle_(stmt.get()), index_(index) {
        set_destructor(destructor);
    }

    auto set_copy_strategy(copy_strategy strategy) noexcept -> void {
        if (strategy == copy_strategy::deep) {
            // NOLINTNEXTLINE(performance-*,cppcoreguidelines-*)
            destructor_ = reinterpret_cast<::sqlite3_destructor_type>(-1); // SQLITE_TRANSIENT
        } else {
            destructor_ = nullptr; // SQLITE_STATIC
        }
    }
    auto set_destructor(::sqlite3_destructor_type destructor) noexcept -> void { destructor_ = destructor; }

private:
    template <typename T>
    friend struct binder;

    ::sqlite3_stmt* handle_;
    int index_;
    ::sqlite3_destructor_type destructor_ = nullptr;
};

[[nodiscard]] constexpr auto to_expected(int result_code) -> std::expected<void, std::error_code> {
    if (result_code != SQLITE_OK) {
        return std::unexpected(make_sqlite_error_code(result_code));
    }
    return {};
}

template <std::integral T>
struct binder<T> {
    [[nodiscard]] auto operator()(bind_context& ctxt, T value) const noexcept -> std::expected<void, std::error_code> {
        if constexpr (sizeof(T) < sizeof(int) || (sizeof(T) == sizeof(int) && std::is_signed_v<T>)) {
            return to_expected(::sqlite3_bind_int(ctxt.handle_, ctxt.index_++, static_cast<int>(value)));
        } else if constexpr (sizeof(T) < sizeof(std::int64_t) ||
                             (sizeof(T) == sizeof(std::int64_t) && std::is_signed_v<T>)) {
            return to_expected(::sqlite3_bind_int64(ctxt.handle_, ctxt.index_++, static_cast<std::int64_t>(value)));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported integral size");
        }
    }
};

template <std::floating_point T>
struct binder<T> {
    [[nodiscard]] auto operator()(bind_context& ctxt, T value) const noexcept -> std::expected<void, std::error_code> {
        if constexpr (sizeof(T) <= sizeof(double)) {
            return to_expected(::sqlite3_bind_double(ctxt.handle_, ctxt.index_++, static_cast<double>(value)));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported floating point size");
        }
    }
};

template <typename T>
    requires std::convertible_to<std::decay_t<T>, std::string_view>
struct binder<T> {
    [[nodiscard]] auto operator()(bind_context& ctxt, const T& value) const noexcept
        -> std::expected<void, std::error_code> {
        if constexpr (std::is_pointer_v<T>) {
            return to_expected(::sqlite3_bind_text(
                ctxt.handle_, ctxt.index_++, static_cast<const char*>(value), -1, ctxt.destructor_));
        } else if constexpr (std::is_array_v<T>) {
            constexpr std::size_t length = sizeof(T) - 1;
            return to_expected(::sqlite3_bind_text(ctxt.handle_,
                                                   ctxt.index_++,
                                                   static_cast<const char*>(value),
                                                   static_cast<int>(length),
                                                   ctxt.destructor_));
        } else {
            std::string_view view = value;
            return to_expected(::sqlite3_bind_text(
                ctxt.handle_, ctxt.index_++, view.data(), static_cast<int>(view.size()), ctxt.destructor_));
        }
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct binder<std::span<T, Extent>> {
    [[nodiscard]] auto operator()(bind_context& ctxt, std::span<T, Extent> value) const noexcept
        -> std::expected<void, std::error_code> {
        return to_expected(::sqlite3_bind_blob64(
            ctxt.handle_, ctxt.index_++, static_cast<const void*>(value.data()), value.size_bytes(), ctxt.destructor_));
    }
};

export struct zeroblob {
    std::size_t size{};
};

template <>
struct binder<zeroblob> {
    [[nodiscard]] auto operator()(bind_context& ctxt, zeroblob value) const noexcept
        -> std::expected<void, std::error_code> {
        return to_expected(::sqlite3_bind_zeroblob64(ctxt.handle_, ctxt.index_++, value.size));
    }
};

template <>
struct binder<std::nullptr_t> {
    [[nodiscard]] auto operator()(bind_context& ctxt, std::nullptr_t) const noexcept
        -> std::expected<void, std::error_code> {
        return to_expected(::sqlite3_bind_null(ctxt.handle_, ctxt.index_++));
    }
};

template <typename T>
concept is_tuple_like = requires { typename std::tuple_size<std::decay_t<T>>::type; };

template <is_tuple_like T>
struct binder<T> {
    template <typename Tuple>
    [[nodiscard]] auto operator()(bind_context& ctxt, Tuple&& tuple) const noexcept
        -> std::expected<void, std::error_code> {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> auto {
            std::expected<void, std::error_code> result{};
            std::ignore = ((result = binder<std::remove_cvref_t<std::tuple_element_t<Is, std::decay_t<Tuple>>>>{}(
                                ctxt, std::get<Is>(std::forward<Tuple>(tuple)))) &&
                           ...);
            return result;
        }(std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
    }
};

export template <typename... Args>
[[nodiscard]] auto bind(bind_context& ctxt, Args&&... args) noexcept -> std::expected<void, std::error_code> {
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = binder<std::remove_cvref_t<Args>>{}(ctxt, std::forward<Args>(args))) && ...);
    return result;
}

export template <typename... Args>
[[nodiscard]] auto bind(statement_handle stmt, int index, Args&&... args) noexcept
    -> std::expected<void, std::error_code> {
    bind_context ctxt{stmt, index};
    return bind(ctxt, std::forward<Args>(args)...);
}

export template <typename... Args>
[[nodiscard]] auto bind(statement_handle stmt, const char* name, Args&&... args) noexcept
    -> std::expected<void, std::error_code> {
    bind_context ctxt{stmt, ::sqlite3_bind_parameter_index(stmt.get(), name)};
    return bind(ctxt, std::forward<Args>(args)...);
}

} // namespace sqlixx
