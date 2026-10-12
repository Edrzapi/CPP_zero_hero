// ============================================================
// Conditionals: if, else, the Ternary Operator
// ============================================================
// Mostly the same as C, Java and C#, plus two C++ additions:
// the if statement with an initialiser (C++17) and if constexpr
// (C++17, decided at compile time).
//
// Build and run:
//   cmake --build build --target control_flow_01_conditionals
//   ./build/bin/control_flow_01_conditionals
// ============================================================

#include <iostream>
#include <map>
#include <string>
#include <type_traits>

// ------------------------------------------------------------
// if / else if / else
// ------------------------------------------------------------
// The condition converts to bool. Any non-zero number and any
// non-null pointer is true. That is the C heritage, and why
// "if (x = 5)" (assignment) compiles; -Wall warns about it.

void basic_if(int temperature)
{
    if (temperature < 0) {
        std::cout << temperature << " is freezing\n";
    } else if (temperature < 15) {
        std::cout << temperature << " is cold\n";
    } else if (temperature < 25) {
        std::cout << temperature << " is mild\n";
    } else {
        std::cout << temperature << " is hot\n";
    }
}

void truthiness()
{
    // Truthiness, inherited from C:
    int items = 3;
    const char* name = nullptr;
    if (items) std::cout << "items is non-zero, so true\n";
    if (!name) std::cout << "name is null, so false\n";
}

// ------------------------------------------------------------
// Logical Operators and Short-Circuiting
// ------------------------------------------------------------
// && and || stop as soon as the answer is known, so the right
// side can safely depend on the left side being true.
// "and", "or", "not" are standard alternative spellings.

void short_circuit()
{
    std::cout << "\n=== Short-Circuiting ===\n";

    const std::string* maybe = nullptr;
    // Safe: the length check never runs when maybe is null.
    if (maybe != nullptr && maybe->size() > 3) {
        std::cout << "long string\n";
    } else {
        std::cout << "null or short, and no crash\n";
    }

    bool ready = true;
    bool cancelled = false;
    if (ready and not cancelled) {   // alternative tokens, same as && and !
        std::cout << "ready and not cancelled\n";
    }
}

// ------------------------------------------------------------
// The Ternary Operator
// ------------------------------------------------------------
// condition ? a : b is an EXPRESSION, so it can initialise a
// const variable, which an if statement cannot.

void ternary(int age)
{
    const std::string status = (age >= 18) ? "adult" : "minor";
    std::cout << age << " -> " << status << '\n';
}

// ------------------------------------------------------------
// if with an Initialiser (C++17)
// ------------------------------------------------------------
// if (init; condition) limits the variable's scope to the if
// and its else branches. Very common with map lookups.

void if_with_initialiser()
{
    std::cout << "\n=== if (init; condition) ===\n";

    std::map<std::string, int> stock{{"apples", 4}, {"pears", 0}};

    if (auto it = stock.find("apples"); it != stock.end()) {
        std::cout << "apples in stock: " << it->second << '\n';
    } else {
        std::cout << "no apples\n";
    }
    // "it" does not exist out here, so the name can be reused safely.

    if (auto it = stock.find("plums"); it == stock.end()) {
        std::cout << "plums not stocked\n";
    }
}

// ------------------------------------------------------------
// if constexpr (C++17)
// ------------------------------------------------------------
// Chooses a branch at COMPILE time. The discarded branch is not
// instantiated, so it may contain code that would not compile
// for this type. Used inside templates (see templates/).

template <typename T>
void describe(const T& value)
{
    if constexpr (std::is_integral_v<T>) {
        std::cout << value << " is an integer\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << value << " is floating point\n";
    } else {
        std::cout << "\"" << value << "\" is something else (size " << value.size() << ")\n";
    }
}

int main()
{
    std::cout << "=== if / else if / else ===\n";
    basic_if(-3);
    basic_if(19);
    truthiness();
    short_circuit();
    std::cout << "\n=== Ternary ===\n";
    ternary(17);
    ternary(30);
    if_with_initialiser();

    std::cout << "\n=== if constexpr ===\n";
    describe(42);
    describe(2.5);
    describe(std::string{"text"});   // .size() only compiled for this call
}
