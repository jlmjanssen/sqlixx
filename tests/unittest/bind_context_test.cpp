// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

TEST_CASE("Management of a binder context", "binders") {
    sqlite_mock mock;
    auto* const dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);

    REQUIRE_CALL(mock, sqlite3_bind_parameter_count(dummy_stmt_handle)).RETURN(3);

    sqlixx::binder_context ctxt{dummy_stmt_handle};

    SECTION("The statement handle is copied") {
        CHECK(ctxt.get() == dummy_stmt_handle);
    }

    SECTION("The default destructor is transient") {
        CHECK(ctxt.get_destructor() == SQLITE_TRANSIENT);
    }

    SECTION("Set the strategy to shallow") {
        ctxt.set_strategy(sqlixx::binder_context::copy::shallow);

        CHECK(ctxt.get_destructor() == SQLITE_STATIC);
    }

    SECTION("Set the destructor directly") {
        ctxt.set_destructor(SQLITE_STATIC);

        CHECK(ctxt.get_destructor() == SQLITE_STATIC);
    }

    SECTION("Get and advance the parameter index") {
        auto index = ctxt.get_and_advance_index();

        CHECK(index);
        CHECK(*index == 1);

        index = ctxt.get_and_advance_index();

        CHECK(index);
        CHECK(*index == 2);
    }

    SECTION("Set the parameter index") {
        auto index = ctxt.get_and_advance_index();

        CHECK(index);
        CHECK(*index == 1);

        CHECK(ctxt.set_index(1));

        index = ctxt.get_and_advance_index();

        CHECK(index);
        CHECK(*index == 1);
    }

    SECTION("Underflow the parameter index") {
        auto result = ctxt.set_index(0);

        CHECK_FALSE(result);
        CHECK(result.error() == sqlixx::errc::invalid_index);
    }

    SECTION("Overflow the parameter index") {
        auto result = ctxt.set_index(5);

        CHECK_FALSE(result);
        CHECK(result.error() == sqlixx::errc::invalid_index);
    }

    SECTION("Overrun the parameter index") {
        auto result = ctxt.set_index(4);

        CHECK(result);

        auto index = ctxt.get_and_advance_index();

        CHECK_FALSE(index);
        CHECK(index.error() == sqlixx::errc::invalid_index);
    }
}
