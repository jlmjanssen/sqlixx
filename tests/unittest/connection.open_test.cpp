// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify connection factory methods behavior and resource initialization", "[connection][open]") {
    GIVEN("A registered SQLite mock instance") {
        sqlite_mock mock;
        const auto* db_name = "test.db";
        auto* dummy_db = reinterpret_cast<::sqlite3*>(0xBAAD5EED);

        WHEN("Opening a connection successfully") {
            AND_WHEN("Invoking factory with default flags") {
                auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

                REQUIRE_CALL(mock, sqlite3_open_v2(_, _, expected_flags, nullptr))
                    .WITH(std::string_view(_1) == db_name)
                    .SIDE_EFFECT(*_2 = dummy_db)
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db)).RETURN(SQLITE_OK);

                auto result = sqlixx::open_connection(db_name);

                THEN("Return a valid connection holding the expected SQLite database handle") {
                    REQUIRE(result.has_value());
                    CHECK(result->get() == dummy_db);
                }
            }

            AND_WHEN("Invoking factory with custom VFS and dynamic flags") {
                const auto* vfs_name = "unix-dotfile";
                auto expected_flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_EXRESCODE;

                REQUIRE_CALL(mock, sqlite3_open_v2(_, _, expected_flags, _))
                    .WITH(std::string_view(_1) == db_name && std::string_view(_4) == vfs_name)
                    .SIDE_EFFECT(*_2 = dummy_db)
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db)).RETURN(SQLITE_OK);

                auto privatecache = sqlixx::open::dyn_flags(SQLITE_OPEN_PRIVATECACHE);
                auto result = sqlixx::open_connection(db_name, vfs_name, sqlixx::open::readonly, privatecache);

                THEN("Apply flags correctly and construct the connection container") {
                    REQUIRE(result.has_value());
                }
            }
        }

        WHEN("Failing to open a connection") {
            AND_WHEN("Aborting with standard flags without a custom error handler") {
                auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

                REQUIRE_CALL(mock, sqlite3_open_v2(_, _, expected_flags, nullptr))
                    .WITH(std::string_view(_1) == db_name)
                    .SIDE_EFFECT(*_2 = dummy_db)
                    .RETURN(SQLITE_CANTOPEN);

                FORBID_CALL(mock, sqlite3_errmsg(_));

                REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db)).RETURN(SQLITE_OK);

                auto result = sqlixx::open_connection(db_name);

                THEN("Return an unexpected error code representing the failure") {
                    REQUIRE_FALSE(result.has_value());
                    CHECK(result.error() == sqlixx::sqlite_errc::cantopen);
                }
            }

            AND_WHEN("Aborting with an allocated handle and an active error handler callback") {
                const auto* bad_db_name = "invalid.db";
                const auto* dummy_errmsg = "Invalid path";
                auto error_handler_called = false;

                auto handler = [&](std::error_code ec, std::string_view msg) noexcept -> void {
                    CHECK(ec == sqlixx::sqlite_errc::cantopen);
                    CHECK(msg == dummy_errmsg);
                    error_handler_called = true;
                };

                REQUIRE_CALL(mock, sqlite3_open_v2(_, _, _, nullptr))
                    .WITH(std::string_view(_1) == bad_db_name)
                    .SIDE_EFFECT(*_2 = dummy_db)
                    .RETURN(SQLITE_CANTOPEN);

                REQUIRE_CALL(mock, sqlite3_errmsg(dummy_db)).RETURN(dummy_errmsg);

                REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db)).RETURN(SQLITE_OK);

                auto result = sqlixx::open_connection(bad_db_name, handler, sqlixx::open::readwrite);

                THEN("Trigger the callback with detailed context and clean up the temporary handle") {
                    REQUIRE_FALSE(result.has_value());
                    CHECK(result.error() == sqlixx::sqlite_errc::cantopen);
                    CHECK(error_handler_called);
                }
            }

            AND_WHEN("Aborting with a null handle pointer and an active error handler callback") {
                const auto* bad_db_name = "invalid.db";
                const auto* vfs_name = "unix-none";
                const auto* expected_errmsg = "No active connection";
                auto error_handler_called = false;

                auto handler = [&](std::error_code ec, std::string_view msg) noexcept -> void {
                    CHECK(ec == sqlixx::sqlite_errc::nomem);
                    CHECK(msg == expected_errmsg);
                    error_handler_called = true;
                };

                REQUIRE_CALL(mock, sqlite3_open_v2(_, _, _, _))
                    .WITH(std::string_view(_1) == bad_db_name && std::string_view(_4) == vfs_name)
                    .SIDE_EFFECT(*_2 = nullptr)
                    .RETURN(SQLITE_NOMEM);

                FORBID_CALL(mock, sqlite3_errmsg(_));
                FORBID_CALL(mock, sqlite3_close_v2(_));

                auto result = sqlixx::open_connection(bad_db_name, vfs_name, handler);

                THEN("Forward fallback diagnostics to handler without calling close hooks") {
                    REQUIRE_FALSE(result.has_value());
                    CHECK(result.error() == sqlixx::sqlite_errc::nomem);
                    CHECK(error_handler_called);
                }
            }
        }
    }
}
