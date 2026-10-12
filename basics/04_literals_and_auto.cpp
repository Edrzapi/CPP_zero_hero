// ============================================================
// Literals, auto and decltype
// ============================================================
// How to write values of each type, and how the compiler
// deduces types for you with auto and decltype.
//
// Build and run:
//   cmake --build build --target basics_04_literals_and_auto
//   ./build/bin/basics_04_literals_and_auto
// ============================================================

#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <vector>

using namespace std::string_literals;       // enables "text"s
using namespace std::string_view_literals;  // enables "text"sv

// ------------------------------------------------------------
// Integer Literals
// ------------------------------------------------------------
// Bases:    42 (decimal), 052 (octal!), 0x2A (hex), 0b101010 (binary, C++14)
// Suffixes: u (unsigned), l (long), ll (long long), combinations like ull
// Digit separators (C++14): 1'000'000
//
// A leading 0 means OCTAL, inherited from C. 010 is eight.

void integer_literals()
{
    std::cout << "=== Integer Literals ===\n";
    std::cout << "42        = " << 42 << '\n';
    std::cout << "052       = " << 052 << "  (octal)\n";      // 42
    std::cout << "0x2A      = " << 0x2A << '\n';              // 42
    std::cout << "0b101010  = " << 0b101010 << '\n';          // 42
    std::cout << "1'000'000 = " << 1'000'000 << '\n';

    // The literal's type comes from its value and suffix.
    static_assert(std::is_same_v<decltype(42), int>);
    static_assert(std::is_same_v<decltype(42u), unsigned int>);
    static_assert(std::is_same_v<decltype(42L), long>);
    static_assert(std::is_same_v<decltype(42ULL), unsigned long long>);
    std::cout << "static_assert checks on literal types passed at compile time\n";
}

// ------------------------------------------------------------
// Floating, Character and String Literals
// ------------------------------------------------------------

void other_literals()
{
    std::cout << "\n=== Floating, Character and String Literals ===\n";

    double d = 3.14;        // double by default
    float f = 3.14f;        // f suffix; "float f = 3.14;" narrows (MSVC warns C4305)
    double sci = 6.02e23;   // scientific notation
    std::cout << d << ' ' << f << ' ' << sci << '\n';

    char c = 'A';                // char literal: single quotes
    char newline = '\n';         // escape sequence
    std::cout << "char 'A' as int: " << static_cast<int>(c)
              << ", newline as int: " << static_cast<int>(newline) << '\n';

    // "text" is a C string: type const char[N] including the '\0'.
    std::cout << "sizeof(\"abc\") = " << sizeof("abc") << " (3 chars + terminator)\n";

    // "text"s is a std::string, "text"sv is a std::string_view.
    auto as_string = "hello"s;
    auto as_view = "hello"sv;
    static_assert(std::is_same_v<decltype(as_string), std::string>);
    static_assert(std::is_same_v<decltype(as_view), std::string_view>);
    std::cout << as_string << ' ' << as_view << '\n';

    // Raw string literals: no escapes needed. Handy for paths and regex.
    const char* path = R"(C:\Users\trainer\notes.txt)";
    const char* json = R"({"name": "Ada", "lang": "C++"})";
    std::cout << path << '\n' << json << '\n';

    // A custom delimiter lets the text itself contain )"
    const char* tricky = R"xyz(contains )" inside)xyz";
    std::cout << tricky << '\n';
}

// ------------------------------------------------------------
// auto
// ------------------------------------------------------------
// auto deduces the type from the initialiser, using the same
// rules as template argument deduction:
//   - top-level const is DROPPED
//   - references are DROPPED (auto makes a copy)
// Write const auto& or auto& when you want those back.

void auto_deduction()
{
    std::cout << "\n=== auto ===\n";

    auto i = 42;            // int
    auto d = 42.0;          // double
    auto s = "text";        // const char*  (NOT std::string)
    auto str = "text"s;     // std::string
    static_assert(std::is_same_v<decltype(s), const char*>);

    const int ci = 10;
    auto copy = ci;                 // int: top-level const dropped
    const auto& ref = ci;           // const int&
    static_assert(std::is_same_v<decltype(copy), int>);
    static_assert(std::is_same_v<decltype(ref), const int&>);

    std::vector<std::string> names{"Ada", "Bjarne", "Grace"};
    for (const auto& name : names) {    // const auto& avoids copying each string
        std::cout << name << ' ';
    }
    std::cout << '\n';

    // Brace initialisation with auto:
    auto a{5};        // int (C++17 rule)
    auto b = {5};     // std::initializer_list<int>, usually not what you want
    static_assert(std::is_same_v<decltype(a), int>);
    std::cout << "auto b = {5} has size " << b.size() << '\n';

    std::cout << i << ' ' << d << ' ' << str << ' ' << copy << ' ' << ref << '\n';
}

// ------------------------------------------------------------
// decltype
// ------------------------------------------------------------
// decltype(name) gives the DECLARED type of a variable, references
// and const included. decltype((name)) with extra parentheses is an
// expression and gives an lvalue reference. That difference matters
// when writing generic code.

void decltype_rules()
{
    std::cout << "\n=== decltype ===\n";

    int x = 1;
    const int& rx = x;

    static_assert(std::is_same_v<decltype(x), int>);
    static_assert(std::is_same_v<decltype((x)), int&>);       // parentheses change the answer
    static_assert(std::is_same_v<decltype(rx), const int&>);
    static_assert(std::is_same_v<decltype(x + 1), int>);      // a prvalue expression

    // decltype(auto) keeps references when deducing a return type.
    std::cout << "decltype checks passed; typeid(x).name() = " << typeid(x).name() << '\n';
    // typeid().name() is implementation-defined: "i" on GCC/Clang, "int" on MSVC.
}

int main()
{
    integer_literals();
    other_literals();
    auto_deduction();
    decltype_rules();
}
