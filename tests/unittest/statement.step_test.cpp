// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify traditional statement execution and iteration mechanics", "[statement][step]") {
    GIVEN("A registered SQLite mock and an active statement handle") {
        sqlite_mock mock;

        auto* dummy_stmt = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
        sqlixx::statement_handle stmt{dummy_stmt};

        WHEN("Stepping through a statement execution") {
            AND_WHEN("The backend returns a data row") {
                REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW);
                auto res = sqlixx::step(stmt);

                THEN("Translate the status to row_state::available") {
                    REQUIRE(res.has_value());
                    CHECK(*res == sqlixx::row_state::available);
                }
            }

            AND_WHEN("The backend signals completion") {
                REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_DONE);
                auto res = sqlixx::step(stmt);

                THEN("Translate the status to step_state::done") {
                    REQUIRE(res.has_value());
                    CHECK(*res == sqlixx::row_state::empty);
                }
            }

            AND_WHEN("The backend encounters an execution error") {
                REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_CORRUPT);
                auto res = sqlixx::step(stmt);

                THEN("Fail with the corresponding database error code") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::sqlite_errc::corrupt);
                }
            }
        }

        WHEN("Resetting state or clearing active parameter bindings") {
            AND_WHEN("Invoking statement reset successfully") {
                REQUIRE_CALL(mock, sqlite3_reset(dummy_stmt)).RETURN(SQLITE_OK);
                auto res = sqlixx::reset(stmt);
                THEN("Return a successful expected container") {
                    CHECK(res.has_value());
                }
            }

            AND_WHEN("Invoking statement reset encounters a database failure") {
                REQUIRE_CALL(mock, sqlite3_reset(dummy_stmt)).RETURN(SQLITE_MISUSE);
                auto res = sqlixx::reset(stmt);
                THEN("Fail and propagate the corresponding error code") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::sqlite_errc::misuse);
                }
            }

            AND_WHEN("Invoking clear bindings successfully") {
                REQUIRE_CALL(mock, sqlite3_clear_bindings(dummy_stmt)).RETURN(SQLITE_OK);
                auto res = sqlixx::clear_bindings(stmt);
                THEN("Return a successful expected container") {
                    CHECK(res.has_value());
                }
            }

            AND_WHEN("Invoking clear bindings encounters a database failure") {
                REQUIRE_CALL(mock, sqlite3_clear_bindings(dummy_stmt)).RETURN(SQLITE_CORRUPT);
                auto res = sqlixx::clear_bindings(stmt);
                THEN("Fail and propagate the corresponding error code") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::sqlite_errc::corrupt);
                }
            }
        }

        WHEN("Simulating a traditional while-loop data hydration script") {
            trompeloeil::sequence seq;

            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_ROW).IN_SEQUENCE(seq);
            REQUIRE_CALL(mock, sqlite3_step(dummy_stmt)).RETURN(SQLITE_DONE).IN_SEQUENCE(seq);

            REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(1).TIMES(2);
            REQUIRE_CALL(mock, sqlite3_column_int(dummy_stmt, 0)).RETURN(10);
            REQUIRE_CALL(mock, sqlite3_column_int(dummy_stmt, 0)).RETURN(20);

            THEN("Iterate and extract records cleanly using stack-allocated targets") {
                int total_sum = 0;
                int current_value = 0;

                while (auto step_res = sqlixx::step(stmt)) {
                    if (*step_res == sqlixx::row_state::empty) {
                        break;
                    }

                    auto read_status = sqlixx::read(stmt, 0, std::tie(current_value));
                    REQUIRE(read_status.has_value());
                    total_sum += current_value;
                }

                CHECK(total_sum == 30);
            }
        }
    }
}
