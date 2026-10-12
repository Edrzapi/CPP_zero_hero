// ============================================================
// Class Templates
// ============================================================
// Every standard container is a class template: std::vector<int>
// and std::vector<std::string> are two different classes generated
// from one definition. This file writes a small generic container,
// then looks at CTAD, member templates and default arguments.
// Compare with Java Zero to Hero's Box<T>.
//
// Build and run:
//   cmake --build build --target templates_02_class_templates
//   ./build/bin/templates_02_class_templates
// ============================================================

#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// ------------------------------------------------------------
// A Generic Box
// ------------------------------------------------------------

template <typename T>
class Box {
public:
    explicit Box(T value) : value_{std::move(value)} {}

    const T& get() const { return value_; }
    void set(T value) { value_ = std::move(value); }

    // A member template: convert Box<T> into Box<U> with a function.
    template <typename F>
    auto map(F func) const
    {
        using U = decltype(func(value_));
        return Box<U>{func(value_)};
    }

private:
    T value_;
};

// ------------------------------------------------------------
// A Fixed-Capacity Stack: Type and Value Parameters
// ------------------------------------------------------------
// No heap allocation at all: storage is a std::array sized by a
// template argument. Different capacities are different types.
// The default argument means FixedStack<int> has capacity 8.

template <typename T, std::size_t Capacity = 8>
class FixedStack {
public:
    void push(const T& value)
    {
        if (size_ == Capacity) throw std::length_error{"FixedStack is full"};
        items_[size_++] = value;
    }

    T pop()
    {
        if (size_ == 0) throw std::out_of_range{"FixedStack is empty"};
        return items_[--size_];
    }

    std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    std::array<T, Capacity> items_{};
    std::size_t size_ = 0;
};

// ------------------------------------------------------------
// Member Functions Defined Outside the Class
// ------------------------------------------------------------
// Each needs the full template header and the qualified name.

template <typename K, typename V>
class Pair {
public:
    Pair(K key, V value);
    std::string describe() const;

private:
    K key_;
    V value_;
};

template <typename K, typename V>
Pair<K, V>::Pair(K key, V value) : key_{std::move(key)}, value_{std::move(value)} {}

template <typename K, typename V>
std::string Pair<K, V>::describe() const
{
    return std::string{"("} + std::to_string(key_) + " -> " + value_ + ")";
}

// ------------------------------------------------------------
// CTAD: Class Template Argument Deduction (C++17)
// ------------------------------------------------------------
// The template arguments can be deduced from constructor
// arguments, just like function templates:
//   std::vector v{1, 2, 3};   // std::vector<int>
//   std::pair p{1, 2.5};      // std::pair<int, double>
//   Box b{42};                // Box<int>
// A deduction guide tells CTAD what to do when the constructor
// alone would deduce something unhelpful, e.g. const char*.

Box(const char*) -> Box<std::string>;   // a string literal makes a Box<std::string>

int main()
{
    std::cout << "=== Box<T> ===\n";
    Box<int> number{42};
    Box word{"hello"};                        // CTAD + deduction guide: Box<std::string>
    auto length = word.map([](const std::string& s) { return s.size(); });   // Box<std::size_t>
    std::cout << "  number = " << number.get() << ", word = " << word.get()
              << ", length = " << length.get() << '\n';
    word.set("changed");
    std::cout << "  after set: " << word.get() << '\n';

    std::cout << "\n=== FixedStack<T, N> ===\n";
    FixedStack<int, 3> small;
    small.push(1);
    small.push(2);
    small.push(3);
    try {
        small.push(4);
    } catch (const std::length_error& e) {
        std::cout << "  caught: " << e.what() << '\n';
    }
    std::cout << "  pop: " << small.pop() << ", size now " << small.size()
              << ", capacity " << FixedStack<int, 3>::capacity() << '\n';
    std::cout << "  default capacity: " << FixedStack<double>::capacity() << '\n';
    std::cout << "  sizeof(FixedStack<int, 3>) = " << sizeof(FixedStack<int, 3>)
              << " (array inside the object, no heap)\n";

    std::cout << "\n=== Out-of-Class Definitions ===\n";
    Pair<int, std::string> entry{7, "seven"};
    std::cout << "  " << entry.describe() << '\n';
}
