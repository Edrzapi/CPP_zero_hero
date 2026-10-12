# C++ Zero to Hero

A structured teaching repository covering modern C++ (C++20) from its C foundations through to templates, concurrency and what happens under the hood. Each topic is a folder of standalone, runnable examples with in-code commentary, built with CMake. Part of the Zero to Hero course family.

---

## Who Is This For?

- Developers learning C++ for the first time, or coming to it from C, Java, C# or Python
- Delegates on instructor-led C++ fundamentals and intermediate courses
- Self-learners who want a clear progression from the basics to the lesser-known corners of the language

No prior C++ experience is assumed. Basic programming awareness and comfort with a command line are helpful.

---

## Prerequisites

| Tool                        | Minimum Version |
|-----------------------------|----------------|
| CMake                       | 3.20           |
| One C++20 compiler:         |                |
| MSVC (Visual Studio 2022)   | 19.30          |
| GCC                         | 13             |
| Clang                       | 17             |

```bash
cmake --version
g++ --version        # or: clang++ --version, or cl (in a Developer PowerShell)
```

GCC 13 and Clang 17 are the first versions whose standard libraries ship `std::format`. Ubuntu 24.04 provides GCC 13 out of the box.

---

## Getting Started

### Windows: Visual Studio 2022 + VS Code (Recommended)

1. Install **Visual Studio 2022** (Community is fine) with the **Desktop development with C++** workload. This provides the MSVC compiler, CMake and Ninja.
2. Install **VS Code** with the **C/C++** (`ms-vscode.cpptools`) and **CMake Tools** (`ms-vscode.cmake-tools`) extensions.
3. Clone and open the folder:

```bash
git clone git@github.com:Edrzapi/CPP_zero_hero.git
code CPP_zero_hero
```

4. When CMake Tools asks for a kit, select **Visual Studio Community 2022 Release - amd64**. It configures the project automatically.
5. Build with **F7** (or the Build button in the status bar).
6. Pick an example with **CMake: Set Launch/Debug Target** (for example `basics_02_types_and_sizes`), then run it with **Shift+F5**, or debug it with **Ctrl+F5** and breakpoints.
7. Run every example as a test with **CMake: Run Tests**, or from the Testing panel.

### Windows: Developer PowerShell

Open **Developer PowerShell for VS 2022** from the Start menu (it puts the compiler on `PATH`), then:

```powershell
cd CPP_zero_hero
cmake -S . -B build                      # Visual Studio generator (multi-config)
cmake --build build                      # Debug by default
ctest --test-dir build -C Debug --output-on-failure
.\build\bin\Debug\basics_02_types_and_sizes.exe
```

Prefer single-config builds? `cmake -S . -B build -G Ninja` puts executables in `build\bin\` directly.

MinGW-w64 (for example GCC 13+ from MSYS2 UCRT64) also works: `cmake -S . -B build -G Ninja`.

### Linux and WSL

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build gdb   # GCC 13 on Ubuntu 24.04

git clone git@github.com:Edrzapi/CPP_zero_hero.git
cd CPP_zero_hero
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/bin/basics_02_types_and_sizes
```

To use Clang instead: `sudo apt install clang` then `CXX=clang++ CC=clang cmake -S . -B build-clang -G Ninja`.

### macOS

```bash
xcode-select --install                   # Apple Clang (Xcode 16 or later recommended)
brew install cmake ninja                 # or: brew install llvm for a newer Clang

cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/bin/basics_02_types_and_sizes
```

> **Note:** No third-party libraries are needed. Everything uses the C++ standard library, plus one small C library inside the repository (`c_interop/02_c_library`). The practical tasks also use what the operating system already provides: SQLite (task 10; on Linux install `libsqlite3-dev`), sockets (task 12) and the `curl` program (task 12 Stretch). Their checker needs Python 3.10 or later.

---

## Repository Structure

