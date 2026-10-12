// ============================================================
// Initialisation Forms and Narrowing
// ============================================================
// C++ has several ways to give a variable its first value, and
// they do not all behave the same. Brace initialisation {} is
// the modern default because it refuses narrowing conversions.
//
// Build and run:
//   cmake --build build --target basics_07_initialisation
//   ./build/bin/basics_07_initialisation
// ============================================================

#include <iostream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// The Forms
// ------------------------------------------------------------
//   int a;          default initialisation: a local int holds garbage
//   int b = 5;      copy initialisation
//   int c(5);       direct initialisation
//   int d{5};       direct list initialisation (braces)
//   int e = {5};    copy list initialisation
//   int f{};        value initialisation: zero for built-in types
//
// Reading an uninitialised local (like "a" above) is undefined
// behaviour. Globals and statics are zero-initialised first.

int g_counter;   // global: zero-initialised, so this is 0

void forms()
{
    std::cout << "=== Initialisation Forms ===\n";

    int b = 5;
    int c(5);
    int d{5};
    int e = {5};
    int f{};          // 0
    double g{};       // 0.0
    int* h{};         // nullptr

    std::cout << b << ' ' << c << ' ' << d << ' ' << e << ' ' << f << ' ' << g
              << " h is null: " << std::boolalpha << (h == nullptr) << '\n';
    std::cout << "global g_counter = " << g_counter << '\n';

    // std::string has a constructor, so "default" initialisation
    // runs it: an empty string, never garbage.
    std::string s;
    std::cout << "default std::string is empty: " << s.empty() << '\n';
}

// ------------------------------------------------------------
// Narrowing
// ------------------------------------------------------------
// Braces reject conversions that can lose information:
//
//   int n{3.7};          // error: narrowing double to int
//   char c{300};         // error: 300 does not fit in char
//   unsigned u{-1};      // error: negative to unsigned
//
// The older forms silently convert (compilers may warn):
//
//   int n = 3.7;         // n == 3, fraction lost
//
// When you DO want the conversion, say so with static_cast.

void narrowing()
{
    std::cout << "\n=== Narrowing ===\n";

    double price = 3.7;
    int whole = static_cast<int>(price);   // explicit and searchable
    std::cout << "static_cast<int>(3.7) = " << whole << '\n';   // 3

    // A constant that fits is not narrowing, even across types:
    char small{65};                         // fine, 65 fits in char
    std::cout << "char small{65} = " << small << '\n';         // A
}

// ------------------------------------------------------------
// Aggregates and Designated Initialisers
// ------------------------------------------------------------
// An aggregate is a plain struct or array with no user-declared
// constructors, no private data and no virtual functions. It can
// be initialised member by member with braces, like in C.
//
// C++20 adds designated initialisers. Unlike C99 they must appear
// in declaration order and cannot be mixed with positional values.

struct Point {
    int x;
    int y;
};

struct Config {
    std::string host = "localhost";   // default member initialiser
    int port = 8080;
    bool verbose = false;
};

void aggregates()
{
    std::cout << "\n=== Aggregates ===\n";

    Point origin{};            // {0, 0}
    Point p{3, 4};
    Point q = {5, 6};
    int grid[3] = {1, 2, 3};

    std::cout << "origin = (" << origin.x << ", " << origin.y << ")\n";
    std::cout << "p = (" << p.x << ", " << p.y << "), q = (" << q.x << ", " << q.y << ")\n";
    std::cout << "grid[2] = " << grid[2] << '\n';

    Config defaults{};
    Config custom{.host = "example.org", .port = 443, .verbose = true};
    // Config wrong{.port = 443, .host = "x"};   // error in C++: out of order

    std::cout << "defaults: " << defaults.host << ':' << defaults.port << '\n';
    std::cout << "custom:   " << custom.host << ':' << custom.port
              << " verbose=" << custom.verbose << '\n';
}

// ------------------------------------------------------------
// The initializer_list Trap
// ------------------------------------------------------------
// If a class has a constructor taking std::initializer_list,
// braces prefer it. std::vector is the famous example:
//
//   std::vector<int> a(3, 1);   // three elements: 1 1 1
//   std::vector<int> b{3, 1};   // two elements:   3 1

void initializer_list_trap()
{
    std::cout << "\n=== vector(3, 1) vs vector{3, 1} ===\n";

    std::vector<int> a(3, 1);
    std::vector<int> b{3, 1};

    std::cout << "a(3, 1) size " << a.size() << ": ";
    for (int v : a) std::cout << v << ' ';
    std::cout << "\nb{3, 1} size " << b.size() << ": ";
    for (int v : b) std::cout << v << ' ';
    std::cout << '\n';
}

// ------------------------------------------------------------
// The Most Vexing Parse
// ------------------------------------------------------------
// Anything that CAN be read as a declaration IS a declaration:
//
//   std::string s();      // declares a FUNCTION returning std::string
//
// Braces avoid it: std::string s{};  is always an object.

void most_vexing_parse()
{
    std::cout << "\n=== Most Vexing Parse ===\n";
    std::string s{};
    std::cout << "std::string s{} is an empty object, size " << s.size() << '\n';
}

// ------------------------------------------------------------
// Static Locals
// ------------------------------------------------------------
// A static local is initialised the first time control passes
// through it, exactly once, and that is thread-safe since C++11.

int next_id()
{
    static int id = 0;   // initialised once, lives until program exit
    return ++id;
}

int main()
{
    forms();
    narrowing();
    aggregates();
    initializer_list_trap();
    most_vexing_parse();

    std::cout << "\n=== Static Locals ===\n";
    int first = next_id();
    int second = next_id();
    int third = next_id();
    std::cout << first << ' ' << second << ' ' << third << '\n';   // 1 2 3
}
