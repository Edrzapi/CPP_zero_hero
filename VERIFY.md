# Verification Guide

How to build, run and check every target in C++ Zero to Hero, and what has already been verified.

---

## Status

| Toolchain | Status |
|-----------|--------|
| MSVC 19.44 (Visual Studio 2022 17.14), Windows 11, Ninja and VS generators | **Verified**: all builds below pass, zero warnings at `/W4`, clean with `/WX`, all 109 default `ctest` tests (82 examples, 13 task starters, 14 task checks) pass in Debug, Release and Debug + AddressSanitizer; the 3 C++23 targets pass with `/std:c++latest`; every example's output was read and checked against its comments; every task solution passes `tasks/check/check.py` (Python 3.13) |
| GCC 15.2.0 (Ubuntu 26.04, WSL 2) | **Verified**: zero warnings at `-Wall -Wextra -Wpedantic`, clean with `-Werror`; all 109 default `ctest` tests pass in Debug, Release and Debug + ASan/UBSan (no sanitizer reports); the 3 C++23 targets build and pass (112 tests) |
| Clang 21.1.8 with libstdc++ 15 (Ubuntu 26.04, WSL 2) | **Verified**: same results as GCC 15 (Debug, Release `-Werror`, ASan/UBSan, C++23 with 112 tests) |
| Clang 21.1.8 with libc++ 21 (`-stdlib=libc++`) | **Verified**: zero warnings, 109 tests pass in Debug, Release `-Werror` and ASan/UBSan. C++23 build: `<generator>` is not in libc++ yet, as expected (section 2) |
| GCC 13.4.0 (the stated minimum) | **Verified**: zero warnings, 109 tests pass in Debug and Release `-Werror` |
| Clang 17.0.6 (the stated minimum) | **Verified**: zero warnings, 109 tests pass in Debug and Release `-Werror`, with both libstdc++ 15 and libstdc++ 13 (`--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/13`) |

Linux tools used: CMake 4.2.3, Ninja 1.13.2, Python 3.14.4, SQLite 3.46.1 (`libsqlite3-dev`), curl from the distribution. The task 10 (system `libsqlite3`) and task 12 (POSIX sockets, `popen` + `curl`) Linux paths are compiled and checked by `check_task10`, `check_task12` and `check_task12_stretch`; the live `task12_solution --lookup Dune` call through `curl` also returned results with GCC, Clang and libc++. The sanitizer runs used `UBSAN_OPTIONS=halt_on_error=1`, so any UBSan report would have failed its test.

MSVC was re-verified after the GCC/Clang fixes (MSVC 19.44.35228, VS-bundled CMake 3.31.6 and Ninja 1.12.1): Debug 109/109, Release `/WX` + C++23 112/112, Debug + ASan + UB demos 109/109, zero `/W4` warnings outside the UB demos.

---

## 1. Build Matrix

Run each configuration from the repository root. Every one should configure, build with no errors, and report `100% tests passed`.

### Linux / WSL (GCC)

```bash
sudo apt install build-essential cmake ninja-build      # Ubuntu 24.04: GCC 13, Ubuntu 26.04: GCC 15
sudo apt install libsqlite3-dev python3                 # for tasks/ (task 10 and the checks)

# A. Default Debug build
cmake -S . -B build-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-gcc 2>&1 | tee build-gcc.log
ctest --test-dir build-gcc --output-on-failure           # expect: 109 tests, 100% passed (fewer without Python or SQLite, see section 2)
grep -c "warning:" build-gcc.log                         # expect: 0

# B. Release, warnings as errors
cmake -S . -B build-gcc-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DCPPZH_WARNINGS_AS_ERRORS=ON
cmake --build build-gcc-rel
ctest --test-dir build-gcc-rel --output-on-failure       # expect: 109 tests, 100% passed

# C. C++23 targets (GCC 14+ needed for <generator> and <print>)
cmake -S . -B build-gcc-23 -G Ninja -DCPPZH_ENABLE_CXX23=ON
cmake --build build-gcc-23
ctest --test-dir build-gcc-23 --output-on-failure        # expect: 112 tests

# D. Sanitizers on the clean code (ASan + UBSan)
cmake -S . -B build-gcc-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCPPZH_SANITIZE=ON
cmake --build build-gcc-asan
ctest --test-dir build-gcc-asan --output-on-failure      # expect: 109 tests, no sanitizer reports

# E. The UB lab (see section 4)
cmake -S . -B build-ub -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCPPZH_BUILD_UB_DEMOS=ON -DCPPZH_SANITIZE=ON
cmake --build build-ub
```

