// ============================================================
// Function Pointers, Functors and Callables
// ============================================================
// C passes behaviour around with function pointers (qsort is the
// textbook example). C++ adds function objects (functors),
// lambdas, member function pointers and std::invoke to call
// any of them the same way.
//
// Build and run:
//   cmake --build build --target functions_03_function_pointers
//   ./build/bin/functions_03_function_pointers
// ============================================================

#include <algorithm>
#include <cstdlib>      // std::qsort
#include <functional>   // std::invoke, std::function
#include <iostream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Function Pointers
// ------------------------------------------------------------
// The type of a pointer to "int f(int, int)" is int (*)(int, int).
// A using alias makes it readable.

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

using BinaryOp = int (*)(int, int);

int apply(BinaryOp op, int a, int b) { return op(a, b); }

void function_pointers()
{
    std::cout << "=== Function Pointers ===\n";

    BinaryOp op = add;          // a function name decays to a pointer, like an array
    std::cout << "apply(add, 3, 4)      = " << apply(op, 3, 4) << '\n';
    op = &multiply;             // & is optional
    std::cout << "apply(multiply, 3, 4) = " << apply(op, 3, 4) << '\n';

    // A table of operations: a common C pattern for simple dispatch.
    BinaryOp table[] = {add, multiply};
    for (BinaryOp f : table) {
        std::cout << f(5, 6) << ' ';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// qsort (C) vs std::sort (C++)
// ------------------------------------------------------------
// qsort takes void* and a comparison function returning <0, 0, >0.
// It cannot inline the comparison and loses all type safety.
// std::sort is a template: the comparison is usually inlined and
// it is typically faster as well as safer.

int compare_ints(const void* a, const void* b)
{
    int lhs = *static_cast<const int*>(a);
    int rhs = *static_cast<const int*>(b);
    return (lhs > rhs) - (lhs < rhs);   // avoids the overflow in "return lhs - rhs;"
}

void qsort_vs_sort()
{
    std::cout << "\n=== qsort vs std::sort ===\n";

    int c_array[] = {42, 7, 19, 3};
    std::qsort(c_array, 4, sizeof(int), compare_ints);
    for (int v : c_array) std::cout << v << ' ';
    std::cout << " (qsort)\n";

    std::vector<int> values{42, 7, 19, 3};
    std::sort(values.begin(), values.end());
    for (int v : values) std::cout << v << ' ';
    std::cout << " (std::sort)\n";
}

// ------------------------------------------------------------
// Function Objects (Functors)
// ------------------------------------------------------------
// Any class with operator() can be called like a function, and
// unlike a function pointer it can carry state. A lambda is the
// compiler writing one of these for you.

class ThresholdCounter {
public:
    explicit ThresholdCounter(int threshold) : threshold_{threshold} {}

    bool operator()(int value)
    {
        if (value > threshold_) {
            ++hits_;
            return true;
        }
        return false;
    }

    int hits() const { return hits_; }

private:
    int threshold_;
    int hits_ = 0;
};

void functors()
{
    std::cout << "\n=== Functors ===\n";

    ThresholdCounter over_ten{10};
    for (int v : {5, 12, 30, 8, 11}) {
        over_ten(v);
    }
    std::cout << "values over 10: " << over_ten.hits() << '\n';   // 3

    // Standard functors from <functional>:
    std::vector<int> values{3, 1, 2};
    std::sort(values.begin(), values.end(), std::greater<>{});
    for (int v : values) std::cout << v << ' ';
    std::cout << "(sorted with std::greater<>)\n";
}

// ------------------------------------------------------------
// Member Function Pointers and std::invoke
// ------------------------------------------------------------
// A pointer to a member function needs an object to be called on,
// using the awkward (obj.*ptr)(args) syntax. std::invoke (C++17)
// calls anything callable with one uniform syntax.

struct Robot {
    std::string name;
    std::string speak(const std::string& word) const { return name + " says " + word; }
};

void member_pointers()
{
    std::cout << "\n=== Member Function Pointers and std::invoke ===\n";

    Robot robot{"R2"};
    std::string (Robot::*method)(const std::string&) const = &Robot::speak;

    std::cout << (robot.*method)("hello") << '\n';                 // raw syntax
    std::cout << std::invoke(method, robot, "beep") << '\n';       // same thing, readable
    std::cout << std::invoke(&Robot::name, robot) << " (data member via invoke)\n";
    std::cout << std::invoke(add, 1, 2) << " (free function via invoke)\n";

    // std::function can hold any of these, as long as the signature matches.
    std::function<std::string(const Robot&, const std::string&)> f = &Robot::speak;
    std::cout << f(robot, "whirr") << '\n';
}

int main()
{
    function_pointers();
    qsort_vs_sort();
    functors();
    member_pointers();
}
