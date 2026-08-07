// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:concepts;

import std;

namespace sqlixx {

template <typename Provider, typename Index>
concept index_provider = requires(Provider provider, Index idx) {
    { provider.set(idx) } -> std::same_as<std::expected<void, std::error_code>>;
    { provider.get_and_advance() } -> std::same_as<std::expected<Index, std::error_code>>;
};

template <typename T>
concept binder_context = requires(T ctxt, int idx) {
    { ctxt.set_index(idx) };
    { ctxt.get_and_advance_index() };
    { ctxt.get_destructor() } -> std::same_as<::sqlite3_destructor_type>;
};

template <typename T>
concept is_reader_context = requires(T ctxt, int idx) {
    { ctxt.set_index(idx) };
    { ctxt.get_and_advance_index() };
} && !requires(T ctxt) {
    { ctxt.get_destructor() };
};

template <typename T>
concept sqlite_integer = std::integral<T> && requires {
    requires((sizeof(T) < sizeof(std::int64_t)) || ((sizeof(T) == sizeof(std::int64_t)) && std::is_signed_v<T>));
};

template <typename T>
concept sqlite_real = std::floating_point<T> && requires { requires(sizeof(T) <= sizeof(double)); };

template <typename T>
concept tuple_like = requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; };

} // namespace sqlixx
