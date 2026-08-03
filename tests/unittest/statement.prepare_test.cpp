// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

TEST_CASE("Preparing a statement", "[statement]") {
    sqlite_mock mock;
    auto* dummy_db_handle = reinterpret_cast<::sqlite3*>(0xBAAD5EED);
    auto* dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
    sqlixx::connection_handle conn{dummy_db_handle};

    SECTION("Preparing a null-terminated string statement with default flags") {
        const auto* sql = "SELECT * FROM USERS;";
        const auto* tail_ptr = sql + std::strlen(sql);

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, 0, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing a null-terminated string statement with static custom flags") {
        const auto* sql = "SELECT * FROM USERS;";
        const auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;
        const auto* tail_ptr = sql + std::strlen(sql);

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, expected_flags, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql, sqlixx::prep::persistent, sqlixx::prep::normalize);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing a null-terminated string statement with dynamic custom flags") {
        const auto* sql = "SELECT * FROM USERS;";
        const auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;
        const auto* tail_ptr = sql + std::strlen(sql);

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, expected_flags, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto dynflags = sqlixx::prep::dyn_flags(SQLITE_PREPARE_NORMALIZE);
        auto result = sqlixx::prepare_statement(conn, sql, sqlixx::prep::persistent, dynflags);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing an std::string_view statement with default flags") {
        const auto sql = std::string_view{"SELECT * FROM USERS;"};
        const auto* tail_ptr = sql.data() + sql.size();

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql.data(), sql.size(), 0, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing an std::string_view statement with static custom flags") {
        const auto sql = std::string_view{"SELECT * FROM USERS;"};
        const auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;
        const auto* tail_ptr = sql.data() + sql.size();

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql.data(), sql.size(), expected_flags, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql, sqlixx::prep::persistent, sqlixx::prep::normalize);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing an std::string_view statement with dynamic custom flags") {
        const auto sql = std::string_view{"SELECT * FROM USERS;"};
        const auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;
        const auto* tail_ptr = sql.data() + sql.size();

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql.data(), sql.size(), expected_flags, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = tail_ptr)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto dynflags = sqlixx::prep::normalize;
        auto result = sqlixx::prepare_statement(conn, sql, sqlixx::prep::persistent, dynflags);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.empty());
    }

    SECTION("Preparing a null-terminated string statement with remainder") {
        const auto* sql = "SELECT * FROM USERS; SELECT * FROM GROUPS;";
        const auto* expected_tail = std::strchr(sql, ';') + 1;

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, 0, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = expected_tail)
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.data() == expected_tail);
        CHECK(result->tail.size() == std::strlen(expected_tail));
    }

    SECTION("Preparing an std::string_view statement with remainder") {
        const auto sql = std::string_view{"SELECT * FROM USERS; SELECT * FROM GROUPS;"};
        const auto expected_tail = sql.substr(sql.find(';') + 1);

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql.data(), sql.size(), 0, _, _))
            .SIDE_EFFECT(*_5 = dummy_stmt_handle)
            .SIDE_EFFECT(*_6 = expected_tail.data())
            .RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK(result);
        CHECK(result->stmt.get() == dummy_stmt_handle);
        CHECK(result->tail.data() == expected_tail.data());
        CHECK(result->tail.size() == expected_tail.size());
    }

    SECTION("Preparing an empty statement") {
        const auto* sql = "";

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, 0, _, _))
            .SIDE_EFFECT(*_5 = nullptr)
            .SIDE_EFFECT(*_6 = sql)
            .RETURN(SQLITE_OK);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK(result);
        CHECK(result->stmt.get() == nullptr);
        CHECK(result->tail.empty());
    }

    SECTION("Error while preparing") {
        const auto* sql = "SELECT * FROM LUSERS;";

        REQUIRE_CALL(mock, sqlite3_prepare_v3(dummy_db_handle, sql, -1, 0, _, _))
            .SIDE_EFFECT(*_5 = nullptr)
            .SIDE_EFFECT(*_6 = nullptr)
            .RETURN(SQLITE_ERROR);

        auto result = sqlixx::prepare_statement(conn, sql);

        CHECK_FALSE(result);
        CHECK(result.error() == sqlixx::sqlite_errc::error);
    }
}
