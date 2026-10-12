// ============================================================
// Small Buffer / Short String Optimisation
// ============================================================
// Many standard types keep small values INSIDE the object instead
// of on the heap. std::string does it for short text (SSO), and
// std::function and std::any do it for small callables and values.
// It is invisible to correct code, but explains performance
// differences and why a short string move is a copy of bytes.
//
// This program counts heap allocations by replacing the global
// operator new: a legal technique, and a handy one for tests.
//
// The numbers are implementation-defined. Typical 64-bit values:
//   std::string inline capacity:  libstdc++ 15, MSVC 15, libc++ 22
//   sizeof(std::string):          libstdc++ 32, MSVC 32, libc++ 24
// MSVC DEBUG builds add iterator-debugging state: sizeof is 40 and
// every string also allocates a small "container proxy", so each
// count below is one higher. Run the Release build for clean numbers:
//   cmake --build build --config Release --target under_the_hood_09_small_buffer_optimisation
//
// Build and run:
//   cmake --build build --target under_the_hood_09_small_buffer_optimisation
//   ./build/bin/under_the_hood_09_small_buffer_optimisation
// ============================================================

#include <any>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <new>
#include <string>

// ------------------------------------------------------------
// Counting Allocations
// ------------------------------------------------------------
// Replacing these two (plus the sized delete) is enough for
// ordinary new and delete; new[] forwards to them by default.

namespace {
std::size_t g_allocations = 0;
}

void* operator new(std::size_t size)
{
    ++g_allocations;
    if (void* p = std::malloc(size == 0 ? 1 : size)) {
        return p;
    }
    throw std::bad_alloc{};
}

void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

template <typename F>
std::size_t allocations_during(F&& action)
{
    std::size_t before = g_allocations;
    action();
    return g_allocations - before;
}

// ------------------------------------------------------------
// std::string
// ------------------------------------------------------------

bool stored_inline(const std::string& s)
{
    // Is the character buffer inside the string object itself?
    // std::less gives a total order even for unrelated pointers.
    const char* object_begin = reinterpret_cast<const char*>(&s);
    const char* object_end = object_begin + sizeof(s);
    std::less<const char*> less;
    return !less(s.data(), object_begin) && less(s.data(), object_end);
}

void short_strings()
{
    std::cout << "=== std::string SSO ===\n";
    std::cout << "  sizeof(std::string)          = " << sizeof(std::string) << '\n';
    std::cout << "  capacity of an empty string  = " << std::string{}.capacity() << '\n';

    for (std::size_t length : {5u, 15u, 16u, 22u, 23u, 40u}) {
        std::string text;
        auto count = allocations_during([&] { text = std::string(length, 'x'); });
        std::cout << "  length " << length << (length < 10 ? " " : "") << ": inline = " << std::boolalpha
                  << stored_inline(text) << ", heap allocations = " << count << '\n';
    }
}

// ------------------------------------------------------------
// std::function and std::any
// ------------------------------------------------------------
// A lambda with a small capture fits in std::function's internal
// buffer; a large capture forces a heap allocation. std::any is
// the same for values.

void small_callables()
{
    std::cout << "\n=== std::function and std::any ===\n";

    int small_state = 1;
    std::array<double, 32> big_state{};

    auto small = allocations_during([&] {
        std::function<int()> f = [small_state] { return small_state; };
        (void)f();
    });
    auto big = allocations_during([&] {
        std::function<double()> f = [big_state] { return big_state[0]; };
        (void)f();
    });
    std::cout << "  std::function, 4-byte capture   : " << small << " allocation(s)\n";
    std::cout << "  std::function, 256-byte capture : " << big << " allocation(s)\n";

    auto any_int = allocations_during([] { std::any a = 42; (void)a; });
    auto any_big = allocations_during([] { std::any a = std::array<double, 32>{}; (void)a; });
    std::cout << "  std::any holding an int         : " << any_int << " allocation(s)\n";
    std::cout << "  std::any holding 256 bytes      : " << any_big << " allocation(s)\n";
}

// ------------------------------------------------------------
// Consequences
// ------------------------------------------------------------
// - Moving a short std::string copies its bytes (nothing to steal),
//   so the moved-from string keeps... whatever the implementation
//   leaves; do not rely on it being empty.
// - std::string_view into a short std::string points INSIDE the
//   string object: moving the string invalidates the view even
//   though no heap memory was freed.
// - Benchmarks with short test strings can be misleading about
//   real data with long strings.

int main()
{
    short_strings();
    small_callables();
}
