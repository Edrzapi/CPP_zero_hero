// ============================================================
// References and Pointers
// ============================================================
// Pointers come from C: a variable that holds an address.
// References are C++: another name for an existing object.
// Java and C# references behave more like C++ pointers that
// cannot do arithmetic and are garbage collected.
//
// Build and run:
//   cmake --build build --target basics_06_references_and_pointers
//   ./build/bin/basics_06_references_and_pointers
// ============================================================

#include <cstddef>
#include <iostream>
#include <memory>    // std::addressof

// ------------------------------------------------------------
// References
// ------------------------------------------------------------
// - must be initialised, and always refer to the same object
// - cannot be null (in a valid program)
// - no special syntax to use: the reference IS the object

void references()
{
    std::cout << "=== References ===\n";

    int score = 10;
    int& alias = score;     // alias is another name for score

    alias += 5;
    std::cout << "score = " << score << '\n';                         // 15
    std::cout << "same address? " << std::boolalpha
              << (&alias == &score) << '\n';                           // true

    // int& unbound;        // error: references must be initialised

    int other = 99;
    alias = other;          // does NOT re-bind: copies 99 into score
    std::cout << "after alias = other, score = " << score << '\n';    // 99

    // const& can bind to a temporary and extends its lifetime.
    const int& temp_ref = 6 * 7;
    std::cout << "const& to a temporary: " << temp_ref << '\n';
}

// ------------------------------------------------------------
// Pointers
// ------------------------------------------------------------
// - hold an address, may be null, may be re-pointed
// - & takes an address, * dereferences
// - nullptr (C++11) replaces NULL and 0. NULL is a macro for an
//   integer, which causes overload surprises; nullptr has its own
//   type, std::nullptr_t.

void pointers()
{
    std::cout << "\n=== Pointers ===\n";

    int value = 42;
    int* ptr = &value;

    std::cout << "value  = " << value << '\n';
    std::cout << "*ptr   = " << *ptr << '\n';
    std::cout << "ptr    = " << ptr << " (an address, differs each run with ASLR)\n";

    *ptr = 100;
    std::cout << "after *ptr = 100, value = " << value << '\n';

    int* nothing = nullptr;
    if (nothing == nullptr) {
        std::cout << "nothing is null; dereferencing it would be undefined behaviour\n";
    }

    // Pointer to pointer: common in C APIs that return a pointer through a parameter.
    int** pp = &ptr;
    std::cout << "**pp   = " << **pp << '\n';
}

// ------------------------------------------------------------
// Pointer Arithmetic
// ------------------------------------------------------------
// Adding n to a T* moves n * sizeof(T) bytes. Only valid within
// one array (or one past the end). Going further is undefined.

void pointer_arithmetic()
{
    std::cout << "\n=== Pointer Arithmetic ===\n";

    int numbers[] = {10, 20, 30, 40};
    int* first = numbers;                 // array decays to a pointer to its first element
    int* third = first + 2;

    std::cout << "*third          = " << *third << '\n';          // 30
    std::cout << "third - first   = " << (third - first) << '\n'; // 2 (a std::ptrdiff_t)
    std::cout << "numbers[1]      = " << numbers[1] << '\n';
    std::cout << "*(numbers + 1)  = " << *(numbers + 1) << '\n';  // a[i] is *(a + i)

    // One past the end is a valid pointer to compare against, never to dereference.
    int* end = numbers + 4;
    for (int* p = numbers; p != end; ++p) {
        std::cout << *p << ' ';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Pass by Pointer vs Pass by Reference
// ------------------------------------------------------------
// C has only pass by value, so "out parameters" are pointers.
// C++ adds references. Rule of thumb:
//   - use a reference when the argument must exist
//   - use a pointer when "no object" (nullptr) is a valid answer

void increment_c_style(int* target)
{
    if (target != nullptr) {
        ++*target;
    }
}

void increment_cpp_style(int& target) { ++target; }

void passing()
{
    std::cout << "\n=== Passing Pointers and References ===\n";

    int n = 1;
    increment_c_style(&n);
    increment_cpp_style(n);
    increment_c_style(nullptr);    // allowed and handled
    std::cout << "n = " << n << '\n';   // 3
}

// ------------------------------------------------------------
// void* and std::addressof
// ------------------------------------------------------------
// void* is C's "pointer to anything". It throws away the type,
// so you must cast back to the right type yourself. In C++ use
// templates instead (see templates/).
// std::addressof gets the real address even if a class has
// overloaded unary operator&.

void void_pointers()
{
    std::cout << "\n=== void* ===\n";

    double d = 2.5;
    void* anything = &d;
    double* back = static_cast<double*>(anything);   // must cast back to the ORIGINAL type
    std::cout << "*back = " << *back << '\n';

    std::cout << "addressof matches &: " << (std::addressof(d) == &d) << '\n';
}

int main()
{
    references();
    pointers();
    pointer_arithmetic();
    passing();
    void_pointers();
}
