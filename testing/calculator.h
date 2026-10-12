// ============================================================
// calculator.h: The Class Under Test
// ============================================================
// The same subject as Java Zero to Hero's Calculator and Python
// Zero to Hero's calculator task, so the test suites can be
// compared side by side.
// ============================================================

#ifndef CPPZH_CALCULATOR_H
#define CPPZH_CALCULATOR_H

#include <vector>

class Calculator {
public:
    int add(int a, int b) const;
    int subtract(int a, int b) const;
    int multiply(int a, int b) const;

    // Throws std::invalid_argument when b is zero.
    double divide(double a, double b) const;

    // Throws std::invalid_argument for an empty list.
    double average(const std::vector<double>& values) const;

    // Throws std::invalid_argument for negative n and
    // std::overflow_error when the result does not fit in 64 bits.
    unsigned long long factorial(int n) const;

    bool is_even(int n) const;

    // Remembers the last result: state that fixtures reset.
    double last_result() const { return last_; }

private:
    mutable double last_ = 0.0;   // mutable: updated by const operations
};

#endif  // CPPZH_CALCULATOR_H
