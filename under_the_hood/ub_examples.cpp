// ============================================================
// Undefined Behaviour Lab (Opt-In)
// ============================================================
// NOT built by default. Every case below is a real bug that
// compiles, often runs "fine", and is undefined behaviour: the
// standard places no requirements at all on what happens. The
// sanitizers turn each one into an immediate, readable report.
//
// Build (GCC or Clang, Linux / WSL / macOS):
//   cmake -S . -B build-ub -DCPPZH_BUILD_UB_DEMOS=ON -DCPPZH_SANITIZE=ON
//   cmake --build build-ub --target under_the_hood_ub_examples
//   ./build-ub/bin/under_the_hood_ub_examples                 (lists the cases)
//   ./build-ub/bin/under_the_hood_ub_examples use-after-free
//
// Build (MSVC): the same commands. CPPZH_SANITIZE adds
// /fsanitize=address, so the memory cases are caught; MSVC has no
// UBSan, so signed-overflow and shift run silently there. The ASan
// runtime DLL must be on PATH: run from a Developer PowerShell, or
// the VS Code terminal after CMake Tools has loaded the VS kit.
//
// Case                caught by
// ------------------  ----------------------------------------------
// signed-overflow     UBSan: "signed integer overflow"
// shift               UBSan: "shift exponent 32 is too large"
// heap-overflow       ASan:  "heap-buffer-overflow"
// stack-overflow      ASan:  "stack-buffer-overflow"
// use-after-free      ASan:  "heap-use-after-free"
// double-free         ASan:  "attempting double-free"
// dangling-view       ASan:  "heap-use-after-free" (string_view into a dead string)
// null-deref          UBSan: "load of null pointer"; otherwise a crash
// uninitialised       MSan only (Clang, Linux). MSVC Debug builds WITHOUT ASan
//                     stop it with "Run-Time Check Failure #3" (/RTC1, exit code 3)
// leak                LeakSanitizer (part of ASan on Linux): "detected memory leaks".
//                     MSVC's ASan does not report leaks.
//
// Verified with MSVC 19.44 /fsanitize=address: heap-overflow, stack-overflow,
// use-after-free, double-free, dangling-view and null-deref are all reported;
// signed-overflow, shift, uninitialised and leak run silently.
// ============================================================

#include <climits>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// The compiler must not be able to see the values involved, or it
// would fold the bug away at compile time (or refuse to compile).
// "one" comes from argc, which only exists at run time.

int signed_overflow(int one)
{
    int big = INT_MAX;
    return big + one;                       // UB: INT_MAX + 1
}

int bad_shift(int one)
{
    int amount = 31 + one;                  // 32
    return 1 << amount;                     // UB: shift >= width of int
}

int heap_overflow(int one)
{
    std::vector<int> v(4);
    int* p = v.data();                      // raw pointer: bypasses any checked access
    return p[3 + one];                      // UB: reads element 4 of 4
}

int stack_overflow(int one)
{
    int local[4] = {1, 2, 3, 4};
    return local[3 + one];                  // UB: one past the end of a stack array
}

int use_after_free(int one)
{
    int* p = new int{41};
    delete p;
    return *p + one;                        // UB: the memory was freed
}

void double_free()
{
    int* p = new int{1};
    delete p;
    delete p;                               // UB: freed twice
}

std::size_t dangling_view()
{
    std::string_view view;
    {
        std::string text(40, 'x');          // long enough to live on the heap (not SSO)
        view = text;
    }                                       // text destroyed here
    return std::strlen(view.data());        // UB: reads freed memory
}

int null_deref(int one)
{
    int value = 7;
    int* p = (one == 1) ? nullptr : &value;
    return *p;                              // UB: dereferencing nullptr
}

int uninitialised(int one)
{
    int x;
    if (one == 2) {
        x = 5;
    }
    return x;                               // UB: x has no value when one != 2
}

void leak()
{
    int* lost = new int[1000];
    lost[0] = 1;                            // never deleted
}

int main(int argc, char* argv[])
{
    const int one = argc > 0 ? 1 : 0;       // 1 at run time, unknown to the compiler

    if (argc < 2) {
        std::cout << "=== Undefined Behaviour Lab ===\n"
                  << "  usage: under_the_hood_ub_examples <case>\n"
                  << "  cases: signed-overflow shift heap-overflow stack-overflow use-after-free\n"
                  << "         double-free dangling-view null-deref uninitialised leak\n";
        return 0;
    }

    const std::string which = argv[1];
    std::cout << "=== Running: " << which << " ===\n" << std::flush;

    if (which == "signed-overflow") std::cout << "  result: " << signed_overflow(one) << '\n';
    else if (which == "shift") std::cout << "  result: " << bad_shift(one) << '\n';
    else if (which == "heap-overflow") std::cout << "  result: " << heap_overflow(one) << '\n';
    else if (which == "stack-overflow") std::cout << "  result: " << stack_overflow(one) << '\n';
    else if (which == "use-after-free") std::cout << "  result: " << use_after_free(one) << '\n';
    else if (which == "double-free") double_free();
    else if (which == "dangling-view") std::cout << "  result: " << dangling_view() << '\n';
    else if (which == "null-deref") std::cout << "  result: " << null_deref(one) << '\n';
    else if (which == "uninitialised") std::cout << "  result: " << uninitialised(one) << '\n';
    else if (which == "leak") leak();
    else {
        std::cout << "  unknown case\n";
        return 1;
    }

    std::cout << "  ...no report? Without sanitizers, UB often looks like it worked.\n";
}
