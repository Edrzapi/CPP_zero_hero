// ============================================================
// Stack vs Heap, new and delete
// ============================================================
// C++ has no garbage collector. Every object has a clear
// lifetime decided by HOW it was created:
//
//   automatic (stack)  created at its declaration, destroyed at
//                      the closing brace of its scope
//   dynamic (heap)     created with new, destroyed with delete,
//                      whenever you say (or never: a leak)
//   static             created before main (or on first use for
//                      static locals), destroyed after main
//
// Java and C# put every object on the heap and every local
// reference on the stack. In C++ an object of class type can live
// directly on the stack, which is both faster and safer.
//
// Build and run:
//   cmake --build build --target memory_01_stack_and_heap
//   ./build/bin/memory_01_stack_and_heap
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// A Class That Reports Its Lifetime
// ------------------------------------------------------------
// The constructor runs when the object is created, the
// destructor (~Tracer) when it is destroyed. Printing from both
// makes lifetimes visible.

class Tracer {
public:
    explicit Tracer(std::string name) : name_{std::move(name)}
    {
        std::cout << "  + construct " << name_ << '\n';
    }

    ~Tracer() { std::cout << "  - destroy   " << name_ << '\n'; }

    // Copying is disabled to keep the output simple (see 04_rule_of...).
    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;

    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ------------------------------------------------------------
// Automatic (Stack) Lifetime
// ------------------------------------------------------------
// Destruction happens in REVERSE order of construction, at the
// end of the enclosing scope, even if an exception is thrown.
// This is the foundation of RAII (next file).

void automatic_lifetime()
{
    std::cout << "=== Stack Objects ===\n";
    Tracer outer{"outer"};
    {
        Tracer inner1{"inner1"};
        Tracer inner2{"inner2"};
        std::cout << "  ... end of inner scope\n";
    }   // inner2 then inner1 destroyed here
    std::cout << "  ... end of function\n";
}   // outer destroyed here

// ------------------------------------------------------------
// Dynamic (Heap) Lifetime with new and delete
// ------------------------------------------------------------
// new allocates memory AND runs the constructor.
// delete runs the destructor AND frees the memory.
// new[] must be matched with delete[].
//
// Raw new/delete is shown here so you recognise it in existing
// code. In new code use std::make_unique / std::make_shared
// (03_smart_pointers.cpp); the C++ Core Guidelines say "no naked
// new".

Tracer* create_on_heap()
{
    return new Tracer{"heap object"};   // outlives this function
}

void dynamic_lifetime()
{
    std::cout << "\n=== Heap Objects ===\n";
    Tracer* p = create_on_heap();
    std::cout << "  using " << p->name() << '\n';
    delete p;          // forget this and the destructor never runs: a leak
    p = nullptr;       // optional, avoids accidental reuse of a dangling pointer

    int* numbers = new int[3]{1, 2, 3};
    std::cout << "  numbers[2] = " << numbers[2] << '\n';
    delete[] numbers;  // delete (without []) here would be undefined behaviour
}

// ------------------------------------------------------------
// What Goes Wrong
// ------------------------------------------------------------
// Each of these compiles. All are undefined behaviour or leaks,
// and AddressSanitizer catches them (under_the_hood/ub_examples).
//
//   int* p = new int{5}; delete p; *p = 6;      use after free
//   int* p = new int{5}; delete p; delete p;    double free
//   int* p = new int[3]; delete p;              mismatched delete
//   void leak() { new int[1000]; }              memory leak
//   int& dangling() { int x = 1; return x; }    dangling reference to the stack
//
// Smart pointers and containers make all of these hard to write.

// ------------------------------------------------------------
// Static Lifetime
// ------------------------------------------------------------

Tracer& logger()
{
    static Tracer instance{"static logger"};   // created on first call only
    return instance;
}

int main()
{
    automatic_lifetime();
    dynamic_lifetime();

    std::cout << "\n=== Static Objects ===\n";
    Tracer& first = logger();   // constructs on this first call
    std::cout << "  first use of " << first.name() << '\n';
    Tracer& second = logger();  // same object, no construction
    std::cout << "  second use of " << second.name() << " (not constructed again)\n";
    std::cout << "end of main: the static logger is destroyed after this line\n";
}
