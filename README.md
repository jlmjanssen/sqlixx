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

`sqlixx` is released under the [MIT License](LICENSE)

---

```C++
import std;
import sqlixx;

int main() {
    if (auto conn = sqlixx::open_connection("file:data.db?mode=ro&cache=private", sqlixx::open::uri)) {
        if (auto prep = sqlixx::prepare_statement(*conn, "SELECT u.id, u.name FROM users AS u WHERE u.status = ?;")) {
            if (auto bind_res = sqlixx::bind_parameters(prep->stmt, "active")) {
                int id;
                std::string_view name;
                for (auto row : sqlixx::execute(prep->stmt)) {
                    if (sqlixx::read_tuple(row, std::tie(id, name))) {
                        std::println("id = {}, name = {}", id, name);
                    }
                }
            }
        }
    }
    return 0;
}
```
