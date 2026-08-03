// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

module;

#include <sqlite3.h>

export module sqlixx:error.sqlite;

import std;

namespace sqlixx {

// NOLINTNEXTLINE(performance-*)
export enum class sqlite_errc : int {
    // clang-format off
    // Primary result codes
    error                   = SQLITE_ERROR,
    perm                    = SQLITE_PERM,
    abort                   = SQLITE_ABORT,
    busy                    = SQLITE_BUSY,
    locked                  = SQLITE_LOCKED,
    nomem                   = SQLITE_NOMEM,
    readonly                = SQLITE_READONLY,
    interrupt               = SQLITE_INTERRUPT,
    ioerr                   = SQLITE_IOERR,
    corrupt                 = SQLITE_CORRUPT,
    notfound                = SQLITE_NOTFOUND,
    full                    = SQLITE_FULL,
    cantopen                = SQLITE_CANTOPEN,
    protocol                = SQLITE_PROTOCOL,
    schema                  = SQLITE_SCHEMA,
    toobig                  = SQLITE_TOOBIG,
    constraint              = SQLITE_CONSTRAINT,
    mismatch                = SQLITE_MISMATCH,
    misuse                  = SQLITE_MISUSE,
    nolfs                   = SQLITE_NOLFS,
    auth                    = SQLITE_AUTH,
    range                   = SQLITE_RANGE,
    notadb                  = SQLITE_NOTADB,

    // Extended result codes
    error_missing_collseq   = SQLITE_ERROR_MISSING_COLLSEQ,
    error_retry             = SQLITE_ERROR_RETRY,
    error_snapshot          = SQLITE_ERROR_SNAPSHOT,
    error_reservesize       = SQLITE_ERROR_RESERVESIZE,
    error_key               = SQLITE_ERROR_KEY,
    error_unable            = SQLITE_ERROR_UNABLE,

    abort_rollback          = SQLITE_ABORT_ROLLBACK,

    busy_recovery           = SQLITE_BUSY_RECOVERY,
    busy_snapshot           = SQLITE_BUSY_SNAPSHOT,
    busy_timeout            = SQLITE_BUSY_TIMEOUT,

    locked_sharedcache      = SQLITE_LOCKED_SHAREDCACHE,
    locked_vtab             = SQLITE_LOCKED_VTAB,

    readonly_recovery       = SQLITE_READONLY_RECOVERY,
    readonly_cantlock       = SQLITE_READONLY_CANTLOCK,
    readonly_rollback       = SQLITE_READONLY_ROLLBACK,
    readonly_dbmoved        = SQLITE_READONLY_DBMOVED,
    readonly_cantinit       = SQLITE_READONLY_CANTINIT,
    readonly_directory      = SQLITE_READONLY_DIRECTORY,

    ioerr_read              = SQLITE_IOERR_READ,
    ioerr_short_read        = SQLITE_IOERR_SHORT_READ,
    ioerr_write             = SQLITE_IOERR_WRITE,
    ioerr_fsync             = SQLITE_IOERR_FSYNC,
    ioerr_dir_fsync         = SQLITE_IOERR_DIR_FSYNC,
    ioerr_truncate          = SQLITE_IOERR_TRUNCATE,
    ioerr_fstat             = SQLITE_IOERR_FSTAT,
    ioerr_unlock            = SQLITE_IOERR_UNLOCK,
    ioerr_rdlock            = SQLITE_IOERR_RDLOCK,
    ioerr_delete            = SQLITE_IOERR_DELETE,
    ioerr_blocked           = SQLITE_IOERR_BLOCKED,
    ioerr_nomem             = SQLITE_IOERR_NOMEM,
    ioerr_access            = SQLITE_IOERR_ACCESS,
    ioerr_checkreservedlock = SQLITE_IOERR_CHECKRESERVEDLOCK,
    ioerr_lock              = SQLITE_IOERR_LOCK,
    ioerr_close             = SQLITE_IOERR_CLOSE,
    ioerr_dir_close         = SQLITE_IOERR_DIR_CLOSE,
    ioerr_shmopen           = SQLITE_IOERR_SHMOPEN,
    ioerr_shmsize           = SQLITE_IOERR_SHMSIZE,
    ioerr_shmlock           = SQLITE_IOERR_SHMLOCK,
    ioerr_shmmap            = SQLITE_IOERR_SHMMAP,
    ioerr_seek              = SQLITE_IOERR_SEEK,
    ioerr_delete_noent      = SQLITE_IOERR_DELETE_NOENT,
    ioerr_mmap              = SQLITE_IOERR_MMAP,
    ioerr_gettemppath       = SQLITE_IOERR_GETTEMPPATH,
    ioerr_convpath          = SQLITE_IOERR_CONVPATH,
    ioerr_vnode             = SQLITE_IOERR_VNODE,
    ioerr_auth              = SQLITE_IOERR_AUTH,
    ioerr_begin_atomic      = SQLITE_IOERR_BEGIN_ATOMIC,
    ioerr_commit_atomic     = SQLITE_IOERR_COMMIT_ATOMIC,
    ioerr_rollback_atomic   = SQLITE_IOERR_ROLLBACK_ATOMIC,
    ioerr_data              = SQLITE_IOERR_DATA,
    ioerr_corruptfs         = SQLITE_IOERR_CORRUPTFS,
    ioerr_in_page           = SQLITE_IOERR_IN_PAGE,
    ioerr_badkey            = SQLITE_IOERR_BADKEY,
    ioerr_codec             = SQLITE_IOERR_CODEC,

