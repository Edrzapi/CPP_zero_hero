// ============================================================
// Generators with std::generator (C++23)
// ============================================================
// C++23 ONLY. Built when CPPZH_ENABLE_CXX23 is ON.
// Library support: GCC 14+ (libstdc++), MSVC 19.40+ (VS 17.10).
// Not yet in libc++ (Clang on macOS) at the time of writing.
//
// A generator is a coroutine that produces values lazily with
// co_yield, like Python's yield or C#'s yield return. Each value
// is computed only when the loop asks for it.
//
// Build and run:
//   cmake -S . -B build -DCPPZH_ENABLE_CXX23=ON
//   cmake --build build --target iteration_03_generator_cxx23
//   ./build/bin/iteration_03_generator_cxx23
// ============================================================

#include <cstdint>
#include <generator>
#include <iostream>
#include <ranges>

// ------------------------------------------------------------
// An Infinite Sequence
// ------------------------------------------------------------
// The loop never ends inside the generator; the CALLER decides
// how many values to take. The coroutine is suspended at each
// co_yield and resumed on the next ++ of the iterator.

std::generator<std::uint64_t> fibonacci()
{
    std::uint64_t a = 0;
    std::uint64_t b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

// ------------------------------------------------------------
// A Finite Generator
// ------------------------------------------------------------

std::generator<int> count_down(int from)
{
    for (int i = from; i > 0; --i) {
        co_yield i;
    }
}

int main()
{
    std::cout << "=== First 10 Fibonacci Numbers ===\n";
    for (auto n : fibonacci() | std::views::take(10)) {
        std::cout << n << ' ';
    }
    std::cout << '\n';

    std::cout << "\n=== Count Down ===\n";
    for (int i : count_down(5)) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}