```
|-- basics/                 # Types, sizes, signed/unsigned, literals, const, references, initialisation
|-- control_flow/           # if/else, if with initialiser, switch, if constexpr
|-- iteration/              # Loops, range-for under the hood, generators (C++23)
|-- functions/              # Parameters, overloading, defaults, inline, lambdas, function pointers
|-- compilation/            # Preprocessor, translation units, headers, linkage, the ODR
|   |-- 02_multi_file/      # A small static library split into headers and sources
|   `-- 03_odr_violation/   # Opt-in: what breaking the One Definition Rule looks like
|-- c_interop/              # C headers, extern "C", RAII wrappers, name mangling, struct layout
|   `-- 02_c_library/       # A C library (cstack.c) called from C++
|-- memory/                 # Stack vs heap, RAII, smart pointers, rule of 0/3/5, move semantics
|-- oop/                    # Classes, inheritance, vtables, slicing, diamonds, CRTP, operators
|   `-- 08_pimpl/           # PIMPL split across header, source and program
|-- collections/            # Containers, iterators, algorithms, ranges and views, span
|-- templates/              # Function/class templates, specialisation, variadics, concepts, SFINAE
|-- exceptions/             # try/catch, custom hierarchies, error codes, optional, noexcept, expected
|-- io/                     # iostreams, string streams, files, std::filesystem, std::format, std::print
|-- datetime/               # chrono durations and clocks, calendar dates, <ctime>, time zones
|-- concurrency/            # Threads, jthread, mutexes, condition variables, atomics, futures
|-- testing/                # assert, static_assert, mini_test.h harness, Calculator tests
|-- under_the_hood/         # constexpr, variant, string_view traps, attributes, placement new,
|                           # copy elision, SSO, compiler flags, the UB lab
|-- tasks/                  # Practical tasks: the Library Manager (see Tasks below)
|   |-- data/               # Shared sample data (CSV, SQL schema, recorded API response)
|   |-- check/              # check.py: compares a program's output with expected.txt
|   `-- 01_bookshelf/ ... 12_library_api/, B_busy_library/
|                           # README.md brief, expected.txt, starter/, solution/
|-- cmake/zero_hero.cmake   # Shared build helpers (warnings, sanitizers, run_<topic> targets)
|-- CMakeLists.txt          # Top-level build: every example, test and option
|-- main.cpp                # Entry point: prints the syllabus
`-- VERIFY.md               # Every target and how to build, run and check it
```

Files inside each folder are numbered in teaching order: `01_...cpp`, `02_...cpp`.

---

## How to Run

Every example is its own executable named `<folder>_<file>`:

```bash
cmake --build build --target basics_02_types_and_sizes
./build/bin/basics_02_types_and_sizes          # Windows VS generator: .\build\bin\Debug\...
```

Run a whole topic, in order:

```bash
cmake --build build --target run_basics          # also run_control_flow, run_oop, run_templates, ...
```

Run everything as a smoke test (each example must exit with 0):

```bash
ctest --test-dir build --output-on-failure       # add -C Debug with the Visual Studio generator
```

Run the unit tests only, or a subset by name:

```bash
./build/bin/testing_calculator_test
./build/bin/testing_calculator_test divide        # tests whose name contains "divide"
```

### Build Options

| Option                          | Default | Effect |
|---------------------------------|---------|--------|
| `CPPZH_WARNINGS_AS_ERRORS`      | OFF     | Adds `-Werror` / `/WX`. The whole repository builds clean with it on. |
| `CPPZH_ENABLE_CXX23`            | OFF     | Builds the C++23 examples: `std::generator`, `std::expected`, `std::print`. |
| `CPPZH_BUILD_UB_DEMOS`          | OFF     | Builds the deliberate undefined behaviour demos (never part of `ctest`). |
| `CPPZH_SANITIZE`                | OFF     | AddressSanitizer + UBSan on GCC/Clang; AddressSanitizer only on MSVC. |

```bash
cmake -S . -B build-ub -DCPPZH_BUILD_UB_DEMOS=ON -DCPPZH_SANITIZE=ON
cmake --build build-ub --target under_the_hood_ub_examples
./build-ub/bin/under_the_hood_ub_examples use-after-free
```

---

## Topics Covered

### Fundamentals and the C Heritage

- **basics**: Program structure and `main`, fundamental types and their platform-dependent sizes (LP64 vs LLP64), fixed-width integers, signed vs unsigned and the usual arithmetic conversions, `std::cmp_less`, literals and `auto`/`decltype` deduction, `const` vs `constexpr` vs `consteval` vs `constinit`, references vs pointers, every initialisation form and narrowing, C strings vs `std::string`, `enum` vs `enum class`, the four named casts and `std::bit_cast`, C arrays, decay and `std::array`.
- **control_flow**: `if`/`else`, truthiness inherited from C, short-circuiting, `if` and `switch` with initialisers (C++17), fall-through and `[[fallthrough]]`, `if constexpr`.
- **iteration**: `for`, `while`, `do`-`while`, range-based `for`, what range-`for` expands to, writing your own iterable type, the temporary-lifetime trap, `std::erase_if`, and `std::generator` (C++23).
- **functions**: Pass by value / reference / const reference / pointer, sink parameters, returning structs, overload resolution, default arguments, what `inline` really means, lambdas and every capture form, dangling captures, generic lambdas, `std::function`, function pointers, `qsort` vs `std::sort`, functors, member function pointers and `std::invoke`.

### How C++ Is Built

- **compilation**: The preprocess / compile / assemble / link pipeline, macros and their pitfalls, `#` and `##`, conditional compilation and compiler detection, `std::source_location`, `assert`. A multi-file static library showing include guards vs `#pragma once`, declarations vs definitions, `extern` variables, internal linkage (`static`, unnamed namespaces), `inline` variables, forward declarations, and the linker errors you get when each rule is broken. An opt-in ODR violation that links silently and gives wrong answers.
- **c_interop**: `<stdio.h>` vs `<cstdio>`, `printf` and `snprintf`, `malloc`/`free` vs `new`/`delete`, valid C that is not valid C++, `extern "C"`, a C library (opaque handle, status codes) called from C++ and wrapped in an RAII class with `std::unique_ptr` and a custom deleter, name mangling on Itanium and MSVC (with `nm`, `c++filt`, `dumpbin`, `undname`), struct padding and `offsetof`, trivially copyable and standard-layout types, bit-fields, and when to keep C.

