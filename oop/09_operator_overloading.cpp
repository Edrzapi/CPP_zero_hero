// ============================================================
// Operator Overloading, Including <=> (C++20)
// ============================================================
// User types can define what +, ==, <, <<, [] and friends mean.
// Done well, a Vector2 or Money type reads like a built-in type.
// C++20's three-way comparison operator <=> ("spaceship") writes
// all six comparisons from one declaration.
//
// Build and run:
//   cmake --build build --target oop_09_operator_overloading
//   ./build/bin/oop_09_operator_overloading
// ============================================================

#include <compare>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// Arithmetic Operators
// ------------------------------------------------------------
// Idiom: write the compound operator (+=) as a member returning
// *this, then write + as a non-member in terms of +=.
// Binary operators as non-members (here "hidden friends", defined
// inside the class) treat both operands symmetrically, so
// 2.0 * v and v * 2.0 can both work.

class Vector2 {
public:
    constexpr Vector2() = default;
    constexpr Vector2(double x, double y) : x_{x}, y_{y} {}

    constexpr double x() const { return x_; }
    constexpr double y() const { return y_; }

    Vector2& operator+=(const Vector2& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    Vector2& operator*=(double factor)
    {
        x_ *= factor;
        y_ *= factor;
        return *this;
    }

    // Unary minus.
    Vector2 operator-() const { return {-x_, -y_}; }

    // Hidden friends: only found by argument-dependent lookup, which
    // keeps them out of unrelated overload sets.
    friend Vector2 operator+(Vector2 lhs, const Vector2& rhs) { return lhs += rhs; }
    friend Vector2 operator*(Vector2 v, double factor) { return v *= factor; }
    friend Vector2 operator*(double factor, Vector2 v) { return v *= factor; }

    // C++20: a defaulted == compares every member in order. != is
    // generated from it automatically.
    friend bool operator==(const Vector2&, const Vector2&) = default;

    // Stream output: must be a non-member, because the left operand
    // is the stream, not a Vector2.
    friend std::ostream& operator<<(std::ostream& out, const Vector2& v)
    {
        return out << '(' << v.x_ << ", " << v.y_ << ')';
    }

private:
    double x_ = 0;
    double y_ = 0;
};

void arithmetic()
{
    std::cout << "=== Arithmetic Operators ===\n";
    Vector2 a{1, 2};
    Vector2 b{3, 4};

    std::cout << "  a + b     = " << a + b << '\n';
    std::cout << "  a * 2     = " << a * 2 << '\n';
    std::cout << "  2 * a     = " << 2 * a << '\n';
    std::cout << "  -a        = " << -a << '\n';
    a += b;
    std::cout << "  a += b    -> " << a << '\n';
    std::cout << "  a == b    : " << std::boolalpha << (a == b) << '\n';
    std::cout << "  a != b    : " << (a != b) << " (generated from ==)\n";
}

// ------------------------------------------------------------
// Three-Way Comparison <=>
// ------------------------------------------------------------
// a <=> b returns an ordering object: less, equal or greater.
// Defaulting it compares members lexicographically in declaration
// order, and the compiler rewrites a < b as (a <=> b) < 0, etc.
// One line replaces six hand-written operators.
//
// Return types:
//   std::strong_ordering  equal values are indistinguishable (ints)
//   std::weak_ordering    equivalent but distinguishable (case-insensitive text)
//   std::partial_ordering some values are unordered (doubles with NaN)

struct Version {
    int major;
    int minor;
    int patch;

    auto operator<=>(const Version&) const = default;   // also gives ==
};

// A hand-written <=> when the default order is wrong:
// compare by surname first, then first name.
class Person {
public:
    Person(std::string first, std::string last) : first_{std::move(first)}, last_{std::move(last)} {}

    std::strong_ordering operator<=>(const Person& other) const
    {
        if (auto cmp = last_ <=> other.last_; cmp != 0) {
            return cmp;
        }
        return first_ <=> other.first_;
    }
    bool operator==(const Person& other) const = default;   // a custom <=> does NOT generate ==

