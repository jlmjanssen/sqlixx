// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

export module sqlixx:error;
import std;

namespace sqlixx {

// NOLINTNEXTLINE(performance-*)
export enum class errc : int {
    general_error = 1,
    invalid_handle = 2,
    invalid_argument = 3,
    invalid_column_index = 4,
    no_active_row = 5,
    size_ = no_active_row
};

class sqlixx_error_category final : public std::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override { return "sqlixx"; }

    [[nodiscard]] auto message(int code) const -> std::string override {
        auto index = static_cast<std::size_t>(code);
        return messages.at(index < messages.size() ? index : 0);
    }

private:
    static constexpr std::array messages{"Unknown error",
                                         "General error",
                                         "Invalid handle",
                                         "Invalid argument",
                                         "Invalid column index",
                                         "No active row"};

    static_assert(messages.size() == 1 + std::to_underlying(errc::size_), "The messages array is not up-to-date.");
};

export [[nodiscard]] auto sqlixx_category() noexcept -> const std::error_category& {
    static sqlixx_error_category instance;
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
