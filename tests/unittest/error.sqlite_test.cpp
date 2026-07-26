// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

TEST_CASE("Error handling with sqlite error codes", "[error]") {
    sqlite_mock mock;

    SECTION("The sqlite error category") {
        const auto& category = sqlixx::sqlite_category();

        SECTION("The name is sqlite") {
            CHECK(std::string_view(category.name()) == "sqlite");
        }

        SECTION("The error message is provided by sqlite") {
            const auto result_code = SQLITE_BUSY;
            const auto* expected_message = "database is locked";

            REQUIRE_CALL(mock, sqlite3_errstr(result_code)).RETURN(expected_message);

            const auto actual_message = category.message(result_code);

            CHECK(actual_message == expected_message);
        }

        SECTION("Non-error result codes are mapped onto 0") {
            auto result_code = GENERATE(SQLITE_ROW, SQLITE_DONE, SQLITE_OK_LOAD_PERMANENTLY);

            CHECK(!category.default_error_condition(result_code));
        }

        SECTION("Extended result codes are mapped onto primary result codes") {
            const auto [primary_result_code, extended_result_code] =
                GENERATE(table<int, int>({{SQLITE_ERROR, SQLITE_ERROR_RETRY},
                                          {SQLITE_BUSY, SQLITE_BUSY_TIMEOUT},
                                          {SQLITE_IOERR, SQLITE_IOERR_WRITE}}));

            const auto condition = category.default_error_condition(extended_result_code);

            CHECK(condition.value() == primary_result_code);
        }
    }

    SECTION("The sqlite error conditions") {
        const auto code = GENERATE(sqlixx::sqlite_errc::error,
                                   sqlixx::sqlite_errc::perm,
                                   sqlixx::sqlite_errc::abort,
                                   sqlixx::sqlite_errc::busy,
                                   sqlixx::sqlite_errc::locked,
                                   sqlixx::sqlite_errc::nomem,
                                   sqlixx::sqlite_errc::readonly,
                                   sqlixx::sqlite_errc::interrupt,
                                   sqlixx::sqlite_errc::ioerr,
                                   sqlixx::sqlite_errc::corrupt,
                                   sqlixx::sqlite_errc::notfound,
                                   sqlixx::sqlite_errc::full,
                                   sqlixx::sqlite_errc::cantopen,
                                   sqlixx::sqlite_errc::protocol,
                                   sqlixx::sqlite_errc::schema,
                                   sqlixx::sqlite_errc::toobig,
                                   sqlixx::sqlite_errc::constraint,
                                   sqlixx::sqlite_errc::mismatch,
                                   sqlixx::sqlite_errc::misuse,
                                   sqlixx::sqlite_errc::nolfs,
                                   sqlixx::sqlite_errc::auth,
                                   sqlixx::sqlite_errc::range,
                                   sqlixx::sqlite_errc::notadb);

        const auto condition = sqlixx::make_error_condition(code);

        CHECK(&condition.category() == &sqlixx::sqlite_category());
        CHECK(condition.value() == std::to_underlying(code));
        CHECK(condition == code);
    }

    SECTION("The unchecked sqlite error codes") {
        const auto [result_code, condition_value] =
            GENERATE(table<int, sqlixx::sqlite_errc>({{SQLITE_ERROR, sqlixx::sqlite_errc::error},
                                                      {SQLITE_ERROR_RETRY, sqlixx::sqlite_errc::error},
                                                      {SQLITE_BUSY, sqlixx::sqlite_errc::busy},
                                                      {SQLITE_BUSY_TIMEOUT, sqlixx::sqlite_errc::busy},
                                                      {SQLITE_IOERR, sqlixx::sqlite_errc::ioerr},
                                                      {SQLITE_IOERR_WRITE, sqlixx::sqlite_errc::ioerr},
                                                      {SQLITE_ROW, static_cast<sqlixx::sqlite_errc>(SQLITE_ROW)},
                                                      {SQLITE_DONE, static_cast<sqlixx::sqlite_errc>(SQLITE_DONE)}}));

        const auto code = sqlixx::make_sqlite_error_code(result_code);

        CHECK(&code.category() == &sqlixx::sqlite_category());
        CHECK(code.value() == result_code);
        CHECK(code == condition_value);
    }

    SECTION("The checked sqlite error codes") {
        SECTION("Non-error result codes are mapped onto 0") {
            const auto result_code = GENERATE(SQLITE_ROW, SQLITE_DONE, SQLITE_OK_LOAD_PERMANENTLY);

            const auto code = sqlixx::make_checked_sqlite_error_code(result_code);

            CHECK_FALSE(code);
        }

        SECTION("Error result codes are mapped onto extended result codes") {
            const auto result_code = GENERATE(
                SQLITE_ERROR, SQLITE_ERROR_RETRY, SQLITE_BUSY, SQLITE_BUSY_TIMEOUT, SQLITE_IOERR, SQLITE_IOERR_WRITE);

            const auto code = sqlixx::make_checked_sqlite_error_code(result_code);

            CHECK(code.value() == result_code);
        }
    }
}
