# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

target_sources(
    sqlixx.sqlixx
    PUBLIC
        FILE_SET CXX_MODULES
            FILES
                sqlixx.cppm
                error.cppm
                error.sqlite.cppm
                handles.cppm
                connection.cppm
                connection.open.cppm
                statement.cppm
                statement.coro.cppm
                statement.prepare.cppm
                statement.step.cppm
                binders.cppm
                readers.cppm
)
