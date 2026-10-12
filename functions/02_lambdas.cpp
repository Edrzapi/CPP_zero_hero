// ============================================================
// Lambdas and Captures
// ============================================================
// A lambda is an unnamed function object written inline. The
// capture list [ ] decides which local variables it can see and
// whether it holds copies or references.
//
// Build and run:
//   cmake --build build --target functions_02_lambdas
//   ./build/bin/functions_02_lambdas
// ============================================================

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Syntax
// ------------------------------------------------------------
//   [captures](parameters) -> return_type { body }
// The return type is usually deduced and can be left out.

void syntax()
{
    std::cout << "=== Lambda Syntax ===\n";

    auto add = [](int a, int b) { return a + b; };
    auto divide = [](double a, double b) -> double { return a / b; };

    std::cout << "add(2, 3)       = " << add(2, 3) << '\n';
    std::cout << "divide(7, 2)    = " << divide(7, 2) << '\n';

    // Most common use: passing behaviour to an algorithm.
    std::vector<int> values{5, 2, 8, 1};
    std::sort(values.begin(), values.end(), [](int a, int b) { return a > b; });
    std::cout << "sorted desc     = ";
    for (int v : values) std::cout << v << ' ';
    std::cout << '\n';

    // Immediately invoked lambda: initialise a const with complex logic.
    const std::string level = [&values] {
        if (values.size() > 3) return "many";
        return "few";
    }();
    std::cout << "level           = " << level << '\n';
}

// ------------------------------------------------------------
// Captures
// ------------------------------------------------------------
//   []        nothing
//   [x]       x by value (a copy made when the lambda is created)
//   [&x]      x by reference
//   [=]       everything used, by value
//   [&]       everything used, by reference
//   [=, &x]   everything by value except x
//   [this]    the current object, by pointer
//   [*this]   a copy of the current object (C++17)
//   [y = expr] init capture (C++14): a new variable, can move into it

void captures()
{
    std::cout << "\n=== Captures ===\n";

    int counter = 10;

    auto by_value = [counter] { return counter; };
    auto by_ref = [&counter] { return counter; };

    counter = 20;
    std::cout << "by value sees     : " << by_value() << " (copied when created)\n";  // 10
    std::cout << "by reference sees : " << by_ref() << '\n';                            // 20

    // Captured copies are const unless the lambda is mutable.
    auto ticker = [count = 0]() mutable { return ++count; };
    ticker();
    ticker();
    std::cout << "mutable ticker    : " << ticker() << '\n';                            // 3

    // Init capture can MOVE a move-only object into the lambda.
    auto owned = std::make_unique<std::string>("owned by the lambda");
    auto speaker = [text = std::move(owned)] { return *text; };
    std::cout << "moved-in capture  : " << speaker() << '\n';
    std::cout << "original is now   : " << (owned ? "set" : "empty") << '\n';
}

// ------------------------------------------------------------
// The Dangling Capture Trap
// ------------------------------------------------------------
// A lambda that captures by reference must not outlive the
// variables it refers to. Returning one from a function is the
// classic mistake:
//
//   auto make_adder(int n) { return [&n](int x) { return x + n; }; }  // n dies: UB
//
// Capture by value when the lambda escapes the current scope.

auto make_adder(int n)
{
    return [n](int x) { return x + n; };   // safe: holds its own copy
}

// ------------------------------------------------------------
// Capturing this
// ------------------------------------------------------------
// [=] used to capture this implicitly; that is deprecated in
// C++20. Write [this] (or [*this] for a copy) explicitly.

class Counter {
public:
    auto incrementer() { return [this] { ++count_; }; }
    int count() const { return count_; }

private:
    int count_ = 0;
};

// ------------------------------------------------------------
// Generic Lambdas and std::function
// ------------------------------------------------------------
// auto parameters make a lambda a template (C++14).
// C++20 also allows an explicit template parameter list.
//
// Every lambda has its own unique type. To store different
// lambdas in one variable or container, use std::function,
// which type-erases them at the cost of a possible heap
// allocation and an indirect call. Prefer auto or a template
// parameter when you do not need that flexibility.

void generic_and_function()
{
    std::cout << "\n=== Generic Lambdas and std::function ===\n";

    auto print_twice = [](const auto& value) { std::cout << value << value << '\n'; };
    print_twice(4);
    print_twice(std::string{"ab"});

    auto first_of = []<typename T>(const std::vector<T>& v) { return v.front(); };   // C++20
    std::cout << "first_of: " << first_of(std::vector<double>{1.5, 2.5}) << '\n';

    std::vector<std::function<int(int)>> pipeline{
        [](int x) { return x + 1; },
        [](int x) { return x * 10; },
        make_adder(5),
    };
    int value = 1;
    for (const auto& step : pipeline) {
        value = step(value);
    }
    std::cout << "pipeline(1) = " << value << '\n';   // ((1 + 1) * 10) + 5 = 25
}

int main()
{
    syntax();
    captures();

    std::cout << "\n=== Escaping Lambdas ===\n";
    auto add_ten = make_adder(10);
    std::cout << "add_ten(5) = " << add_ten(5) << '\n';

    Counter c;
    auto inc = c.incrementer();
    inc();
    inc();
    std::cout << "counter via [this] = " << c.count() << '\n';

    generic_and_function();
}
