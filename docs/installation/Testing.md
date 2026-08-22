# Testing & Coverage

This page covers how to build and run the `notex_tests` suite, and how to generate a coverage report from it.

## Requirements

GoogleTest is fetched automatically by CMake, so no manual setup is needed to run the tests. Coverage reporting additionally needs [`gcovr`](https://gcovr.com/), which you can install with:

```bash
pip install gcovr
```

or using the package manager of your choice.

## Running the Test Suite

1. Configure the build with tests enabled:

   ```bash
   cmake -S . -B build -DTESTS=ON
   ```

2. Build:

   ```bash
   cmake --build build --parallel $(nproc)
   ```

3. Run the suite with CTest:

   ```bash
   ctest --test-dir build --output-on-failure
   ```

> [!TIP]
> You can also run the `notex_tests` binary directly, which gives you GoogleTest's own, more verbose output:
>
> ```bash
> ./bin/notex_tests
> ```
>
> To run a single test or a subset matching a pattern, pass `--gtest_filter`:
>
> ```bash
> ./bin/notex_tests --gtest_filter=Test_Manager.*
> ```

> [!NOTE]
> Combine `-DTESTS=ON` with `-DDEBUG=ON` to also build with warnings treated as errors, matching the checks the project is developed against.

## Coverage Report

> [!WARNING]
> `COVERAGE` requires `TESTS` to also be `ON`. Configuring with `-DCOVERAGE=ON` alone fails with a CMake error.

1. Configure the build with tests and coverage enabled:

   ```bash
   cmake -S . -B build -DTESTS=ON -DCOVERAGE=ON
   ```

2. Build the `coverage` target:

   ```bash
   cmake --build build --target coverage
   ```

   This builds `notex_tests`, runs it, and generates an HTML report with `gcovr` in one step.

3. Open the report:

   ```bash
   build/coverage/index.html
   ```

---

<div align="center">
<sub>
<b>NoTeX</b>
<br style="margin-bottom: 0.3rem;">

[Home](../README.md) &nbsp;•&nbsp; [Installation](./INSTALLATION.md) &nbsp;•&nbsp; [Usage](../usage/USAGE.md) &nbsp;•&nbsp; [Implementation](../implementation/IMPLEMENTATION.md)

</sub>
</div>
