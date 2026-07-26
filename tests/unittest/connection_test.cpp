// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

TEST_CASE("Lifetime management of a connection_handle", "[connection][handle]") {
    auto* dummy_db_handle = reinterpret_cast<::sqlite3*>(0xBAAD5EED);

    SECTION("A connection_handle can be copied") {
        STATIC_CHECK(std::is_copy_constructible_v<sqlixx::connection_handle>);
        STATIC_CHECK(std::is_copy_assignable_v<sqlixx::connection_handle>);
    }

    SECTION("A connection_handle can be moved") {
        STATIC_CHECK(std::is_move_constructible_v<sqlixx::connection_handle>);
        STATIC_CHECK(std::is_move_assignable_v<sqlixx::connection_handle>);
    }

    SECTION("A default constructed connection_handle") {
        sqlixx::connection_handle conn;

        CHECK_FALSE(conn);
        CHECK(conn.get() == nullptr);
    }

    SECTION("An explicit constructed connection_handle") {
        sqlixx::connection_handle conn{dummy_db_handle};

        CHECK(conn);
        CHECK(conn.get() == dummy_db_handle);
    }

    SECTION("A copy constructed connection_handle") {
        sqlixx::connection_handle conn1{dummy_db_handle};
        sqlixx::connection_handle conn2{conn1};

        CHECK(conn1);
        CHECK(conn1.get() == dummy_db_handle);

        CHECK(conn2);
        CHECK(conn2.get() == dummy_db_handle);
    }

    SECTION("A move constructed connection_handle") {
        sqlixx::connection_handle conn1{dummy_db_handle};
        sqlixx::connection_handle conn2{std::move(conn1)};

        CHECK(conn2);
        CHECK(conn2.get() == dummy_db_handle);
    }

    SECTION("A copy assigned connection_handle") {
        sqlixx::connection_handle conn1{dummy_db_handle};
        sqlixx::connection_handle conn2;

        conn2 = conn1;

        CHECK(conn1);
        CHECK(conn1.get() == dummy_db_handle);

        CHECK(conn2);
        CHECK(conn2.get() == dummy_db_handle);
    }

    SECTION("A move assigned connection_handle") {
        sqlixx::connection_handle conn1;
        sqlixx::connection_handle conn2{dummy_db_handle};

        conn1 = std::move(conn2);

        CHECK(conn1);
        CHECK(conn1.get() == dummy_db_handle);
    }
}

TEST_CASE("Lifetime management of a connection", "[connection][handle]") {
    sqlite_mock mock;
    auto* dummy_db_handle = reinterpret_cast<::sqlite3*>(0xBAAD5EED);

    SECTION("A connection cannot be copied") {
        STATIC_CHECK_FALSE(std::is_copy_constructible_v<sqlixx::connection>);
        STATIC_CHECK_FALSE(std::is_copy_assignable_v<sqlixx::connection>);
    }

    SECTION("A connection can be moved") {
        STATIC_CHECK(std::is_move_constructible_v<sqlixx::connection>);
        STATIC_CHECK(std::is_move_assignable_v<sqlixx::connection>);
    }

    SECTION("A default constructed connection") {
        sqlixx::connection_handle conn;

        CHECK_FALSE(conn);
        CHECK(conn.get() == nullptr);
    }

    SECTION("An explicit constructed connection_handle") {
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(0);

        sqlixx::connection conn{dummy_db_handle};

        CHECK(conn);
        CHECK(conn.get() == dummy_db_handle);
    }

    SECTION("A move constructed connection") {
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(0);

        sqlixx::connection conn1{dummy_db_handle};
        sqlixx::connection conn2{std::move(conn1)};

        CHECK_FALSE(conn1);
        CHECK(conn1.get() == nullptr);

        CHECK(conn2);
        CHECK(conn2.get() == dummy_db_handle);
    }

    SECTION("A move assigned connection") {
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(0);

        sqlixx::connection conn1{dummy_db_handle};
        sqlixx::connection conn2;

        conn2 = std::move(conn1);

        CHECK_FALSE(conn1);
        CHECK(conn1.get() == nullptr);

        CHECK(conn2);
        CHECK(conn2.get() == dummy_db_handle);
    }

    SECTION("A self-move assigned connection") {
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(0);

        sqlixx::connection conn{dummy_db_handle};

        conn = std::move(conn);

        CHECK(conn);
        CHECK(conn.get() == dummy_db_handle);
    }

    SECTION("A released connection") {
        sqlixx::connection conn{dummy_db_handle};

        auto* released_db_handle = conn.release();

        CHECK_FALSE(conn);
        CHECK(conn.get() == nullptr);
        CHECK(released_db_handle == dummy_db_handle);
    }
}

TEST_CASE("conversion management of a connection", "[connection][handle]") {
    sqlite_mock mock;
    auto* dummy_db_handle = reinterpret_cast<::sqlite3*>(0xBAAD5EED);

    SECTION("Implicit conversion to connection_handle") {
        REQUIRE_CALL(mock, sqlite3_close_v2(dummy_db_handle)).RETURN(0);

        sqlixx::connection conn{dummy_db_handle};
        sqlixx::connection_handle conn_handle = conn;

        CHECK(conn);
        CHECK(conn.get() == dummy_db_handle);

        CHECK(conn_handle);
        CHECK(conn_handle.get() == dummy_db_handle);
    }
}
