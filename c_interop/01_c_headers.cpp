// ============================================================
// C Headers, the C Standard Library and Where C++ Differs
// ============================================================
// C++ includes almost all of the C standard library. This file
// covers the <stdio.h> vs <cstdio> question, using printf and
// malloc safely from C++, and the places where valid C is not
// valid C++.
//
// Build and run:
//   cmake --build build --target c_interop_01_c_headers
//   ./build/bin/c_interop_01_c_headers
// ============================================================

// ------------------------------------------------------------
// <stdio.h> vs <cstdio>
// ------------------------------------------------------------
// Every C header xxx.h has a C++ twin cxxx:
//   <stdio.h>  -> <cstdio>     <string.h> -> <cstring>
//   <stdlib.h> -> <cstdlib>    <math.h>   -> <cmath>
//
// <cstdio> guarantees std::printf; it MAY also declare ::printf.
// <stdio.h> guarantees ::printf; it MAY also declare std::printf.
// In practice both usually give you both, which is why code that
// forgets std:: often compiles on one platform and not another.
// Rule: in C++ include <cxxx> and write std::. In a header shared
// with C code you must use <xxx.h>, since C has no <cxxx>.

#include <cinttypes>   // PRId64 and friends
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>

// ------------------------------------------------------------
// printf from C++
// ------------------------------------------------------------
// printf is fast and familiar but not type-safe: the format
// string must match the argument types or behaviour is undefined.
// GCC and Clang check literal format strings (-Wformat, in -Wall);
// MSVC checks some cases with /analyze.
// Modern alternatives: iostreams, std::format (C++20) and
// std::print (C++23). See io/.

void using_printf()
{
    std::cout << "=== printf ===\n";
    std::cout.flush();   // cout and printf share stdout; flush before mixing them

    int count = 3;
    double price = 4.5;
    const char* item = "widgets";
    std::size_t size = sizeof(double);
    std::int64_t big = 9'000'000'000;

    std::printf("%d %s at %.2f each\n", count, item, price);
    std::printf("size_t uses %%zu: %zu\n", size);
    std::printf("int64_t uses PRId64: %" PRId64 "\n", big);
    // std::printf("%d\n", price);   // wrong specifier: undefined behaviour

    // snprintf never writes past the buffer and always terminates it.
    // It returns the length it WANTED to write: if that is >= the
    // buffer size, the output was truncated.
    char buffer[16];
    int needed = std::snprintf(buffer, sizeof buffer, "%s-%d", "order", 123456789);
    std::printf("snprintf wrote \"%s\" (%d chars, truncated: %s)\n", buffer, needed,
                needed >= static_cast<int>(sizeof buffer) ? "yes" : "no");
}

// ------------------------------------------------------------
// malloc/free vs new/delete
// ------------------------------------------------------------
// malloc returns raw bytes: no constructor runs. free releases
// them: no destructor runs. new/delete do both. Never mix them:
// memory from malloc must go to free, from new to delete, from
// new[] to delete[]. Mixing is undefined behaviour.
//
// In C, the void* from malloc converts silently to int*. In C++
// it does not: you need a cast. That is deliberate.

void malloc_and_new()
{
    std::cout << "\n=== malloc/free vs new/delete ===\n";

    // C style:
    int* c_numbers = static_cast<int*>(std::malloc(5 * sizeof(int)));   // C: no cast needed
    if (c_numbers == nullptr) {
        return;   // malloc reports failure with nullptr
    }
    for (int i = 0; i < 5; ++i) c_numbers[i] = i * i;
    std::cout << "malloc'd c_numbers[4] = " << c_numbers[4] << '\n';
    std::free(c_numbers);

    // C++ style. new throws std::bad_alloc on failure instead of returning nullptr.
    int* cpp_numbers = new int[5]{0, 1, 4, 9, 16};
    std::cout << "new[]'d cpp_numbers[4] = " << cpp_numbers[4] << '\n';
    delete[] cpp_numbers;

    // Modern C++ style: no manual release at all (see memory/).
    auto owned = std::make_unique<int[]>(5);
    owned[4] = 16;
    std::cout << "unique_ptr owned[4] = " << owned[4] << '\n';
}

// ------------------------------------------------------------
// memcpy, memset, strcmp
// ------------------------------------------------------------
// Fine for raw bytes and trivially copyable types (see
// 04_structs_and_layout.cpp). Never memcpy or memset a class with
// a constructor, virtual functions or owning members such as
// std::string: GCC warns with -Wclass-memaccess.

void memory_functions()
{
    std::cout << "\n=== memcpy, memset, strcmp ===\n";

    int source[4] = {1, 2, 3, 4};
    int target[4];
    std::memcpy(target, source, sizeof source);
    std::cout << "memcpy'd target[3] = " << target[3] << '\n';

    char flags[8];
    std::memset(flags, '-', sizeof flags - 1);
    flags[7] = '\0';
    std::cout << "memset flags = " << flags << '\n';

    std::cout << "strcmp(\"apple\", \"banana\") < 0: " << std::boolalpha
              << (std::strcmp("apple", "banana") < 0) << '\n';
}

// ------------------------------------------------------------
// Valid C That Is Not Valid C++
// ------------------------------------------------------------
// - int* p = malloc(n);            implicit void* conversion
// - int n = 5; int vla[n];         variable length arrays (C99; GCC
//                                  accepts in C++ as an extension, -Wpedantic warns)
// - struct P p = {.y = 2, .x = 1}; out-of-order designated initialisers
// - char s[3] = "abc";             no room for '\0' (C allows, C++ rejects)
// - int class = 1;                 C++ keywords: class, new, delete,
//                                  template, this, namespace...
// - void f();                      in C (before C23) means "unknown parameters";
//                                  in C++ means "no parameters"
// - restrict                       C99 keyword; C++ has only __restrict extensions
// - _Generic                       C11; C++ uses overloading and templates
// - sizeof('a')                    is sizeof(int) in C, 1 in C++

void c_vs_cpp_differences()
{
    std::cout << "\n=== C vs C++ Differences ===\n";
    std::cout << "sizeof('a') in C++ = " << sizeof('a') << " (it is sizeof(int) in C)\n";
}

int main()
{
    using_printf();
    malloc_and_new();
    memory_functions();
    c_vs_cpp_differences();
}

// ------------------------------------------------------------
// When to Keep C
// ------------------------------------------------------------
// - A library consumed from many languages (Python ctypes, C#
//   P/Invoke, Rust FFI, Go cgo): expose a C API. C has a stable
//   ABI on every platform; C++ does not.
// - Very small embedded targets where the toolchain only supports C.
// - Existing, well-tested C code: wrap it (02_c_library) rather
//   than rewrite it.
// - Kernel and firmware codebases with C-only coding standards.
// A common pattern: implement in C++, export an extern "C" API
// with opaque handles and error codes, and catch every exception
// at the boundary, because an exception must never cross into C.
