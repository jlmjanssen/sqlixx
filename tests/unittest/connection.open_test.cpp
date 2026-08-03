// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

TEST_CASE("Opening a database connection", "[connection]") {
    sqlite_mock mock;
    const auto* db_name = "test.db";
    const auto* vfs_name = "unix-posix";
    auto* dummy_db_handle = reinterpret_cast<::sqlite3*>(0xBAAD5EED);

    SECTION("Opening with default flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::open_connection(db_name);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
    }

    SECTION("Opening with static custom flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::open_connection(db_name, sqlixx::open::readwrite, sqlixx::open::fullmutex);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
    }

    SECTION("Opening with dynamic custom flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto dynflags = sqlixx::open::dyn_flags(SQLITE_OPEN_SHAREDCACHE);
        auto result = sqlixx::open_connection(db_name, sqlixx::open::readwrite, dynflags);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
    }

    SECTION("Opening with a vfs and default flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, vfs_name))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::open_connection(db_name, vfs_name);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
    }

    SECTION("Opening with a vfs and custom flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, vfs_name))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::open_connection(db_name, vfs_name, sqlixx::open::readwrite);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
    }

    SECTION("Opening with an error handler and default flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void { on_error_called = true; };

        auto result = sqlixx::open_connection(db_name, on_error);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
        CHECK_FALSE(on_error_called);
    }

    SECTION("Opening with an error handler and custom flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void { on_error_called = true; };

        auto result = sqlixx::open_connection(db_name, on_error, sqlixx::open::readwrite);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
        CHECK_FALSE(on_error_called);
    }

    SECTION("Opening with a vfs, an error handler, and default flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, vfs_name))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void { on_error_called = true; };

        auto result = sqlixx::open_connection(db_name, vfs_name, on_error);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
        CHECK_FALSE(on_error_called);
    }

    SECTION("Opening with a vfs, an error handler, and custom flags") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, vfs_name))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void { on_error_called = true; };

        auto result = sqlixx::open_connection(db_name, vfs_name, on_error, sqlixx::open::readwrite);

        CHECK(result);
        CHECK(result->get() == dummy_db_handle);
        CHECK_FALSE(on_error_called);
    }

    SECTION("Error while opening") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_CANTOPEN);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::open_connection(db_name);

        CHECK(!result);
        CHECK(result.error() == sqlixx::sqlite_errc::cantopen);
    }

    SECTION("Out-of-memory error while opening") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = nullptr)
            .RETURN(SQLITE_NOMEM);
        FORBID_CALL(mock, sqlite3_close_v2(dummy_db_handle));

        auto result = sqlixx::open_connection(db_name);

        CHECK(!result);
        CHECK(result.error() == sqlixx::sqlite_errc::nomem);
    }

    SECTION("Error while opening with an error handler") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;
        const auto* expected_errmsg = "unable to open database file";

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = dummy_db_handle)
            .RETURN(SQLITE_CANTOPEN);
        REQUIRE_CALL(mock, sqlite3_errmsg(dummy_db_handle)).RETURN(expected_errmsg);
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(SQLITE_OK);

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void {
            CHECK(ec == sqlixx::sqlite_errc::cantopen);
            CHECK(msg == expected_errmsg);
            on_error_called = true;
        };

        auto result = sqlixx::open_connection(db_name, on_error);

        CHECK(!result);
        CHECK(result.error() == sqlixx::sqlite_errc::cantopen);
        CHECK(on_error_called);
    }

    SECTION("Out-of-memory error while opening with an error handler") {
        const auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

        REQUIRE_CALL(mock, sqlite3_open_v2(db_name, _, expected_flags, nullptr))
            .SIDE_EFFECT(*_2 = nullptr)
            .RETURN(SQLITE_NOMEM);
        FORBID_CALL(mock, sqlite3_errmsg(dummy_db_handle));
        FORBID_CALL(mock, sqlite3_close_v2(dummy_db_handle));

        bool on_error_called = false;
        auto on_error = [&](auto ec, auto msg) noexcept -> void {
            CHECK(ec == sqlixx::sqlite_errc::nomem);
            CHECK(msg.length() != 0);
            on_error_called = true;
        };

        auto result = sqlixx::open_connection(db_name, on_error);

        CHECK(!result);
        CHECK(result.error() == sqlixx::sqlite_errc::nomem);
        CHECK(on_error_called);
    }
}
