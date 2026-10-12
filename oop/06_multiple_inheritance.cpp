// ============================================================
// Multiple Inheritance, the Diamond and Virtual Inheritance
// ============================================================
// C++ allows a class to have several base classes. Java and C#
// allow only one base class plus any number of interfaces. The
// C++ equivalent of an interface is a class with only pure virtual
// functions, and inheriting several of those is common and safe.
// Inheriting several classes WITH DATA is where the diamond
// problem appears.
//
// Build and run:
//   cmake --build build --target oop_06_multiple_inheritance
//   ./build/bin/oop_06_multiple_inheritance
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// Interface-Style Multiple Inheritance
// ------------------------------------------------------------
// Pure abstract classes ("interfaces"): no data, so no ambiguity.

class Printable {
public:
    virtual ~Printable() = default;
    virtual std::string to_text() const = 0;
};

class Saveable {
public:
    virtual ~Saveable() = default;
    virtual bool save(const std::string& path) const = 0;
};

class Invoice : public Printable, public Saveable {
public:
    explicit Invoice(int number) : number_{number} {}
    std::string to_text() const override { return "Invoice #" + std::to_string(number_); }
    bool save(const std::string& path) const override
    {
        std::cout << "  saving " << to_text() << " to " << path << '\n';
        return true;
    }

private:
    int number_;
};

void print_it(const Printable& p) { std::cout << "  printing: " << p.to_text() << '\n'; }
void save_it(const Saveable& s) { s.save("invoices/42.txt"); }

// ------------------------------------------------------------
// The Diamond Problem
// ------------------------------------------------------------
/*
            Device
           /      \
      Scanner    Printer
           \      /
          Photocopier
*/
// The diagram is a /* */ comment because a // comment line
// ending in a backslash continues onto the next line (GCC
// and Clang warn about it).
//
// Without virtual inheritance a Photocopier contains TWO Device
// sub-objects (one via Scanner, one via Printer). Any access to a
// Device member is ambiguous, and the two copies can disagree.

namespace without_virtual {

struct Device {
    explicit Device(std::string name) : id{std::move(name)} {}
    std::string id;
};

struct Scanner : Device {
    Scanner() : Device{"scanner-side"} {}
};

struct Printer : Device {
    Printer() : Device{"printer-side"} {}
};

struct Photocopier : Scanner, Printer {};

void demo()
{
    std::cout << "\n=== Diamond WITHOUT virtual Inheritance ===\n";
    Photocopier copier;
    // std::cout << copier.id;   // error: ambiguous, which Device?
    std::cout << "  Scanner::id = " << copier.Scanner::id << '\n';
    std::cout << "  Printer::id = " << copier.Printer::id << '\n';
    std::cout << "  sizeof(Photocopier) holds two Devices: " << sizeof(Photocopier)
              << " bytes vs sizeof(Device) " << sizeof(Device) << '\n';
}

}  // namespace without_virtual

// ------------------------------------------------------------
// Virtual Inheritance
// ------------------------------------------------------------
// "virtual public Device" means: share ONE Device sub-object
// among everyone who inherits it virtually.
//
// The catch: the MOST DERIVED class constructs the virtual base
// directly. Scanner's and Printer's calls to Device(...) are
// ignored when building a Photocopier. If the most derived class
// does not call it, Device's default constructor is used (and if
// there is none, it is a compile error).
//
// Cost: an extra pointer or offset per object and a slower path
// to reach the base. Used by std::iostream itself (basic_iostream
// inherits basic_istream and basic_ostream, both virtually
// derived from basic_ios).

namespace with_virtual {

struct Device {
    explicit Device(std::string name) : id{std::move(name)}
    {
        std::cout << "  Device constructed with \"" << this->id << "\"\n";
    }
    std::string id;
};

struct Scanner : virtual Device {
    Scanner() : Device{"scanner-side"} {}   // ignored when part of a Photocopier
};

struct Printer : virtual Device {
    Printer() : Device{"printer-side"} {}   // ignored when part of a Photocopier
};

struct Photocopier : Scanner, Printer {
    Photocopier() : Device{"photocopier"} {}   // the most derived class decides
};

void demo()
{
    std::cout << "\n=== Diamond WITH virtual Inheritance ===\n";
    Photocopier copier;
    std::cout << "  copier.id = " << copier.id << " (one shared Device, no ambiguity)\n";

    Scanner standalone;   // on its own, Scanner does construct Device itself
    std::cout << "  standalone Scanner id = " << standalone.id << '\n';
}

}  // namespace with_virtual

// ------------------------------------------------------------
// Guidance
// ------------------------------------------------------------
// - Multiple inheritance of interfaces: fine, common.
// - Multiple inheritance of implementation: think twice, prefer
//   composition.
// - Virtual inheritance: only when you genuinely have a diamond
//   with shared state. Rare in application code.

int main()
{
    std::cout << "=== Implementing Two Interfaces ===\n";
    Invoice invoice{42};
    print_it(invoice);
    save_it(invoice);

    without_virtual::demo();
    with_virtual::demo();
}
