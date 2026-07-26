// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify context-driven statement parameter binding mechanisms and type dispatching", "[binders]") {
    GIVEN("A registered SQLite mock and an active statement handle") {
        using namespace std::string_view_literals;
        sqlite_mock mock;

        auto* const dummy_stmt = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
        sqlixx::statement_handle stmt{dummy_stmt};

        WHEN("Binding fundamental numeric data types via one-shot bind overloads") {
            AND_WHEN("Passing standard integer types") {
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 42)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 2, 1)).RETURN(SQLITE_OK);

                std::int64_t large_val = 9000000000000LL;
                REQUIRE_CALL(mock, sqlite3_bind_int64(dummy_stmt, 3, large_val)).RETURN(SQLITE_OK);

                THEN("Route integrals correctly via one-shot explicit index binding") {
                    CHECK(sqlixx::bind(stmt, 1, 42).has_value());
                    CHECK(sqlixx::bind(stmt, 2, true).has_value());
                    CHECK(sqlixx::bind(stmt, 3, large_val).has_value());
                }
            }

            AND_WHEN("Passing floating point types") {
                REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt, 5, 3.14)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt, 6, 2.5)).RETURN(SQLITE_OK);

                THEN("Cast precision fields to double via explicit index binding") {
                    CHECK(sqlixx::bind(stmt, 5, 3.14).has_value());
                    CHECK(sqlixx::bind(stmt, 6, 2.5f).has_value());
                }
            }
        }

        WHEN("Binding text string primitives with varying lifecycle requirements") {
            const char* c_str = "hello";

            AND_WHEN("Using an explicit bind_context with copy_strategy::deep") {
                sqlixx::bind_context ctxt{stmt, sqlixx::bind_context::copy_strategy::deep};

                REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 1, c_str, -1, _))
                    .WITH(_5 == reinterpret_cast<void (*)(void*)>(-1))
                    .RETURN(SQLITE_OK);

                REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 2, _, 4, _))
                    .WITH(std::string_view(_3) == "test" && _5 == reinterpret_cast<void (*)(void*)>(-1))
                    .RETURN(SQLITE_OK);

                THEN("Dispatch strings as SQLITE_TRANSIENT and advance the context index") {
                    CHECK(sqlixx::bind(ctxt, c_str).has_value());
                    CHECK(sqlixx::bind(ctxt, "test").has_value());
                }
            }

            AND_WHEN("Using an explicit bind_context with copy_strategy::shallow") {
                AND_WHEN("Relying on implicit index binding") {
                    sqlixx::bind_context ctxt{stmt, sqlixx::bind_context::copy_strategy::shallow};
                    auto view = "static_text"sv;

                    REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 1, view.data(), 11, _))
                        .WITH(_5 == reinterpret_cast<void (*)(void*)>(0))
                        .RETURN(SQLITE_OK);

                    THEN("Dispatch views as SQLITE_STATIC leveraging context strategy") {
                        CHECK(sqlixx::bind(ctxt, view).has_value());
                    }
                }

                AND_WHEN("Providing explicit index binding") {
                    sqlixx::bind_context ctxt{stmt, 1, sqlixx::bind_context::copy_strategy::shallow};
                    auto view = "static_text"sv;

                    REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 1, view.data(), 11, _))
                        .WITH(_5 == reinterpret_cast<void (*)(void*)>(0))
                        .RETURN(SQLITE_OK);

                    THEN("Dispatch views as SQLITE_STATIC leveraging context strategy") {
                        CHECK(sqlixx::bind(ctxt, view).has_value());
                    }
                }
            }

            AND_WHEN("Using a custom destructor function pointer") {
                AND_WHEN("Relying on implicit index binding") {
                    auto* dummy_destructor = reinterpret_cast<::sqlite3_destructor_type>(0x12345678);
                    sqlixx::bind_context ctxt{stmt, dummy_destructor};

                    REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 1, c_str, -1, _))
                        .WITH(_5 == dummy_destructor)
                        .RETURN(SQLITE_OK);

                    THEN("Forward the raw destructor pointer straight to SQLite") {
                        CHECK(sqlixx::bind(ctxt, c_str).has_value());
                    }
                }

                AND_WHEN("Providing explicit index binding") {
                    auto* dummy_destructor = reinterpret_cast<::sqlite3_destructor_type>(0x12345678);
                    sqlixx::bind_context ctxt{stmt, 1, dummy_destructor};

                    REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 1, c_str, -1, _))
                        .WITH(_5 == dummy_destructor)
                        .RETURN(SQLITE_OK);

                    THEN("Forward the raw destructor pointer straight to SQLite") {
                        CHECK(sqlixx::bind(ctxt, c_str).has_value());
                    }
                }
            }
        }

        WHEN("Binding raw data blobs through standard span views") {
            std::array<std::uint8_t, 4> data{0xDE, 0xAD, 0xBE, 0xEF};
            std::span<std::uint8_t> blob_span(data);

            AND_WHEN("Context uses default/deep copy strategy") {
                sqlixx::bind_context ctxt{stmt};

                REQUIRE_CALL(mock, sqlite3_bind_blob64(dummy_stmt, 1, blob_span.data(), 4, _))
                    .WITH(_5 == reinterpret_cast<void (*)(void*)>(-1))
                    .RETURN(SQLITE_OK);

                THEN("Pass data slices using transient descriptor rules") {
                    CHECK(sqlixx::bind(ctxt, blob_span).has_value());
                }
            }
        }

        WHEN("Binding special database types or raw metadata handles") {
            sqlixx::bind_context ctxt{stmt};

            THEN("Route zero-blobs and null-pointers seamlessly and sequence indexes") {
                REQUIRE_CALL(mock, sqlite3_bind_zeroblob64(dummy_stmt, 1, 1024U)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_null(dummy_stmt, 2)).RETURN(SQLITE_OK);

                CHECK(sqlixx::bind(ctxt, sqlixx::zeroblob{.size = 1024U}, nullptr).has_value());
            }
        }

        WHEN("Binding parameters through complex indexing or automated packers") {
            AND_WHEN("Resolving a parameter address via its alphanumeric name descriptor") {
                const char* param_name = ":user_id";
                REQUIRE_CALL(mock, sqlite3_bind_parameter_index(dummy_stmt, _))
                    .WITH(std::string_view(_2) == param_name)
                    .RETURN(5);
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 5, 1337)).RETURN(SQLITE_OK);

                THEN("Locate the column offset index and apply the value variant via one-shot bind") {
                    CHECK(sqlixx::bind(stmt, param_name, 1337).has_value());
                }
            }

            AND_WHEN("Injecting values using an automatically incrementing context index") {
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 10)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 2, 20)).RETURN(SQLITE_OK);

                sqlixx::bind_context ctxt{stmt, 1};

                THEN("Apply current offset and progress state on each successful injection") {
                    CHECK(sqlixx::bind(ctxt, 10).has_value());
                    CHECK(sqlixx::bind(ctxt, 20).has_value());
                }
            }

            AND_WHEN("Processing heterogeneous argument lists from a single invocation wrapper") {
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 100)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt, 2, 5.5)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_null(dummy_stmt, 3)).RETURN(SQLITE_OK);

                THEN("Unpack argument fields and distribute them over orderly sequence columns") {
                    CHECK(sqlixx::bind(stmt, 1, 100, 5.5, nullptr).has_value());
                }
            }

            AND_WHEN("Unpacking compile-time tuple collections into sql fields") {
                auto data_tuple = std::make_tuple(42, 3.14, "text");

                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 42)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt, 2, 3.14)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 3, _, -1, _))
                    .WITH(std::string_view(_3) == "text" && _5 == reinterpret_cast<void (*)(void*)>(-1))
                    .RETURN(SQLITE_OK);

                THEN("Apply tuple expansion properties and flush ordered content sequentially") {
                    CHECK(sqlixx::bind(stmt, 1, data_tuple).has_value());
                }
            }

            AND_WHEN("Unpacking complex nested tuple collections into sql fields") {
                auto nested_tuple = std::make_tuple(42, std::make_tuple(3.14, "nested_text"), nullptr);

                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 42)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_double(dummy_stmt, 2, 3.14)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_text(dummy_stmt, 3, _, -1, _))
                    .WITH(std::string_view(_3) == "nested_text" && _5 == reinterpret_cast<void (*)(void*)>(-1))
                    .RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_null(dummy_stmt, 4)).RETURN(SQLITE_OK);

                THEN("Recursively expand all tuple layers and flush content into sequential columns") {
                    CHECK(sqlixx::bind(stmt, 1, nested_tuple).has_value());
                }
            }
        }

        WHEN("Encountering parameter binding disruptions from the SQLite database layer") {
            AND_WHEN("A discrete singular parameter binding operation fails") {
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 99)).RETURN(SQLITE_TOOBIG);
                auto res = sqlixx::bind(stmt, 1, 99);

                THEN("Intercept status results and return an unexpected error container mapping") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::sqlite_errc::toobig);
                }
            }

            AND_WHEN("A sequential folder packing pipeline encounters a failure halfway through") {
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 1, 10)).RETURN(SQLITE_OK);
                REQUIRE_CALL(mock, sqlite3_bind_int(dummy_stmt, 2, 20)).RETURN(SQLITE_MISMATCH);

                FORBID_CALL(mock, sqlite3_bind_int(dummy_stmt, 3, 30));

                auto res = sqlixx::bind(stmt, 1, 10, 20, 30);

                THEN("Halt the fold-expression chain instantly via short-circuit evaluations") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::sqlite_errc::mismatch);
                }
            }
        }
    }
}
