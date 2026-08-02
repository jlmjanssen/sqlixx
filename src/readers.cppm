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
    explicit row_context(::sqlite3_stmt* stmt_handle) noexcept
        : handle_(stmt_handle), available_columns_(::sqlite3_data_count(stmt_handle)) {}

    [[nodiscard]] constexpr auto get() const noexcept -> ::sqlite3_stmt* { return handle_; }

    [[nodiscard]] constexpr auto seek_column_index(int column_index) noexcept -> std::expected<void, std::error_code> {
        if (column_index < 0 || column_index > available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        current_column_index_ = column_index;
        return {};
    }

    [[nodiscard]] constexpr auto current_column_index() const noexcept -> std::expected<int, std::error_code> {
        if (current_column_index_ >= available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        return current_column_index_;
    }

    [[nodiscard]] constexpr auto advance_column_index() noexcept -> std::expected<void, std::error_code> {
        if (current_column_index_ >= available_columns_) [[unlikely]] {
            return std::unexpected(errc::invalid_column_index);
        }
        ++current_column_index_;
        return {};
    }

private:
    ::sqlite3_stmt* handle_;
    int available_columns_;
    int current_column_index_{0};
};

template <typename T>
concept column_provider = requires(T row) {
    { row.get() } -> std::same_as<::sqlite3_stmt*>;
    { row.current_column_index() } -> std::same_as<std::expected<int, std::error_code>>;
    { row.advance_column_index() } -> std::same_as<std::expected<void, std::error_code>>;
    requires noexcept(row.get());
    requires noexcept(row.current_column_index());
    requires noexcept(row.advance_column_index());
};

template <std::integral T>
struct reader<T> {
    [[nodiscard]] auto operator()(column_provider auto& row, T& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = row.current_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        if constexpr (sizeof(T) < sizeof(int) || (sizeof(T) == sizeof(int) && std::is_signed_v<T>)) {
            value = static_cast<T>(::sqlite3_column_int(row.get(), *column_index));
        } else if constexpr (sizeof(T) < sizeof(std::int64_t) ||
                             (sizeof(T) == sizeof(std::int64_t) && std::is_signed_v<T>)) {
            value = static_cast<T>(::sqlite3_column_int64(row.get(), *column_index));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported integral size");
        }

        return row.advance_column_index();
    }
};

template <std::floating_point T>
struct reader<T> {
    [[nodiscard]] auto operator()(column_provider auto& row, T& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = row.current_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        value = static_cast<T>(::sqlite3_column_double(row.get(), *column_index));

        return row.advance_column_index();
    }
};

template <>
struct reader<std::string_view> {
    [[nodiscard]] auto operator()(column_provider auto& row, std::string_view& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = row.current_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        const auto* const raw_text = ::sqlite3_column_text(row.get(), *column_index);
        if (raw_text != nullptr) [[likely]] {
            const int bytes = ::sqlite3_column_bytes(row.get(), *column_index);
            // NOLINTNEXTLINE(cppcoreguidelines-*)
            value = {reinterpret_cast<const char*>(raw_text), static_cast<std::size_t>(bytes)};
        } else {
            value = {};
        }

        return row.advance_column_index();
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct reader<std::span<T, Extent>> {
    [[nodiscard]] auto operator()(column_provider auto& row, std::span<T, Extent>& value) const noexcept
        -> std::expected<void, std::error_code> {
        const auto column_index = row.current_column_index();
        if (!column_index) [[unlikely]] {
            return std::unexpected(column_index.error());
        }

        const auto* const raw_blob = ::sqlite3_column_blob(row.get(), *column_index);

        if (raw_blob != nullptr) [[likely]] {
            const int bytes = ::sqlite3_column_bytes(row.get(), *column_index);
            const std::size_t element_count = static_cast<std::size_t>(bytes) / sizeof(T);
            if constexpr (Extent != std::dynamic_extent) {
                if (element_count != Extent) [[unlikely]] {
                    return std::unexpected(errc::general_error);
                }
            }
            // NOLINTNEXTLINE(cppcoreguidelines-*)
            const auto* const typed_data = reinterpret_cast<const T*>(raw_blob);
            value = {typed_data, element_count};
        } else {
            value = {};
        }

        return row.advance_column_index();
    }
};

template <typename T>
concept tuple_like = requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; };

template <tuple_like T>
struct reader<T> {
    template <typename Tuple>
    [[nodiscard]] auto operator()(column_provider auto& row, Tuple&& tuple) const noexcept
        -> std::expected<void, std::error_code> {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> auto {
            std::expected<void, std::error_code> result{};
            std::ignore =
                ((result = reader<std::remove_cvref_t<std::tuple_element_t<Is, std::remove_cvref_t<Tuple>>>>{}(
                      row, std::get<Is>(std::forward<Tuple>(tuple)))) &&
                 ...);
            return result;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
    }
};

export template <typename... Ts>
[[nodiscard]] auto read(row_context& row, Ts&&... values) noexcept -> std::expected<void, std::error_code> {
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = reader<std::remove_cvref_t<Ts>>{}(row, std::forward<Ts>(values))) && ...);
    return result;
}

export template <typename... Ts>
[[nodiscard]] auto read(statement_handle stmt, int column, Ts&&... values) noexcept
    -> std::expected<void, std::error_code> {
    row_context row{stmt.get()};
    auto result = row.seek_column_index(column);
    if (!result) {
        return result;
    }
    return read(row, std::forward<Ts>(values)...);
}

} // namespace sqlixx
