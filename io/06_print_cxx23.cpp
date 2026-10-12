// ============================================================
// std::print and std::println (C++23)
// ============================================================
// C++23 ONLY. Built when CPPZH_ENABLE_CXX23 is ON.
// Library support: GCC 14+, Clang 18+ (libc++), MSVC 19.37+.
//
// std::print = std::format + write to stdout, without building a
// temporary std::string and without iostream state. It also
// writes Unicode correctly to a Windows console. This is the
// modern replacement for both printf and std::cout << chains.
//
// Build and run:
//   cmake -S . -B build -DCPPZH_ENABLE_CXX23=ON
//   cmake --build build --target io_06_print_cxx23
//   ./build/bin/io_06_print_cxx23
// ============================================================

#include <cstdio>
#include <print>
#include <string>

int main()
{
    std::println("=== std::print ===");

    std::string language = "C++";
    int year = 2023;

    std::print("  {} gained std::print in {}.\n", language, year);
    std::println("  println adds the newline: {:>8.3f}", 3.14159);

    // Write to another C stream, such as stderr.
    std::println(stderr, "  (this line went to stderr)");

    // Same format specs and custom formatters as std::format (05_format.cpp).
    std::println("  [{:^12}]", "centred");
}