    corrupt_vtab            = SQLITE_CORRUPT_VTAB,
    corrupt_sequence        = SQLITE_CORRUPT_SEQUENCE,
    corrupt_index           = SQLITE_CORRUPT_INDEX,

    cantopen_notempdir      = SQLITE_CANTOPEN_NOTEMPDIR,
    cantopen_isdir          = SQLITE_CANTOPEN_ISDIR,
    cantopen_fullpath       = SQLITE_CANTOPEN_FULLPATH,
    cantopen_convpath       = SQLITE_CANTOPEN_CONVPATH,
    cantopen_dirtywal       = SQLITE_CANTOPEN_DIRTYWAL,
    cantopen_symlink        = SQLITE_CANTOPEN_SYMLINK,

    constraint_check        = SQLITE_CONSTRAINT_CHECK,
    constraint_commithook   = SQLITE_CONSTRAINT_COMMITHOOK,
    constraint_foreignkey   = SQLITE_CONSTRAINT_FOREIGNKEY,
    constraint_function     = SQLITE_CONSTRAINT_FUNCTION,
    constraint_notnull      = SQLITE_CONSTRAINT_NOTNULL,
    constraint_primarykey   = SQLITE_CONSTRAINT_PRIMARYKEY,
    constraint_trigger      = SQLITE_CONSTRAINT_TRIGGER,
    constraint_unique       = SQLITE_CONSTRAINT_UNIQUE,
    constraint_vtab         = SQLITE_CONSTRAINT_VTAB,
    constraint_rowid        = SQLITE_CONSTRAINT_ROWID,
    constraint_pinned       = SQLITE_CONSTRAINT_PINNED,
    constraint_datatype     = SQLITE_CONSTRAINT_DATATYPE,

    auth_user               = SQLITE_AUTH_USER
    // clang-format on
};

class sqlite_error_category final : public std::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override { return "sqlite"; }

    [[nodiscard]] auto message(int code) const -> std::string override { return ::sqlite3_errstr(code); }

    [[nodiscard]] auto default_error_condition(int code) const noexcept -> std::error_condition override {
        // NOLINTNEXTLINE(cppcoreguidelines-*,readability-*)
        int primary_code = code & 0xff;
        if (primary_code == SQLITE_ROW || primary_code == SQLITE_DONE) {
            primary_code = SQLITE_OK;
        }
        return {primary_code, *this};
    }
};

export [[nodiscard]] auto sqlite_category() noexcept -> const std::error_category& {
    static const sqlite_error_category instance;
    return instance;
}

export [[nodiscard]] auto make_error_condition(sqlite_errc cond) noexcept -> std::error_condition {
    return {std::to_underlying(cond), sqlite_category()};
}

export [[nodiscard]] auto make_sqlite_error_code(int code) noexcept -> std::error_code {
    return {code, sqlite_category()};
}

export [[nodiscard]] auto make_checked_sqlite_error_code(int code) noexcept -> std::error_code {
    // NOLINTNEXTLINE(cppcoreguidelines-*,readability-*)
    const int primary_code = code & 0xff;
    if (primary_code == SQLITE_OK || primary_code == SQLITE_ROW || primary_code == SQLITE_DONE) {
        code = SQLITE_OK;
    }
    return {code, sqlite_category()};
}

} // namespace sqlixx

template <>
struct std::is_error_condition_enum<sqlixx::sqlite_errc> : std::true_type {};
