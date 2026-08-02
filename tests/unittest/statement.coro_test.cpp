// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify coroutine-generator execution pipeline", "[statement][coro]") {
    GIVEN("A registered SQLite mock and a statement handle") {
        sqlite_mock mock;

        auto* dummy_stmt = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
        sqlixx::statement_handle stmt{dummy_stmt};

        WHEN("Executing a query that streams data rows successfully") {
            trompeloeil::sequence seq;

            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_DONE).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_reset(dummy_stmt)).RETURN(SQLITE_OK).IN_SEQUENCE(seq);

            REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(1).TIMES(2);
            REQUIRE_CALL(mock, sqlite3_column_int(dummy_stmt, 0)).RETURN(100);
            REQUIRE_CALL(mock, sqlite3_column_int(dummy_stmt, 0)).RETURN(200);

            THEN("Iterate fluently using a modern range-based for-loop") {
                int total_sum = 0;
                int current_value = 0;

                for (auto row : sqlixx::execute(stmt)) {
                    auto read_status = sqlixx::read(*row, std::tie(current_value));
                    REQUIRE(read_status.has_value());
                    total_sum += current_value;
                }

                CHECK(total_sum == 300);
            }
        }

        WHEN("Executing a query that hits a backend database error") {
            trompeloeil::sequence seq;

            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_CORRUPT).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_reset(dummy_stmt)).RETURN(SQLITE_OK).IN_SEQUENCE(seq);

            REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(1);
            REQUIRE_CALL(mock, sqlite3_column_int(dummy_stmt, 0)).RETURN(50);

            THEN("Intercept the error through the noexcept callback and halt iteration") {
                int total_sum = 0;
                int current_value = 0;
                std::error_code intercepted_error{};

                auto rows = sqlixx::execute(stmt);

                for (auto row : rows) {
                    if (!row) {
                        intercepted_error = row.error();
                        break;
                    }
                    auto read_status = sqlixx::read(*row, std::tie(current_value));
                    REQUIRE(read_status.has_value());
                    total_sum += current_value;
                }

                CHECK(total_sum == 50);
                CHECK(intercepted_error == sqlixx::sqlite_errc::corrupt);
            }
        }

        WHEN("Executing a query that hits an error but no error handler is provided") {
            trompeloeil::sequence seq;

            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_CORRUPT).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_reset(dummy_stmt)).RETURN(SQLITE_OK).IN_SEQUENCE(seq);

            THEN("Gracefully halt iteration without invoking or crashing on a null callback") {
                int row_count = 0;

                auto rows = sqlixx::execute(stmt);

                for (auto row : rows) {
                    if (!row)
                        break;
                    std::ignore = row;
                    ++row_count;
                }

                CHECK(row_count == 0);
            }
        }
    }
}
