// ============================================================
// mini_test.h: A Tiny, Self-Contained Test Harness
// ============================================================
// About 100 lines, header-only, no downloads. It provides:
//
//   TEST_CASE("name") { ... }       define and register a test
//   CHECK(condition)                fail the test if false
//   CHECK_EQ(actual, expected)      fail with both values printed
//   CHECK_NEAR(actual, expected, tolerance)   for floating point
//   CHECK_THROWS(expression, ExceptionType)
//   mini_test::run_all(argc, argv)  run every test (or those whose
//                                   name contains argv[1]) and
//                                   return the number of failures
//
// It exists to show how test frameworks work inside: a registry
// filled by static objects before main, macros that capture the
// expression text and line number, and failures reported by
// throwing an exception that the runner catches.
//
// ------------------------------------------------------------
// The Real Frameworks
// ------------------------------------------------------------
// GoogleTest (Google) and Catch2 are the two most common. Both are
// usually pulled in by CMake's FetchContent:
//
//   include(FetchContent)
//   FetchContent_Declare(googletest
//       GIT_REPOSITORY https://github.com/google/googletest.git
//       GIT_TAG        v1.15.2)
//   FetchContent_MakeAvailable(googletest)
//   add_executable(calculator_gtest calculator_gtest.cpp)
//   target_link_libraries(calculator_gtest PRIVATE GTest::gtest_main)
//   include(GoogleTest)
//   gtest_discover_tests(calculator_gtest)
//
// The same test in each style:
//
//   mini_test:  TEST_CASE("divide by zero throws") {
//                   CHECK_THROWS(calc.divide(1, 0), std::invalid_argument);
//               }
//   GoogleTest: TEST(Calculator, DivideByZeroThrows) {
//                   EXPECT_THROW(calc.divide(1, 0), std::invalid_argument);
//               }
//   Catch2:     TEST_CASE("divide by zero throws", "[calculator]") {
//                   REQUIRE_THROWS_AS(calc.divide(1, 0), std::invalid_argument);
//               }
//
// What the real ones add: EXPECT (continue after a failure) vs
// ASSERT/REQUIRE (stop the test), fixtures with SetUp/TearDown,
// parameterised tests, death tests, mocking (GoogleMock), XML/JUnit
// reports for CI, and IDE integration (VS Code Test Explorer, VS
// Test Explorer). JUnit's @BeforeEach maps to a fixture's SetUp().
// ============================================================

#pragma once

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mini_test {

// Thrown by a failed check. Deliberately NOT derived from
// std::exception, so code under test that catches std::exception
// cannot swallow a test failure by accident.
struct Failure {
    std::string message;
};

struct TestCase {
    std::string name;
    std::function<void()> body;
};

// A function-local static avoids the "static initialisation order
// fiasco": the registry is created on first use, whichever test
// file's static Registrar runs first.
inline std::vector<TestCase>& registry()
{
    static std::vector<TestCase> tests;
    return tests;
}

// Each TEST_CASE defines one static Registrar. Its constructor
// runs before main and adds the test to the registry.
struct Registrar {
    Registrar(std::string name, std::function<void()> body)
    {
        registry().push_back({std::move(name), std::move(body)});
    }
};

inline std::string where(const char* file, int line)
{
    std::ostringstream out;
    out << file << ':' << line << ": ";
    return out.str();
}

// Compare and report both values. Both types must support == and <<.
template <typename A, typename B>
void check_equal(const A& actual, const B& expected, const char* actual_text,
                 const char* expected_text, const char* file, int line)
{
    if (!(actual == expected)) {
        std::ostringstream out;
        out << where(file, line) << "CHECK_EQ(" << actual_text << ", " << expected_text
            << ") failed: got " << actual << ", expected " << expected;
        throw Failure{out.str()};
    }
}

inline void check_near(double actual, double expected, double tolerance, const char* actual_text,
                       const char* file, int line)
{
    if (std::fabs(actual - expected) > tolerance) {
        std::ostringstream out;
        out << where(file, line) << "CHECK_NEAR(" << actual_text << ") failed: got " << actual
            << ", expected " << expected << " +/- " << tolerance;
        throw Failure{out.str()};
    }
}

// Runs every test whose name contains the filter (all if empty).
// Returns the number of failures, used as the process exit code.
inline int run_all(int argc = 0, char* argv[] = nullptr)
{
    const std::string_view filter = (argc > 1 && argv != nullptr) ? argv[1] : "";
    int passed = 0;
    int failed = 0;

    for (const auto& test : registry()) {
        if (!filter.empty() && test.name.find(filter) == std::string::npos) {
            continue;
        }
        try {
            test.body();
            ++passed;
            std::cout << "  [PASS] " << test.name << '\n';
        } catch (const Failure& f) {
            ++failed;
            std::cout << "  [FAIL] " << test.name << "\n         " << f.message << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "  [FAIL] " << test.name << "\n         unexpected exception: " << e.what() << '\n';
        } catch (...) {
            ++failed;
            std::cout << "  [FAIL] " << test.name << "\n         unexpected non-standard exception\n";
        }
    }

    std::cout << "\n  " << passed << " passed, " << failed << " failed\n";
    return failed;
}

}  // namespace mini_test

// ------------------------------------------------------------
// Macros
// ------------------------------------------------------------
// Macros are needed for two things functions cannot do: turn the
// checked expression into text (#expr) and record __FILE__ and
// __LINE__ at the place of the check. MT_CONCAT builds unique
// names from __LINE__ (the extra level of indirection makes the
// preprocessor expand __LINE__ before pasting).

#define MT_CONCAT_IMPL(a, b) a##b
#define MT_CONCAT(a, b) MT_CONCAT_IMPL(a, b)

#define TEST_CASE(name)                                                          \
    static void MT_CONCAT(mt_test_body_, __LINE__)();                            \
    static const ::mini_test::Registrar MT_CONCAT(mt_registrar_, __LINE__){     \
        name, &MT_CONCAT(mt_test_body_, __LINE__)};                              \
    static void MT_CONCAT(mt_test_body_, __LINE__)()

#define CHECK(condition)                                                                       \
    do {                                                                                       \
        if (!(condition)) {                                                                    \
            throw ::mini_test::Failure{::mini_test::where(__FILE__, __LINE__) + "CHECK(" #condition ") failed"}; \
        }                                                                                      \
    } while (false)

#define CHECK_EQ(actual, expected) \
    ::mini_test::check_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#define CHECK_NEAR(actual, expected, tolerance) \
    ::mini_test::check_near((actual), (expected), (tolerance), #actual, __FILE__, __LINE__)

#define CHECK_THROWS(expression, exception_type)                                               \
    do {                                                                                       \
        bool mt_caught_expected = false;                                                       \
        try {                                                                                  \
            static_cast<void>(expression);                                                     \
        } catch (const exception_type&) {                                                      \
            mt_caught_expected = true;                                                         \
        } catch (...) {                                                                        \
            throw ::mini_test::Failure{::mini_test::where(__FILE__, __LINE__) +                \
                                       "CHECK_THROWS(" #expression ") threw the wrong type"}; \
        }                                                                                      \
        if (!mt_caught_expected) {                                                             \
            throw ::mini_test::Failure{::mini_test::where(__FILE__, __LINE__) +                \
                                       "CHECK_THROWS(" #expression ") did not throw " #exception_type}; \
        }                                                                                      \
    } while (false)