    std::string full_name() const { return first_ + ' ' + last_; }

private:
    std::string first_;
    std::string last_;
};

void comparisons()
{
    std::cout << "\n=== Three-Way Comparison ===\n";

    Version v1{1, 4, 2};
    Version v2{1, 10, 0};
    std::cout << "  1.4.2 <  1.10.0 : " << (v1 < v2) << '\n';    // true: 4 < 10 numerically
    std::cout << "  1.4.2 >= 1.10.0 : " << (v1 >= v2) << '\n';
    std::cout << "  1.4.2 == 1.4.2  : " << (v1 == Version{1, 4, 2}) << '\n';

    auto result = v1 <=> v2;
    std::cout << "  (v1 <=> v2) < 0 : " << (result < 0) << '\n';

    Person ada{"Ada", "Lovelace"};
    Person alan{"Alan", "Turing"};
    Person adam{"Adam", "Lovelace"};
    std::cout << "  " << ada.full_name() << " < " << alan.full_name() << " : " << (ada < alan) << '\n';
    std::cout << "  " << adam.full_name() << " < " << ada.full_name() << " : " << (adam < ada) << '\n';
}

// ------------------------------------------------------------
// Subscript, Call, Increment and Conversion Operators
// ------------------------------------------------------------

class Grid {
public:
    Grid(std::size_t rows, std::size_t cols) : cols_{cols}, cells_(rows * cols) {}

    // operator[]: provide const and non-const versions. Returning a
    // pointer to the start of a row makes grid[r][c] work.
    // (C++23 allows several arguments: grid[r, c].)
    int* operator[](std::size_t row) { return cells_.data() + row * cols_; }
    const int* operator[](std::size_t row) const { return cells_.data() + row * cols_; }

    // operator(): here used for 2D access, as in many maths libraries.
    int& operator()(std::size_t row, std::size_t col) { return cells_.at(row * cols_ + col); }
    int operator()(std::size_t row, std::size_t col) const { return cells_.at(row * cols_ + col); }

private:
    std::size_t cols_;
    std::vector<int> cells_;
};

class Counter {
public:
    Counter& operator++()            // prefix: ++c, returns the NEW value by reference
    {
        ++value_;
        return *this;
    }
    Counter operator++(int)          // postfix: c++, the dummy int marks it; returns the OLD value
    {
        Counter old = *this;
        ++value_;
        return old;
    }
    int value() const { return value_; }

    // explicit operator bool: usable in if (counter) but not in
    // counter + 1 or int n = counter. Streams and smart pointers
    // use this.
    explicit operator bool() const { return value_ != 0; }

private:
    int value_ = 0;
};

void other_operators()
{
    std::cout << "\n=== [], (), ++ and Conversion ===\n";

    Grid grid{2, 3};
    grid(1, 2) = 42;
    grid[0][1] = 7;
    const Grid& view = grid;
    std::cout << "  grid(1, 2) = " << view(1, 2) << ", grid[0][1] = " << view[0][1] << '\n';

    Counter c;
    if (!c) std::cout << "  counter starts false\n";
    Counter before = c++;
    ++c;
    std::cout << "  before = " << before.value() << ", now = " << c.value() << '\n';   // 0, 2
    if (c) std::cout << "  counter is now true\n";
    // int n = c;   // error: the conversion is explicit
}

// ------------------------------------------------------------
// Rules of Thumb
// ------------------------------------------------------------
// - Keep the usual meaning: + adds, == compares. No clever tricks.
// - Do not overload &&, || or the comma operator: the overloads
//   lose short-circuit evaluation and sequencing.
// - Do not overload unary & (address-of).
// - Return *this by reference from = and compound assignments.
// - If you define one comparison, default <=> and == instead of
//   writing all six.

int main()
{
    arithmetic();
    comparisons();
    other_operators();
}
