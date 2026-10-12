// ============================================================
// Fundamental Types and Their Sizes
// ============================================================
// C++ inherits C's integer and floating-point types. Unlike
// Java and C#, their sizes are NOT fixed by the standard: only
// minimum ranges and an ordering are guaranteed. This file
// prints what your compiler and platform actually use.
//
// Build and run:
//   cmake --build build --target basics_02_types_and_sizes
//   ./build/bin/basics_02_types_and_sizes
// ============================================================

#include <climits>   // CHAR_BIT
#include <cstddef>   // std::size_t, std::ptrdiff_t, std::byte
#include <cstdint>   // std::int32_t and friends
#include <iostream>
#include <limits>    // std::numeric_limits

// ------------------------------------------------------------
// What the Standard Guarantees
// ------------------------------------------------------------
// 1 == sizeof(char) <= sizeof(short) <= sizeof(int)
//                   <= sizeof(long)  <= sizeof(long long)
// char  is at least 8 bits    short is at least 16 bits
// int   is at least 16 bits   long  is at least 32 bits
// long long is at least 64 bits
//
// Common data models:
//   LP64  (Linux, macOS, 64-bit):  int 32, long 64, pointer 64
//   LLP64 (Windows, 64-bit):       int 32, long 32, pointer 64
// So "long" is 64 bits on Linux and 32 bits on Windows. This is
// the classic portability trap when moving C code between them.

void print_sizes()
{
    std::cout << "=== sizeof (in bytes; CHAR_BIT = " << CHAR_BIT << ") ===\n";
    std::cout << "bool        : " << sizeof(bool) << '\n';
    std::cout << "char        : " << sizeof(char) << '\n';         // always 1
    std::cout << "short       : " << sizeof(short) << '\n';
    std::cout << "int         : " << sizeof(int) << '\n';
    std::cout << "long        : " << sizeof(long) << '\n';         // 4 on Windows, 8 on Linux
    std::cout << "long long   : " << sizeof(long long) << '\n';
    std::cout << "float       : " << sizeof(float) << '\n';
    std::cout << "double      : " << sizeof(double) << '\n';
    std::cout << "long double : " << sizeof(long double) << '\n'; // 8 on MSVC, 16 on x86-64 GCC
    std::cout << "void*       : " << sizeof(void*) << '\n';
    std::cout << "std::size_t : " << sizeof(std::size_t) << '\n';
}

// ------------------------------------------------------------
// Fixed-Width Integers (<cstdint>)
// ------------------------------------------------------------
// When the exact size matters (file formats, network packets,
// hardware registers, hashing) use the fixed-width aliases.
// std::int8_t is usually a typedef for signed char, so printing
// it with << shows a character, not a number. Cast to int first.

void fixed_width_integers()
{
    std::cout << "\n=== Fixed-Width Integers ===\n";

    std::int8_t   tiny  = 65;
    std::uint16_t port  = 8080;
    std::int32_t  count = 2'000'000;              // ' is the digit separator (C++14)
    std::uint64_t big   = 18'000'000'000'000'000'000ULL;

    std::cout << "int8_t printed directly : " << tiny << '\n';                   // A
    std::cout << "int8_t cast to int      : " << static_cast<int>(tiny) << '\n'; // 65
    std::cout << "uint16_t                : " << port << '\n';
    std::cout << "int32_t                 : " << count << '\n';
    std::cout << "uint64_t                : " << big << '\n';

    // The "fast" and "least" variants trade exact size for speed or availability.
    std::cout << "sizeof(int_fast16_t)    : " << sizeof(std::int_fast16_t) << '\n';
    std::cout << "sizeof(int_least16_t)   : " << sizeof(std::int_least16_t) << '\n';

    // std::size_t: unsigned, result of sizeof, used for sizes and indexes.
    // std::ptrdiff_t: signed, result of subtracting two pointers.
    std::cout << "size_t max              : " << std::numeric_limits<std::size_t>::max() << '\n';
}

// ------------------------------------------------------------
// Ranges with std::numeric_limits
// ------------------------------------------------------------
// The C way is INT_MAX, LONG_MIN etc. from <climits>.
// The C++ way is std::numeric_limits<T>, which works in templates.

void numeric_ranges()
{
    std::cout << "\n=== numeric_limits ===\n";
    std::cout << "int    : " << std::numeric_limits<int>::min()
              << " to " << std::numeric_limits<int>::max() << '\n';
    std::cout << "short  : " << std::numeric_limits<short>::min()
              << " to " << std::numeric_limits<short>::max() << '\n';
    std::cout << "double : lowest " << std::numeric_limits<double>::lowest()
              << ", max " << std::numeric_limits<double>::max() << '\n';

    // min() for floating types is the smallest POSITIVE normal value,
    // not the most negative one. lowest() is the most negative.
    std::cout << "double min() (smallest positive) : "
              << std::numeric_limits<double>::min() << '\n';
    std::cout << "double epsilon                   : "
              << std::numeric_limits<double>::epsilon() << '\n';
    std::cout << "digits10 for double              : "
              << std::numeric_limits<double>::digits10 << '\n';      // 15

    // char may be signed or unsigned depending on the platform
    // (signed on x86, unsigned on ARM Linux). Use signed char or
    // unsigned char when you need small numbers.
    std::cout << "char is signed here?             : " << std::boolalpha
              << std::numeric_limits<char>::is_signed << '\n';
}

// ------------------------------------------------------------
// Floating Point
// ------------------------------------------------------------
// float and double are IEEE 754 on every mainstream platform.
// 0.1 has no exact binary representation, so comparisons with
// == are unreliable. Compare with a tolerance instead.

void floating_point()
{
    std::cout << "\n=== Floating Point ===\n";

    double a = 0.1 + 0.2;
    std::cout.precision(17);
    std::cout << "0.1 + 0.2       = " << a << '\n';               // 0.30000000000000004
    std::cout << "(0.1+0.2)==0.3  : " << (a == 0.3) << '\n';      // false

    const double tolerance = 1e-9;
    bool close_enough = (a - 0.3) < tolerance && (0.3 - a) < tolerance;
    std::cout << "within 1e-9     : " << close_enough << '\n';    // true
    std::cout.precision(6);  // back to the default

    // Special values. Dividing a double by zero gives inf (IEEE),
    // but dividing an int by zero is undefined behaviour.
    double inf = std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();
    std::cout << "infinity        : " << inf << '\n';
    std::cout << "NaN == NaN      : " << (nan == nan) << '\n';    // false: NaN is never equal
}

// ------------------------------------------------------------
// bool and std::byte
// ------------------------------------------------------------

void bool_and_byte()
{
    std::cout << "\n=== bool and std::byte ===\n";

    // bool converts to int as 0 or 1. Any non-zero number converts to true.
    bool flag = true;
    int as_int = flag;           // 1 (integral promotion)
    bool from_int = (42 != 0);   // write the test out; "bool b = 42;" also works
    std::cout << "true as int     : " << as_int << '\n';
    std::cout << "42 != 0         : " << from_int << '\n';

    // std::byte (C++17) is raw memory: no arithmetic, only bit operations.
    // Use it instead of unsigned char when you mean "bytes", not "numbers".
    std::byte b{0b1010'0000};
    b |= std::byte{0b0000'0101};
    std::cout << "std::byte value : " << std::to_integer<int>(b) << '\n'; // 165
}

int main()
{
    print_sizes();
    fixed_width_integers();
    numeric_ranges();
    floating_point();
    bool_and_byte();
}
