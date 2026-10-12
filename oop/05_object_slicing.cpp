// ============================================================
// Object Slicing
// ============================================================
// Copying a derived object INTO a base object (by value) keeps
// only the base part. The derived data and the derived vtable are
// "sliced off". It cannot happen in Java or C#, where class-type
// variables are always references, so it surprises people moving
// to C++.
//
// Build and run:
//   cmake --build build --target oop_05_object_slicing
//   ./build/bin/oop_05_object_slicing
// ============================================================

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Animal {
public:
    explicit Animal(std::string name) : name_{std::move(name)} {}
    virtual ~Animal() = default;

    // Allowing copies is what makes slicing possible (see the fix below).
    Animal(const Animal&) = default;
    Animal& operator=(const Animal&) = default;

    virtual std::string speak() const { return name_ + " makes a sound"; }

protected:
    std::string name_;
};

class Dog : public Animal {
public:
    Dog(std::string name, std::string trick) : Animal{std::move(name)}, trick_{std::move(trick)} {}
    std::string speak() const override { return name_ + " barks and does " + trick_; }

private:
    std::string trick_;
};

// ------------------------------------------------------------
// Where Slicing Happens
// ------------------------------------------------------------

void by_value(Animal a) { std::cout << "  by value:     " << a.speak() << '\n'; }      // SLICED
void by_reference(const Animal& a) { std::cout << "  by reference: " << a.speak() << '\n'; }

void slicing_examples()
{
    std::cout << "=== Slicing ===\n";
    Dog rex{"Rex", "roll over"};

    by_value(rex);          // copies only the Animal part: Animal::speak runs
    by_reference(rex);      // no copy: Dog::speak runs

    Animal copy = rex;      // sliced at the assignment
    std::cout << "  Animal copy:  " << copy.speak() << '\n';

    // A container of base objects slices every element.
    std::vector<Animal> zoo;
    zoo.push_back(rex);
    std::cout << "  vector<Animal>: " << zoo[0].speak() << '\n';

    // Store pointers to keep the dynamic type.
    std::vector<std::unique_ptr<Animal>> real_zoo;
    real_zoo.push_back(std::make_unique<Dog>("Fido", "fetch"));
    std::cout << "  vector<unique_ptr<Animal>>: " << real_zoo[0]->speak() << '\n';
}

// ------------------------------------------------------------
// Preventing It
// ------------------------------------------------------------
// - pass polymorphic objects by reference or pointer
// - store them through pointers (unique_ptr / shared_ptr)
// - make polymorphic base classes non-copyable, so slicing is a
//   compile error instead of a silent bug:
//       Base(const Base&) = delete;  Base& operator=(const Base&) = delete;
//   or make them protected so only derived classes can copy
// - if you need copies, add a virtual clone() (below)

class Shape {
public:
    virtual ~Shape() = default;
    Shape(const Shape&) = delete;              // no slicing possible
    Shape& operator=(const Shape&) = delete;

    virtual std::unique_ptr<Shape> clone() const = 0;   // "virtual copy constructor"
    virtual std::string name() const = 0;

protected:
    Shape() = default;
};

class Hexagon : public Shape {
public:
    explicit Hexagon(int id) : id_{id} {}
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Hexagon>(id_); }
    std::string name() const override { return "hexagon #" + std::to_string(id_); }

private:
    int id_;
};

void clone_example()
{
    std::cout << "\n=== Copying Through a Virtual clone() ===\n";
    std::unique_ptr<Shape> original = std::make_unique<Hexagon>(6);
    std::unique_ptr<Shape> copy = original->clone();   // full Hexagon, not sliced
    std::cout << "  copy is a " << copy->name() << '\n';
    // Shape s = *original;   // error: copy constructor is deleted
}

int main()
{
    slicing_examples();
    clone_example();
}
