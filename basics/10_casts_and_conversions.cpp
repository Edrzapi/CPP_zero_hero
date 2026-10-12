// ============================================================
// Casts and Conversions
// ============================================================
// C has one cast, (T)value, which does whatever it takes. C++
// splits that into four named casts so the intent is visible
// and the compiler can refuse the dangerous ones.
//
// Build and run:
//   cmake --build build --target basics_10_casts_and_conversions
//   ./build/bin/basics_10_casts_and_conversions
// ============================================================

#include <bit>        // std::bit_cast (C++20)
#include <cstdint>
#include <cstring>
#include <iostream>

// ------------------------------------------------------------
// Implicit Conversions
// ------------------------------------------------------------
// Widening (int to long long, float to double) is always safe.
// Narrowing (double to int, long long to int) can lose data.
// bool, char and enums convert to int in arithmetic.

void implicit_conversions()
{
    std::cout << "=== Implicit Conversions ===\n";

    int i = 7;
    long long wide = i;           // widening: safe
    double d = i;                 // int to double: exact for ints up to 2^53
    double half = i / 2;          // integer division FIRST, then converted: 3.0
    double real_half = i / 2.0;   // one operand double, so the division is double: 3.5

    std::cout << wide << ' ' << d << ' ' << half << ' ' << real_half << '\n';
}

// ------------------------------------------------------------
// static_cast
// ------------------------------------------------------------
// The everyday cast: numeric conversions, enum to int, void* back
// to its real type, and up or down a class hierarchy WITHOUT a
// run-time check (use dynamic_cast for a checked downcast).

void static_cast_demo()
{
    std::cout << "\n=== static_cast ===\n";

    double ratio = 0.75;
    int percent = static_cast<int>(ratio * 100);
    std::cout << "percent = " << percent << '\n';

    char letter = 'C';
    std::cout << "letter as number = " << static_cast<int>(letter) << '\n';  // 67
}

// ------------------------------------------------------------
// const_cast
// ------------------------------------------------------------
// Adds or removes const. The only legitimate use is calling an
// old C API that takes char* but does not modify it. Writing to
// an object that was DEFINED const is undefined behaviour.

std::size_t legacy_length(char* text)   // imagine an old C library function
{
    return std::strlen(text);
}

void const_cast_demo()
{
    std::cout << "\n=== const_cast ===\n";
    const char* message = "read only";
    std::size_t n = legacy_length(const_cast<char*>(message));   // safe: legacy_length only reads
    std::cout << "legacy_length = " << n << '\n';
}

// ------------------------------------------------------------
// reinterpret_cast and std::bit_cast
// ------------------------------------------------------------
// reinterpret_cast reinterprets the bits of a pointer as another
// pointer type. Reading an object through a pointer to an
// unrelated type breaks the "strict aliasing" rule and is
// undefined behaviour (char, unsigned char and std::byte are the
// exceptions: you may always inspect the raw bytes).
//
// To view the bits of a float as an integer, C programmers use a
// union or a pointer cast. In C++ use std::memcpy or, since C++20,
// std::bit_cast, which is constexpr and always defined.

void bit_level_casts()
{
    std::cout << "\n=== reinterpret_cast and bit_cast ===\n";

    float f = 1.0f;

    // Safe: inspecting raw bytes through unsigned char*.
    const auto* bytes = reinterpret_cast<const unsigned char*>(&f);
    std::cout << "bytes of 1.0f: ";
    for (std::size_t i = 0; i < sizeof f; ++i) {
        std::cout << static_cast<int>(bytes[i]) << ' ';   // 0 0 128 63 on little-endian
    }
    std::cout << '\n';

    // UB:   auto bits = *reinterpret_cast<std::uint32_t*>(&f);
    // Good: std::bit_cast copies the bits into a new object.
    auto bits = std::bit_cast<std::uint32_t>(f);
    std::cout << std::hex << "bit_cast<uint32_t>(1.0f) = 0x" << bits << std::dec << '\n';  // 0x3f800000

    // Pointer to integer and back: std::uintptr_t is wide enough to hold an address.
    int target = 5;
    auto address = reinterpret_cast<std::uintptr_t>(&target);
    int* back = reinterpret_cast<int*>(address);
    std::cout << "round trip through uintptr_t: " << *back << '\n';
}

// ------------------------------------------------------------
// The C-Style Cast
// ------------------------------------------------------------
// (T)x tries, in order: const_cast, static_cast, static_cast then
// const_cast, reinterpret_cast, reinterpret_cast then const_cast.
// It will silently pick a dangerous one. It is also hard to grep.
// Keep it for C code; in C++ use the named casts.

void c_style_cast()
{
    std::cout << "\n=== C-Style Cast ===\n";
    double d = 9.99;
    int a = (int)d;               // C style: works, but hides intent
    int b = int(d);               // functional style: the same thing
    int c = static_cast<int>(d);  // C++ style: preferred
    std::cout << a << ' ' << b << ' ' << c << '\n';
}

int main()
{
    implicit_conversions();
    static_cast_demo();
    const_cast_demo();
    bit_level_casts();
    c_style_cast();
}
