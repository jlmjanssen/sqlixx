// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

#include "sqlite_mock.hpp"

#include <sqlite3.h>

extern "C" {
const char* __wrap_sqlite3_errstr(int rc) {
    const char* __real_sqlite3_errstr(int rc);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_errstr(rc);
    return __real_sqlite3_errstr(rc);
}

const char* __wrap_sqlite3_errmsg(sqlite3* db) {
    const char* __real_sqlite3_errmsg(sqlite3 * db);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_errmsg(db);
    return __real_sqlite3_errmsg(db);
}

int __wrap_sqlite3_extended_errcode(sqlite3* db) {
    int __real_sqlite3_extended_errcode(sqlite3 * db);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_extended_errcode(db);
    return __real_sqlite3_extended_errcode(db);
}

int __wrap_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs) {
    int __real_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_open_v2(filename, ppDb, flags, zVfs);
    return __real_sqlite3_open_v2(filename, ppDb, flags, zVfs);
}

int __wrap_sqlite3_close_v2(sqlite3* db) {
    int __real_sqlite3_close_v2(sqlite3 * db);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_close_v2(db);
    return __real_sqlite3_close_v2(db);
}

int __wrap_sqlite3_prepare_v3(
    sqlite3* db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail) {
    int __real_sqlite3_prepare_v3(
        sqlite3 * db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
    return __real_sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
}

int __wrap_sqlite3_finalize(sqlite3_stmt* pStmt) {
    int __real_sqlite3_finalize(sqlite3_stmt * pStmt);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_finalize(pStmt);
    return __real_sqlite3_finalize(pStmt);
}

int __wrap_sqlite3_step(::sqlite3_stmt* stmt) {
    int __real_sqlite3_step(::sqlite3_stmt*);
    if (sqlite_mock::mock_ != nullptr) {
        return sqlite_mock::mock_->sqlite3_step(stmt);
    }
    return __real_sqlite3_step(stmt);
}

int __wrap_sqlite3_reset(::sqlite3_stmt* stmt) {
    int __real_sqlite3_reset(::sqlite3_stmt*);
    if (sqlite_mock::mock_ != nullptr) {
        return sqlite_mock::mock_->sqlite3_reset(stmt);
    }
    return __real_sqlite3_reset(stmt);
}

int __wrap_sqlite3_clear_bindings(::sqlite3_stmt* stmt) {
    int __real_sqlite3_clear_bindings(::sqlite3_stmt*);
    if (sqlite_mock::mock_ != nullptr) {
        return sqlite_mock::mock_->sqlite3_clear_bindings(stmt);
    }
    return __real_sqlite3_clear_bindings(stmt);
}

int __wrap_sqlite3_bind_int(sqlite3_stmt* pStmt, int i, int iValue) {
    int __real_sqlite3_bind_int(sqlite3_stmt * pStmt, int i, int iValue);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_int(pStmt, i, iValue);
    return __real_sqlite3_bind_int(pStmt, i, iValue);
}

int __wrap_sqlite3_bind_int64(sqlite3_stmt* pStmt, int i, sqlite3_int64 iValue) {
    int __real_sqlite3_bind_int64(sqlite3_stmt * pStmt, int i, sqlite3_int64 iValue);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_int64(pStmt, i, iValue);
    return __real_sqlite3_bind_int64(pStmt, i, iValue);
}

int __wrap_sqlite3_bind_double(sqlite3_stmt* pStmt, int i, double rValue) {
    int __real_sqlite3_bind_double(sqlite3_stmt * pStmt, int i, double rValue);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_double(pStmt, i, rValue);
    return __real_sqlite3_bind_double(pStmt, i, rValue);
}

int __wrap_sqlite3_bind_text(sqlite3_stmt* pStmt, int i, const char* zData, int nData, void (*xDel)(void*)) {
    int __real_sqlite3_bind_text(sqlite3_stmt * pStmt, int i, const char* zData, int nData, void (*xDel)(void*));
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_text(pStmt, i, zData, nData, xDel);
    return __real_sqlite3_bind_text(pStmt, i, zData, nData, xDel);
}

int __wrap_sqlite3_bind_blob64(
    sqlite3_stmt* pStmt, int i, const void* zData, sqlite3_uint64 nData, void (*xDel)(void*)) {
    int __real_sqlite3_bind_blob64(
        sqlite3_stmt * pStmt, int i, const void* zData, sqlite3_uint64 nData, void (*xDel)(void*));
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_blob64(pStmt, i, zData, nData, xDel);
    return __real_sqlite3_bind_blob64(pStmt, i, zData, nData, xDel);
}

int __wrap_sqlite3_bind_zeroblob64(sqlite3_stmt* pStmt, int i, sqlite3_uint64 nData) {
    int __real_sqlite3_bind_zeroblob64(sqlite3_stmt * pStmt, int i, sqlite3_uint64 nData);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_zeroblob64(pStmt, i, nData);
    return __real_sqlite3_bind_zeroblob64(pStmt, i, nData);
}

int __wrap_sqlite3_bind_null(sqlite3_stmt* pStmt, int i) {
    int __real_sqlite3_bind_null(sqlite3_stmt * pStmt, int i);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_null(pStmt, i);
    return __real_sqlite3_bind_null(pStmt, i);
}

int __wrap_sqlite3_bind_parameter_index(sqlite3_stmt* pStmt, const char* zName) {
    int __real_sqlite3_bind_parameter_index(sqlite3_stmt * pStmt, const char* zName);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_bind_parameter_index(pStmt, zName);
    return __real_sqlite3_bind_parameter_index(pStmt, zName);
}

int __wrap_sqlite3_data_count(sqlite3_stmt* stmt) {
    int __real_sqlite3_data_count(sqlite3_stmt * stmt);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_data_count(stmt);
    return __real_sqlite3_data_count(stmt);
}

int __wrap_sqlite3_column_int(sqlite3_stmt* stmt, int column) {
    int __real_sqlite3_column_int(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_int(stmt, column);
    return __real_sqlite3_column_int(stmt, column);
}

sqlite3_int64 __wrap_sqlite3_column_int64(sqlite3_stmt* stmt, int column) {
    sqlite3_int64 __real_sqlite3_column_int64(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_int64(stmt, column);
    return __real_sqlite3_column_int64(stmt, column);
}

double __wrap_sqlite3_column_double(sqlite3_stmt* stmt, int column) {
    double __real_sqlite3_column_double(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_double(stmt, column);
    return __real_sqlite3_column_double(stmt, column);
}

const unsigned char* __wrap_sqlite3_column_text(sqlite3_stmt* stmt, int column) {
    const unsigned char* __real_sqlite3_column_text(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_text(stmt, column);
    return __real_sqlite3_column_text(stmt, column);
}

const void* __wrap_sqlite3_column_blob(sqlite3_stmt* stmt, int column) {
    const void* __real_sqlite3_column_blob(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_blob(stmt, column);
    return __real_sqlite3_column_blob(stmt, column);
}

int __wrap_sqlite3_column_bytes(sqlite3_stmt* stmt, int column) {
    int __real_sqlite3_column_bytes(sqlite3_stmt * stmt, int column);
    if (sqlite_mock::mock_ != nullptr)
        return sqlite_mock::mock_->sqlite3_column_bytes(stmt, column);
    return __real_sqlite3_column_bytes(stmt, column);
}
}
