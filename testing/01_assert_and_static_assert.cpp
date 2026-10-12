// ============================================================
// assert, static_assert and Checking Your Assumptions
// ============================================================
// The two checks built into the language, before any framework:
//
//   assert(condition)          run time, debug builds only (<cassert>)
//   static_assert(condition)   compile time, always
//
// Like Python's doctests in Python Zero to Hero, these keep small
// facts about the code next to the code itself.
//
// Build and run:
//   cmake --build build --target testing_01_assert_and_static_assert
//   ./build/bin/testing_01_assert_and_static_assert
// ============================================================

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>

// ------------------------------------------------------------
// static_assert: Facts the Compiler Proves
// ------------------------------------------------------------
// Free at run time. Ideal for platform assumptions, type
// properties and constexpr function results. The message is
// optional since C++17.

static_assert(sizeof(std::int32_t) == 4, "int32_t must be exactly 4 bytes");
static_assert(std::is_trivially_copyable_v<std::array<int, 3>>);

constexpr int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

static_assert(gcd(12, 18) == 6);   // a tiny unit test that runs inside the compiler
static_assert(gcd(7, 5) == 1);

// ------------------------------------------------------------
// assert: Preconditions and Invariants at Run Time
// ------------------------------------------------------------
// assert documents what MUST be true if the program is correct.
// It is for programmer errors, not for user input or I/O failures
// (validate those properly; asserts vanish in Release builds).
//
// Release builds define NDEBUG, which turns assert into nothing.
// So never put side effects inside it:
//     assert(file.open("x"));   // in Release, the file is never opened

double percentage(int part, int whole)
{
    assert(whole > 0 && "whole must be positive");
    assert(part >= 0 && part <= whole);
    return 100.0 * part / whole;
}

// ------------------------------------------------------------
// Checking Results by Hand
// ------------------------------------------------------------
// The simplest possible test: compare and report. Every framework
// is a more convenient version of this function.

int g_failures = 0;

void expect(bool condition, const std::string& description)
{
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << description << '\n';
    if (!condition) ++g_failures;
}

int main()
{
    std::cout << "=== static_assert ===\n";
    std::cout << "  all static_asserts passed, or this would not have compiled\n";

    std::cout << "\n=== assert ===\n";
#ifdef NDEBUG
    std::cout << "  NDEBUG is set: asserts are compiled out in this build\n";
#else
    std::cout << "  asserts are active in this build\n";
#endif
    std::cout << "  percentage(1, 4) = " << percentage(1, 4) << '\n';
    // percentage(1, 0);   // Debug: "Assertion failed: whole > 0 && ..." then abort

    std::cout << "\n=== Hand-Rolled Checks ===\n";
    expect(gcd(100, 75) == 25, "gcd(100, 75) == 25");
    expect(percentage(3, 3) == 100.0, "percentage(3, 3) == 100");
    expect(std::string{"abc"}.size() == 3, "string size");

    std::cout << "\n  " << g_failures << " failure(s)\n";
    return g_failures == 0 ? 0 : 1;   // non-zero exit code tells ctest / CI that tests failed
}

// ------------------------------------------------------------
// Looking Ahead: Contracts (C++26)
// ------------------------------------------------------------
// C++26 adds contract assertions to the language:
//     double percentage(int part, int whole)
//         pre(whole > 0)
//         post(r: r >= 0.0);
// with build-time choice of what happens on violation. Until your
// compiler supports them, assert is the tool.
