// ============================================================
// Exceptions vs Error Codes vs std::optional
// ============================================================
// C++ has several ways to report failure, and real codebases
// mix them. Knowing which to use where is a design decision:
//
//   exceptions        rare, serious failures the immediate caller
//                     usually cannot fix (out of memory, corrupt file)
//   error codes       C APIs, very hot paths, code built with
//                     exceptions disabled (games, embedded, some
//                     Google-style codebases)
//   std::optional     "no value" is a normal, expected outcome
//                     (lookup misses, parse of user input)
//   std::expected     a value OR an error object (C++23, see
//                     05_expected_cxx23.cpp)
//
// Build and run:
//   cmake --build build --target exceptions_03_error_codes_and_optional
//   ./build/bin/exceptions_03_error_codes_and_optional
// ============================================================

#include <cerrno>
#include <charconv>       // std::from_chars (C++17)
#include <cstdlib>        // std::strtol
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

// ------------------------------------------------------------
// C Style: Return Codes and errno
// ------------------------------------------------------------
// The result comes back through a pointer, the return value is a
// status, and some functions also set the global errno. Easy to
// ignore by accident; every caller must check.

int parse_c_style(const char* text, long* out)
{
    char* end = nullptr;
    errno = 0;
    long value = std::strtol(text, &end, 10);
    if (end == text || *end != '\0') return -1;   // not a number / trailing junk
    if (errno == ERANGE) return -2;               // out of range for long
    *out = value;
    return 0;
}

// ------------------------------------------------------------
// C++17: std::from_chars and std::errc
// ------------------------------------------------------------
// Fast, locale-independent, never throws, never allocates. It
// returns a struct with a pointer and an std::errc code. The
// standard library's own choice for performance-critical parsing.

std::optional<int> parse_int(std::string_view text)
{
    int value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size()) {
        return std::nullopt;   // invalid, out of range, or trailing characters
    }
    return value;
}

// ------------------------------------------------------------
// std::error_code
// ------------------------------------------------------------
// A portable error value: a number plus a category that knows how
// to describe it. The <filesystem> functions offer an overload
// taking std::error_code& instead of throwing.

std::error_code check_port(int port)
{
    if (port < 1 || port > 65535) {
        return std::make_error_code(std::errc::invalid_argument);
    }
    return {};   // a default error_code means "no error"
}

// ------------------------------------------------------------
// std::optional
// ------------------------------------------------------------
// optional<T> holds a T or nothing. It never allocates: the T is
// stored inside. Check it like a pointer, read it with * or
// .value() (throws std::bad_optional_access if empty), or supply
// a fallback with .value_or().

std::optional<std::string> find_email(const std::string& user)
{
    static const std::map<std::string, std::string> directory{{"ada", "ada@example.org"}};
    if (auto it = directory.find(user); it != directory.end()) {
        return it->second;
    }
    return std::nullopt;
}

int main()
{
    std::cout << "=== C Style: Return Codes ===\n";
    for (const char* text : {"123", "12x", "99999999999999999999"}) {
        long value = 0;
        int status = parse_c_style(text, &value);
        std::cout << "  \"" << text << "\" -> status " << status;
        if (status == 0) std::cout << ", value " << value;
        std::cout << '\n';
    }

    std::cout << "\n=== std::from_chars + optional ===\n";
    for (std::string_view text : {"42", "-7", "4x2", "", "9999999999"}) {
        auto parsed = parse_int(text);
        std::cout << "  \"" << text << "\" -> " << (parsed ? std::to_string(*parsed) : "not an int") << '\n';
    }

    std::cout << "\n=== std::error_code ===\n";
    for (int port : {8080, 70000}) {
        if (auto ec = check_port(port)) {   // converts to true when it holds an error
            std::cout << "  port " << port << ": error " << ec.value() << " (" << ec.message() << ")\n";
        } else {
            std::cout << "  port " << port << ": ok\n";
        }
    }

    std::cout << "\n=== std::optional ===\n";
    std::cout << "  ada -> " << find_email("ada").value_or("(none)") << '\n';
    std::cout << "  bob -> " << find_email("bob").value_or("(none)") << '\n';
    try {
        std::cout << find_email("bob").value();
    } catch (const std::bad_optional_access& e) {
        std::cout << "  .value() on an empty optional threw: " << e.what() << '\n';
    }

    // ------------------------------------------------------------
    // Choosing
    // ------------------------------------------------------------
    // Ask: "is failure here normal?" If yes, return optional /
    // expected / an error code. If it means something is broken,
    // throw. Constructors cannot return a status, so a constructor
    // that cannot establish its invariant should throw (or use a
    // factory function returning optional).
}
