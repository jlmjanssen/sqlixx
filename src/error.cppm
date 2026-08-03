// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

export module sqlixx:error;

import std;

namespace sqlixx {
// NOLINTNEXTLINE(performance-*)
export enum class errc : int { invalid_index = 1, invalid_size = 2, size_ = invalid_size };

class sqlixx_error_category final : public std::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override { return "sqlixx"; }

    [[nodiscard]] auto message(int code) const -> std::string override {
        static constexpr std::array messages{"Unknown error", "Invalid index", "Invalid size"};
        static_assert(messages.size() == 1 + std::to_underlying(errc::size_), "The messages array is not up-to-date.");

        auto index = static_cast<std::size_t>(code);
        return messages.at(index < messages.size() ? index : 0);
    }
};

export [[nodiscard]] auto sqlixx_category() noexcept -> const std::error_category& {
    static const sqlixx_error_category instance;
    return instance;
}

export [[nodiscard]] auto make_error_code(errc code) noexcept -> std::error_code {
    return {std::to_underlying(code), sqlixx_category()};
}

} // namespace sqlixx

namespace std {
template <>
struct is_error_code_enum<sqlixx::errc> : std::true_type {};
} // namespace std
