// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

export module sqlixx:index;

import std;
import :error;

namespace sqlixx {

template <typename Index = int>
struct index_provider {
    index_provider(Index begin, Index end) noexcept : begin_(begin), end_(end), current_(begin) {}

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

} // namespace sqlixx
