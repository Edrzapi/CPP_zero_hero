// ============================================================
// Structured Bindings (C++17)
// ============================================================
// auto [a, b, c] = something;  unpacks a struct, array, pair or
// tuple into named variables. Small feature, large readability
// win, with a few rules worth knowing about what the names really
// are.
//
// Build and run:
//   cmake --build build --target under_the_hood_02_structured_bindings
//   ./build/bin/under_the_hood_02_structured_bindings
// ============================================================

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>

// ------------------------------------------------------------
// The Three Things You Can Bind
// ------------------------------------------------------------
// 1. arrays (C arrays and std::array)
// 2. tuple-like types: std::pair, std::tuple, and anything that
//    specialises std::tuple_size / std::tuple_element and get<>
// 3. structs whose non-static data members are all public

struct Measurement {
    std::string sensor;
    double value;
    int timestamp;
};

std::pair<int, int> divide(int a, int b) { return {a / b, a % b}; }

void the_basics()
{
    std::cout << "=== Basics ===\n";

    int point[2] = {3, 4};
    auto [x, y] = point;
    std::cout << "  array : x = " << x << ", y = " << y << '\n';

    auto [quotient, remainder] = divide(17, 5);
    std::cout << "  pair  : 17 / 5 = " << quotient << " remainder " << remainder << '\n';

    std::tuple<std::string, int, bool> row{"Ada", 36, true};
    auto [name, age, active] = row;
    std::cout << "  tuple : " << name << ", " << age << ", " << std::boolalpha << active << '\n';

    Measurement m{"thermometer", 21.5, 1700000000};
    auto [sensor, value, when] = m;
    std::cout << "  struct: " << sensor << " = " << value << " at " << when << '\n';
}

// ------------------------------------------------------------
// What the Names Really Are
// ------------------------------------------------------------
// The compiler creates ONE hidden variable (a copy, or a reference
// if you write auto& / const auto&), and the names refer to its
// members. So:
//   auto [a, b] = s;    a and b are members of a hidden COPY of s
//   auto& [a, b] = s;   a and b refer to the members of s itself

void copies_and_references()
{
    std::cout << "\n=== auto vs auto& ===\n";
    Measurement m{"hygrometer", 40.0, 0};

    auto [s1, v1, t1] = m;   // copy
    v1 = 99.0;
    std::cout << "  after modifying the copy     : m.value = " << m.value << '\n';   // 40

    auto& [s2, v2, t2] = m;  // reference
    v2 = 99.0;
    std::cout << "  after modifying the reference: m.value = " << m.value << '\n';   // 99

    // Unused names still need a name; [[maybe_unused]] silences the warning.
    // (C++26 allows _ as a placeholder for "do not care".)
    [[maybe_unused]] auto [unused_sensor, unused_value, unused_time] = m;
    std::cout << "  sizes: " << s1.size() << ' ' << s2.size() << ' ' << t1 + t2 << '\n';
}

// ------------------------------------------------------------
// The Everyday Use: Maps
// ------------------------------------------------------------

void with_maps()
{
    std::cout << "\n=== Iterating and Inserting into Maps ===\n";
    std::map<std::string, int> stock{{"apples", 3}, {"pears", 0}};

    for (auto& [fruit, count] : stock) {
        count += 10;   // modifies the map: bound by reference
    }
    for (const auto& [fruit, count] : stock) {
        std::cout << "  " << fruit << ": " << count << '\n';
    }

    if (auto [it, inserted] = stock.insert({"plums", 5}); inserted) {
        std::cout << "  inserted " << it->first << '\n';
    }
}

// ------------------------------------------------------------
// Making Your Own Type Bindable (Tuple Protocol)
// ------------------------------------------------------------
// For a class with private data, provide get<I>() and specialise
// std::tuple_size and std::tuple_element. This is exactly how
// std::pair and std::tuple support it.

class Colour {
public:
    constexpr Colour(int r, int g, int b) : r_{r}, g_{g}, b_{b} {}

    template <std::size_t I>
    constexpr int get() const
    {
        if constexpr (I == 0) return r_;
        else if constexpr (I == 1) return g_;
        else return b_;
    }

private:
    int r_;
    int g_;
    int b_;
};

template <>
struct std::tuple_size<Colour> : std::integral_constant<std::size_t, 3> {};

template <std::size_t I>
struct std::tuple_element<I, Colour> {
    using type = int;
};

int main()
{
    the_basics();
    copies_and_references();
    with_maps();

    std::cout << "\n=== A Custom Tuple-Like Class ===\n";
    Colour teal{0, 128, 128};
    auto [r, g, b] = teal;
    std::cout << "  r = " << r << ", g = " << g << ", b = " << b << '\n';

    // C++20: structured bindings can be captured by lambdas.
    auto describe = [r, g, b] { return r + g + b; };
    std::cout << "  sum via a lambda capture: " << describe() << '\n';
}