### Linux / WSL (Clang)

Repeat A, B and D with Clang:

```bash
sudo apt install clang
CC=clang CXX=clang++ cmake -S . -B build-clang -G Ninja -DCPPZH_WARNINGS_AS_ERRORS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure
```

Optionally also with libc++ (`sudo apt install libc++-dev libc++abi-dev`, then add `-DCMAKE_CXX_FLAGS=-stdlib=libc++`). Expect `datetime_04_time_zones` to print its "not available" message there, which is correct.

### Windows (MSVC), as verified

From a **Developer PowerShell for VS 2022**:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DCPPZH_WARNINGS_AS_ERRORS=ON -DCPPZH_ENABLE_CXX23=ON
cmake --build build-rel
ctest --test-dir build-rel --output-on-failure            # 112 tests

cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCPPZH_BUILD_UB_DEMOS=ON -DCPPZH_SANITIZE=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure           # 109 tests

# Visual Studio generator (multi-config): executables in build-vs\bin\Debug\
cmake -S . -B build-vs
cmake --build build-vs --target run_testing
ctest --test-dir build-vs -C Debug --output-on-failure
```

In VS Code: CMake Tools kit **Visual Studio Community 2022 Release - amd64**, then **CMake: Build** and **CMake: Run Tests**.

---

## 2. Running Targets

Every example is an executable named `<folder>_<file>`. With Ninja or Makefiles it is at `build/bin/<target>`; with the Visual Studio generator at `build\bin\Debug\<target>.exe`.

```bash
cmake --build build --target <target>
./build/bin/<target>
```

Run a whole topic in order with `cmake --build build --target run_<topic>`.

### Default Targets (82 examples, all registered with ctest)

| Topic | Targets | `run_` target |
|-------|---------|---------------|
| entry point | `zero_hero` (not a ctest test) | |
| basics | `basics_01_program_structure` `basics_02_types_and_sizes` `basics_03_signed_and_unsigned` `basics_04_literals_and_auto` `basics_05_const_and_constexpr` `basics_06_references_and_pointers` `basics_07_initialisation` `basics_08_strings` `basics_09_enums` `basics_10_casts_and_conversions` `basics_11_arrays` | `run_basics` |
| control_flow | `control_flow_01_conditionals` `control_flow_02_switch` `control_flow_03_grading` | `run_control_flow` |
| iteration | `iteration_01_loops` `iteration_02_range_for_under_the_hood` | `run_iteration` |
| functions | `functions_01_functions` `functions_02_lambdas` `functions_03_function_pointers` | `run_functions` |
| compilation | `compilation_01_preprocessor` `compilation_02_multi_file` (links library `compilation_geometry`) | `run_compilation` |
| c_interop | `c_interop_01_c_headers` `c_interop_02_c_library` (links C library `c_interop_cstack`) `c_interop_03_name_mangling` `c_interop_04_structs_and_layout` | `run_c_interop` |
| memory | `memory_01_stack_and_heap` `memory_02_raii` `memory_03_smart_pointers` `memory_04_rule_of_zero_three_five` `memory_05_move_semantics` | `run_memory` |
| oop | `oop_01_classes` `oop_02_constructors` `oop_03_inheritance` `oop_04_virtual_functions` `oop_05_object_slicing` `oop_06_multiple_inheritance` `oop_07_crtp` `oop_08_pimpl` (links library `oop_pimpl_widget`) `oop_09_operator_overloading` | `run_oop` |
| collections | `collections_01_vector` `collections_02_other_sequences` `collections_03_associative` `collections_04_iterators` `collections_05_algorithms` `collections_06_ranges_and_views` `collections_07_span` | `run_collections` |
| templates | `templates_01_function_templates` `templates_02_class_templates` `templates_03_specialisation` `templates_04_variadic_and_folds` `templates_05_concepts` `templates_06_sfinae` | `run_templates` |
| exceptions | `exceptions_01_exceptions` `exceptions_02_custom_exceptions` `exceptions_03_error_codes_and_optional` `exceptions_04_noexcept_and_safety` | `run_exceptions` |
| io | `io_01_iostream` `io_02_string_streams` `io_03_fstream` `io_04_filesystem` `io_05_format` | `run_io` |
| datetime | `datetime_01_durations_and_clocks` `datetime_02_calendar` `datetime_03_c_time` `datetime_04_time_zones` | `run_datetime` |
| concurrency | `concurrency_01_threads` `concurrency_02_mutexes` `concurrency_03_condition_variable` `concurrency_04_atomics` `concurrency_05_async_and_future` | `run_concurrency` |
| testing | `testing_01_assert_and_static_assert` `testing_calculator_test` (links library `testing_calculator`) | `run_testing` |
| under_the_hood | `under_the_hood_01_constexpr_and_consteval` `under_the_hood_02_structured_bindings` `under_the_hood_03_variant_and_any` `under_the_hood_04_string_view_lifetimes` `under_the_hood_05_attributes` `under_the_hood_06_user_defined_literals` `under_the_hood_07_placement_new_and_alignment` `under_the_hood_08_copy_elision` `under_the_hood_09_small_buffer_optimisation` `under_the_hood_10_compiler_explorer` | `run_under_the_hood` |

### Practical Task Targets (27 ctest tests)

Every task folder builds `taskNN_starter` and `taskNN_solution` (`NN` is `01` to `12`, or `B`). See `tasks/README.md`.

| ctest test | What it runs | Passes when |
|------------|--------------|-------------|
| `task01_starter` ... `task12_starter`, `taskB_starter` (13) | the starter, with `tasks/data` for 08 to 12 | it prints `not implemented` |
| `check_task01` ... `check_task12`, `check_task12_stretch`, `check_taskB` (14) | `tasks/check/check.py` on the solution, working directory `build/tasks` | the checker prints `PASS` |

The 14 `check_task` tests are only registered when CMake finds Python 3.10 or later. Task 10 is only built when SQLite is found: always with MSVC (`winsqlite3`), otherwise `libsqlite3-dev`. Expect 95 tests without Python, 107 without SQLite, 94 without either.

Run the checks by hand from the repository root, for example:

```bash
py -3.13 tasks/check/check.py 07 -- build/bin/task07_solution
py -3.13 tasks/check/check.py 10 -- build/bin/task10_solution tasks/data
py -3.13 tasks/check/check.py 12 -- build/bin/task12_solution tasks/data {port}
```

### C++23 Targets (`-DCPPZH_ENABLE_CXX23=ON`)

| Target | Needs |
|--------|-------|
| `iteration_03_generator_cxx23` | `<generator>`: GCC 14+, MSVC 19.40+ (not in libc++ yet) |
| `exceptions_05_expected_cxx23` | `<expected>`: GCC 12+, Clang 16+ (libc++), MSVC 19.33+ |
| `io_06_print_cxx23` | `<print>`: GCC 14+, Clang 18+ (libc++), MSVC 19.37+ |

If a compiler lacks one of these headers, the C++23 build fails on that target only; the default build is unaffected.

### Opt-In Undefined Behaviour Targets (`-DCPPZH_BUILD_UB_DEMOS=ON`)

Never part of ctest or the `run_` targets. Warnings are never errors for these.

| Target | What to look for |
|--------|------------------|
| `compilation_03_odr_violation` | Two different `inline magic_number()` bodies link silently; at least one line prints the wrong value. MSVC Debug: `second.cpp expects 2, sees 1`. |
| `concurrency_ub_data_race` | Lost updates (MSVC Debug printed 258148 of 800000). For a real diagnosis on Linux: `g++ -std=c++20 -g -O1 -fsanitize=thread concurrency/ub_data_race.cpp -o race && ./race` reports `ThreadSanitizer: data race`. |
| `under_the_hood_ub_examples <case>` | See section 4. |

---

## 3. Output Checkpoints

ctest only checks the exit code. These lines confirm the examples behave as their comments say. Values marked "MSVC" were observed on 19.44 and differ legitimately elsewhere.

| Target | Expect |
|--------|--------|
| `basics_02_types_and_sizes` | `long` is 4 on Windows, 8 on Linux; `0.1 + 0.2 = 0.30000000000000004` |
| `basics_03_signed_and_unsigned` | `(unsigned)-1 < 1u : false`, `std::cmp_less(-1, 1u) : true`, `-7 / 2 = -3` |
| `basics_07_initialisation` | `a(3, 1) size 3: 1 1 1` and `b{3, 1} size 2: 3 1` |
| `compilation_02_multi_file` | `shared_counter (inline) = 3`, `same address in both files? true`, per-file counters `1` and `2` |
| `c_interop_03_name_mangling` | MSVC: `?add@@YAHHH@Z`, `?add@maths@@YAHHH@Z`, `?add@Calculator@@QEBAHHH@Z`, `c_add`. GCC/Clang: the first section prints `(not available on this compiler: use nm)`, then `_Z3addii -> add(int, int)`, `maths::add(int, int)`, `Calculator::add(int, int) const` |
| `c_interop_04_structs_and_layout` | `sizeof(Wasteful) = 12`, `sizeof(Compact) = 8` |
| `oop_04_virtual_functions` | `sizeof(Polymorphic) = 16 (includes the vptr)`; `FancyPrinter copies=1` then `copies=5` |
| `oop_06_multiple_inheritance` | Without virtual: two different ids; with virtual: `Device constructed with "photocopier"` |
| `memory_04_rule_of_zero_three_five` | `a[0] = 42, b[0] = 7 (independent copies)`, `a.size() = 0` after the move |
| `exceptions_04_noexcept_and_safety` | `noexcept move on reallocation: move move`, `throwing move on reallocation: copy copy`, basic size 2 vs strong size 0 |
| `concurrency_02_mutexes` / `04_atomics` | `expected 800000, got 800000` |
| `testing_calculator_test` | `12 passed, 0 failed` |
| `under_the_hood_08_copy_elision` | No COPY or MOVE for RVO and NRVO; one MOVE for `make_either`; COPY then MOVE for `pass_through(c)` |
| `under_the_hood_09_small_buffer_optimisation` | Release: length 15 inline with 0 allocations, length 16 heap with 1. MSVC Debug adds one proxy allocation each. libc++ keeps up to 22 characters inline. |
| `datetime_04_time_zones` | MSVC, GCC 15 and Clang 21 with libstdc++: four zones listed, `London in July ... 13:00 BST`. libc++ 21: the "does not provide C++20 time zones yet" message |

---

## 4. The UB Lab

```bash
cmake -S . -B build-ub -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCPPZH_BUILD_UB_DEMOS=ON -DCPPZH_SANITIZE=ON
cmake --build build-ub --target under_the_hood_ub_examples
for c in signed-overflow shift heap-overflow stack-overflow use-after-free \
         double-free dangling-view null-deref uninitialised leak; do
    ./build-ub/bin/under_the_hood_ub_examples "$c"
