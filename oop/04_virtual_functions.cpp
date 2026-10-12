// ============================================================
// Virtual Functions, Abstract Classes and the vtable
// ============================================================
// In Java and C# every method is virtual by default (C# needs
// "virtual" too, Java does not). In C++ functions are NOT virtual
// unless you say so: you only pay for dynamic dispatch where you
// ask for it. This file covers virtual, override, final, pure
// virtual (abstract) classes, virtual destructors, how the vtable
// works, and dynamic_cast.
//
// Build and run:
//   cmake --build build --target oop_04_virtual_functions
//   ./build/bin/oop_04_virtual_functions
// ============================================================

#include <iostream>
#include <memory>
#include <numbers>    // std::numbers::pi (C++20)
#include <string>
#include <typeinfo>
#include <vector>

// ------------------------------------------------------------
// An Abstract Base Class
// ------------------------------------------------------------
// "= 0" makes a function PURE virtual: no implementation here,
// and the class cannot be instantiated. This is C++'s interface /
// abstract class (no separate "interface" keyword).
//
// A class used polymorphically MUST have a virtual destructor.
// Otherwise "delete base_ptr" (or a unique_ptr<Shape>) destroys
// only the Shape part of a Circle: undefined behaviour.

class Shape {
public:
    virtual ~Shape() = default;

    virtual double area() const = 0;                 // pure virtual
    virtual std::string name() const { return "shape"; }   // virtual with a default

    // Non-virtual function calling virtual ones: the "template
    // method" pattern. Derived classes customise the steps.
    std::string summary() const { return name() + " with area " + std::to_string(area()); }
};

// ------------------------------------------------------------
// override and final
// ------------------------------------------------------------
// override: the compiler checks a base virtual with this exact
// signature exists. Without it, a typo (a missing const, a
// different parameter type) silently creates a NEW function.
// final on a function: no further overriding.
// final on a class: no further derivation (and lets the compiler
// devirtualise calls).

class Circle : public Shape {
public:
    explicit Circle(double radius) : radius_{radius} {}
    double area() const override { return std::numbers::pi * radius_ * radius_; }
    std::string name() const override { return "circle"; }

private:
    double radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(double width, double height) : width_{width}, height_{height} {}
    double area() const override { return width_ * height_; }
    std::string name() const override { return "rectangle"; }
    // double area() override { ... }   // error: no virtual area() WITHOUT const to override

protected:
    double width_;
    double height_;
};

class Square final : public Rectangle {
public:
    explicit Square(double side) : Rectangle{side, side} {}
    std::string name() const override { return "square"; }
};

// class Cube : public Square {};   // error: Square is final

// ------------------------------------------------------------
// Polymorphism Through Pointers and References
// ------------------------------------------------------------
// Dynamic dispatch only happens through a pointer or reference.
// A container of polymorphic objects therefore holds pointers,
// normally std::unique_ptr<Base>.

void polymorphism()
{
    std::cout << "=== Polymorphism ===\n";

    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(1.0));
    shapes.push_back(std::make_unique<Rectangle>(2.0, 3.0));
    shapes.push_back(std::make_unique<Square>(2.0));

    for (const auto& shape : shapes) {
        std::cout << "  " << shape->summary() << '\n';
    }
    // Shape s;   // error: cannot instantiate an abstract class
}   // virtual destructor: each object is destroyed correctly

// ------------------------------------------------------------
// The vtable Model
// ------------------------------------------------------------
// The standard does not mandate it, but every major compiler
// implements virtual functions the same way:
//
//   - each polymorphic CLASS has one static table of function
//     pointers, the vtable: one slot per virtual function
//   - each OBJECT gets a hidden pointer, the vptr, set by the
//     constructor to point at its class's vtable
//   - a virtual call  shape->area()  becomes roughly
//         (*shape->vptr[slot_of_area])(shape)
//     one extra load and an indirect call
//
//       Circle object          Circle vtable
//       +-----------+          +---------------------+
//       | vptr -----+--------> | ~Circle             |
//       | radius_   |          | Circle::area        |
//       +-----------+          | Circle::name        |
//                              +---------------------+
//
// Costs: one pointer per object, an indirect call that usually
// cannot be inlined. Usually small; it matters in tight loops.

struct Plain {
    double value;
    double get() const { return value; }
};

struct Polymorphic {
    double value;
    virtual ~Polymorphic() = default;
    virtual double get() const { return value; }
};

void vtable_cost()
{
    std::cout << "\n=== The Hidden vptr ===\n";
    std::cout << "  sizeof(Plain)       = " << sizeof(Plain) << '\n';         // 8
    std::cout << "  sizeof(Polymorphic) = " << sizeof(Polymorphic)
              << " (includes the vptr)\n";                                     // 16 on 64-bit
}

// ------------------------------------------------------------
// Virtual Calls in Constructors and Destructors
// ------------------------------------------------------------
// While Base's constructor runs, the object IS a Base: the vptr
// points at Base's vtable. A virtual call there does NOT reach the
// derived override (Java and C# do dispatch to it, which is
// dangerous in its own way). Calling a PURE virtual there is
// undefined behaviour.

class Base {
public:
    Base() { std::cout << "  Base constructor calls hello(): " << hello() << '\n'; }
    virtual ~Base() = default;
    virtual std::string hello() const { return "Base::hello"; }
};

class Derived : public Base {
public:
    Derived() { std::cout << "  Derived constructor calls hello(): " << hello() << '\n'; }
    std::string hello() const override { return "Derived::hello"; }
};

// ------------------------------------------------------------
// Default Arguments Are Not Virtual
// ------------------------------------------------------------
// The function is chosen at run time, but the default argument is
// chosen at compile time from the STATIC type. Never give an
// override a different default.

class Printer {
public:
    virtual ~Printer() = default;
    virtual void print(int copies = 1) const { std::cout << "  Printer copies=" << copies << '\n'; }
};

class FancyPrinter : public Printer {
public:
    void print(int copies = 5) const override { std::cout << "  FancyPrinter copies=" << copies << '\n'; }
};

// ------------------------------------------------------------
// dynamic_cast and typeid (RTTI)
// ------------------------------------------------------------
// dynamic_cast<Derived*>(base_ptr) checks at run time and returns
// nullptr if the object is not a Derived (or throws std::bad_cast
// for references). Needing it often suggests a missing virtual
// function, but it is the right tool for optional capabilities.

void rtti()
{
    std::cout << "\n=== dynamic_cast and typeid ===\n";

    std::unique_ptr<Shape> shape = std::make_unique<Square>(3.0);

    if (auto* rect = dynamic_cast<Rectangle*>(shape.get())) {
        std::cout << "  it is a Rectangle (a Square is-a Rectangle), area " << rect->area() << '\n';
    }
    if (dynamic_cast<Circle*>(shape.get()) == nullptr) {
        std::cout << "  it is not a Circle\n";
    }
    const Shape& ref = *shape;
    std::cout << "  typeid(ref).name() = " << typeid(ref).name() << " (implementation-defined text)\n";
}

int main()
{
    polymorphism();
    vtable_cost();

    std::cout << "\n=== Virtual Calls During Construction ===\n";
    Derived d;

    std::cout << "\n=== Default Arguments Use the Static Type ===\n";
    FancyPrinter fancy;
    const Printer& as_base = fancy;
    as_base.print();   // FancyPrinter::print runs, but with copies=1 from Printer
    fancy.print();     // copies=5

    rtti();
}
