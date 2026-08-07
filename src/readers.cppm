// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:readers;

import std;
import :index;

namespace sqlixx {

export struct reader_context {
    explicit reader_context(::sqlite3_stmt* stmt_handle) noexcept
        : handle_(stmt_handle), column_index_(0, ::sqlite3_data_count(stmt_handle)) {}

    [[nodiscard]] constexpr auto get() const noexcept -> ::sqlite3_stmt* { return handle_; }

    [[nodiscard]] constexpr auto set_column_index(int column_index) noexcept -> std::expected<void, std::error_code> {
        return column_index_.set(column_index);
    }

    [[nodiscard]] constexpr auto get_and_advance_column_index() noexcept -> std::expected<int, std::error_code> {
        return column_index_.get_and_advance();
    }

private:
    ::sqlite3_stmt* handle_;
    checked_index<int> column_index_;
};

template <typename T>
struct reader;

template <std::integral T>
struct reader<T> {
    [[nodiscard]] auto operator()(reader_context& ctxt, T& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = ctxt.get_and_advance_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        value = static_cast<T>(::sqlite3_column_int64(ctxt.get(), *column_index));

        return {};
    }
};

template <std::floating_point T>
struct reader<T> {
    [[nodiscard]] auto operator()(reader_context& ctxt, T& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = ctxt.get_and_advance_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        value = static_cast<T>(::sqlite3_column_double(ctxt.get(), *column_index));

        return {};
    }
};

template <>
struct reader<std::string_view> {
    [[nodiscard]] auto operator()(reader_context& ctxt, std::string_view& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = ctxt.get_and_advance_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        const auto* const raw_text = ::sqlite3_column_text(ctxt.get(), *column_index);
        if (raw_text != nullptr) [[likely]] {
            const int bytes = ::sqlite3_column_bytes(ctxt.get(), *column_index);
            // NOLINTNEXTLINE(cppcoreguidelines-*)
            value = {reinterpret_cast<const char*>(raw_text), static_cast<std::size_t>(bytes)};
        } else {
            value = {};
        }

        return {};
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct reader<std::span<T, Extent>> {
    [[nodiscard]] auto operator()(reader_context& ctxt, std::span<T, Extent>& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = ctxt.get_and_advance_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        const auto* const raw_blob = ::sqlite3_column_blob(ctxt.get(), *column_index);

        if (raw_blob != nullptr) [[likely]] {
            const int bytes = ::sqlite3_column_bytes(ctxt.get(), *column_index);
            const std::size_t element_count = static_cast<std::size_t>(bytes) / sizeof(T);
            if constexpr (Extent != std::dynamic_extent) {
                if (element_count != Extent) [[unlikely]] {
                    return std::unexpected(errc::invalid_size);
                }
            }
            // NOLINTNEXTLINE(cppcoreguidelines-*)
            const auto* const typed_data = reinterpret_cast<const T*>(raw_blob);
            value = {typed_data, element_count};
        } else {
            value = {};
        }

        return {};
    }
};

template <typename T>
concept tuple_like = requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; };

template <tuple_like T>
struct reader<T> {
    template <typename Tuple>
    [[nodiscard]] auto operator()(reader_context& ctxt, Tuple&& tuple) const noexcept
        -> std::expected<void, std::error_code> {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> auto {
            std::expected<void, std::error_code> result{};
            std::ignore =
                ((result = reader<std::remove_cvref_t<std::tuple_element_t<Is, std::remove_cvref_t<Tuple>>>>{}(
                      ctxt, std::get<Is>(std::forward<Tuple>(tuple)))) &&
                 ...);
            return result;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
    }
};

} // namespace sqlixx
