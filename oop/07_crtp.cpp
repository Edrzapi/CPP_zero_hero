// ============================================================
// Static vs Dynamic Polymorphism: CRTP
// ============================================================
// Dynamic polymorphism (virtual functions) picks the function at
// RUN time through the vtable. Static polymorphism picks it at
// COMPILE time through templates: no vptr, no indirect call, and
// the call can be inlined. The trade-off: no common base type, so
// you cannot put different types in one container.
//
// The Curiously Recurring Template Pattern (CRTP) is the classic
// way to do static polymorphism with inheritance:
//
//   template <typename Derived> class Base { ... };
//   class Widget : public Base<Widget> { ... };   // passes ITSELF
//
// Inside Base, static_cast<Derived&>(*this) reaches the derived
// class's functions with no virtual call.
//
// Build and run:
//   cmake --build build --target oop_07_crtp
//   ./build/bin/oop_07_crtp
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// Dynamic Version (for comparison)
// ------------------------------------------------------------

class DynamicShape {
public:
    virtual ~DynamicShape() = default;
    virtual double area() const = 0;
    void describe() const { std::cout << "  dynamic: area " << area() << '\n'; }
};

class DynamicSquare : public DynamicShape {
public:
    explicit DynamicSquare(double side) : side_{side} {}
    double area() const override { return side_ * side_; }

private:
    double side_;
};

// ------------------------------------------------------------
// Static Version with CRTP
// ------------------------------------------------------------
// describe() calls derived().area(): resolved at compile time.
// If Derived forgets area(), the error appears at compile time.

template <typename Derived>
class StaticShape {
public:
    void describe() const { std::cout << "  static:  area " << derived().area() << '\n'; }

private:
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    // Private constructor + friend: only the real Derived can
    // inherit from StaticShape<Derived>. Catches the typo
    // "class Circle : public StaticShape<Square>".
    StaticShape() = default;
    friend Derived;
};

class StaticSquare : public StaticShape<StaticSquare> {
public:
    explicit StaticSquare(double side) : side_{side} {}
    double area() const { return side_ * side_; }   // not virtual

private:
    double side_;
};

// Generic code over "anything with describe()". Each call is
// compiled separately for each type.
template <typename Shape>
void show(const Shape& shape)
{
    shape.describe();
}

// ------------------------------------------------------------
// CRTP Mixins: Adding Behaviour
// ------------------------------------------------------------
// A common real use: inject operators or utilities into many
// classes. Here a mixin adds a counter of live objects to any
// class that inherits it. Because the base is a template, every
// Derived gets its OWN static counter.

template <typename Derived>
class InstanceCounter {
public:
    static int live() { return count_; }

protected:
    InstanceCounter() { ++count_; }
    InstanceCounter(const InstanceCounter&) { ++count_; }
    InstanceCounter& operator=(const InstanceCounter&) = default;
    ~InstanceCounter() { --count_; }

private:
    static inline int count_ = 0;   // one counter PER Derived type
};

class Order : public InstanceCounter<Order> {
public:
    explicit Order(int id) : id_{id} {}
    int id() const { return id_; }

private:
    int id_;
};

class Customer : public InstanceCounter<Customer> {
public:
    explicit Customer(std::string name) : name_{std::move(name)} {}
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ------------------------------------------------------------
// Which to Use
// ------------------------------------------------------------
// virtual: heterogeneous collections, plugins, types chosen at
//          run time, stable ABI across libraries.
// CRTP / templates: types known at compile time, hot paths where
//          the indirect call matters, mixins.
// C++23 "deducing this" (explicit object parameters) makes many
// CRTP uses simpler:  void describe(this const auto& self).
// C++20 concepts (templates/05_concepts.cpp) give templates the
// clear interface that CRTP used to provide.

int main()
{
    std::cout << "=== Dynamic vs Static Dispatch ===\n";
    DynamicSquare dyn{3};
    StaticSquare stat{3};
    const DynamicShape& base_ref = dyn;
    base_ref.describe();
    show(stat);

    std::cout << "  sizeof(DynamicSquare) = " << sizeof(DynamicSquare)
              << " (vptr + double), sizeof(StaticSquare) = " << sizeof(StaticSquare) << '\n';

    std::cout << "\n=== CRTP Mixin: Per-Type Instance Counter ===\n";
    Order a{1};
    {
        Order b{2};
        Order c = b;
        Customer ada{"Ada"};
        std::cout << "  live Orders: " << Order::live() << ", live Customers: " << Customer::live() << '\n';
        std::cout << "  ids: " << a.id() << ' ' << b.id() << ' ' << c.id() << '\n';
    }
    std::cout << "  live Orders: " << Order::live() << ", live Customers: " << Customer::live() << '\n';
}
