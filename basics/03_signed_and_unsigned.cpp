// ============================================================
// Signed and Unsigned Arithmetic
// ============================================================
// The rules C++ inherits from C for mixing signed and unsigned
// integers, what wraps, what is undefined, and the C++20 tools
// that make comparisons safe.
//
// Build and run:
//   cmake --build build --target basics_03_signed_and_unsigned
//   ./build/bin/basics_03_signed_and_unsigned
// ============================================================

#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>   // std::cmp_less, std::in_range (C++20)
#include <vector>

// ------------------------------------------------------------
// Unsigned Wraps, Signed Overflow is Undefined
// ------------------------------------------------------------
// Unsigned arithmetic is defined modulo 2^N: UINT_MAX + 1 == 0.
// Signed overflow is UNDEFINED BEHAVIOUR. The compiler may assume
// it never happens and optimise on that basis, e.g. it can turn
// "x + 1 > x" into "true". Java and C# wrap signed ints; C++ does not.
//
// Since C++20 signed integers are guaranteed to be two's
// complement, but overflow is still undefined.
// See under_the_hood/ub_examples.cpp for a live demo with UBSan.

void wrapping()
{
    std::cout << "=== Unsigned Wrap-Around (defined) ===\n";

    unsigned int u = std::numeric_limits<unsigned int>::max();
    std::cout << "max unsigned      : " << u << '\n';
    ++u;
    std::cout << "max unsigned + 1  : " << u << '\n';    // 0

    unsigned int zero = 0;
    --zero;
    std::cout << "0u - 1            : " << zero << '\n'; // 4294967295 for 32-bit unsigned

    // int big = std::numeric_limits<int>::max();
    // ++big;   // UNDEFINED BEHAVIOUR. Not "wraps to INT_MIN".

    // Check BEFORE the operation instead:
    int a = std::numeric_limits<int>::max() - 5;
    int b = 10;
    if (b > 0 && a > std::numeric_limits<int>::max() - b) {
        std::cout << "a + b would overflow, refusing to add\n";
    }
}

// ------------------------------------------------------------
// The Usual Arithmetic Conversions
// ------------------------------------------------------------
// When a signed and an unsigned operand of the same rank meet,
// the signed one is converted to unsigned. So -1 becomes
// 4294967295 and "-1 < 1u" is FALSE.
//
//   int s = -1; unsigned u = 1;
//   if (s < u) ...   // false, and -Wsign-compare / MSVC C4018 warn
//
// Below the conversion is written out with static_cast so you
// can see exactly what the compiler does behind your back.

void usual_conversions()
{
    std::cout << "\n=== Mixing Signed and Unsigned ===\n";

    int s = -1;
    unsigned int u = 1;

    std::cout << "-1 as unsigned          : " << static_cast<unsigned int>(s) << '\n';
    std::cout << "(unsigned)-1 < 1u       : " << std::boolalpha
              << (static_cast<unsigned int>(s) < u) << '\n';         // false

    // C++20: std::cmp_less and friends compare the mathematical values.
    std::cout << "std::cmp_less(-1, 1u)   : " << std::cmp_less(s, u) << '\n'; // true

    // std::in_range checks whether a value fits in another type.
    std::cout << "in_range<uint8_t>(300)  : " << std::in_range<std::uint8_t>(300) << '\n'; // false
    std::cout << "in_range<uint8_t>(200)  : " << std::in_range<std::uint8_t>(200) << '\n'; // true
}

// ------------------------------------------------------------
// Integer Promotion
// ------------------------------------------------------------
// Arithmetic on anything smaller than int (char, short, bool,
// uint8_t) first promotes the operands to int.

void promotion()
{
    std::cout << "\n=== Integer Promotion ===\n";

    std::uint8_t x = 200;
    std::uint8_t y = 100;
    auto sum = x + y;              // int, not uint8_t
    std::cout << "uint8 200 + 100 = " << sum << " (type is int, no wrap)\n";  // 300

    std::uint8_t stored = static_cast<std::uint8_t>(x + y);
    std::cout << "stored back into uint8_t = " << static_cast<int>(stored) << '\n'; // 44

    // ~ on a small unsigned type also promotes, a classic C bug:
    std::uint8_t mask = 0x0F;
    std::cout << "~mask as int   = " << ~mask << '\n';                             // -16
    std::cout << "~mask as uint8 = " << static_cast<int>(static_cast<std::uint8_t>(~mask)) << '\n'; // 240
}

// ------------------------------------------------------------
// Division, Modulo and Shifts
// ------------------------------------------------------------
// Integer division truncates toward zero (guaranteed since C++11).
// The sign of % follows the dividend.
// Shifting by >= the width of the type is undefined.
// Right-shifting a negative number is arithmetic since C++20.

void division_and_shifts()
{
    std::cout << "\n=== Division and Shifts ===\n";
    std::cout << " 7 /  2 = " << 7 / 2 << '\n';      //  3
    std::cout << "-7 /  2 = " << -7 / 2 << '\n';     // -3, not -4
    std::cout << "-7 %  2 = " << -7 % 2 << '\n';     // -1
    std::cout << " 7 % -2 = " << 7 % -2 << '\n';     //  1

    unsigned int one = 1;
    std::cout << "1u << 4 = " << (one << 4) << '\n'; // 16
    int negative = -16;
    std::cout << "-16 >> 2 = " << (negative >> 2) << '\n';  // -4 (C++20: arithmetic shift)
}

// ------------------------------------------------------------
// Sizes and Indexes: The size_t Trap
// ------------------------------------------------------------
// .size() returns an unsigned std::size_t. Counting down with it
// is a classic infinite loop:
//
//   for (std::size_t i = v.size() - 1; i >= 0; --i)   // i >= 0 is always true
//
// and v.size() - 1 on an EMPTY vector is a huge number.
// Options: count down with a signed index from std::ssize (C++20),
// use reverse iterators, or use std::views::reverse.

void size_t_trap()
{
    std::cout << "\n=== Counting Down Safely ===\n";

    std::vector<int> values{10, 20, 30};

    for (auto i = std::ssize(values) - 1; i >= 0; --i) {   // ssize returns a signed type
        std::cout << values[static_cast<std::size_t>(i)] << ' ';
    }
    std::cout << '\n';

    std::vector<int> empty;
    std::cout << "empty.size() - 1 = " << empty.size() - 1 << "  (wrapped!)\n";
}

int main()
{
    wrapping();
    usual_conversions();
    promotion();
    division_and_shifts();
    size_t_trap();
}
