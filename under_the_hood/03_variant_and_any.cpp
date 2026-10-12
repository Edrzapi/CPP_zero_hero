// ============================================================
// std::variant, std::visit and std::any (C++17)
// ============================================================
// std::variant<A, B, C> holds exactly ONE of a fixed list of
// types: a type-safe union. It is a closed alternative to an
// inheritance hierarchy, with no heap allocation and no virtual
// calls. std::any holds a value of ANY copyable type, checked at
// run time: flexible, and rarely the right choice.
//
// Build and run:
//   cmake --build build --target under_the_hood_03_variant_and_any
//   ./build/bin/under_the_hood_03_variant_and_any
// ============================================================

#include <any>
#include <cmath>
#include <iostream>
#include <numbers>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

// ------------------------------------------------------------
// From C's union to std::variant
// ------------------------------------------------------------
// C's tagged union: a struct with a "which one" field and a union.
// Nothing stops you reading the wrong member (undefined
// behaviour), and a union cannot safely hold a std::string.
// std::variant tracks the active type itself and destroys it
// properly.

void variant_basics()
{
    std::cout << "=== variant Basics ===\n";

    std::variant<int, double, std::string> value = 42;
    std::cout << "  index() = " << value.index() << " (int)\n";

    value = std::string{"now a string"};
    std::cout << "  holds string? " << std::boolalpha << std::holds_alternative<std::string>(value) << '\n';
    std::cout << "  get<std::string> = " << std::get<std::string>(value) << '\n';

    // Asking for the wrong type throws...
    try {
        std::cout << std::get<int>(value);
    } catch (const std::bad_variant_access&) {
        std::cout << "  get<int> threw bad_variant_access\n";
    }
    // ...or get_if returns nullptr instead of throwing.
    if (const auto* d = std::get_if<double>(&value); d == nullptr) {
        std::cout << "  get_if<double> returned nullptr\n";
    }

    std::cout << "  sizeof(variant<int, double, string>) = " << sizeof(value)
              << " (largest member + index; no heap)\n";
}

// ------------------------------------------------------------
// std::visit and the "overloaded" Pattern
// ------------------------------------------------------------
// visit calls a function with whatever the variant currently
// holds. With a set of lambdas, one per type, the compiler checks
// every alternative is handled: add a type to the variant and
// forget a lambda, and it does not compile.

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;   // deduction guide (optional in C++20 on most compilers)

struct Circle {
    double radius;
};
struct Rectangle {
    double width;
    double height;
};
struct Triangle {
    double base;
    double height;
};

using Shape = std::variant<Circle, Rectangle, Triangle>;

double area(const Shape& shape)
{
    return std::visit(overloaded{
                          [](const Circle& c) { return std::numbers::pi * c.radius * c.radius; },
                          [](const Rectangle& r) { return r.width * r.height; },
                          [](const Triangle& t) { return 0.5 * t.base * t.height; },
                      },
                      shape);
}

void visiting()
{
    std::cout << "\n=== visit: Closed Polymorphism ===\n";

    std::vector<Shape> shapes{Circle{1}, Rectangle{2, 3}, Triangle{4, 5}};
    for (const auto& s : shapes) {
        std::cout << "  area = " << area(s) << '\n';
    }

    // A generic lambda with if constexpr is the other common style.
    std::variant<int, std::string> item = "text";
    std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "  an int: " << v << '\n';
        } else {
            std::cout << "  a string of length " << v.size() << '\n';
        }
    }, item);
}

// ------------------------------------------------------------
// variant vs Virtual Functions
// ------------------------------------------------------------
//   variant + visit                 virtual functions
//   closed set of types             open: anyone can add a derived class
//   easy to add OPERATIONS          easy to add TYPES
//   values, no heap, no vptr        usually heap + pointer per object
//   all types known in one place    types can live in other libraries

// ------------------------------------------------------------
// std::any
// ------------------------------------------------------------
// Holds a single value of any copyable type, with run-time type
// checking via any_cast. Small values are stored inside the any
// (small buffer optimisation); larger ones on the heap.
// Use cases: plugin property bags, passing opaque user data
// through a library. If you know the possible types, use variant.

void any_demo()
{
    std::cout << "\n=== std::any ===\n";

    std::any box = 7;
    std::cout << "  holds int? " << (box.type() == typeid(int)) << ", value " << std::any_cast<int>(box) << '\n';

    box = std::string{"replaced by a string"};
    if (auto* s = std::any_cast<std::string>(&box)) {   // pointer form: nullptr on mismatch
        std::cout << "  " << *s << '\n';
    }

    try {
        std::cout << std::any_cast<double>(box);
    } catch (const std::bad_any_cast&) {
        std::cout << "  any_cast<double> threw bad_any_cast\n";
    }

    box.reset();
    std::cout << "  has_value after reset: " << box.has_value() << '\n';
}

int main()
{
    variant_basics();
    visiting();
    any_demo();
}
