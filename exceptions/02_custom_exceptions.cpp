// ============================================================
// Custom Exception Hierarchies and Nested Exceptions
// ============================================================
// The same three-level hierarchy as Java Zero to Hero:
//   ShapeException > InvalidDimensionException > InvalidSquareException
// built on the standard classes so that callers who only know
// std::exception can still catch and report them. Then wrapping a
// low-level error in a higher-level one, the C++ version of Java's
// "new Exception(message, cause)".
//
// Build and run:
//   cmake --build build --target exceptions_02_custom_exceptions
//   ./build/bin/exceptions_02_custom_exceptions
// ============================================================

#include <cstddef>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// ------------------------------------------------------------
// The Hierarchy
// ------------------------------------------------------------
// Derive from the most fitting standard class and inherit its
// constructors. Extra data goes in members with accessors.
// what() returns const char*, so keep the message in a member
// (std::runtime_error already does that for us).

// std::to_string(2.0) gives "2.000000"; a stream gives "2".
std::string number_text(double value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

class ShapeException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;   // inherit the (const std::string&) constructor
};

class InvalidDimensionException : public ShapeException {
public:
    InvalidDimensionException(const std::string& message, double value)
        : ShapeException{message + " (got " + number_text(value) + ")"}, value_{value}
    {
    }
    double value() const noexcept { return value_; }

private:
    double value_;
};

class InvalidSquareException : public InvalidDimensionException {
public:
    InvalidSquareException(double width, double height)
        : InvalidDimensionException{"height must equal width " + number_text(width), height}
    {
    }
};

// ------------------------------------------------------------
// Code That Throws Them
// ------------------------------------------------------------

class Rectangle {
public:
    Rectangle(double width, double height) : width_{width}, height_{height}
    {
        if (width <= 0) throw InvalidDimensionException{"width must be positive", width};
        if (height <= 0) throw InvalidDimensionException{"height must be positive", height};
    }
    double area() const { return width_ * height_; }

protected:
    double width_;
    double height_;
};

class Square : public Rectangle {
public:
    Square(double width, double height) : Rectangle{width, height}
    {
        if (width != height) throw InvalidSquareException{width, height};
    }
};

// ------------------------------------------------------------
// Catching at Different Levels
// ------------------------------------------------------------

void try_build(double w, double h)
{
    try {
        Square s{w, h};
        std::cout << "  built a square with area " << s.area() << '\n';
    } catch (const InvalidSquareException& e) {          // most specific first
        std::cout << "  InvalidSquareException: " << e.what() << '\n';
    } catch (const InvalidDimensionException& e) {
        std::cout << "  InvalidDimensionException (value " << e.value() << "): " << e.what() << '\n';
    } catch (const ShapeException& e) {
        std::cout << "  ShapeException: " << e.what() << '\n';
    }
}

// ------------------------------------------------------------
// Nested Exceptions: Wrapping with a Cause
// ------------------------------------------------------------
// std::throw_with_nested(outer) throws an object that is BOTH the
// outer exception and holds the currently handled one.
// std::rethrow_if_nested digs the inner one out again.

void load_config_value()
{
    throw std::out_of_range{"key 'timeout' not found"};
}

void start_service()
{
    try {
        load_config_value();
    } catch (...) {
        std::throw_with_nested(std::runtime_error{"service failed to start"});
    }
}

void print_exception_chain(const std::exception& e, int depth = 0)
{
    std::cout << "  " << std::string(static_cast<std::size_t>(depth) * 2, ' ') << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& inner) {
        print_exception_chain(inner, depth + 1);
    }
}

int main()
{
    std::cout << "=== A Custom Hierarchy ===\n";
    try_build(3, 3);
    try_build(3, 4);
    try_build(-1, 5);

    std::cout << "\n=== Catching Through the std::exception Base ===\n";
    try {
        Square bad{2, 5};
    } catch (const std::exception& e) {   // callers that know nothing about shapes
        std::cout << "  std::exception: " << e.what() << '\n';
    }

    std::cout << "\n=== Nested Exceptions ===\n";
    try {
        start_service();
    } catch (const std::exception& e) {
        print_exception_chain(e);
    }
}
