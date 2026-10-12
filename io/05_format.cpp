// ============================================================
// std::format: Type-Safe Formatting (C++20)
// ============================================================
// Python-style format strings for C++: as readable as printf,
// type-safe like iostreams, and the format string is checked at
// COMPILE time. A mismatch such as std::format("{:d}", "text") is
// a compile error, not undefined behaviour.
//
// Library support: GCC 13+, Clang 17+ (libc++), MSVC 19.29+.
// Older compilers: the {fmt} library (fmtlib) is the original and
// has the same syntax under fmt::.
//
// Build and run:
//   cmake --build build --target io_05_format
//   ./build/bin/io_05_format
// ============================================================

#include <format>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Basics
// ------------------------------------------------------------
// {} is replaced by the next argument. {0}, {1} pick by position.
// The format spec goes after a colon: {:spec}.

void basics()
{
    std::cout << "=== Basics ===\n";
    std::string name = "Ada";
    int age = 36;

    std::cout << std::format("  {} is {} years old\n", name, age);
    std::cout << std::format("  {1} comes after {0}; {0} again\n", "first", "second");
    std::cout << std::format("  braces are escaped by doubling: {{}} and {}\n", 42);
}

// ------------------------------------------------------------
// Format Specs: [fill][align][sign][#][0][width][.precision][type]
// ------------------------------------------------------------

void specs()
{
    std::cout << "\n=== Format Specs ===\n";
    double pi = 3.14159265;

    std::cout << std::format("  [{:.2f}]   fixed, 2 decimals\n", pi);
    std::cout << std::format("  [{:10.3f}] width 10, right aligned (numbers default)\n", pi);
    std::cout << std::format("  [{:<10}] left aligned\n", "left");
    std::cout << std::format("  [{:^10}] centred\n", "mid");
    std::cout << std::format("  [{:*>10}] fill with *\n", 42);
    std::cout << std::format("  [{:+}] always show the sign\n", 5);
    std::cout << std::format("  [{:08.2f}] zero padded\n", -pi);
    std::cout << std::format("  [{:x}] [{:#x}] [{:#b}] [{:o}] hex, prefixed hex, binary, octal\n", 255, 255, 5, 8);
    std::cout << std::format("  [{:e}] scientific\n", 123456.789);
    std::cout << std::format("  [{}] [{:d}] bool as text and as number\n", true, true);

    // Width and precision can come from arguments too: {:{}.{}f}
    int width = 12;
    int precision = 4;
    std::cout << std::format("  [{:{}.{}f}] dynamic width and precision\n", pi, width, precision);
}

// ------------------------------------------------------------
// Formatting Your Own Types
// ------------------------------------------------------------
// Specialise std::formatter<T>. The simplest way is to inherit
// from an existing formatter and reuse its parse() so that width,
// alignment etc. still work.

struct Money {
    long long pence;
};

template <>
struct std::formatter<Money> : std::formatter<std::string> {
    auto format(const Money& m, std::format_context& ctx) const
    {
        auto text = std::format("GBP {}.{:02}", m.pence / 100, m.pence % 100);
        return std::formatter<std::string>::format(text, ctx);
    }
};

void custom_types()
{
    std::cout << "\n=== Custom Formatter ===\n";
    Money price{1999};
    std::cout << std::format("  price: {}\n", price);
    std::cout << std::format("  [{:>14}] (width and alignment still work)\n", price);
}

// ------------------------------------------------------------
// Writing Into Existing Buffers
// ------------------------------------------------------------
// format_to writes through an output iterator (no temporary
// string). formatted_size tells you how long the result will be.

void format_to_demo()
{
    std::cout << "\n=== format_to and formatted_size ===\n";
    std::vector<char> buffer;
    std::format_to(std::back_inserter(buffer), "{}-{}-{}", 2026, 10, 12);
    std::cout << "  buffer holds: " << std::string(buffer.begin(), buffer.end()) << '\n';
    std::cout << "  formatted_size(\"{:>20}\", 1) = " << std::formatted_size("{:>20}", 1) << '\n';
}

// ------------------------------------------------------------
// Run-Time Format Strings
// ------------------------------------------------------------
// The compile-time check needs a literal. For a format string
// that is only known at run time (loaded from a file, translated
// text) use std::vformat with std::make_format_args, which throws
// std::format_error on a bad string instead of failing to compile.

void runtime_format()
{
    std::cout << "\n=== Run-Time Format Strings ===\n";
    std::string pattern = "  {} scored {}\n";
    std::string who = "Grace";
    int score = 97;
    std::cout << std::vformat(pattern, std::make_format_args(who, score));

    try {
        std::string broken = "{:d}";
        std::string text = "not a number";
        std::cout << std::vformat(broken, std::make_format_args(text));
    } catch (const std::format_error& e) {
        std::cout << "  format_error: " << e.what() << '\n';
    }
}

int main()
{
    basics();
    specs();
    custom_types();
    format_to_demo();
    runtime_format();
}
