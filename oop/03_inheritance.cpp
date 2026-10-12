// ============================================================
// Inheritance
// ============================================================
// Deriving one class from another, calling base constructors,
// construction and destruction order, protected access, inherited
// constructors, and name hiding. Virtual functions (needed for
// run-time polymorphism) are in the next file.
//
// Build and run:
//   cmake --build build --target oop_03_inheritance
//   ./build/bin/oop_03_inheritance
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// A Base Class
// ------------------------------------------------------------
// protected: accessible to derived classes, private to everyone
// else. Use sparingly: protected data couples every derived class
// to the base's implementation. Protected FUNCTIONS are fine.

class Vehicle {
public:
    Vehicle(std::string make, int wheels) : make_{std::move(make)}, wheels_{wheels}
    {
        std::cout << "  Vehicle constructor (" << make_ << ")\n";
    }
    ~Vehicle() { std::cout << "  Vehicle destructor\n"; }

    std::string describe() const { return make_ + " with " + std::to_string(wheels_) + " wheels"; }
    void honk() const { std::cout << "  Vehicle::honk: beep\n"; }
    void honk(int times) const { std::cout << "  Vehicle::honk(" << times << ")\n"; }

protected:
    const std::string& make() const { return make_; }

private:
    std::string make_;
    int wheels_;
};

// ------------------------------------------------------------
// Public Inheritance: "is-a"
// ------------------------------------------------------------
// class Car : public Vehicle  means a Car IS a Vehicle and can be
// used wherever a Vehicle& or Vehicle* is expected.
// The base is constructed FIRST (via the initialiser list) and
// destroyed LAST.
//
// Default inheritance for "class" is private; for "struct" it is
// public. Always write "public" explicitly.

class Car : public Vehicle {
public:
    Car(std::string make, int doors) : Vehicle{std::move(make), 4}, doors_{doors}
    {
        std::cout << "  Car constructor\n";
    }
    ~Car() { std::cout << "  Car destructor\n"; }

    std::string details() const
    {
        // make() is protected in Vehicle: accessible here.
        return make() + " car, " + std::to_string(doors_) + " doors";
    }

    // ---- Name Hiding ----
    // Declaring honk() here HIDES every Vehicle::honk overload,
    // including honk(int). "using Vehicle::honk;" brings them back.
    using Vehicle::honk;
    void honk() const { std::cout << "  Car::honk: parp parp\n"; }

private:
    int doors_;
};

// ------------------------------------------------------------
// Inheriting Constructors
// ------------------------------------------------------------
// "using Base::Base;" reuses the base constructors when the
// derived class adds no data needing initialisation.

class Motorbike : public Vehicle {
public:
    using Vehicle::Vehicle;
    void wheelie() const { std::cout << "  " << make() << " does a wheelie\n"; }
};

// ------------------------------------------------------------
// Composition vs Inheritance
// ------------------------------------------------------------
// "has-a" relationships should be members, not bases. A Garage
// HAS vehicles; it is not a kind of vehicle. Prefer composition
// unless you need substitutability.
//
// private inheritance means "implemented in terms of" and is
// almost always better written as a private member.

void use_as_vehicle(const Vehicle& v)
{
    std::cout << "  as a Vehicle: " << v.describe() << '\n';
    v.honk();   // NOT virtual: always calls Vehicle::honk, even for a Car
}

int main()
{
    std::cout << "=== Construction Order: Base First ===\n";
    {
        Car car{"Mini", 3};
        std::cout << "  " << car.details() << '\n';

        std::cout << "\n=== Name Hiding and using ===\n";
        car.honk();
        car.honk(2);   // only compiles because of "using Vehicle::honk;"

        std::cout << "\n=== Passing a Car as a Vehicle& ===\n";
        use_as_vehicle(car);
        std::cout << "  (Vehicle::honk ran because honk is not virtual: see 04_virtual_functions)\n";

        std::cout << "\n=== Destruction Order: Derived First ===\n";
    }

    std::cout << "\n=== Inherited Constructors ===\n";
    Motorbike bike{"Ducati", 2};
    bike.wheelie();
    std::cout << "  end of main\n";
}
