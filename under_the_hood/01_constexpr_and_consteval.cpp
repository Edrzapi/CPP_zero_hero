// ============================================================
// Compile-Time Programming: constexpr, consteval, constinit
// ============================================================
// basics/05 introduced the keywords. This file goes further:
// lookup tables built by the compiler, constexpr containers
// (C++20), detecting compile-time evaluation, and turning bad
// input into a COMPILE error.
//
// Build and run:
//   cmake --build build --target under_the_hood_01_constexpr_and_consteval
//   ./build/bin/under_the_hood_01_constexpr_and_consteval
// ============================================================

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// ------------------------------------------------------------
// Lookup Tables Generated at Compile Time
// ------------------------------------------------------------
// The table is computed by the compiler and stored in the binary
// as read-only data. Zero start-up cost, no static init order
// issues. Here: a CRC-32 table, as used in zip and PNG.

constexpr std::array<std::uint32_t, 256> make_crc_table()
{
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c & 1u) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        }
        table[i] = c;
    }
    return table;
}

constexpr auto kCrcTable = make_crc_table();

constexpr std::uint32_t crc32(std::string_view data)
{
    std::uint32_t crc = 0xFFFFFFFFu;
    for (char ch : data) {
        crc = kCrcTable[(crc ^ static_cast<std::uint8_t>(ch)) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

// The standard check value for CRC-32 is 0xCBF43926 for "123456789".
static_assert(crc32("123456789") == 0xCBF43926u);

// ------------------------------------------------------------
// constexpr std::vector and std::string (C++20)
// ------------------------------------------------------------
// Since C++20, new/delete are allowed during constant evaluation,
// as long as everything allocated is freed before it ends
// ("transient allocation"). So vector and string work inside
// constexpr functions; the result must be copied into something
// non-allocating (a number, a std::array) to survive.
// Support: GCC 12+, Clang 15+ (libc++), MSVC 19.29+.

constexpr int sum_of_squares_of_evens(int n)
{
    std::vector<int> values;
    for (int i = 1; i <= n; ++i) {
        if (i % 2 == 0) values.push_back(i * i);
    }
    int total = 0;
    for (int v : values) total += v;
    return total;   // the vector is gone; only the int escapes
}

static_assert(sum_of_squares_of_evens(10) == 4 + 16 + 36 + 64 + 100);

constexpr std::size_t count_words(std::string_view text)
{
    std::string copy{text};   // a constexpr std::string
    std::size_t words = 0;
    bool in_word = false;
    for (char c : copy) {
        if (c == ' ') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++words;
        }
    }
    return words;
}

static_assert(count_words("the quick  brown fox") == 4);

// ------------------------------------------------------------
// Compile Errors from Bad Input
// ------------------------------------------------------------
// Throwing inside a constexpr function is fine at run time (it
// throws), but during constant evaluation it makes the expression
// "not a constant": a compile error at the call site.

constexpr int parse_digit(char c)
{
    if (c < '0' || c > '9') {
        throw std::invalid_argument{"not a digit"};
    }
    return c - '0';
}

constexpr int seven = parse_digit('7');      // fine
// constexpr int oops = parse_digit('x');    // error: throw in a constant expression

// ------------------------------------------------------------
// consteval: Compile Time Only
// ------------------------------------------------------------
// A consteval ("immediate") function can only be called with
// constant arguments. Use it when a run-time call would be a bug,
// for example hashing strings for a switch statement.

consteval std::uint32_t hash(std::string_view text) { return crc32(text); }

std::string_view command_kind(std::string_view command)
{
    switch (crc32(command)) {   // run time for the input...
    case hash("start"): return "lifecycle";   // ...compile time for the labels
    case hash("stop"):  return "lifecycle";
    case hash("status"): return "query";
    default: return "unknown";
    }
}

// ------------------------------------------------------------
// std::is_constant_evaluated (C++20)
// ------------------------------------------------------------
// Lets one constexpr function take a different path at compile
// time (portable, slow) and at run time (fast, maybe intrinsics).
// C++23 adds the cleaner "if consteval { ... }".

constexpr const char* where_am_i()
{
    if (std::is_constant_evaluated()) {
        return "compile time";
    }
    return "run time";
}

// ------------------------------------------------------------
// constinit: No Static Initialisation Order Fiasco
// ------------------------------------------------------------
// Globals in different .cpp files are initialised in an
// unspecified order. If one's constructor reads another that is
// not initialised yet, you read garbage. constinit guarantees
// initialisation at compile time (or a compile error), so it can
// never be "not yet initialised".

constinit int g_max_connections = 16;   // mutable at run time, initialised at compile time

int main()
{
    std::cout << "=== Compile-Time Tables ===\n" << std::hex;
    std::cout << "  crc32(\"123456789\") = 0x" << crc32("123456789") << '\n';
    std::cout << "  crc32(\"hello\")     = 0x" << crc32("hello") << " (computed at run time from the same function)\n";
    std::cout << std::dec;

    std::cout << "\n=== constexpr vector and string ===\n";
    constexpr int squares = sum_of_squares_of_evens(10);
    std::cout << "  sum of squares of evens to 10 = " << squares << '\n';
    std::cout << "  words in a constant phrase    = " << count_words("compile time counts words") << '\n';
    std::cout << "  parse_digit('7') as constant  = " << seven << '\n';

    std::cout << "\n=== consteval in switch labels ===\n";
    for (std::string_view cmd : {"start", "status", "dance"}) {
        std::cout << "  " << cmd << " -> " << command_kind(cmd) << '\n';
    }

    std::cout << "\n=== is_constant_evaluated ===\n";
    constexpr const char* at_compile_time = where_am_i();
    const char* at_run_time = where_am_i();
    std::cout << "  constexpr variable: " << at_compile_time << '\n';
    std::cout << "  ordinary call     : " << at_run_time << '\n';

    std::cout << "\n=== constinit ===\n";
    g_max_connections *= 2;
    std::cout << "  g_max_connections = " << g_max_connections << '\n';
}
