# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

target_sources(
    sqlixx.sqlixx
    PUBLIC
        FILE_SET CXX_MODULES
            FILES
                sqlixx.cppm
                concepts.cppm
                error.cppm
                error.sqlite.cppm
                handles.cppm
                connection.cppm
                connection.open.cppm
                index.cppm
                binder_context.cppm
                binders.cppm
                readers.cppm
                statement.cppm
                statement.prepare.cppm
                statement.bind.cppm
                statement.read.cppm
                statement.step.cppm
                statement.coro.cppm
)
