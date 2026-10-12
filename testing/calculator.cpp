// ============================================================
// calculator.cpp: Implementation of Calculator
// ============================================================

#include "calculator.h"

#include <limits>
#include <numeric>
#include <stdexcept>

int Calculator::add(int a, int b) const
{
    int result = a + b;
    last_ = result;
    return result;
}

int Calculator::subtract(int a, int b) const
{
    int result = a - b;
    last_ = result;
    return result;
}

int Calculator::multiply(int a, int b) const
{
    int result = a * b;
    last_ = result;
    return result;
}

double Calculator::divide(double a, double b) const
{
    if (b == 0.0) {
        throw std::invalid_argument{"cannot divide by zero"};
    }
    last_ = a / b;
    return last_;
}

double Calculator::average(const std::vector<double>& values) const
{
    if (values.empty()) {
        throw std::invalid_argument{"cannot average an empty list"};
    }
    last_ = std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
    return last_;
}

unsigned long long Calculator::factorial(int n) const
{
    if (n < 0) {
        throw std::invalid_argument{"factorial of a negative number"};
    }
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) {
        const auto factor = static_cast<unsigned long long>(i);
        if (result > std::numeric_limits<unsigned long long>::max() / factor) {
            throw std::overflow_error{"factorial does not fit in 64 bits"};
        }
        result *= factor;
    }
    last_ = static_cast<double>(result);
    return result;
}

bool Calculator::is_even(int n) const { return n % 2 == 0; }
