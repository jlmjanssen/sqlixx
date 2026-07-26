// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;
#include <sqlite3.h>
export module sqlixx:readers;
import std;
import :error;
import :statement;

namespace sqlixx {

template <typename T>
struct reader;

export struct row_context {
    explicit row_context(statement_handle stmt, int column = 0) noexcept
        : handle_(stmt.get()), column_(column), available_columns_(::sqlite3_data_count(stmt.get())) {}

    [[nodiscard]] constexpr auto available_columns() const noexcept -> int { return available_columns_; }
    [[nodiscard]] constexpr auto column_index() const noexcept -> int { return column_; }

private:
    template <typename T>
    friend struct reader;

    ::sqlite3_stmt* handle_;
    int column_;
    int available_columns_;
};

template <std::integral T>
struct reader<T> {
    [[nodiscard]] auto operator()(row_context& ctxt, T& value) const noexcept -> std::expected<void, std::error_code> {
        if (ctxt.column_ >= ctxt.available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        if constexpr (sizeof(T) < sizeof(int) || (sizeof(T) == sizeof(int) && std::is_signed_v<T>)) {
            value = static_cast<T>(::sqlite3_column_int(ctxt.handle_, ctxt.column_));
        } else if constexpr (sizeof(T) < sizeof(std::int64_t) ||
                             (sizeof(T) == sizeof(std::int64_t) && std::is_signed_v<T>)) {
            value = static_cast<T>(::sqlite3_column_int64(ctxt.handle_, ctxt.column_));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported integral size");
        }
        ctxt.column_++;
        return {};
    }
};

template <std::floating_point T>
struct reader<T> {
    [[nodiscard]] auto operator()(row_context& ctxt, T& value) const noexcept -> std::expected<void, std::error_code> {
        if (ctxt.column_ >= ctxt.available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        value = static_cast<T>(::sqlite3_column_double(ctxt.handle_, ctxt.column_));
        ctxt.column_++;
        return {};
    }
};

template <>
struct reader<std::string_view> {
    [[nodiscard]] auto operator()(row_context& ctxt, std::string_view& value) const noexcept
        -> std::expected<void, std::error_code> {
        if (ctxt.column_ >= ctxt.available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        const auto* const void_text = static_cast<const void*>(::sqlite3_column_text(ctxt.handle_, ctxt.column_));
        if (void_text != nullptr) [[likely]] {
            const auto* const text = static_cast<const char*>(void_text);
            const int bytes = ::sqlite3_column_bytes(ctxt.handle_, ctxt.column_);
            value = {text, static_cast<std::size_t>(bytes)};
        } else {
            value = {};
        }
        ctxt.column_++;
        return {};
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct reader<std::span<T, Extent>> {
    [[nodiscard]] auto operator()(row_context& ctxt, std::span<T, Extent>& value) const noexcept
        -> std::expected<void, std::error_code> {
        if (ctxt.column_ >= ctxt.available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        const auto* data = static_cast<const std::byte*>(::sqlite3_column_blob(ctxt.handle_, ctxt.column_));
        if (data != nullptr) [[likely]] {
            const int bytes = ::sqlite3_column_bytes(ctxt.handle_, ctxt.column_);
            value = {data, static_cast<std::size_t>(bytes)};
        } else {
            value = {};
        }
        ctxt.column_++;
        return {};
    }
};

template <typename T>
concept tuple_like = requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; };

template <tuple_like T>
struct reader<T> {
    template <typename Tuple>
    [[nodiscard]] auto operator()(row_context& ctxt, Tuple&& tuple) const noexcept
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

export template <typename... Ts>
[[nodiscard]] auto read(row_context& ctxt, Ts&&... values) noexcept -> std::expected<void, std::error_code> {
    if (ctxt.available_columns() == 0) [[unlikely]] {
        return std::unexpected(errc::no_active_row);
    }
    if (ctxt.column_index() < 0 || ctxt.column_index() >= ctxt.available_columns()) {
        return std::unexpected(errc::invalid_column_index);
    }
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = reader<std::remove_cvref_t<Ts>>{}(ctxt, std::forward<Ts>(values))) && ...);
    return result;
}

export template <typename... Ts>
[[nodiscard]] auto read(statement_handle stmt, int column, Ts&&... values) noexcept
    -> std::expected<void, std::error_code> {
    row_context ctxt{stmt, column};
    return read(ctxt, std::forward<Ts>(values)...);
}

} // namespace sqlixx
