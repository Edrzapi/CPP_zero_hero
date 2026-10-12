// ============================================================
// The Preprocessor and the Build Pipeline
// ============================================================
// C++ inherits C's four-stage build. Knowing which stage an error
// comes from is half of fixing it.
//
//   1. Preprocess  #include, #define, #if: pure text substitution
//   2. Compile     one translation unit (.cpp + everything it
//                  includes) into assembly / machine code
//   3. Assemble    into an object file (.o or .obj)
//   4. Link        object files + libraries into an executable
//
// Each .cpp file is compiled ALONE. The compiler knows nothing
// about other .cpp files; the linker joins them up afterwards.
// "undefined reference" / "unresolved external symbol" are
// LINKER errors: something was declared but never defined.
//
// See each stage yourself (GCC or Clang; MSVC in brackets):
//   g++ -std=c++20 -E 01_preprocessor.cpp > pre.ii    (cl /E)    preprocessed text
//   g++ -std=c++20 -S 01_preprocessor.cpp             (cl /FA)   assembly listing
//   g++ -std=c++20 -c 01_preprocessor.cpp             (cl /c)    object file only
//   g++ 01_preprocessor.o -o pre                      (link)     link step
// The -E output of this small file is tens of thousands of lines,
// because <iostream> pulls in a lot. That is why big C++ projects
// build slowly, and why PIMPL (oop/08_pimpl) and modules exist.
//
// Build and run:
//   cmake --build build --target compilation_01_preprocessor
//   ./build/bin/compilation_01_preprocessor
// ============================================================

// ------------------------------------------------------------
// #include "..." vs #include <...>
// ------------------------------------------------------------
// <...> searches the system and -I include paths.
// "..." searches the current file's folder first, then the same
// places as <...>. Use "" for your own headers, <> for libraries.

#include <cassert>
#include <iostream>
#include <source_location>   // C++20
#include <string_view>

// ------------------------------------------------------------
// Object-Like and Function-Like Macros
// ------------------------------------------------------------
// Macros are text replacement with no knowledge of types, scope
// or precedence. Every argument should be parenthesised, and
// even then arguments with side effects are evaluated twice.

#define BUFFER_SIZE 256
#define BAD_SQUARE(x) x * x            // missing parentheses
#define GOOD_SQUARE(x) ((x) * (x))     // better, still evaluates x twice

// The C++ replacement: a constexpr function. Typed, scoped,
// evaluates its argument once, visible in the debugger.
constexpr int square(int x) { return x * x; }

void macro_pitfalls()
{
    std::cout << "=== Macro Pitfalls ===\n";
    std::cout << "BUFFER_SIZE         = " << BUFFER_SIZE << '\n';
    std::cout << "BAD_SQUARE(1 + 2)   = " << BAD_SQUARE(1 + 2) << "  (1 + 2 * 1 + 2)\n";  // 5
    std::cout << "GOOD_SQUARE(1 + 2)  = " << GOOD_SQUARE(1 + 2) << '\n';                 // 9
    std::cout << "square(1 + 2)       = " << square(1 + 2) << '\n';                      // 9

    // GOOD_SQUARE(i++) expands to ((i++) * (i++)): two unsequenced
    // modifications of i, which is undefined behaviour. square(i++) is fine.
}

// ------------------------------------------------------------
// Stringising (#) and Token Pasting (##)
// ------------------------------------------------------------
// Two things only macros can do: turn an expression into a string
// and glue tokens into a new identifier. That is why assert() and
// test frameworks are still macros.

#define SHOW(expr) std::cout << #expr << " = " << (expr) << '\n'
#define MAKE_GETTER(name) \
    int get_##name() { return name; }

int width = 640;
int height = 480;
MAKE_GETTER(width)    // defines get_width()
MAKE_GETTER(height)   // defines get_height()

void stringise_and_paste()
{
    std::cout << "\n=== # and ## ===\n";
    SHOW(6 * 7);
    SHOW(get_width() * get_height());
}

// ------------------------------------------------------------
// Conditional Compilation
// ------------------------------------------------------------
// Used for platform differences, debug-only code and feature
// detection. The compiler never sees the branches not taken.

void conditional_compilation()
{
    std::cout << "\n=== Conditional Compilation ===\n";

#if defined(_MSC_VER) && !defined(__clang__)
    std::cout << "Compiler: MSVC, _MSC_VER = " << _MSC_VER << '\n';
#elif defined(__clang__)
    std::cout << "Compiler: Clang " << __clang_major__ << '.' << __clang_minor__ << '\n';
#elif defined(__GNUC__)
    std::cout << "Compiler: GCC " << __GNUC__ << '.' << __GNUC_MINOR__ << '\n';
#else
    std::cout << "Compiler: unknown\n";
#endif

#if defined(_WIN32)
    std::cout << "Platform: Windows\n";
#elif defined(__APPLE__)
    std::cout << "Platform: macOS\n";
#elif defined(__linux__)
    std::cout << "Platform: Linux\n";
#endif

    // __cplusplus is the language version: 202002L for C++20.
    // MSVC reports 199711L unless /Zc:__cplusplus is set (CMake sets it here).
    std::cout << "__cplusplus = " << __cplusplus << '\n';

#ifdef NDEBUG
    std::cout << "NDEBUG is defined: assert() is switched off (Release build)\n";
#else
    std::cout << "NDEBUG not defined: assert() is active (Debug build)\n";
#endif

    // #error stops compilation with a message, e.g. to require C++20:
#if __cplusplus < 202002L
#error "C++ Zero to Hero needs C++20"
#endif
}

// ------------------------------------------------------------
// Predefined Macros and std::source_location
// ------------------------------------------------------------
// __FILE__, __LINE__ and __func__ have been the logging tools
// since C. C++20 adds std::source_location, which can be a
// default argument: it records where the CALLER is.

void log(std::string_view message,
         const std::source_location where = std::source_location::current())
{
    std::cout << where.file_name() << ':' << where.line()
              << " [" << where.function_name() << "] " << message << '\n';
}

void predefined_macros()
{
    std::cout << "\n=== __FILE__, __LINE__ and source_location ===\n";
    std::cout << "__LINE__ = " << __LINE__ << ", __func__ = " << __func__ << '\n';
    log("logged with the caller's location");
}

// ------------------------------------------------------------
// assert
// ------------------------------------------------------------
// assert(condition) aborts with a message in debug builds and
// compiles to nothing when NDEBUG is defined. Never put code with
// side effects inside it. static_assert is checked at compile time.

void assertions()
{
    std::cout << "\n=== assert and static_assert ===\n";
    int divisor = 4;
    assert(divisor != 0 && "divisor must not be zero");   // the string is a trick to add a message
    static_assert(sizeof(int) >= 2, "int must be at least 16 bits");
    std::cout << "100 / divisor = " << 100 / divisor << '\n';
}

int main()
{
    macro_pitfalls();
    stringise_and_paste();
    conditional_compilation();
    predefined_macros();
    assertions();
}

// ------------------------------------------------------------
// A Note on C++20 Modules
// ------------------------------------------------------------
// Modules (import std; export module geometry;) replace textual
// inclusion with compiled interfaces: faster builds, no macro
// leakage, no include-order problems. MSVC supports them well,
// GCC and Clang are catching up, and CMake 3.28+ can build them
// with Ninja. This repository sticks to headers because that is
// what almost every existing codebase uses.
