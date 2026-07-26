// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

TEST_CASE("Lifetime management of a statement_handle", "[statement][handle]") {
    auto* dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0xBAAD5EED);

    SECTION("A statement_handle can be copied") {
        STATIC_CHECK(std::is_copy_constructible_v<sqlixx::statement_handle>);
        STATIC_CHECK(std::is_copy_assignable_v<sqlixx::statement_handle>);
    }

    SECTION("A statement_handle can be moved") {
        STATIC_CHECK(std::is_move_constructible_v<sqlixx::statement_handle>);
        STATIC_CHECK(std::is_move_assignable_v<sqlixx::statement_handle>);
    }

    SECTION("A default constructed statement_handle") {
        sqlixx::statement_handle stmt;

        CHECK_FALSE(stmt);
        CHECK(stmt.get() == nullptr);
    }

    SECTION("An explicit constructed statement_handle") {
        sqlixx::statement_handle stmt{dummy_stmt_handle};

        CHECK(stmt);
        CHECK(stmt.get() == dummy_stmt_handle);
    }

    SECTION("A copy constructed statement_handle") {
        sqlixx::statement_handle stmt1{dummy_stmt_handle};
        sqlixx::statement_handle stmt2{stmt1};

        CHECK(stmt1);
        CHECK(stmt1.get() == dummy_stmt_handle);

        CHECK(stmt2);
        CHECK(stmt2.get() == dummy_stmt_handle);
    }

    SECTION("A move constructed statement_handle") {
        sqlixx::statement_handle stmt1{dummy_stmt_handle};
        sqlixx::statement_handle stmt2{std::move(stmt1)};

        CHECK(stmt2);
        CHECK(stmt2.get() == dummy_stmt_handle);
    }

    SECTION("A copy assigned statement_handle") {
        sqlixx::statement_handle stmt1{dummy_stmt_handle};
        sqlixx::statement_handle stmt2;

        stmt2 = stmt1;

        CHECK(stmt1);
        CHECK(stmt1.get() == dummy_stmt_handle);

        CHECK(stmt2);
        CHECK(stmt2.get() == dummy_stmt_handle);
    }

    SECTION("A move assigned statement_handle") {
        sqlixx::statement_handle stmt1;
        sqlixx::statement_handle stmt2{dummy_stmt_handle};

        stmt1 = std::move(stmt2);

        CHECK(stmt1);
        CHECK(stmt1.get() == dummy_stmt_handle);
    }
}

TEST_CASE("Lifetime management of a statement", "[statement][handle]") {
    sqlite_mock mock;
    auto* dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0xBAAD5EED);

    SECTION("A statement cannot be copied") {
        STATIC_CHECK_FALSE(std::is_copy_constructible_v<sqlixx::statement>);
        STATIC_CHECK_FALSE(std::is_copy_assignable_v<sqlixx::statement>);
    }

    SECTION("A statement can be moved") {
        STATIC_CHECK(std::is_move_constructible_v<sqlixx::statement>);
        STATIC_CHECK(std::is_move_assignable_v<sqlixx::statement>);
    }

    SECTION("A default constructed statement") {
        sqlixx::statement_handle stmt;

        CHECK_FALSE(stmt);
        CHECK(stmt.get() == nullptr);
    }

    SECTION("An explicit constructed statement_handle") {
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(0);

        sqlixx::statement stmt{dummy_stmt_handle};

        CHECK(stmt);
        CHECK(stmt.get() == dummy_stmt_handle);
    }

    SECTION("A move constructed statement") {
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(0);

        sqlixx::statement stmt1{dummy_stmt_handle};
        sqlixx::statement stmt2{std::move(stmt1)};

        CHECK_FALSE(stmt1);
        CHECK(stmt1.get() == nullptr);

        CHECK(stmt2);
        CHECK(stmt2.get() == dummy_stmt_handle);
    }

    SECTION("A move assigned statement") {
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(0);

        sqlixx::statement stmt1{dummy_stmt_handle};
        sqlixx::statement stmt2;

        stmt2 = std::move(stmt1);

        CHECK_FALSE(stmt1);
        CHECK(stmt1.get() == nullptr);

        CHECK(stmt2);
        CHECK(stmt2.get() == dummy_stmt_handle);
    }

    SECTION("A self-move assigned statement") {
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(0);

        sqlixx::statement stmt{dummy_stmt_handle};

        stmt = std::move(stmt);

        CHECK(stmt);
        CHECK(stmt.get() == dummy_stmt_handle);
    }

    SECTION("A released statement") {
        sqlixx::statement stmt{dummy_stmt_handle};

        auto* released_db_handle = stmt.release();

        CHECK_FALSE(stmt);
        CHECK(stmt.get() == nullptr);
        CHECK(released_db_handle == dummy_stmt_handle);
    }
}

TEST_CASE("conversion management of a statement", "[statement][handle]") {
    sqlite_mock mock;
    auto* dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0xBAAD5EED);

    SECTION("Implicit conversion to statement_handle") {
        REQUIRE_CALL(mock, sqlite3_finalize(dummy_stmt_handle)).RETURN(0);

        sqlixx::statement stmt{dummy_stmt_handle};
        sqlixx::statement_handle stmt_handle = stmt;

        CHECK(stmt);
        CHECK(stmt.get() == dummy_stmt_handle);

        CHECK(stmt_handle);
        CHECK(stmt_handle.get() == dummy_stmt_handle);
    }
}
