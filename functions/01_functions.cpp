// ============================================================
// Functions: Parameters, Overloading, Defaults and inline
// ============================================================
// How arguments are passed (by value, by reference, by const
// reference, by pointer), how the compiler picks between
// overloads, and what inline really means.
//
// Build and run:
//   cmake --build build --target functions_01_functions
//   ./build/bin/functions_01_functions
// ============================================================

#include <iostream>
#include <string>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// Declaration vs Definition
// ------------------------------------------------------------
// A declaration (prototype) tells the compiler a function exists.
// A definition provides the body. A function must be declared
// before it is called; the definition can come later or live in
// another file (see compilation/).

int add(int a, int b);   // declaration only

// ------------------------------------------------------------
// Passing Arguments
// ------------------------------------------------------------
// By value:           the function gets its own copy
// By reference (T&):  the function works on the caller's object
// By const ref:       no copy, no modification: the default for
//                     anything bigger than a couple of words
// By pointer (T*):    C style; can be nullptr
//
// Java passes object references by value; C++ copies the WHOLE
// object when passing by value. Passing a std::vector by value
// copies every element.

void by_value(int n)                              // caller unaffected
{
    n = 100;
    std::cout << "inside by_value     : " << n << '\n';
}
void by_reference(int& n) { n = 100; }            // caller's variable changes
std::size_t by_const_ref(const std::vector<int>& v) { return v.size(); }  // no copy
void by_pointer(int* n) { if (n) *n = 200; }

void parameter_passing()
{
    std::cout << "=== Passing Arguments ===\n";

    int x = 1;
    by_value(x);
    std::cout << "after by_value      : " << x << '\n';   // 1
    by_reference(x);
    std::cout << "after by_reference  : " << x << '\n';   // 100
    by_pointer(&x);
    std::cout << "after by_pointer    : " << x << '\n';   // 200

    std::vector<int> big(1'000'000, 7);
    std::cout << "by_const_ref size   : " << by_const_ref(big) << " (no copy made)\n";
}

// ------------------------------------------------------------
// Sink Parameters: Pass by Value and Move
// ------------------------------------------------------------
// When a function KEEPS its argument (stores it in a member, for
// example) take it by value and std::move it into place. Callers
// with a temporary pay for one move, callers with a named object
// pay for one copy. See memory/05_move_semantics.cpp.

class Person {
public:
    explicit Person(std::string name) : name_{std::move(name)} {}
    const std::string& name() const { return name_; }   // return by const ref: no copy

private:
    std::string name_;
};

// ------------------------------------------------------------
// Returning Values
// ------------------------------------------------------------
// Return by value. The copy is elided (guaranteed since C++17 for
// temporaries), so returning a big object is cheap.
// Several results: return a struct (clearer than out-parameters).
// NEVER return a reference or pointer to a local variable: it is
// destroyed when the function returns.

struct MinMax {
    int min;
    int max;
};

MinMax min_max(const std::vector<int>& values)
{
    MinMax result{values.front(), values.front()};
    for (int v : values) {
        if (v < result.min) result.min = v;
        if (v > result.max) result.max = v;
    }
    return result;
}

// int& broken() { int local = 5; return local; }   // dangling reference: UB (and a warning)

// ------------------------------------------------------------
// Overloading
// ------------------------------------------------------------
// Same name, different parameter lists. The return type alone
// cannot distinguish overloads. The compiler picks the best match:
// exact match, then promotion (char to int, float to double), then
// standard conversion (int to double), then user conversions.
// Ambiguity is a compile error.

std::string describe(int) { return "int"; }
std::string describe(double) { return "double"; }
std::string describe(const std::string&) { return "std::string"; }
std::string describe(const char*) { return "const char*"; }

void overloading()
{
    std::cout << "\n=== Overloading ===\n";
    std::cout << "describe(1)     -> " << describe(1) << '\n';
    std::cout << "describe(1.0)   -> " << describe(1.0) << '\n';
    std::cout << "describe(1.0f)  -> " << describe(1.0f) << " (float promoted)\n";
    std::cout << "describe('a')   -> " << describe('a') << " (char promoted)\n";
    std::cout << "describe(\"hi\")  -> " << describe("hi") << " (exact match beats conversion)\n";
    std::cout << "describe(std::string{\"hi\"}) -> " << describe(std::string{"hi"}) << '\n';
    // describe(1L);   // error: long -> int and long -> double are equally good: ambiguous
}

// ------------------------------------------------------------
// Default Arguments
// ------------------------------------------------------------
// Defaults go on the DECLARATION (usually in the header), only
// for trailing parameters, and are filled in at the call site.

std::string greet(const std::string& name, const std::string& greeting = "Hello",
                  char punctuation = '!')
{
    return greeting + ", " + name + punctuation;
}

// ------------------------------------------------------------
// inline
// ------------------------------------------------------------
// In modern C++, inline does NOT mean "please inline this call";
// optimisers decide that themselves. It means "this definition
// may appear in several translation units", which is what lets
// you define functions in headers. Member functions defined
// inside a class body are implicitly inline.
// See compilation/02_multi_file for the One Definition Rule.

inline int twice(int n) { return 2 * n; }

// ------------------------------------------------------------
// Recursion
// ------------------------------------------------------------
// Supported, but there is no guaranteed tail-call optimisation
// and the stack is typically 1 MB (Windows) to 8 MB (Linux).

unsigned long long factorial(unsigned n)
{
    return n <= 1 ? 1ULL : n * factorial(n - 1);
}

// ------------------------------------------------------------
// [[nodiscard]] and noexcept on Functions
// ------------------------------------------------------------
// [[nodiscard]]: warn if the caller ignores the return value.
// noexcept: promise the function does not throw (see exceptions/).

[[nodiscard]] bool save_file(const std::string& name) noexcept
{
    return !name.empty();
}

int main()
{
    parameter_passing();

    std::cout << "\n=== Sink Parameters ===\n";
    Person ada{"Ada Lovelace"};
    std::cout << ada.name() << '\n';

    std::cout << "\n=== Returning a Struct ===\n";
    auto [lowest, highest] = min_max({4, 9, -2, 7});   // structured binding
    std::cout << "min " << lowest << ", max " << highest << '\n';

    overloading();

    std::cout << "\n=== Default Arguments ===\n";
    std::cout << greet("Ada") << '\n';
    std::cout << greet("Alan", "Good morning") << '\n';
    std::cout << greet("Grace", "Hi", '.') << '\n';

    std::cout << "\n=== inline, Recursion, [[nodiscard]] ===\n";
    std::cout << "twice(21)     = " << twice(21) << '\n';
    std::cout << "factorial(20) = " << factorial(20) << '\n';
    std::cout << "add(2, 3)     = " << add(2, 3) << '\n';

    // save_file("x");   // warning: ignoring return value declared [[nodiscard]]
    if (save_file("notes.txt")) {
        std::cout << "save_file result checked\n";
    }
}

// The definition promised by the declaration at the top.
int add(int a, int b) { return a + b; }
