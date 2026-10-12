// ============================================================
// SFINAE, and Why Concepts Replaced It
// ============================================================
// SFINAE: "Substitution Failure Is Not An Error". When the
// compiler substitutes template arguments into a function
// template's SIGNATURE and the result is invalid, that template is
// quietly removed from the overload set instead of causing an
// error. Before C++20, library authors used this deliberately to
// enable or disable overloads. You will meet it in every pre-2020
// codebase and in the standard library's own headers.
//
// Build and run:
//   cmake --build build --target templates_06_sfinae
//   ./build/bin/templates_06_sfinae
// ============================================================

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// std::enable_if
// ------------------------------------------------------------
// enable_if<Condition, T>::type exists only when Condition is
// true. Using it in the return type makes substitution fail (and
// the overload disappear) when the condition is false.

template <typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
classify(T)
{
    return "integral (enable_if)";
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, std::string>::type
classify(T)
{
    return "floating point (enable_if)";
}

// The same pair in C++20. Compare how much easier it is to read.
std::string classify20(std::integral auto) { return "integral (concept)"; }
std::string classify20(std::floating_point auto) { return "floating point (concept)"; }

// ------------------------------------------------------------
// Detecting a Member: the void_t Idiom
// ------------------------------------------------------------
// "Does T have a .size() member?" before C++20. The partial
// specialisation is only valid when decltype(...) compiles.

template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<const T&>().size())>> : std::true_type {};

// C++20 equivalent: one line.
template <typename T>
concept HasSize = requires(const T& t) { t.size(); };

static_assert(has_size<std::string>::value && HasSize<std::string>);
static_assert(!has_size<int>::value && !HasSize<int>);

// ------------------------------------------------------------
// Expression SFINAE with Trailing Return Types
// ------------------------------------------------------------
// decltype in the return type: the overload exists only when
// "a + b" is valid. The second overload is a fallback; the int vs
// long parameter makes the first preferred when both work.

template <typename A, typename B>
auto try_add_impl(const A& a, const B& b, int) -> decltype(a + b)
{
    return a + b;
}

template <typename A, typename B>
std::string try_add_impl(const A&, const B&, long)
{
    return "cannot add these";
}

template <typename A, typename B>
auto try_add(const A& a, const B& b)
{
    return try_add_impl(a, b, 0);   // 0 is an int: exact match for the first overload
}

// ------------------------------------------------------------
// Why Concepts Replaced It
// ------------------------------------------------------------
// - SFINAE hides the requirement inside the return type or a
//   dummy template parameter; a concept puts it in plain sight.
// - A failed SFINAE overload produces "no matching function"
//   followed by every candidate and why it failed. A concept
//   failure names the unsatisfied requirement.
// - Concepts subsume each other, so "more constrained wins"
//   works naturally; with enable_if, overlapping conditions are
//   ambiguous and must be made mutually exclusive by hand.
// - Concepts work on non-template members of class templates
//   (a trailing requires clause), which SFINAE cannot do cleanly.
// Keep the ability to READ SFINAE; write concepts.

int main()
{
    std::cout << "=== enable_if vs Concepts ===\n";
    std::cout << "  classify(1)     -> " << classify(1) << '\n';
    std::cout << "  classify(1.5)   -> " << classify(1.5) << '\n';
    std::cout << "  classify20(1)   -> " << classify20(1) << '\n';
    std::cout << "  classify20(1.5) -> " << classify20(1.5) << '\n';
    // classify("text");  // error: no matching function (both overloads removed by SFINAE)

    std::cout << "\n=== Detection ===\n";
    std::cout << "  has_size<std::vector<int>> = " << std::boolalpha
              << has_size<std::vector<int>>::value << '\n';
    std::cout << "  HasSize<double>            = " << HasSize<double> << '\n';

    std::cout << "\n=== Expression SFINAE ===\n";
    std::cout << "  try_add(2, 3)                  -> " << try_add(2, 3) << '\n';
    std::cout << "  try_add(std::string{\"a\"}, \"b\") -> " << try_add(std::string{"a"}, "b") << '\n';
    std::cout << "  try_add(std::vector<int>{}, 1) -> " << try_add(std::vector<int>{}, 1) << '\n';
}
