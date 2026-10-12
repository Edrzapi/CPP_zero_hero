// ============================================================
// Concepts and requires Clauses (C++20)
// ============================================================
// A concept is a named compile-time predicate on types: "T can be
// added", "T is a range of numbers". Constraining a template with
// one gives:
//   - documentation in the signature
//   - errors at the CALL site that name the unmet requirement,
//     instead of pages of errors from deep inside the template
//   - overloading on requirements (pick the best-matching version)
// Roughly what Java's <T extends Comparable<T>> and C#'s
// "where T : IComparable" do, but structural: a type satisfies a
// concept if the expressions compile, no inheritance needed.
//
// Build and run:
//   cmake --build build --target templates_05_concepts
//   ./build/bin/templates_05_concepts
// ============================================================

#include <concepts>
#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Using Standard Concepts
// ------------------------------------------------------------
// <concepts> provides std::integral, std::floating_point,
// std::same_as, std::convertible_to, std::totally_ordered,
// std::invocable and more. <ranges> adds range concepts.

// Four equivalent spellings:
template <typename T>
    requires std::integral<T>
T twice_a(T x) { return x * 2; }

template <std::integral T>          // constrained template parameter
T twice_b(T x) { return x * 2; }

template <typename T>
T twice_c(T x) requires std::integral<T> { return x * 2; }   // trailing requires clause

std::integral auto twice_d(std::integral auto x) { return x * 2; }   // constrained auto

// ------------------------------------------------------------
// Writing a Concept
// ------------------------------------------------------------
// A requires EXPRESSION lists operations that must compile, and
// optionally what their results must satisfy.

template <typename T>
concept Shape = requires(const T& shape) {
    { shape.area() } -> std::convertible_to<double>;
    { shape.name() } -> std::convertible_to<std::string>;
};

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

struct Square {
    double side;
    double area() const { return side * side; }
    std::string name() const { return "square"; }
};

struct Circle {
    double radius;
    double area() const { return 3.14159 * radius * radius; }
    std::string name() const { return "circle"; }
};

struct NotAShape {
    int value;
};

// No base class, no virtual functions: Square and Circle satisfy
// Shape just by having the right members.
void describe(const Shape auto& shape)
{
    std::cout << "  " << shape.name() << " with area " << shape.area() << '\n';
}

static_assert(Shape<Square>);
static_assert(!Shape<NotAShape>);

// describe(NotAShape{1}) fails with a message along the lines of:
//   "the constraint Shape<NotAShape> was not satisfied:
//    the required expression shape.area() is invalid"

// ------------------------------------------------------------
// Overloading on Constraints
// ------------------------------------------------------------
// When several constrained overloads match, the MORE constrained
// one wins. std::random_access_iterator subsumes
// std::bidirectional_iterator, so the vector call picks the first.

template <std::ranges::random_access_range R>
std::string access_kind(const R&) { return "random access: can jump to any element"; }

template <std::ranges::bidirectional_range R>
std::string access_kind(const R&) { return "bidirectional: must walk step by step"; }

// ------------------------------------------------------------
// Constraining a Class Template
// ------------------------------------------------------------

template <Numeric T>
class Statistics {
public:
    void add(T value)
    {
        total_ += value;
        ++count_;
    }
    double mean() const { return count_ ? static_cast<double>(total_) / count_ : 0.0; }

private:
    T total_{};
    int count_ = 0;
};

// Statistics<std::string> s;   // error: std::string does not satisfy Numeric

// ------------------------------------------------------------
// requires Inside if constexpr
// ------------------------------------------------------------
// A requires expression is also just a bool, usable anywhere.

template <typename T>
std::string size_info(const T& value)
{
    if constexpr (requires { value.size(); }) {
        return "has size() = " + std::to_string(value.size());
    } else {
        return "has no size()";
    }
}

int main()
{
    std::cout << "=== Standard Concepts ===\n";
    std::cout << "  " << twice_a(2) << ' ' << twice_b(3) << ' ' << twice_c(4) << ' ' << twice_d(5) << '\n';
    // twice_a(2.5);   // error: double does not satisfy std::integral

    std::cout << "\n=== A Custom Concept ===\n";
    describe(Square{2});
    describe(Circle{1});

    std::cout << "\n=== Most Constrained Overload Wins ===\n";
    std::cout << "  vector: " << access_kind(std::vector<int>{1, 2}) << '\n';
    std::cout << "  list:   " << access_kind(std::list<int>{1, 2}) << '\n';

    std::cout << "\n=== Constrained Class Template ===\n";
    Statistics<int> stats;
    for (int x : {3, 4, 8}) stats.add(x);
    std::cout << "  mean = " << stats.mean() << '\n';

    std::cout << "\n=== requires Expression as a bool ===\n";
    std::cout << "  string: " << size_info(std::string{"abc"}) << '\n';
    std::cout << "  int:    " << size_info(42) << '\n';
}