### Memory and Object-Oriented Programming

- **memory**: Automatic, dynamic and static lifetime, `new`/`delete` and what goes wrong with them, RAII and scope guards, `unique_ptr`, `shared_ptr`, `weak_ptr` and reference cycles, custom deleters, the rule of zero/three/five with a hand-written owning buffer and copy-and-swap, value categories, `std::move`, move-only types, and why move constructors must be `noexcept`.
- **oop**: Classes vs structs, invariants, `const` member functions, `mutable`, `static` members, `friend`, constructors in depth (initialiser lists, declaration-order initialisation, delegating, `explicit`), inheritance and construction order, name hiding, inherited constructors, virtual functions, `override`/`final`, abstract classes, virtual destructors, the vtable model, virtual calls during construction, `dynamic_cast`, object slicing and virtual `clone()`, multiple inheritance, the diamond and virtual inheritance, static polymorphism with CRTP, PIMPL to speed up builds, and operator overloading including `<=>`.

### The Standard Library

- **collections**: `std::vector` (capacity, invalidation, `erase`), `deque`, `list`, `forward_list`, `stack`, `queue`, `priority_queue`, `map`/`set` and the unordered containers with a custom hash, iterator categories and adaptors, the classic algorithms, C++20 ranges, projections and lazy views, and `std::span`.
- **templates**: Function and class templates, deduction, non-type parameters, abbreviated templates, CTAD and deduction guides, full and partial specialisation, writing type traits, variadic templates, fold expressions, perfect forwarding, concepts and `requires` clauses, overloading on constraints, and SFINAE (`enable_if`, `void_t`) with why concepts replaced it.
- **exceptions**: `throw`/`try`/`catch`, the standard hierarchy, stack unwinding, rethrowing, a three-level custom hierarchy (as in Java Zero to Hero), nested exceptions, error codes and `errno`, `std::from_chars`, `std::error_code`, `std::optional`, `noexcept`, the basic/strong/nothrow guarantees, and `std::expected` (C++23).
- **io**: Standard streams and manipulators, validating input and stream state, string streams for building and parsing text, text and binary files, `std::filesystem`, `std::format` with custom formatters, and `std::print` (C++23).
- **datetime**: `std::chrono` durations, clocks and time points, C++20 calendar types and date arithmetic, the C `<ctime>` API and bridging to `chrono`, and time zones (guarded by a feature-test macro where library support is missing).
- **concurrency**: `std::thread`, `std::jthread` and `stop_token`, passing arguments, exceptions across threads, `mutex`, `lock_guard`, `unique_lock`, `scoped_lock` and deadlock avoidance, `shared_mutex`, a blocking producer/consumer queue with `condition_variable`, atomics, `compare_exchange`, memory orders, `atomic_flag`, `std::async`, futures, promises and `packaged_task`. An opt-in data race demo (the C++ version of Java's `RaceConditionRunner`).

### Testing

- **testing**: `assert` and `static_assert`, a tiny self-contained test harness (`mini_test.h`) that shows how frameworks work inside, and a `Calculator` test suite with fixtures, table-driven tests, floating-point tolerance and exception checks. The comments show the same tests in GoogleTest and Catch2 and how to add either with CMake `FetchContent`.

### Under the Hood

- **under_the_hood**: Compile-time lookup tables, `constexpr` `std::vector`/`std::string` (C++20), `consteval` switch labels, `std::is_constant_evaluated`, structured bindings and the tuple protocol, `std::variant`/`std::visit` vs virtual functions, `std::any`, `std::string_view` lifetime traps, standard attributes, user-defined literals and strong types, placement `new`, `std::launder` and alignment, copy elision (RVO and NRVO), the small string optimisation measured by replacing `operator new`, compiler warning flags, sanitizers and Compiler Explorer tips. The opt-in UB lab has ten bugs to run under AddressSanitizer and UBSan.

---

## Tasks

The [`tasks/`](tasks/README.md) folder holds the **Library Manager**, one practical project shared by every Zero to
Hero repo: 12 tasks and a bonus in Python, Java, C#, C++ and JavaScript, and eight tasks (S1 to S8) in SQL's own
order on the same database. Every repo uses the same briefs, the same sample data and the same expected output; only
the code differs.

- **One task per session.** Each task is the end-of-session check and builds on the one before, so by the end the
  learner has one working program.
- **Three levels:** Core (everyone), Stretch (for those who finish early) and Challenge (the strongest learners, or
  homework).
- **Every task folder** has the brief (`README.md`), the exact expected output (`expected.txt`), a `starter/` that
  runs as it is (unfinished parts say "not implemented") and a `solution/` with Core and Stretch.
- **One checker:** `tasks/check/check.py` (Python 3.10 or later, standard library only) runs a program and prints
  `PASS`, or `FAIL` with the first line that differs from `expected.txt`. On Windows, `py` works in place of
  `python`.

[tasks/README.md](tasks/README.md) has the session to task map, the 3-day and 5-day selections, and the run and
check command for every task. To check the first task's solution, from the repo root:

```bash
cmake --build build --target task01_solution && python tasks/check/check.py 01 -- build/bin/task01_solution
```

---

## Teaching Approach

- Every file is a standalone, runnable program with its own build target
- Concepts are explained in comments alongside working code; the output each line prints is noted where it helps
- Mistakes are shown next to the correct approach, as commented-out code, so the build stays clean
- Undefined behaviour is never part of the default build: it lives in clearly labelled, opt-in targets
- Topics progress from the C heritage through modern C++ to the lesser-known material
- No third-party libraries: the focus is on the language and the standard library

---

## Tech Stack

| Technology       | Purpose                                                    |
|------------------|------------------------------------------------------------|
| C++20            | Language (C++23 examples are opt-in and clearly marked)    |
| C11              | The small C library in `c_interop/`                         |
| CMake 3.20+      | Build system (Ninja or Visual Studio generators)           |
| CTest            | Runs every example and the unit tests                      |
| mini_test.h      | In-repo test harness (no download)                         |
| MSVC, GCC, Clang | Supported compilers                                        |

No Boost. No Qt. No package managers. No third-party test frameworks.

---

## What's Next?

This course covers C++ from its C foundations to modern C++20. For the same structured approach in other languages, see the companion repositories:

- [Python Zero to Hero](https://github.com/Edrzapi/Python_zero_hero)
- [Java Zero to Hero](https://github.com/Edrzapi/Java_zero_hero)
- [C# Zero to Hero](https://github.com/Edrzapi/CS_zero_hero)
- [JavaScript Zero to Hero](https://github.com/Edrzapi/JavaScript_zero_hero)
- [SQL Zero to Hero](https://github.com/Edrzapi/SQL_zero_hero)
