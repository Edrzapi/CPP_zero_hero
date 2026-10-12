// ============================================================
// calculator_test.cpp: Unit Tests for Calculator
// ============================================================
// The C++ counterpart of Java Zero to Hero's CalculatorTest:
// happy paths, edge cases and exception paths, using the tiny
// harness in mini_test.h.
//
// Naming: "method: scenario -> expected behaviour", so a failure
// line explains itself without opening the test.
//
// Build and run:
//   cmake --build build --target testing_calculator_test
//   ./build/bin/testing_calculator_test               all tests
//   ./build/bin/testing_calculator_test divide        only tests whose name contains "divide"
//   ctest --test-dir build -R calculator --output-on-failure
//
// A failing check prints, for example:
//   [FAIL] add: two positives -> sum
//          calculator_test.cpp:41: CHECK_EQ(calc.add(2, 3), 6) failed: got 5, expected 6
// ============================================================

#include "calculator.h"
#include "mini_test.h"

#include <limits>
#include <stdexcept>
#include <vector>

// ------------------------------------------------------------
// Fixtures
// ------------------------------------------------------------
// JUnit's @BeforeEach and unittest's setUp() give every test a
// fresh object. In C++ just create the object inside the test:
// its constructor is the setup and its destructor is the teardown,
// and nothing leaks between tests. For shared setup, a small
// struct (a "fixture") does the same job.

struct CalculatorFixture {
    Calculator calc;
    std::vector<double> readings{2.0, 4.0, 9.0};
};

// ------------------------------------------------------------
// Arithmetic: Happy Paths and Edge Cases
// ------------------------------------------------------------

TEST_CASE("add: two positives -> sum")
{
    Calculator calc;
    CHECK_EQ(calc.add(2, 3), 5);
}

TEST_CASE("add: negative and positive -> signed result")
{
    Calculator calc;
    CHECK_EQ(calc.add(-7, 3), -4);
    CHECK_EQ(calc.add(0, 0), 0);
}

TEST_CASE("subtract: smaller minus larger -> negative")
{
    Calculator calc;
    CHECK_EQ(calc.subtract(3, 10), -7);
}

TEST_CASE("multiply: by zero -> zero")
{
    Calculator calc;
    CHECK_EQ(calc.multiply(123, 0), 0);
    CHECK_EQ(calc.multiply(-4, 5), -20);
}

// ------------------------------------------------------------
// Floating Point: Compare with a Tolerance
// ------------------------------------------------------------
// 1.0 / 3.0 is not exactly representable. CHECK_EQ on doubles is
// fragile; CHECK_NEAR states how close is close enough.

TEST_CASE("divide: one by three -> 0.3333 within tolerance")
{
    Calculator calc;
    CHECK_NEAR(calc.divide(1.0, 3.0), 0.3333, 0.0001);
}

TEST_CASE("divide: by zero -> throws invalid_argument")
{
    Calculator calc;
    CHECK_THROWS(calc.divide(1.0, 0.0), std::invalid_argument);
}

// ------------------------------------------------------------
// Using a Fixture
// ------------------------------------------------------------

TEST_CASE("average: fixture readings -> mean")
{
    CalculatorFixture f;
    CHECK_NEAR(f.calc.average(f.readings), 5.0, 1e-9);
    CHECK_NEAR(f.calc.last_result(), 5.0, 1e-9);
}

TEST_CASE("average: empty list -> throws invalid_argument")
{
    CalculatorFixture f;
    CHECK_THROWS(f.calc.average({}), std::invalid_argument);
}

// ------------------------------------------------------------
// Table-Driven Tests (Parameterised)
// ------------------------------------------------------------
// The equivalent of JUnit's @CsvSource or GoogleTest's
// TEST_P: a table of inputs and expected outputs, one loop.

TEST_CASE("factorial: table of known values")
{
    struct Row {
        int n;
        unsigned long long expected;
    };
    const Row rows[] = {{0, 1}, {1, 1}, {5, 120}, {10, 3'628'800}, {20, 2'432'902'008'176'640'000ULL}};

    Calculator calc;
    for (const auto& row : rows) {
        CHECK_EQ(calc.factorial(row.n), row.expected);
    }
}

TEST_CASE("factorial: negative -> throws invalid_argument")
{
    Calculator calc;
    CHECK_THROWS(calc.factorial(-1), std::invalid_argument);
}

TEST_CASE("factorial: 21 overflows 64 bits -> throws overflow_error")
{
    Calculator calc;
    CHECK_THROWS(calc.factorial(21), std::overflow_error);
}

TEST_CASE("is_even: negatives and zero")
{
    Calculator calc;
    CHECK(calc.is_even(0));
    CHECK(calc.is_even(-4));
    CHECK(!calc.is_even(-3));
    CHECK(!calc.is_even(std::numeric_limits<int>::max()));
}

// ------------------------------------------------------------
// The Runner
// ------------------------------------------------------------
// All TEST_CASEs registered themselves before main started.
// The return value is the number of failures: 0 means success,
// which is what ctest and CI pipelines check.

int main(int argc, char* argv[])
{
    std::cout << "=== Calculator Tests ===\n";
    return mini_test::run_all(argc, argv);
}
