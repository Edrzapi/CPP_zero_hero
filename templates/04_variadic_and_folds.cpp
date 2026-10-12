// ============================================================
// Variadic Templates, Fold Expressions, Perfect Forwarding
// ============================================================
// A parameter pack (typename... Ts) accepts any number of types.
// It is how std::make_unique, std::tuple, std::format and
// emplace_back accept "any arguments". C++11 needed recursion to
// unpack a pack; C++17 fold expressions do it in one line.
//
// Build and run:
//   cmake --build build --target templates_04_variadic_and_folds
//   ./build/bin/templates_04_variadic_and_folds
// ============================================================

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

// ------------------------------------------------------------
// The C Way: printf and va_list
// ------------------------------------------------------------
// C's variadic functions (int printf(const char*, ...)) lose all
// type information; the callee must trust the format string.
// Variadic templates keep every argument's type.

// ------------------------------------------------------------
// C++11: Recursion
// ------------------------------------------------------------
// Peel off the first argument, recurse on the rest, stop with an
// overload for the empty case.

void print_all_recursive() { std::cout << '\n'; }   // base case

template <typename First, typename... Rest>
void print_all_recursive(const First& first, const Rest&... rest)
{
    std::cout << first << ' ';
    print_all_recursive(rest...);   // pack expansion: rest... becomes the remaining arguments
}

// ------------------------------------------------------------
// C++17: Fold Expressions
// ------------------------------------------------------------
// (pack op ...)        unary right fold:  a1 op (a2 op (a3 op a4))
// (... op pack)        unary left fold:   ((a1 op a2) op a3) op a4
// (init op ... op pack) binary left fold with an initial value
// An empty pack is an error for most operators in a unary fold;
// a binary fold with an init value handles it.

template <typename... Ts>
auto sum(const Ts&... values)
{
    return (values + ... + 0);   // binary right fold: works for zero arguments too
}

template <typename... Ts>
void print_all(const Ts&... values)
{
    ((std::cout << values << ' '), ...);   // fold over the comma operator
    std::cout << '\n';
}

template <typename... Ts>
bool all_positive(const Ts&... values)
{
    return ((values > 0) && ...);    // true for an empty pack
}

template <typename T, typename... Ts>
bool is_one_of(const T& value, const Ts&... options)
{
    return ((value == options) || ...);
}

// sizeof...(pack) counts the arguments at compile time.
template <typename... Ts>
constexpr std::size_t count_args(const Ts&...)
{
    return sizeof...(Ts);
}

// ------------------------------------------------------------
// Perfect Forwarding
// ------------------------------------------------------------
// A "forwarding reference" is T&& where T is deduced. It binds to
// both lvalues and rvalues and remembers which it got.
// std::forward<T>(arg) passes it on unchanged: lvalues stay
// lvalues (copied), rvalues stay rvalues (moved). This is how
// make_unique and emplace_back construct objects without extra
// copies.

class Widget {
public:
    Widget(std::string name, int size) : name_{std::move(name)}, size_{size}
    {
        std::cout << "  Widget(" << name_ << ", " << size_ << ")\n";
    }

private:
    std::string name_;
    int size_;
};

template <typename T, typename... Args>
std::unique_ptr<T> my_make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void report(const std::string&) { std::cout << "  received an lvalue (will copy)\n"; }
void report(std::string&&) { std::cout << "  received an rvalue (can move)\n"; }

template <typename T>
void relay(T&& arg)
{
    report(std::forward<T>(arg));    // without forward, arg is always an lvalue here
}

// ------------------------------------------------------------
// Tuples and Packs
// ------------------------------------------------------------
// std::apply calls a function with a tuple's elements as
// arguments: a pack expansion at run time.

int main()
{
    std::cout << "=== Recursion (C++11 style) ===\n  ";
    print_all_recursive(1, 2.5, "three", std::string{"four"});

    std::cout << "\n=== Fold Expressions (C++17) ===\n";
    std::cout << "  sum(1, 2, 3, 4)       = " << sum(1, 2, 3, 4) << '\n';
    std::cout << "  sum()                 = " << sum() << '\n';
    std::cout << "  sum(1, 2.5)           = " << sum(1, 2.5) << '\n';
    std::cout << "  print_all: ";
    print_all('a', 42, "text", 3.14);
    std::cout << "  all_positive(3, 1, 2) = " << std::boolalpha << all_positive(3, 1, 2) << '\n';
    std::cout << "  all_positive(3, -1)   = " << all_positive(3, -1) << '\n';
    std::cout << "  is_one_of(7, 1, 7, 9) = " << is_one_of(7, 1, 7, 9) << '\n';
    static_assert(count_args(1, 'x', 2.0) == 3);
    std::cout << "  count_args(1, 'x', 2.0) = " << count_args(1, 'x', 2.0) << '\n';

    std::cout << "\n=== Perfect Forwarding ===\n";
    auto w = my_make_unique<Widget>("gear", 12);
    std::string name = "kept";
    relay(name);                     // lvalue
    relay(std::string{"temporary"}); // rvalue
    relay(std::move(name));          // rvalue

    std::cout << "\n=== std::tuple and std::apply ===\n";
    std::tuple<int, double, std::string> record{1, 9.5, "row"};
    std::cout << "  get<2>(record) = " << std::get<2>(record) << '\n';
    std::cout << "  apply(print_all, record): ";
    std::apply([](const auto&... fields) { print_all(fields...); }, record);
}
