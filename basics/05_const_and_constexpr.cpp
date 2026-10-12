// ============================================================
// const, constexpr, consteval and constinit
// ============================================================
// const means "I promise not to change this". constexpr means
// "this can be worked out at compile time". They answer
// different questions, which is why C++ has both.
//
// Build and run:
//   cmake --build build --target basics_05_const_and_constexpr
//   ./build/bin/basics_05_const_and_constexpr
// ============================================================

#include <array>
#include <iostream>

// ------------------------------------------------------------
// const
// ------------------------------------------------------------
// A const variable cannot be modified after initialisation, but
// its value may only be known at run time.

void const_basics(int runtime_value)
{
    std::cout << "=== const ===\n";

    const int limit = runtime_value * 2;   // fine: const, but computed at run time
    // limit = 5;                          // error: assignment of read-only variable

    std::cout << "limit = " << limit << '\n';

    // const with pointers: read the declaration right to left.
    int a = 1;
    int b = 2;

    const int* p1 = &a;     // pointer to const int: can't change *p1, can re-point p1
    p1 = &b;
    // *p1 = 3;             // error

    int* const p2 = &a;     // const pointer to int: can change *p2, can't re-point p2
    *p2 = 10;
    // p2 = &b;             // error

    const int* const p3 = &b;   // neither
    std::cout << "*p1 = " << *p1 << ", a = " << a << ", *p3 = " << *p3 << '\n';

    // "East const" puts const after the type: int const* is the same as const int*.
}

// ------------------------------------------------------------
// const Member Functions and const References
// ------------------------------------------------------------
// const is part of a function's contract. A const member function
// promises not to modify the object, so it can be called on const
// objects and through const references.

class Account {
public:
    explicit Account(double opening) : balance_{opening} {}

    double balance() const { return balance_; }         // callable on const Account
    void deposit(double amount) { balance_ += amount; } // not const

private:
    double balance_;
};

void print_balance(const Account& account)   // const& : no copy, no modification
{
    std::cout << "Balance: " << account.balance() << '\n';
    // account.deposit(10);   // error: deposit is not a const member function
}

// ------------------------------------------------------------
// constexpr
// ------------------------------------------------------------
// A constexpr variable MUST be computable at compile time.
// A constexpr function CAN run at compile time if its arguments
// are constants, and runs at run time otherwise.

constexpr int square(int n) { return n * n; }

constexpr int factorial(int n)
{
    int result = 1;
    for (int i = 2; i <= n; ++i) {   // loops are fine in constexpr since C++14
        result *= i;
    }
    return result;
}

void constexpr_basics(int runtime_value)
{
    std::cout << "\n=== constexpr ===\n";

    constexpr int side = 4;
    constexpr int area = square(side);   // computed by the compiler
    static_assert(area == 16);           // proof: checked at compile time

    // Array sizes must be constant expressions.
    std::array<int, factorial(4)> slots{};
    std::cout << "slots.size() = " << slots.size() << '\n';   // 24

    // The same function at run time:
    int runtime_square = square(runtime_value);
    std::cout << "square(" << runtime_value << ") at run time = " << runtime_square << '\n';

    // constexpr int bad = square(runtime_value);  // error: not a constant expression
}

// ------------------------------------------------------------
// consteval and constinit (C++20)
// ------------------------------------------------------------
// consteval: the function MUST run at compile time (an
//            "immediate function"). Calling it with a run-time
//            value is a compile error.
// constinit: the variable must be initialised at compile time,
//            but it is NOT const and can change later. It removes
//            the "static initialisation order fiasco" for globals.
// More in under_the_hood/01_constexpr_and_consteval.cpp.

consteval int kilobytes(int n) { return n * 1024; }

constinit int g_request_count = 0;   // zero cost start-up, still mutable

void consteval_and_constinit()
{
    std::cout << "\n=== consteval and constinit ===\n";

    constexpr int buffer = kilobytes(4);
    std::cout << "kilobytes(4) = " << buffer << '\n';
    // int n = 4; kilobytes(n);   // error: n is not a constant expression

    ++g_request_count;
    ++g_request_count;
    std::cout << "g_request_count = " << g_request_count << '\n';
}

// ------------------------------------------------------------
// #define vs constexpr
// ------------------------------------------------------------
// In C, constants are often macros:  #define MAX_USERS 100
// Macros have no type, no scope and do not appear in the
// debugger. In C++ prefer constexpr (or an enum) every time.

#define LEGACY_MAX_USERS 100        // C style, kept to show the contrast
constexpr int max_users = 100;      // C++ style: typed and scoped

int main(int argc, char*[])
{
    // argc is used as a value the compiler cannot know in advance.
    const int runtime_value = argc + 2;

    const_basics(runtime_value);

    Account account{50.0};
    account.deposit(25.0);
    print_balance(account);

    constexpr_basics(runtime_value);
    consteval_and_constinit();

    std::cout << "\nLEGACY_MAX_USERS = " << LEGACY_MAX_USERS
              << ", max_users = " << max_users << '\n';
}
