// ============================================================
// Function Templates
// ============================================================
// A template is a recipe the compiler uses to generate code for
// each type you use it with ("instantiation"). Unlike Java
// generics (type erasure, objects only) and C# generics (shared
// code for reference types), every C++ instantiation is separate,
// fully typed machine code, and works with int and double as well
// as class types.
//
// Build and run:
//   cmake --build build --target templates_01_function_templates
//   ./build/bin/templates_01_function_templates
// ============================================================

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// A First Template
// ------------------------------------------------------------
// "typename T" (or the older "class T") introduces a type
// parameter. The body only has to make sense for the types it is
// actually called with; the error appears at the call site
// otherwise (05_concepts.cpp makes those errors readable).

template <typename T>
T largest(const T& a, const T& b)
{
    return (a < b) ? b : a;
}

// ------------------------------------------------------------
// Template Argument Deduction
// ------------------------------------------------------------
// T is deduced from the arguments. Both arguments must deduce the
// SAME T, so largest(1, 2.5) fails: T cannot be int and double.
// Fix it with an explicit argument: largest<double>(1, 2.5).

void deduction()
{
    std::cout << "=== Deduction ===\n";
    std::cout << "  largest(3, 7)            = " << largest(3, 7) << '\n';               // T = int
    std::cout << "  largest(2.5, 1.5)        = " << largest(2.5, 1.5) << '\n';           // T = double
    std::cout << "  largest<std::string>(...)= "
              << largest<std::string>("apple", "banana") << '\n';   // explicit: compares strings, not pointers
    // largest(1, 2.5);                  // error: deduced conflicting types for T
    std::cout << "  largest<double>(1, 2.5)  = " << largest<double>(1, 2.5) << '\n';
}

// ------------------------------------------------------------
// Several Parameters and auto Return Types
// ------------------------------------------------------------
// The result type of a + b depends on both types. auto lets the
// compiler work it out.

template <typename A, typename B>
auto add(const A& a, const B& b)
{
    return a + b;
}

// ------------------------------------------------------------
// Non-Type Template Parameters
// ------------------------------------------------------------
// Values known at compile time can be template parameters too:
// integers, enums, pointers and (C++20) floating point and simple
// class types. std::array<T, N> uses one for its size.

template <int Times>
std::string repeat(const std::string& s)
{
    std::string out;
    for (int i = 0; i < Times; ++i) out += s;
    return out;
}

// Deduce an array's size from its type.
template <typename T, std::size_t N>
constexpr std::size_t length_of(const T (&)[N])
{
    return N;
}

// ------------------------------------------------------------
// Abbreviated Function Templates (C++20)
// ------------------------------------------------------------
// "auto" in a parameter list makes a function a template without
// the template <...> line. Each auto is a separate type parameter.

void print_pair(const auto& first, const auto& second)
{
    std::cout << "  (" << first << ", " << second << ")\n";
}

// ------------------------------------------------------------
// Templates Live in Headers
// ------------------------------------------------------------
// The compiler needs the template's BODY at every call site to
// instantiate it, so templates are normally defined entirely in
// header files. Declaring a template in a header and defining it
// in a .cpp gives linker errors, unless you explicitly instantiate
// the types you need in that .cpp:
//     template int largest<int>(const int&, const int&);
// Every instantiation is compiled again in every translation unit
// that uses it (and merged by the linker): one reason heavy
// template code builds slowly.

// ------------------------------------------------------------
// Overloading Templates and Non-Templates
// ------------------------------------------------------------
// A non-template function that matches exactly is preferred over
// a template. Here C strings get their own version, because the
// template would compare POINTER addresses.

const char* largest(const char* a, const char* b)
{
    return std::string{a} < std::string{b} ? b : a;
}

int main()
{
    deduction();

    std::cout << "\n=== auto Return Type ===\n";
    std::cout << "  add(1, 2.5)              = " << add(1, 2.5) << '\n';
    std::cout << "  add(string, \"!\")         = " << add(std::string{"hi"}, "!") << '\n';

    std::cout << "\n=== Non-Type Parameters ===\n";
    std::cout << "  repeat<3>(\"ab\")          = " << repeat<3>("ab") << '\n';
    int data[7]{};
    static_assert(length_of(data) == 7);
    std::cout << "  length_of(int[7])        = " << length_of(data) << '\n';

    std::cout << "\n=== Abbreviated Templates ===\n";
    print_pair(1, "one");
    print_pair(2.5, std::vector<int>{}.size());

    std::cout << "\n=== Non-Template Overload Wins ===\n";
    std::cout << "  largest(\"pear\", \"apple\") = " << largest("pear", "apple") << '\n';
}
