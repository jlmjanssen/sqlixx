// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

export module sqlixx:index;

import std;
import :error;

namespace sqlixx {

template <typename Provider, typename Index>
concept index_provider = requires(Provider provider, Index idx) {
    { provider.set(idx) } -> std::convertible_to<std::expected<void, std::error_code>>;
    { provider.get_and_advance() } -> std::convertible_to<std::expected<Index, std::error_code>>;
};

template <typename Index = int>
struct checked_index {
    constexpr checked_index(Index begin, Index end) noexcept : begin_(begin), end_(end), current_(begin) {}

    [[nodiscard]] constexpr auto set(Index index) noexcept -> std::expected<void, std::error_code> {
        if (index < begin_ || index > end_) [[unlikely]] {
            return std::unexpected(errc::invalid_index);
        }

        current_ = index;
        return {};
    }

    [[nodiscard]] constexpr auto get_and_advance() noexcept -> std::expected<Index, std::error_code> {
        if (current_ >= end_) [[unlikely]] {
            return std::unexpected(errc::invalid_index);
        }

        return current_++;
    }

private:
    Index begin_;
    Index end_;
    Index current_;
};

template <typename Index = int>
struct unchecked_index {
    constexpr explicit unchecked_index(Index begin) noexcept : current_(begin) {}

    [[nodiscard]] constexpr auto set(Index index) noexcept -> std::expected<void, std::error_code> {
        current_ = index;
        return {};
    }

    [[nodiscard]] constexpr auto get_and_advance() noexcept -> std::expected<Index, std::error_code> {
        return current_++;
    }

private:
    Index current_;
};

} // namespace sqlixx
