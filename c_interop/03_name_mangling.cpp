// ============================================================
// Name Mangling and the C ABI
// ============================================================
// The linker only knows symbol NAMES, not types. To support
// overloading, namespaces and classes, a C++ compiler encodes the
// full signature into each symbol name ("mangling" or
// "decoration"). extern "C" switches that off for a function, so
// it gets the plain C name every other language can find.
//
// There are two main schemes, and they are not compatible:
//   Itanium C++ ABI  GCC and Clang on Linux, macOS, MinGW
//   MSVC             Visual Studio and clang-cl on Windows
// This is why a C++ library built with MSVC cannot be linked into
// a MinGW program, while a C API built with either one can.
//
// See the real symbols after building:
//   Linux / WSL / MinGW:
//     nm build/bin/c_interop_03_name_mangling | grep add
//     nm -C build/bin/c_interop_03_name_mangling | grep add   (-C demangles)
//     echo _ZN5maths3addEii | c++filt                          (demangle one name)
//   macOS: same commands; every symbol gets an extra leading _
//   Windows (Developer PowerShell, Debug build):
//     dumpbin /symbols build\c_interop\CMakeFiles\c_interop_03_name_mangling.dir\Debug\03_name_mangling.cpp.obj | findstr add
//     undname "?add@@YAHHH@Z"                                  (demangle one name)
//   (with the Ninja generator the .obj sits in ...\c_interop_03_name_mangling.dir\ without Debug\)
//
// Build and run:
//   cmake --build build --target c_interop_03_name_mangling
//   ./build/bin/c_interop_03_name_mangling
// ============================================================

#include <iostream>

#if defined(__GNUC__) && !defined(_MSC_VER)
#include <cstdlib>
#include <cxxabi.h>   // abi::__cxa_demangle: the Itanium ABI's run-time demangler
#endif

// ------------------------------------------------------------
// Recording the Decorated Name
// ------------------------------------------------------------
// MSVC provides __FUNCDNAME__: the decorated name of the current
// function, as a string literal. GCC and Clang have no equivalent
// macro, so on those compilers the expected Itanium names are
// demangled at run time instead (see itanium_names below).
// Each function stores its own name in g_last_name when called.

#if defined(_MSC_VER)
#define DECORATED_NAME __FUNCDNAME__
#else
#define DECORATED_NAME "(not available on this compiler: use nm)"
#endif

const char* g_last_name = "";

// ------------------------------------------------------------
// Functions to Mangle
// ------------------------------------------------------------
//                                    Itanium                 MSVC (x64)
// int add(int, int)                  _Z3addii                ?add@@YAHHH@Z
// double add(double, double)         _Z3adddd                ?add@@YANNN@Z
// int maths::add(int, int)           _ZN5maths3addEii        ?add@maths@@YAHHH@Z
// int Calculator::add(int, int) const _ZNK10Calculator3addEii ?add@Calculator@@QEBAHHH@Z
// extern "C" int c_add(int, int)     c_add                   c_add
//
// Reading _ZN5maths3addEii: _Z (mangled) N...E (nested name)
// 5maths (5-char name) 3add (3-char name) ii (int, int).
// Reading ?add@@YAHHH@Z: Y (free function) A (__cdecl) H (returns
// int) HH (int, int) @Z (end). Itanium does not encode the return
// type of ordinary functions; neither scheme lets overloads differ
// by return type alone, because a call does not say which it wants.

int add(int a, int b)
{
    g_last_name = DECORATED_NAME;
    return a + b;
}

double add(double a, double b)
{
    g_last_name = DECORATED_NAME;
    return a + b;
}

namespace maths {
int add(int a, int b)
{
    g_last_name = DECORATED_NAME;
    return a + b;
}
}  // namespace maths

struct Calculator {
    int add(int a, int b) const
    {
        g_last_name = DECORATED_NAME;
        return a + b;
    }
};

// extern "C": no mangling, and therefore no overloading.
// extern "C" double c_add(double, double);   // error: a second c_add with C linkage
extern "C" int c_add(int a, int b)
{
    g_last_name = DECORATED_NAME;
    return a + b;
}

void show_decorated_names()
{
    std::cout << "=== Decorated Names Recorded by the Compiler ===\n";
    add(1, 2);
    std::cout << "add(int, int)              -> " << g_last_name << '\n';
    add(1.0, 2.0);
    std::cout << "add(double, double)        -> " << g_last_name << '\n';
    maths::add(1, 2);
    std::cout << "maths::add(int, int)       -> " << g_last_name << '\n';
    Calculator{}.add(1, 2);
    std::cout << "Calculator::add(...) const -> " << g_last_name << '\n';
    c_add(1, 2);
    std::cout << "extern \"C\" c_add          -> " << g_last_name << '\n';
}


// ------------------------------------------------------------
// Demangling Itanium Names (GCC and Clang only)
// ------------------------------------------------------------

void itanium_names()
{
    std::cout << "\n=== Itanium Names Demangled ===\n";
#if defined(__GNUC__) && !defined(_MSC_VER)
    const char* names[] = {
        "_Z3addii",
        "_Z3adddd",
        "_ZN5maths3addEii",
        "_ZNK10Calculator3addEii",
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1Ev",
    };
    for (const char* mangled : names) {
        int status = 0;
        char* readable = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
        std::cout << mangled << "\n    -> " << (status == 0 ? readable : "(not demangled)") << '\n';
        std::free(readable);   // __cxa_demangle allocates with malloc
    }
#else
    std::cout << "MSVC build: run  undname \"?add@maths@@YAHHH@Z\"  in a Developer prompt instead.\n";
#endif
}

int main()
{
    show_decorated_names();
    itanium_names();
}
