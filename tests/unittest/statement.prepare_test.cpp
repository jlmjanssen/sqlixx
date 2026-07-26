// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify statement preparation factory behavior and tail string processing", "[statement][prepare]") {
    GIVEN("A registered SQLite mock and an active connection handle") {
        using namespace std::string_view_literals;
        sqlite_mock mock;

        auto* dummy_db = reinterpret_cast<::sqlite3*>(0xBAAD5EED);
        auto* dummy_stmt = reinterpret_cast<::sqlite3_stmt*>(0xBABEFACE);
        sqlixx::connection_handle conn(dummy_db);

        WHEN("Preparing a statement from a traditional null-terminated C-string") {
            const auto* sql = "SELECT * FROM users;";

            AND_WHEN("Invoking preparation with default flags") {
                REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db, _, -1, 0, _, _))
                    .WITH(std::string_view(_2) == sql)
                    .SIDE_EFFECT(*_5 = dummy_stmt)
                    .SIDE_EFFECT(*_6 = sql + std::strlen(sql))
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt)).RETURN(SQLITE_OK);

                auto result = sqlixx::prepare_statement(conn, sql);

                THEN("Return a valid statement holding the handle and an empty tail view") {
                    REQUIRE(result.has_value());
                    CHECK(result->stmt.get() == dummy_stmt);
                    CHECK(result->tail.empty());
                }
            }

            AND_WHEN("Invoking preparation with custom dynamic flags") {
                REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db, _, -1, SQLITE_PREPARE_PERSISTENT, _, _))
                    .WITH(std::string_view(_2) == sql)
                    .SIDE_EFFECT(*_5 = dummy_stmt)
                    .SIDE_EFFECT(*_6 = sql + std::strlen(sql))
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt)).RETURN(SQLITE_OK);

                auto dynamic_persistent = sqlixx::prep::dyn_flags(SQLITE_PREPARE_PERSISTENT);
                auto result = sqlixx::prepare_statement(conn, sql, dynamic_persistent);

                THEN("Forward flags correctly and pass validation hooks") {
                    REQUIRE(result.has_value());
                }
            }
        }

        WHEN("Preparing a statement from a modern std::string_view") {
            AND_WHEN("Multiple statements are chained together inside the view context") {
                auto sql = "SELECT 1; SELECT 2;"sv;
                auto expected_len = static_cast<int>(sql.size());
                auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;
                const auto* const expected_tail_ptr = sql.data() + 9;

                REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db, sql.data(), expected_len, expected_flags, _, _))
                    .SIDE_EFFECT(*_5 = dummy_stmt)
                    .SIDE_EFFECT(*_6 = expected_tail_ptr)
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt)).RETURN(SQLITE_OK);

                auto result = sqlixx::prepare_statement(conn, sql, sqlixx::prep::persistent, sqlixx::prep::normalize);

                THEN("Extract the exact remaining tail sub-string view correctly") {
                    REQUIRE(result.has_value());
                    CHECK(result->stmt.get() == dummy_stmt);
                    CHECK(result->tail == " SELECT 2;"sv);
                }
            }

            AND_WHEN("The single statement consumes the complete length of the view container") {
                auto sql = "SELECT 1;"sv;
                auto expected_len = static_cast<int>(sql.size());
                const auto* const expected_tail_ptr = sql.data() + sql.size();

                REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db, sql.data(), expected_len, 0, _, _))
                    .SIDE_EFFECT(*_5 = dummy_stmt)
                    .SIDE_EFFECT(*_6 = expected_tail_ptr)
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt)).RETURN(SQLITE_OK);

                auto result = sqlixx::prepare_statement(conn, sql);

                THEN("Return a valid statement object alongside an empty tail reference") {
                    REQUIRE(result.has_value());
                    CHECK(result->stmt.get() == dummy_stmt);
                    CHECK(result->tail.empty());
                }
            }
        }

        WHEN("Failing to prepare a statement due to bad syntax") {
            const auto* const sql = "SELECT INVALID SYNTAX;";

            REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db, _, -1, 0, _, _))
                .WITH(std::string_view(_2) == sql)
                .SIDE_EFFECT(*_5 = nullptr)
                .RETURN(SQLITE_ERROR);

            FORBID_CALL(mock, sqlite3_finalize(_));

            auto result = sqlixx::prepare_statement(conn, sql);

            THEN("Return an unexpected error code container without leaking resources") {
                REQUIRE_FALSE(result.has_value());
                CHECK(result.error() == sqlixx::sqlite_errc::error);
            }
        }
    }
}
