// ============================================================
// Calling a C Library from C++ and Wrapping It in RAII
// ============================================================
// cstack.c is compiled by the C compiler; this file by the C++
// compiler. extern "C" in cstack.h makes the names match at link
// time. Then a small C++ class turns the C API's manual
// create/destroy and status codes into automatic clean-up and
// exceptions.
//
// Build by hand:
//   gcc -std=c11 -c cstack.c -o cstack.o
//   g++ -std=c++20 main.cpp cstack.o -o c_library
//
// Build and run:
//   cmake --build build --target c_interop_02_c_library
//   ./build/bin/c_interop_02_c_library
// ============================================================

#include "cstack.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

// ------------------------------------------------------------
// Using the C API Directly
// ------------------------------------------------------------
// Every call returns a status that must be checked, and every
// exit path must call cstack_destroy. Forget one early return
// and the memory leaks.

void raw_c_usage()
{
    std::cout << "=== Using the C API Directly ===\n";

    cstack* stack = cstack_create(2);
    if (stack == nullptr) {
        std::cout << "create failed\n";
        return;
    }

    for (int i = 1; i <= 5; ++i) {
        cstack_status status = cstack_push(stack, i * 10);
        if (status != CSTACK_OK) {
            std::cout << "push failed: " << cstack_status_message(status) << '\n';
            cstack_destroy(stack);   // must remember on EVERY early exit
            return;
        }
    }
    std::cout << "size after 5 pushes (capacity grew from 2): " << cstack_size(stack) << '\n';

    int value = 0;
    while (cstack_pop(stack, &value) == CSTACK_OK) {   // out-parameter
        std::cout << value << ' ';
    }
    std::cout << '\n';

    cstack_status status = cstack_pop(stack, &value);
    std::cout << "pop on empty: " << cstack_status_message(status) << '\n';

    cstack_destroy(stack);
}

// ------------------------------------------------------------
// The RAII Wrapper
// ------------------------------------------------------------
// - The constructor acquires the resource; the destructor
//   releases it. No path can forget.
// - std::unique_ptr with a custom deleter does the releasing, so
//   the class needs no hand-written destructor and is
//   automatically move-only (copying a handle would mean a double
//   destroy). This is the rule of zero (see memory/).
// - Status codes become exceptions or std::optional, whichever
//   suits the caller.
// - The C type stays incomplete here: unique_ptr with a deleter
//   never needs to know what is inside a cstack.

class IntStack {
public:
    explicit IntStack(std::size_t initial_capacity = 4)
        : handle_{cstack_create(initial_capacity)}
    {
        if (!handle_) {
            throw std::bad_alloc{};
        }
    }

    void push(int value)
    {
        check(cstack_push(handle_.get(), value));
    }

    // Empty stack is an expected outcome, not an error: optional.
    std::optional<int> pop()
    {
        int value = 0;
        cstack_status status = cstack_pop(handle_.get(), &value);
        if (status == CSTACK_EMPTY) {
            return std::nullopt;
        }
        check(status);
        return value;
    }

    std::size_t size() const { return cstack_size(handle_.get()); }

    // Escape hatch for C functions that need the raw handle.
    cstack* native_handle() const { return handle_.get(); }

private:
    struct Destroyer {
        void operator()(cstack* stack) const noexcept { cstack_destroy(stack); }
    };

    static void check(cstack_status status)
    {
        if (status != CSTACK_OK) {
            throw std::runtime_error{std::string{"cstack: "} + cstack_status_message(status)};
        }
    }

    std::unique_ptr<cstack, Destroyer> handle_;
};

void raii_usage()
{
    std::cout << "\n=== Using the RAII Wrapper ===\n";

    IntStack stack{2};
    for (int i = 1; i <= 5; ++i) {
        stack.push(i * 10);
    }
    std::cout << "size: " << stack.size() << '\n';

    while (auto value = stack.pop()) {
        std::cout << *value << ' ';
    }
    std::cout << '\n';

    // Move-only: ownership transfers, the source becomes empty.
    IntStack moved = std::move(stack);
    moved.push(99);
    std::cout << "moved stack size: " << moved.size() << '\n';
    // IntStack copy = moved;   // error: unique_ptr makes the class non-copyable

    // An exception thrown here would still destroy "moved" during unwinding.
}   // cstack_destroy runs here, automatically

int main()
{
    raw_c_usage();
    raii_usage();
}

// ------------------------------------------------------------
// Rules at the C/C++ Boundary
// ------------------------------------------------------------
// - Never let a C++ exception propagate into C code (for example
//   out of a callback you passed to a C library): C cannot unwind
//   it. Catch everything at the boundary and return a status code.
// - Only pass C-compatible types across: fundamental types,
//   pointers, and standard-layout structs (04_structs_and_layout).
// - Memory allocated by the C library must be freed by the C
//   library (cstack_destroy), never with delete.
