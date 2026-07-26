// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

import std;
import sqlixx;

TEST_CASE("Error handling with sqlixx error codes", "[error]") {
    SECTION("The sqlixx error category") {
        const auto& category = sqlixx::sqlixx_category();
        const auto unknown = category.message(0);

        SECTION("The name is sqlixx") {
            CHECK(std::string_view(category.name()) == "sqlixx");
        }

        SECTION("The unknown error string is not empty") {
            CHECK(unknown.length() != 0);
        }

        SECTION("Valid errors are not unknown") {
            const auto code = GENERATE(sqlixx::errc::general_error,
                                       sqlixx::errc::invalid_handle,
                                       sqlixx::errc::invalid_argument,
                                       sqlixx::errc::invalid_column_index,
                                       sqlixx::errc::no_active_row);

            const auto message = category.message(std::to_underlying(code));

            CHECK(message.length() != 0);
            CHECK(message != unknown);
        }

        SECTION("Invalid errors are unknown") {
            const auto code = GENERATE(-1, 0, std::to_underlying(sqlixx::errc::size_) + 1);

            const auto message = category.message(code);

            CHECK(message == unknown);
        }
    }

    SECTION("The sqlixx error codes") {
        const auto code = GENERATE(sqlixx::errc::general_error,
                                   sqlixx::errc::invalid_handle,
                                   sqlixx::errc::invalid_argument,
                                   sqlixx::errc::invalid_column_index,
                                   sqlixx::errc::no_active_row);

        const auto error = sqlixx::make_error_code(code);

        SECTION("The category is sqlixx") {
            const auto& category = error.category();

            CHECK(&category == &sqlixx::sqlixx_category());
        }

        SECTION("The error code value matches") {
            CHECK(error.value() == std::to_underlying(code));
            CHECK(error == code);
        }
    }
}
