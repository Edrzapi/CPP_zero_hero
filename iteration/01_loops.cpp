// ============================================================
// Loops: for, while, do-while and range-based for
// ============================================================
// The three C loops plus the range-based for loop (C++11),
// which is the C++ equivalent of Java's enhanced for and
// C#'s foreach.
//
// Build and run:
//   cmake --build build --target iteration_01_loops
//   ./build/bin/iteration_01_loops
// ============================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Classic for
// ------------------------------------------------------------
// for (init; condition; step). Prefer ++i to i++ out of habit:
// for ints it makes no difference, for iterators it can avoid
// a copy.

void classic_for()
{
    std::cout << "=== Classic for ===\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    // Two variables, counting in opposite directions.
    for (int lo = 0, hi = 9; lo < hi; ++lo, --hi) {
        std::cout << '(' << lo << ',' << hi << ") ";
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// while and do-while
// ------------------------------------------------------------
// while checks first; do-while always runs at least once.

void while_loops()
{
    std::cout << "\n=== while and do-while ===\n";

    int n = 27;
    int steps = 0;
    while (n != 1) {                       // Collatz sequence
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        ++steps;
    }
    std::cout << "27 reaches 1 in " << steps << " steps\n";   // 111

    int attempts = 0;
    do {
        ++attempts;
    } while (attempts < 0);                // false at once, but the body ran
    std::cout << "do-while body ran " << attempts << " time\n";
}

// ------------------------------------------------------------
// Range-Based for
// ------------------------------------------------------------
// for (declaration : range) works on anything with begin() and
// end(): C arrays, every standard container, std::string, and
// your own classes (see 02_range_for_under_the_hood.cpp).
//
// Choose the loop variable carefully:
//   for (auto x : v)          copies each element
//   for (auto& x : v)         modifies elements in place
//   for (const auto& x : v)   reads without copying (the default choice)

void range_for()
{
    std::cout << "\n=== Range-Based for ===\n";

    std::vector<std::string> names{"Ada", "Alan", "Grace"};
    for (const auto& name : names) {
        std::cout << name << ' ';
    }
    std::cout << '\n';

    std::vector<int> prices{10, 20, 30};
    for (auto& price : prices) {
        price *= 2;                        // changes the vector
    }
    for (int price : prices) std::cout << price << ' ';
    std::cout << '\n';

    // Maps give key/value pairs; structured bindings unpack them.
    std::map<std::string, int> ages{{"Ada", 36}, {"Alan", 41}};
    for (const auto& [name, age] : ages) {
        std::cout << name << " is " << age << '\n';
    }

    // C++20: an init-statement in range-for.
    for (int index = 0; const auto& name : names) {
        std::cout << index++ << ": " << name << '\n';
    }
}

// ------------------------------------------------------------
// break, continue and Leaving Nested Loops
// ------------------------------------------------------------
// C++ has no labelled break (Java does). To leave several loops
// at once, put them in a function and return, or use a flag.
// goto works and is occasionally used in C cleanup code, but is
// rarely the clearest option in C++.

bool find_pair(const std::vector<int>& values, int target, int& a, int& b)
{
    for (std::size_t i = 0; i < values.size(); ++i) {
        for (std::size_t j = i + 1; j < values.size(); ++j) {
            if (values[i] + values[j] == target) {
                a = values[i];
                b = values[j];
                return true;               // leaves both loops
            }
        }
    }
    return false;
}

void break_and_continue()
{
    std::cout << "\n=== break and continue ===\n";

    for (int i = 1; i <= 10; ++i) {
        if (i % 2 == 0) continue;          // skip even numbers
        if (i > 7) break;                  // stop entirely
        std::cout << i << ' ';
    }
    std::cout << '\n';                     // 1 3 5 7

    int a = 0;
    int b = 0;
    if (find_pair({3, 8, 5, 12, 1}, 13, a, b)) {
        std::cout << "pair summing to 13: " << a << " + " << b << '\n';  // 8 + 5
    }
}

int main()
{
    classic_for();
    while_loops();
    range_for();
    break_and_continue();
}
