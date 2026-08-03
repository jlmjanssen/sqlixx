# sqlixx: sqlite on C++ steroids

<!--
SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
SPDX-License-Identifier: BSL-1.0
-->

![Library status](https://img.shields.io/badge/Library%20status-under%20development-lightgrey)
[![pre-commit](https://img.shields.io/github/actions/workflow/status/jlmjanssen/sqlixx/pre-commit.yaml?label=pre-commit&logo=pre-commit&logoColor=white)](https://github.com/jlmjanssen/sqlixx/actions/workflows/pre-commit.yaml)
[![CI Tests](https://github.com/jlmjanssen/sqlixx/actions/workflows/ci-tests.yaml/badge.svg)](https://github.com/jlmjanssen/sqlixx/actions/workflows/ci-tests.yaml)
[![codecov](https://codecov.io/gh/jlmjanssen/sqlixx/graph/badge.svg?token=CCO7X7SUDG)](https://codecov.io/gh/jlmjanssen/sqlixx)

`sqlixx` is a C++23 module library that wraps the SQLite database library.

**Implements**: `module sqlixx` with connections, statements, binders, and readers.

**Status**: Experimental

---

## License

`sqlixx` is released under the [Boost Software License](LICENSE).

---

```C++
import std;
import sqlixx;

int main() {
    if (auto conn = sqlixx::open_connection("user.db", sqlixx::open::readwrite)) {
        const auto sql = "SELECT id, name FROM users WHERE status = ?";
        if (auto prep = sqlixx::prepare_statement(*conn, sql)) {
            int id;
            std::string_view name;
            std::ignore = sqlixx::bind(prep->stmt, "active");
            for (auto row : sqlixx::execute(prep->stmt)) {
                std::ignore = sqlixx::read(*row, id, name);
                std::println("id = {}, name = {}", id, name);
            }
        }
    }
    return 0;
}
```
