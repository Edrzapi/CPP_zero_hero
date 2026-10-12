// ============================================================
// std::expected: a Value or an Error (C++23)
// ============================================================
// C++23 ONLY. Built when CPPZH_ENABLE_CXX23 is ON.
// Library support: GCC 12+, Clang 16+ (libc++), MSVC 19.33+.
//
// expected<T, E> holds either a T (success) or an E (the error),
// like Rust's Result or a typed Either. Compared with:
//   exceptions:   the error is in the signature and cannot be
//                 ignored silently; no unwinding cost
//   optional:     it says WHY there is no value
//   error codes:  the value and the error travel together
// The monadic operations (and_then, transform, or_else) chain
// steps without nested if statements.
//
// Build and run:
//   cmake -S . -B build -DCPPZH_ENABLE_CXX23=ON
//   cmake --build build --target exceptions_05_expected_cxx23
//   ./build/bin/exceptions_05_expected_cxx23
// ============================================================

#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

enum class ParseError { Empty, NotANumber, OutOfRange };

std::string_view to_string(ParseError e)
{
    switch (e) {
    case ParseError::Empty:      return "empty input";
    case ParseError::NotANumber: return "not a number";
    case ParseError::OutOfRange: return "out of range";
    }
    return "unknown";
}

// ------------------------------------------------------------
// Returning expected
// ------------------------------------------------------------
// Return the value directly on success; wrap the error in
// std::unexpected on failure.

std::expected<int, ParseError> parse_int(std::string_view text)
{
    if (text.empty()) return std::unexpected{ParseError::Empty};
    int value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec == std::errc::result_out_of_range) return std::unexpected{ParseError::OutOfRange};
    if (ec != std::errc{} || ptr != text.data() + text.size()) return std::unexpected{ParseError::NotANumber};
    return value;
}

std::expected<int, ParseError> check_percentage(int value)
{
    if (value < 0 || value > 100) return std::unexpected{ParseError::OutOfRange};
    return value;
}

int main()
{
    std::cout << "=== Checking an expected ===\n";
    for (std::string_view input : {"42", "", "4x", "99999999999"}) {
        auto result = parse_int(input);
        if (result) {
            std::cout << "  \"" << input << "\" -> " << *result << '\n';
        } else {
            std::cout << "  \"" << input << "\" -> error: " << to_string(result.error()) << '\n';
        }
    }

    std::cout << "\n=== Chaining with and_then / transform ===\n";
    for (std::string_view input : {"75", "150", "abc"}) {
        auto as_fraction = parse_int(input)
                               .and_then(check_percentage)                      // may fail again
                               .transform([](int pct) { return pct / 100.0; }); // runs only on success
        if (as_fraction) {
            std::cout << "  \"" << input << "\" -> " << *as_fraction << '\n';
        } else {
            std::cout << "  \"" << input << "\" -> " << to_string(as_fraction.error()) << '\n';
        }
    }

    std::cout << "\n=== value_or ===\n";
    std::cout << "  parse_int(\"oops\").value_or(-1) = " << parse_int("oops").value_or(-1) << '\n';
}
