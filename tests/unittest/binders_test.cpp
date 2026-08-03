// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

#define CHECK_BINDER_SUPPORT(Type, Expected)                                                                           \
    STATIC_CHECK([]<typename U>() {                                                                                    \
        return requires { typename sqlixx::binder_t<U>; };                                                             \
    }.template operator()<Type>() == Expected)

TEST_CASE("Calling binders explicitly", "binders") {
    sqlite_mock mock;
    auto* const dummy_stmt_handle = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
    sqlixx::statement_handle stmt{dummy_stmt_handle};

    SECTION("Check binder support") {
        CHECK_BINDER_SUPPORT(std::int64_t, true);
        CHECK_BINDER_SUPPORT(std::uint64_t, false);
        CHECK_BINDER_SUPPORT(double, true);
        CHECK_BINDER_SUPPORT(long double, false);
    }

    SECTION("Binding a single parameter") {
        REQUIRE_CALL(mock, sqlite3_bind_parameter_count(dummy_stmt_handle)).RETURN(1);

        sqlixx::binder_context ctxt{stmt.get()};

        SECTION("Binding a bool") {
            const bool value = true;

            REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<bool>{}(ctxt, value));
        }

        SECTION("Binding an int") {
            const int value = 3;

            REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<int>{}(ctxt, value));
        }

        SECTION("Binding an unsigned int") {
            const unsigned value = 3;

            REQUIRE_CALL(mock, sqlite3_bind_int64(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<unsigned>{}(ctxt, value));
        }

        SECTION("Binding an std::int64_t") {
            const std::int64_t value = 3LL;

            REQUIRE_CALL(mock, sqlite3_bind_int64(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<long long>{}(ctxt, value));
        }

        SECTION("Binding an enum") {
            enum color : std::uint8_t { red, green, blue };
            const auto value = green;

            REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<color>{}(ctxt, value));
        }

        SECTION("Binding an enum class") {
            enum class color : std::uint8_t { red, green, blue };
            const auto value = color::green;

            REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, int(value))).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<color>{}(ctxt, value));
        }

        SECTION("Binding a float") {
            const float value = 3.14f;

            REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<float>{}(ctxt, value));
        }

        SECTION("Binding a double") {
            const double value = 3.14;

            REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt_handle, 1, value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<double>{}(ctxt, value));
        }

        SECTION("Binding a nullptr") {
            REQUIRE_CALL(mock, sqlite3_bind_null(dummy_stmt_handle, 1)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::nullptr_t>{}(ctxt, nullptr));
        }

        SECTION("Binding text (null-terminated string)") {
            const char* value = "hello";

            REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt_handle, 1, value, -1, SQLITE_TRANSIENT)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<const char*>{}(ctxt, value));
        }

        SECTION("Binding text (null-terminated utf-8 string)") {
            const char8_t* value = u8"hello";

            REQUIRE_CALL(
                mock,
                sqlite3_bind_text(dummy_stmt_handle, 1, reinterpret_cast<const char*>(value), -1, SQLITE_TRANSIENT))
                .RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<const char8_t*>{}(ctxt, value));
        }

        SECTION("Binding text (std::string_view)") {
            const std::string_view value = "hello";

            REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt_handle, 1, value.data(), value.size(), SQLITE_TRANSIENT))
                .RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::string_view>{}(ctxt, value));
        }

        SECTION("Binding text (std::u8string_view)") {
            const std::u8string_view value = u8"hello";

            REQUIRE_CALL(
                mock,
                sqlite3_bind_text(
                    dummy_stmt_handle, 1, reinterpret_cast<const char*>(value.data()), value.size(), SQLITE_TRANSIENT))
                .RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::u8string_view>{}(ctxt, value));
        }

        SECTION("Binding a blob (std::span)") {
            const std::array<std::byte, 4> data{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}};
            const std::span value{data};

            REQUIRE_CALL(
                mock,
                sqlite3_bind_blob64(
                    dummy_stmt_handle, 1, static_cast<const void*>(value.data()), value.size_bytes(), SQLITE_TRANSIENT))
                .RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::span<const std::byte>>{}(ctxt, value));
        }

        SECTION("Binding a zeroblob") {
            const sqlixx::zeroblob value{.size = 1024};

            REQUIRE_CALL(mock, sqlite3_bind_zeroblob64(dummy_stmt_handle, 1, value.size)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<sqlixx::zeroblob>{}(ctxt, value));
        }

        SECTION("Binding an std::optional with a value") {
            const std::optional<int> value = 42;

            REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, *value)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::optional<int>>{}(ctxt, value));
        }

        SECTION("Binding an std::optional containing nullopt") {
            const std::optional<int> value = std::nullopt;

            REQUIRE_CALL(mock, sqlite3_bind_null(dummy_stmt_handle, 1)).RETURN(SQLITE_OK);

            CHECK(sqlixx::binder_t<std::optional<int>>{}(ctxt, value));
        }
    }

    SECTION("Binding multiple parameters via tuple") {
        REQUIRE_CALL(mock, sqlite3_bind_parameter_count(dummy_stmt_handle)).RETURN(2);

        sqlixx::binder_context ctxt{stmt.get()};
        const std::tuple value{42, 3.14};

        REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt_handle, 1, 42)).RETURN(SQLITE_OK);
        REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt_handle, 2, 3.14)).RETURN(SQLITE_OK);

        CHECK(sqlixx::binder_t<decltype(value)>{}(ctxt, value));
    }
}
