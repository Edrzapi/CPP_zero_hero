// ============================================================
// Exceptions: throw, try, catch and Stack Unwinding
// ============================================================
// Mechanically similar to Java and C#, with three differences
// that matter:
//   - you can throw ANY type (but should throw classes derived
//     from std::exception)
//   - there is no "finally": destructors (RAII) do that job
//   - there are no checked exceptions; noexcept marks functions
//     that promise not to throw (04_noexcept_and_safety.cpp)
//
// Build and run:
//   cmake --build build --target exceptions_01_exceptions
//   ./build/bin/exceptions_01_exceptions
// ============================================================

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// The Standard Hierarchy (main branches)
// ------------------------------------------------------------
//   std::exception                  what()
//   +-- std::logic_error            bugs: preconditions broken
//   |   +-- std::invalid_argument
//   |   +-- std::out_of_range       vector::at, string::at
//   |   +-- std::length_error
//   +-- std::runtime_error          conditions outside the program's control
//   |   +-- std::overflow_error, std::range_error
//   |   +-- std::system_error       carries an std::error_code
//   +-- std::bad_alloc              new failed
//   +-- std::bad_cast               dynamic_cast on a reference
//   +-- std::bad_optional_access, std::bad_variant_access ...

double safe_divide(double a, double b)
{
    if (b == 0.0) {
        throw std::invalid_argument{"division by zero"};
    }
    return a / b;
}

// ------------------------------------------------------------
// Throwing and Catching
// ------------------------------------------------------------
// Throw by VALUE, catch by (const) REFERENCE. Catching by value
// copies the exception and slices derived types (GCC warns with
// -Wcatch-value). Order catch blocks from most to least specific:
// the first match wins.

void basic_catching()
{
    std::cout << "=== try / catch ===\n";

    try {
        std::cout << "  10 / 4 = " << safe_divide(10, 4) << '\n';
        double result = safe_divide(10, 0);                          // throws: the rest is skipped
        std::cout << "  10 / 0 = " << result << '\n';
        std::cout << "  never printed\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  invalid_argument: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cout << "  some other std::exception: " << e.what() << '\n';
    }

    std::vector<int> v{1, 2, 3};
    try {
        std::cout << v.at(5);
    } catch (const std::logic_error& e) {   // out_of_range IS-A logic_error
        std::cout << "  caught via base class logic_error: " << e.what() << '\n';
    }

    // catch (...) catches anything, even non-std types. Use it at
    // boundaries (main, threads, C callbacks) to stop exceptions
    // escaping, and usually rethrow or log.
    try {
        throw 42;   // legal, but do not do this in real code
    } catch (...) {
        std::cout << "  catch (...) caught a thrown int\n";
    }
}

// ------------------------------------------------------------
// Stack Unwinding: Why There Is No finally
// ------------------------------------------------------------
// When an exception leaves a scope, every fully constructed local
// object in that scope is destroyed, in reverse order. Resources
// held by RAII objects are therefore released automatically.

class Resource {
public:
    explicit Resource(std::string name) : name_{std::move(name)} { std::cout << "  acquire " << name_ << '\n'; }
    ~Resource() { std::cout << "  release " << name_ << '\n'; }
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

private:
    std::string name_;
};

void level_three()
{
    Resource r{"file handle"};
    throw std::runtime_error{"disk full"};
}

void level_two()
{
    Resource r{"database connection"};
    level_three();
    std::cout << "  never printed\n";
}

void unwinding()
{
    std::cout << "\n=== Stack Unwinding ===\n";
    try {
        level_two();
    } catch (const std::runtime_error& e) {
        std::cout << "  caught in unwinding(): " << e.what() << " (both resources already released)\n";
    }
}

// ------------------------------------------------------------
// Rethrowing
// ------------------------------------------------------------
// "throw;" on its own rethrows the CURRENT exception object,
// keeping its dynamic type. "throw e;" would throw a copy of the
// static type: sliced to the catch parameter's type.

void log_and_rethrow()
{
    try {
        throw std::out_of_range{"index 9 of 3"};
    } catch (const std::exception& e) {
        std::cout << "  logging: " << e.what() << '\n';
        throw;   // the original std::out_of_range continues upwards
    }
}

void rethrowing()
{
    std::cout << "\n=== Rethrowing ===\n";
    try {
        log_and_rethrow();
    } catch (const std::out_of_range& e) {
        std::cout << "  still an out_of_range after rethrow: " << e.what() << '\n';
    }
}

// ------------------------------------------------------------
// What Happens If Nobody Catches It
// ------------------------------------------------------------
// An exception escaping main (or a thread function, or a noexcept
// function) calls std::terminate, which aborts the program by
// default. Whether the stack is unwound first is implementation
// defined, so destructors may not run.
//
// Exceptions and destructors: a destructor that throws during
// unwinding also calls std::terminate. Destructors are noexcept by
// default; never let one throw.

int main()
{
    basic_catching();
    unwinding();
    rethrowing();
}
