# Contributing

- Use C++17 and standard library facilities first.
- Keep platform-specific code under `src/platform/<os>`.
- Add unit tests for parser and formatting changes.
- Run `cmake -B build -DBUILD_TESTS=ON` and `cmake --build build`, then `ctest --test-dir build --output-on-failure`.
