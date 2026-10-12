// ============================================================
// Template Specialisation and Type Traits
// ============================================================
// A specialisation provides a different implementation for
// particular template arguments. Full specialisation fixes every
// argument; partial specialisation (class templates only) fixes
// some, or matches a pattern such as "any pointer". Type traits,
// the compile-time questions in <type_traits>, are built from
// exactly these tools.
//
// Build and run:
//   cmake --build build --target templates_03_specialisation
//   ./build/bin/templates_03_specialisation
// ============================================================

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ------------------------------------------------------------
// Full Specialisation of a Class Template
// ------------------------------------------------------------
// Primary template first, then "template <>" with the arguments.

template <typename T>
struct TypeName {
    static std::string get() { return "unknown type"; }
};

template <>
struct TypeName<int> {
    static std::string get() { return "int"; }
};

template <>
struct TypeName<double> {
    static std::string get() { return "double"; }
};

template <>
struct TypeName<std::string> {
    static std::string get() { return "std::string"; }
};

// ------------------------------------------------------------
// Partial Specialisation: Matching Patterns
// ------------------------------------------------------------
// "Any pointer to something" and "any vector of something".
// The pattern recurses into TypeName for the inner type.

template <typename T>
struct TypeName<T*> {
    static std::string get() { return "pointer to " + TypeName<T>::get(); }
};

template <typename T>
struct TypeName<std::vector<T>> {
    static std::string get() { return "vector of " + TypeName<T>::get(); }
};

// ------------------------------------------------------------
// Specialising Function Templates
// ------------------------------------------------------------
// Function templates can be FULLY specialised but not partially.
// In practice prefer an ordinary overload: specialisations do not
// take part in overload resolution, which surprises people.

template <typename T>
std::string describe(const T&) { return "a value"; }

template <>
std::string describe<bool>(const bool& b) { return b ? "yes" : "no"; }   // full specialisation

std::string describe(const char* s) { return std::string{"text: "} + s; }   // overload: preferred style

// ------------------------------------------------------------
// Writing Your Own Type Traits
// ------------------------------------------------------------
// A trait is a template whose specialisations answer a question
// at compile time. std::true_type and std::false_type hold the
// value. The standard library's is_pointer works the same way.

template <typename T>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;   // the _v shortcut, as in the standard

static_assert(is_vector_v<std::vector<int>>);
static_assert(!is_vector_v<int>);

// ------------------------------------------------------------
// Using Standard Traits with if constexpr
// ------------------------------------------------------------

template <typename T>
std::string kind_of(const T&)
{
    if constexpr (std::is_integral_v<T>) {
        return "integral";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "floating point";
    } else if constexpr (std::is_pointer_v<T>) {
        return "pointer";
    } else if constexpr (is_vector_v<T>) {
        return "vector";
    } else {
        return "something else";
    }
}

// Type transformations: traits that produce a TYPE rather than a value.
using Raw = std::remove_cvref_t<const std::string&>;   // C++20: strips const and &
static_assert(std::is_same_v<Raw, std::string>);

int main()
{
    std::cout << "=== Full and Partial Specialisation ===\n";
    std::cout << "  " << TypeName<int>::get() << '\n';
    std::cout << "  " << TypeName<double*>::get() << '\n';
    std::cout << "  " << TypeName<std::vector<std::string>>::get() << '\n';
    std::cout << "  " << TypeName<std::vector<int*>>::get() << '\n';
    std::cout << "  " << TypeName<char>::get() << '\n';

    std::cout << "\n=== Function Template Specialisation vs Overload ===\n";
    std::cout << "  describe(42)     -> " << describe(42) << '\n';
    std::cout << "  describe(true)   -> " << describe(true) << '\n';
    std::cout << "  describe(\"hi\")   -> " << describe("hi") << '\n';

    std::cout << "\n=== Traits ===\n";
    int n = 0;
    std::cout << "  kind_of(1)            = " << kind_of(1) << '\n';
    std::cout << "  kind_of(2.0)          = " << kind_of(2.0) << '\n';
    std::cout << "  kind_of(&n)           = " << kind_of(&n) << '\n';
    std::cout << "  kind_of(vector<int>)  = " << kind_of(std::vector<int>{}) << '\n';
    std::cout << "  kind_of(std::string)  = " << kind_of(std::string{}) << '\n';
}
