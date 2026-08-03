// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include "sqlite_mock.hpp"

import std;
import sqlixx;

using trompeloeil::_;

SCENARIO("Verify column and tuple reader mechanics via reader_context", "[readers]") {
    GIVEN("A registered SQLite mock and a statement handle with data") {
        using namespace std::string_view_literals;
        sqlite_mock mock;

        auto* const dummy_stmt = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
        sqlixx::statement_handle stmt{dummy_stmt};

        WHEN("Reading columns without an active row or with bad indices") {
            AND_WHEN("The data count reports no active rows") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(0);

                int value = 0;
                auto res = sqlixx::read(stmt, 0, value);

                THEN("Fail with no_active_row error") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::errc::invalid_index);
                }
            }

            AND_WHEN("The requested column index is out of bounds") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(2).TIMES(2);

                int val_low = 0;
                int val_high = 0;
                auto res_low = sqlixx::read(stmt, -1, val_low);
                auto res_high = sqlixx::read(stmt, 2, val_high);

                THEN("Fail with invalid_index error") {
                    REQUIRE_FALSE(res_low.has_value());
                    CHECK(res_low.error() == sqlixx::errc::invalid_index);
                    REQUIRE_FALSE(res_high.has_value());
                    CHECK(res_high.error() == sqlixx::errc::invalid_index);
                }
            }
        }

        WHEN("Extracting primitive scalar types from an active row") {
            REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(5).TIMES(AT_LEAST(1));

            AND_WHEN("Reading integral values") {
                REQUIRE_CALL(mock, sqlite3_column_int64(dummy_stmt, 0)).RETURN(42LL);
                REQUIRE_CALL(mock, sqlite3_column_int64(dummy_stmt, 1)).RETURN(9000000000000LL);

                THEN("Route correctly to the matching C API type size") {
                    int val_int = 0;
                    std::int64_t val_int64 = 0;

                    REQUIRE(sqlixx::read(stmt, 0, val_int).has_value());
                    REQUIRE(sqlixx::read(stmt, 1, val_int64).has_value());
                    CHECK(val_int == 42);
                    CHECK(val_int64 == 9000000000000LL);
                }
            }

            AND_WHEN("Reading floating point values") {
                REQUIRE_CALL(mock, sqlite3_column_double(dummy_stmt, 2)).RETURN(3.14);

                THEN("Route directly to sqlite3_column_double") {
                    double val_double = 0.0;
                    REQUIRE(sqlixx::read(stmt, 2, val_double).has_value());
                    CHECK(val_double == Catch::Approx(3.14));
                }
            }
        }

        WHEN("Extracting view, span or pointer types from an active row") {
            REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(5).TIMES(AT_LEAST(1));

            AND_WHEN("Reading text entries as string_views") {
                const auto* text_ptr = "hello";
                REQUIRE_CALL(mock, sqlite3_column_text(dummy_stmt, 0))
                    .RETURN(reinterpret_cast<const unsigned char*>(text_ptr));
                REQUIRE_CALL(mock, sqlite3_column_bytes(dummy_stmt, 0)).RETURN(5);

                THEN("Construct a valid string view with matching length") {
                    std::string_view val_view;
                    auto res = sqlixx::read(stmt, 0, val_view);
                    REQUIRE(res.has_value());
                    CHECK(val_view == "hello"sv);
                }
            }

            AND_WHEN("Reading binary data as byte spans") {
                static std::array<std::byte, 4> raw_data{
                    std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};
                REQUIRE_CALL(mock, sqlite3_column_blob(dummy_stmt, 1)).RETURN(raw_data.data());
                REQUIRE_CALL(mock, sqlite3_column_bytes(dummy_stmt, 1)).RETURN(4);

                THEN("Expose the data safely inside a generic const byte span") {
                    std::span<const std::byte> val_span;
                    auto res = sqlixx::read(stmt, 1, val_span);
                    REQUIRE(res.has_value());
                    CHECK(val_span.size() == 4);
                    CHECK(val_span.data() == raw_data.data());
                }
            }
        }

        WHEN("Unpacking rows into multi-argument lists or standard tuples") {
            AND_WHEN("Extracting parallel sequential columns into individual targets") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(3).TIMES(AT_LEAST(1));
                REQUIRE_CALL(mock, sqlite3_column_int64(dummy_stmt, 0)).RETURN(100LL);
                REQUIRE_CALL(mock, sqlite3_column_double(dummy_stmt, 1)).RETURN(2.5);

                int a = 0;
                double b = 0.0;
                auto status = sqlixx::read(stmt, 0, a, b);

                THEN("Hydrate all references and increment column indexes correctly") {
                    REQUIRE(status.has_value());
                    CHECK(a == 100);
                    CHECK(b == Catch::Approx(2.5));
                }
            }

            AND_WHEN("Unpacking data into a structured tuple-like target") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(2).TIMES(AT_LEAST(1));
                REQUIRE_CALL(mock, sqlite3_column_int64(dummy_stmt, 0)).RETURN(7LL);
                REQUIRE_CALL(mock, sqlite3_column_double(dummy_stmt, 1)).RETURN(4.4);

                THEN("Hydrate an existing lvalue tuple container directly") {
                    std::tuple<int, double> result;
                    auto status = sqlixx::read(stmt, 0, result);

                    REQUIRE(status.has_value());
                    CHECK(std::get<0>(result) == 7);
                    CHECK(std::get<1>(result) == Catch::Approx(4.4));
                }

                THEN("Hydrate individual variables inline using std::tie references") {
                    int id = 0;
                    double value = 0.0;

                    // Perfect forwarding via Ts&& lost de rvalue-tuple van std::tie direct op!
                    auto status = sqlixx::read(stmt, 0, std::tie(id, value));

                    REQUIRE(status.has_value());
                    CHECK(id == 7);
                    CHECK(value == Catch::Approx(4.4));
                }
            }
        }

        WHEN("Encountering database NULL values or unpacking failures") {
            AND_WHEN("A text or blob column contains a database NULL value") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(2).TIMES(2);

                REQUIRE_CALL(mock, sqlite3_column_text(dummy_stmt, 0)).RETURN(nullptr);
                REQUIRE_CALL(mock, sqlite3_column_blob(dummy_stmt, 1)).RETURN(nullptr);

                std::string_view val_view{"fallback"};
                std::span<const std::byte> val_span;

                auto res_text = sqlixx::read(stmt, 0, val_view);
                auto res_blob = sqlixx::read(stmt, 1, val_span);

                THEN("Return empty containers safely without crashing") {
                    REQUIRE(res_text.has_value());
                    CHECK(val_view.empty());
                    REQUIRE(res_blob.has_value());
                    CHECK(val_span.empty());
                }
            }

            AND_WHEN("Unpacking multiple columns but the statement has no active row") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(0);

                int a = 0;
                double b = 0.0;
                auto status = sqlixx::read(stmt, 0, a, b);

                THEN("Fail inside the multi-unpacker with no_active_row") {
                    REQUIRE_FALSE(status.has_value());
                    CHECK(status.error() == sqlixx::errc::invalid_index);
                }
            }

            AND_WHEN("Unpacking more columns than there are in the row") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(1);
                REQUIRE_CALL(mock, sqlite3_column_int64(dummy_stmt, 0)).RETURN(2LL);

                int i = 0;
                sqlixx::reader_context ctxt{stmt.get()};

                AND_WHEN("Reading an excess integral value in a tie") {
                    int j = 0;

                    auto status = sqlixx::read(ctxt, std::tie(i, j));

                    THEN("Fail with invalid_index") {
                        REQUIRE_FALSE(status);
                        CHECK(status.error() == sqlixx::errc::invalid_index);
                    }
                }

                AND_WHEN("Reading an excess real value in a tie") {
                    double d = 0.0;

                    auto status = sqlixx::read(ctxt, std::tie(i, d));

                    THEN("Fail with invalid_index") {
                        REQUIRE_FALSE(status);
                        CHECK(status.error() == sqlixx::errc::invalid_index);
                    }
                }

                AND_WHEN("Reading an excess textual value in a tie") {
                    std::string_view s;

                    auto status = sqlixx::read(ctxt, std::tie(i, s));

                    THEN("Fail with invalid_index") {
                        REQUIRE_FALSE(status);
                        CHECK(status.error() == sqlixx::errc::invalid_index);
                    }
                }

                AND_WHEN("Reading an excess binary value in a tie") {
                    std::span<const std::byte> b;

                    auto status = sqlixx::read(ctxt, std::tie(i, b));

                    THEN("Fail with invalid_index") {
                        REQUIRE_FALSE(status);
                        CHECK(status.error() == sqlixx::errc::invalid_index);
                    }
                }

                AND_WHEN("Reading an excess integral value in a parameter pack") {
                    int j = 0;

                    auto status = sqlixx::read(ctxt, i, j);

                    THEN("Fail with invalid_index") {
                        REQUIRE_FALSE(status);
                        CHECK(status.error() == sqlixx::errc::invalid_index);
                    }
                }
            }

            AND_WHEN("Reading a structured tuple but the statement has no active row") {
                REQUIRE_CALL(mock, sqlite3_data_count(dummy_stmt)).RETURN(0);

                std::tuple<int, double> result;
                auto res = sqlixx::read(stmt, 0, result);

                THEN("Propagate the inner unexpected status error all the way out") {
                    REQUIRE_FALSE(res.has_value());
                    CHECK(res.error() == sqlixx::errc::invalid_index);
                }
            }
        }
    }
}
