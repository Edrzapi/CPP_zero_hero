// ============================================================
// Standard Attributes
// ============================================================
// Attributes, written [[name]], give the compiler extra
// information: things to warn about, optimisation hints, or
// layout permissions. Unknown attributes are ignored (with a
// warning), so they are safe to use across compilers.
//
// Build and run:
//   cmake --build build --target under_the_hood_05_attributes
//   ./build/bin/under_the_hood_05_attributes
// ============================================================

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ------------------------------------------------------------
// [[nodiscard]] (C++17, message in C++20)
// ------------------------------------------------------------
// Warn when a caller ignores the return value. Put it on
// functions whose result IS the point (error codes, factory
// functions, "empty()" style queries that look like commands).
// On a TYPE it applies to every function returning that type.

[[nodiscard("check whether the save worked")]] bool save(const std::string& path)
{
    return !path.empty();
}

struct [[nodiscard]] ErrorCode {
    int value;
};

ErrorCode connect(int port) { return ErrorCode{port > 0 ? 0 : 1}; }

// save("x");        // warning: ignoring return value ... "check whether the save worked"
// connect(80);      // warning: ignoring a [[nodiscard]] type
// (void)connect(80);   // an explicit cast says "I really mean to ignore it"

// ------------------------------------------------------------
// [[maybe_unused]] and [[deprecated]]
// ------------------------------------------------------------

[[deprecated("use area_v2, which handles negative sizes")]]
int area_v1(int w, int h) { return w * h; }

int area_v2(int w, int h) { return (w < 0 || h < 0) ? 0 : w * h; }
// area_v1(2, 3);    // warning: 'area_v1' is deprecated: use area_v2 ...

void maybe_unused_demo([[maybe_unused]] int debug_level)
{
    [[maybe_unused]] const bool verbose = true;   // e.g. only read inside an assert
}

// ------------------------------------------------------------
// [[noreturn]]
// ------------------------------------------------------------
// The function never returns normally (it throws, exits or loops
// forever). The compiler stops warning about missing returns after
// a call to it, and can optimise the calling path.

[[noreturn]] void fail(const std::string& message)
{
    throw std::runtime_error{message};
}

int checked_index(const std::vector<int>& v, std::size_t i)
{
    if (i < v.size()) {
        return v[i];
    }
    fail("index out of range");   // no "missing return" warning after this
}

// ------------------------------------------------------------
// [[likely]] and [[unlikely]] (C++20)
// ------------------------------------------------------------
// Hints for branch layout: keep the hot path straight-line. Only
// worth adding when profiling shows a hot branch the compiler got
// wrong; wrong hints make code slower.

int classify(int value)
{
    if (value >= 0) [[likely]] {
        return 1;
    } else [[unlikely]] {
        return -1;
    }
}

// ------------------------------------------------------------
// [[no_unique_address]] (C++20)
// ------------------------------------------------------------
// An empty member (a stateless allocator, comparator or policy)
// normally still takes at least one byte, plus padding. This
// attribute lets it share an address with another member, so it
// costs nothing.
// MSVC accepts the standard spelling but ignores it for ABI
// reasons; it honours [[msvc::no_unique_address]] instead.

#if defined(_MSC_VER)
#define CPPZH_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define CPPZH_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

struct EmptyPolicy {
    static int apply(int x) { return x; }
};

struct Plain {
    int value;
    EmptyPolicy policy;
};

struct Compressed {
    int value;
    CPPZH_NO_UNIQUE_ADDRESS EmptyPolicy policy;
};

// ------------------------------------------------------------
// Others Worth Knowing
// ------------------------------------------------------------
// [[fallthrough]]   intended fall-through in a switch (control_flow/02_switch)
// [[assume(expr)]]  C++23: the optimiser may assume expr is true;
//                   undefined behaviour if it is not
// [[carries_dependency]]  rarely used memory-model detail
// Vendor attributes live in namespaces: [[gnu::always_inline]],
// [[msvc::forceinline]], [[clang::lifetimebound]] (catches some
// dangling string_view returns).

int main()
{
    std::cout << "=== [[nodiscard]] ===\n";
    if (save("report.txt")) {
        std::cout << "  save result checked\n";
    }
    if (auto ec = connect(80); ec.value == 0) {
        std::cout << "  connected\n";
    }

    std::cout << "\n=== [[deprecated]] and [[maybe_unused]] ===\n";
    std::cout << "  area_v2(2, 3) = " << area_v2(2, 3) << '\n';
    maybe_unused_demo(3);

    std::cout << "\n=== [[noreturn]] ===\n";
    try {
        std::cout << "  checked_index(..., 1) = " << checked_index({1, 2, 3}, 1) << '\n';
        std::cout << "  checked_index(..., 9) = " << checked_index({1, 2, 3}, 9) << '\n';
    } catch (const std::runtime_error& e) {
        std::cout << "  " << e.what() << '\n';
    }

    std::cout << "\n=== [[likely]] ===\n";
    std::cout << "  classify(5) = " << classify(5) << ", classify(-5) = " << classify(-5) << '\n';

    std::cout << "\n=== [[no_unique_address]] ===\n";
    std::cout << "  sizeof(Plain)      = " << sizeof(Plain) << '\n';        // 8: int + 1-byte member + padding
    std::cout << "  sizeof(Compressed) = " << sizeof(Compressed) << '\n';   // 4: the empty member is free
    std::cout << "  policy still works: " << Compressed{5, {}}.policy.apply(7) << '\n';
}