done
```

| Case | GCC/Clang (ASan + UBSan) expected | MSVC 19.44 `/fsanitize=address` observed |
|------|-----------------------------------|-------------------------------------------|
| signed-overflow | `runtime error: signed integer overflow` | silent (no UBSan on MSVC) |
| shift | `runtime error: shift exponent 32 is too large` | silent |
| heap-overflow | `AddressSanitizer: heap-buffer-overflow` | reported |
| stack-overflow | `runtime error: index 4 out of bounds` (UBSan) then `AddressSanitizer: stack-buffer-overflow` | reported |
| use-after-free | `AddressSanitizer: heap-use-after-free` | reported |
| double-free | `AddressSanitizer: attempting double-free` | reported |
| dangling-view | `AddressSanitizer: heap-use-after-free` | reported |
| null-deref | `runtime error: load of null pointer` or `SEGV` | reported (access-violation) |
| uninitialised | not caught (needs Clang `-fsanitize=memory`) | silent under ASan; a plain MSVC Debug build stops with Run-Time Check Failure #3 (exit code 3) |
| leak | `LeakSanitizer: detected memory leaks` | silent (MSVC ASan has no leak checker) |

MSVC note: the ASan runtime DLL must be on `PATH`. A Developer PowerShell, or VS Code after selecting the VS kit, provides it. `CPPZH_SANITIZE` removes `/RTC1` (incompatible with ASan) and turns off incremental linking.

---

## 5. Portability Watch List

Places most likely to need attention on a compiler other than MSVC. Each was written defensively; check them first if anything fails.

| File | Risk | Mitigation already in place |
|------|------|-----------------------------|
| `io/05_format.cpp`, `datetime/04_time_zones.cpp` | `<format>` needs GCC 13+ / Clang 17+ | time zones guarded by `__cpp_lib_chrono` and `__cpp_lib_format`, plus a run-time catch for a missing tz database |
| `concurrency/01_threads.cpp` | `std::jthread` missing in older libc++ | guarded by `__cpp_lib_jthread` |
| `under_the_hood/07_placement_new_and_alignment.cpp` | `hardware_destructive_interference_size` | guarded by `__cpp_lib_hardware_interference_size` |
| `under_the_hood/01_constexpr_and_consteval.cpp` | `constexpr std::vector` / `std::string` need GCC 12+, Clang 15+ with libc++ | none needed on the stated minimums |
| `c_interop/03_name_mangling.cpp` | `<cxxabi.h>` | included only when `__GNUC__` is defined and `_MSC_VER` is not |
| `datetime/03_c_time.cpp` | `localtime_s` vs `localtime_r` | `_MSC_VER` / `_WIN32` / POSIX branches |
| `under_the_hood/05_attributes.cpp` | `[[no_unique_address]]` ignored by MSVC | `[[msvc::no_unique_address]]` on MSVC; sizes printed, not asserted |
| `under_the_hood/09_small_buffer_optimisation.cpp` | replaces global `operator new` | standard-conforming replacement; numbers are printed, not asserted |
| `under_the_hood/02_structured_bindings.cpp` | lambda capture of structured bindings (C++20) needs Clang 16+ | within the stated minimum (Clang 17) |
| `tasks/10_sqlite/*/db.h` | `<winsqlite/winsqlite3.h>` exists only in the Windows SDK | `__has_include` picks `<sqlite3.h>` elsewhere; CMake links `SQLite::SQLite3` and skips the task with a message if it is missing |
| `tasks/12_library_api/solution/server.cpp` | Winsock vs POSIX sockets | POSIX branch verified on GCC 15 and Clang 21 (`close`, `timeval` for `SO_RCVTIMEO`, `SO_REUSEADDR` on POSIX only); the empty POSIX `NetworkSession` is `[[maybe_unused]]` |
| `tasks/12_library_api/solution/openlibrary.cpp` | `popen` mode `"rb"` is Windows-only | `"r"` on POSIX |
| `tasks/B_busy_library/solution/main.cpp` | `<latch>` needs GCC 11+, Clang 14+ with libc++ | within the stated minimums |

### Fixed During the GCC/Clang Verification

| File | What GCC or Clang reported | Fix |
|------|----------------------------|-----|
| `datetime/02_calendar.cpp` | GCC and Clang: `2024y.is_leap()` is one preprocessing number, so `operator""y.is_leap` is not found (MSVC accepted it) | `(2024y).is_leap()`, with a comment |
| `under_the_hood/01_constexpr_and_consteval.cpp` | Clang with libstdc++: the `std::string(string_view)` constructor is not a constant expression | `std::string copy(text.begin(), text.end())` |
| `tasks/08`, `09`, `12` `json.h` (4 copies) | Clang with libstdc++: `std::pair<std::string, JsonValue>` instantiated while `JsonValue` is incomplete | `JsonValue() = default;` declared in the struct |
| `concurrency/04_atomics.cpp` | Clang 17: `counter.is_lock_free()` needs `__atomic_is_lock_free` from libatomic (link error) | prints `std::atomic<long>::is_always_lock_free` (compile time) |
| `oop/06_multiple_inheritance.cpp` | GCC `-Wcomment`: a `//` diagram line ending in `\` continues onto the next line | diagram moved into a `/* */` comment |
| `main.cpp` | GCC `-Wsequence-point` on `number++` inside a `<<` chain | increment on its own line |
| `tasks/12_library_api/solution/server.cpp` | GCC/Clang `-Wunused-variable`: the POSIX `NetworkSession` is empty | `[[maybe_unused]]` |
| `tasks/01` to `05` `starter/main.cpp` | Clang `-Wunused-const-variable` on the data constants the student has not used yet | `[[maybe_unused]]` on each, with a comment |
