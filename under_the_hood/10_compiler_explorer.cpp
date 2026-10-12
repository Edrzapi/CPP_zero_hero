// ============================================================
// Compiler Flags, Warnings, Sanitizers and Compiler Explorer
// ============================================================
// The compiler is the best code reviewer you have, if you let it
// talk. This file collects the flags worth knowing and a few
// small functions worth pasting into Compiler Explorer
// (https://godbolt.org) to see what the optimiser does.
//
// Build and run:
//   cmake --build build --target under_the_hood_10_compiler_explorer
//   ./build/bin/under_the_hood_10_compiler_explorer
// ============================================================

#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

// ------------------------------------------------------------
// Warning Flags
// ------------------------------------------------------------
// GCC / Clang:
//   -Wall -Wextra          the essential pair (this repo uses them)
//   -Wpedantic             reject compiler extensions (VLAs, etc.)
//   -Werror                warnings fail the build (CPPZH_WARNINGS_AS_ERRORS=ON)
//   -Wshadow               a local hides another variable
//   -Wconversion           implicit narrowing (int64 -> int32, double -> int)
//   -Wsign-conversion      implicit signed <-> unsigned
//   -Wnon-virtual-dtor     polymorphic class without a virtual destructor
//   -Wold-style-cast       C-style casts in C++ code
//   -Wnull-dereference, -Wdouble-promotion, -Wimplicit-fallthrough
//   -Weverything           Clang only; for exploring, not for builds
// MSVC:
//   /W4                    roughly -Wall -Wextra (this repo uses it)
//   /WX                    warnings as errors
//   /permissive-           standard conformance (this repo uses it)
//   /analyze               static analysis (SAL, null checks)
//
// Code that compiles but each flag catches:
//
//   int f(unsigned u, int i) { return u < i; }        // -Wsign-compare (in -Wall)
//   int g(double d) { return d; }                     // -Wconversion: double -> int
//   void h(int x) { int x2 = x; { int x2 = 0; } }     // -Wshadow
//   if (x = 0) { }                                    // -Wparentheses: assignment as condition
//   struct B { virtual void f(); };                   // -Wnon-virtual-dtor
//
// Static analysers go further: clang-tidy (with the
// cppcoreguidelines-* and bugprone-* checks), cppcheck, and the
// MSVC Code Analysis built into Visual Studio.

// ------------------------------------------------------------
// Sanitizers
// ------------------------------------------------------------
// Run-time instrumentation that catches bugs at the moment they
// happen, with a stack trace, instead of a crash much later:
//   -fsanitize=address      ASan: out of bounds, use after free, leaks (Linux)
//   -fsanitize=undefined    UBSan: signed overflow, bad shifts, null, misalignment
//   -fsanitize=thread       TSan: data races (cannot combine with ASan)
//   -fsanitize=memory       MSan (Clang, Linux): reads of uninitialised memory
// MSVC: /fsanitize=address only (no UBSan, no TSan).
// Configure this repo with -DCPPZH_SANITIZE=ON, and see
// under_the_hood/ub_examples.cpp (-DCPPZH_BUILD_UB_DEMOS=ON).
// Debug builds with sanitizers are typically 2x slower: fine for
// tests, not for production.

// ------------------------------------------------------------
// Compiler Explorer Tips
// ------------------------------------------------------------
// - Pick two compilers side by side (GCC and Clang, or x86-64 and
//   ARM64) and compare the assembly.
// - Always look at -O2 (or /O2) as well as -O0. Debug assembly
//   says almost nothing about real performance.
// - Mark the function you care about as non-inline or give it
//   external linkage, otherwise the optimiser may delete it.
// - Hover over an instruction for its documentation; the colours
//   link each source line to its instructions.
// - "Add new > Execution" runs the program; "Opt remarks" shows
//   why something was or was not inlined or vectorised.
// - quick-bench.com runs Google Benchmark online for timing
//   comparisons.
// The functions below are good first experiments.

// 1. A loop the optimiser replaces with a formula. Clang -O2
//    computes n*(n-1)/2 without any loop at all.
std::uint64_t sum_below(std::uint64_t n)
{
    std::uint64_t total = 0;
    for (std::uint64_t i = 0; i < n; ++i) total += i;
    return total;
}

// 2. Signed overflow being undefined LETS the optimiser assume
//    i + 1 > i. With unsigned (which wraps) it cannot. Compare the
//    assembly of these two.
bool always_bigger_signed(int i) { return i + 1 > i; }            // -O2: "return true"
bool always_bigger_unsigned(unsigned i) { return i + 1 > i; }     // needs a real comparison

// 3. Bounds checking cost: operator[] vs at(). Look for the extra
//    compare-and-branch in the at() version.
int read_unchecked(const std::vector<int>& v, std::size_t i) { return v[i]; }
int read_checked(const std::vector<int>& v, std::size_t i) { return v.at(i); }

// 4. Auto-vectorisation: at -O2/-O3 this becomes SIMD
//    instructions (paddd / vpaddd on x86) processing 4 or 8 ints
//    at a time. -march=native widens it further.
void add_arrays(int* __restrict out, const int* a, const int* b, std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i) out[i] = a[i] + b[i];
}

int main()
{
    std::cout << "=== Functions to Explore on godbolt.org ===\n";
    std::cout << "  sum_below(1000)            = " << sum_below(1000) << '\n';
    std::cout << "  always_bigger_signed(5)    = " << std::boolalpha << always_bigger_signed(5) << '\n';
    std::cout << "  always_bigger_unsigned(5)  = " << always_bigger_unsigned(5) << '\n';

    std::vector<int> v{1, 2, 3};
    std::cout << "  read_unchecked / checked   = " << read_unchecked(v, 1) << " / " << read_checked(v, 2) << '\n';

    int a[4] = {1, 2, 3, 4};
    int b[4] = {10, 20, 30, 40};
    int out[4] = {};
    add_arrays(out, a, b, 4);
    std::cout << "  add_arrays                 = " << out[0] << ' ' << out[1] << ' ' << out[2] << ' ' << out[3] << '\n';

    std::cout << "\n  Paste any of these functions into https://godbolt.org and compare -O0 with -O2.\n";
}
