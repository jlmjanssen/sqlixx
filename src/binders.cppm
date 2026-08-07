// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:binders;

import std;
import :concepts;
import :error.sqlite;

namespace sqlixx {

template <typename T>
struct binder;

template <typename T>
struct binder_registry;

export template <typename T>
using binder_t = binder_registry<std::remove_cvref_t<T>>::type;

template <sqlite_integer T>
struct binder<T> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, T value) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then([&ctxt, value](int idx) noexcept -> auto {
            if constexpr ((sizeof(T) < sizeof(int)) || ((sizeof(T) == sizeof(int)) && std::is_signed_v<T>)) {
                return ctxt.to_expected(::sqlite3_bind_int(ctxt.get(), idx, static_cast<int>(value)));
            } else {
                return ctxt.to_expected(::sqlite3_bind_int64(ctxt.get(), idx, static_cast<std::int64_t>(value)));
            }
        });
    }
};

template <sqlite_integer T>
struct binder_registry<T> {
    using type = binder<T>;
};

template <typename T>
    requires std::is_enum_v<T>
struct binder<T> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, T value) const noexcept
        -> std::expected<void, std::error_code> {
        return binder_t<std::underlying_type_t<T>>{}(ctxt, std::to_underlying(value));
    }
};

template <typename T>
    requires std::is_enum_v<T>
struct binder_registry<T> {
    using type = binder<T>;
};

template <sqlite_real T>
struct binder<T> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, T value) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then([&ctxt, value](int idx) noexcept -> auto {
            return ctxt.to_expected(::sqlite3_bind_double(ctxt.get(), idx, static_cast<double>(value)));
        });
    }
};

template <sqlite_real T>
struct binder_registry<T> {
    using type = binder<T>;
};

template <>
struct binder<std::nullptr_t> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, std::nullptr_t) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then(
            [&ctxt](int idx) noexcept -> auto { return ctxt.to_expected(::sqlite3_bind_null(ctxt.get(), idx)); });
    }
};

template <>
struct binder_registry<std::nullptr_t> {
    using type = binder<std::nullptr_t>;
};

template <typename Char>
struct binder_cstring {
    auto operator()(binder_context auto& ctxt, const Char* value, int size = -1) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then([&ctxt, value, size](int idx) noexcept -> auto {
            // NOLINTNEXTLINE(cppcoreguidelines-*)
            const auto* cstr = reinterpret_cast<const char*>(value);
            return ctxt.to_expected(sqlite3_bind_text(ctxt.get(), idx, cstr, size, ctxt.get_destructor()));
        });
    }
};

template <>
struct binder_registry<const char*> {
    using type = binder_cstring<char>;
};

template <>
struct binder_registry<char*> {
    using type = binder_cstring<char>;
};

template <>
struct binder_registry<const char8_t*> {
    using type = binder_cstring<char8_t>;
};

template <>
struct binder_registry<char8_t*> {
    using type = binder_cstring<char8_t>;
};

template <>
struct binder<std::string_view> {
    auto operator()(binder_context auto& ctxt, std::string_view value) const noexcept
        -> std::expected<void, std::error_code> {
        return binder_cstring<char>{}(ctxt, value.data(), static_cast<int>(value.size()));
    }
};

template <>
struct binder_registry<std::string_view> {
    using type = binder<std::string_view>;
};

template <>
struct binder<std::u8string_view> {
    auto operator()(binder_context auto& ctxt, std::u8string_view value) const noexcept
        -> std::expected<void, std::error_code> {
        return binder_cstring<char8_t>{}(ctxt, value.data(), static_cast<int>(value.size()));
    }
};

template <>
struct binder_registry<std::u8string_view> {
    using type = binder<std::u8string_view>;
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct binder<std::span<T, Extent>> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, std::span<T, Extent> value) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then([&ctxt, value](int idx) noexcept -> auto {
            return ctxt.to_expected(::sqlite3_bind_blob64(
                ctxt.get(), idx, static_cast<const void*>(value.data()), value.size_bytes(), ctxt.get_destructor()));
        });
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct binder_registry<std::span<T, Extent>> {
    using type = binder<std::span<T, Extent>>;
};

export struct zeroblob {
    std::size_t size{};
};

template <>
struct binder<zeroblob> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, zeroblob value) const noexcept
        -> std::expected<void, std::error_code> {
        return ctxt.get_and_advance_index().and_then([&ctxt, value](int idx) noexcept -> auto {
            return ctxt.to_expected(::sqlite3_bind_zeroblob64(ctxt.get(), idx, value.size));
        });
    }
};

template <>
struct binder_registry<zeroblob> {
    using type = binder<zeroblob>;
};

template <typename T>
concept is_tuple_like = requires { typename std::tuple_size<std::decay_t<T>>::type; };

template <is_tuple_like T>
struct binder<T> {
    template <typename Tuple>
    auto operator()(binder_context auto& ctxt, Tuple&& tuple) const noexcept -> std::expected<void, std::error_code> {
        using TupleD = std::decay_t<Tuple>;
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> auto {
            std::expected<void, std::error_code> result{};
            std::ignore = ((result = binder_t<std::tuple_element_t<Is, TupleD>>{}(
                                ctxt, std::get<Is>(std::forward<Tuple>(tuple)))) &&
                           ...);
            return result;
        }(std::make_index_sequence<std::tuple_size_v<TupleD>>{});
    }
};

template <is_tuple_like T>
struct binder_registry<T> {
    using type = binder<T>;
};

template <typename T>
struct binder<std::optional<T>> {
    [[nodiscard]] auto operator()(binder_context auto& ctxt, const std::optional<T>& value) const noexcept
        -> std::expected<void, std::error_code> {
        return value ? binder_t<T>{}(ctxt, *value) : binder_t<std::nullptr_t>{}(ctxt, nullptr);
    }
};

template <typename T>
struct binder_registry<std::optional<T>> {
    using type = binder<std::optional<T>>;
};

} // namespace sqlixx
