// ============================================================
// std::string_view and Its Lifetime Traps
// ============================================================
// std::string_view (C++17) is a pointer and a length: a read-only
// window onto characters owned by someone else. Passing and
// slicing it never allocates, which makes it ideal for function
// PARAMETERS. It owns nothing, so it must never outlive the text
// it points at, and that is easy to get wrong.
//
// The broken examples are in comments. under_the_hood/ub_examples
// (opt-in, with AddressSanitizer) runs one so you can see the
// report.
//
// Build and run:
//   cmake --build build --target under_the_hood_04_string_view_lifetimes
//   ./build/bin/under_the_hood_04_string_view_lifetimes
// ============================================================

#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// The Good Use: Parameters
// ------------------------------------------------------------
// One function accepts a std::string, a string literal, a char
// array or a slice of any of them, with no copy. (A const
// std::string& parameter would force a temporary std::string to
// be built from every literal.)

std::size_t count_vowels(std::string_view text)
{
    std::size_t count = 0;
    for (char c : text) {
        if (std::string_view{"aeiouAEIOU"}.find(c) != std::string_view::npos) ++count;
    }
    return count;
}

void good_uses()
{
    std::cout << "=== Parameters and Cheap Slices ===\n";

    std::string owned = "Programming in C++";
    std::cout << "  from std::string : " << count_vowels(owned) << '\n';
    std::cout << "  from a literal   : " << count_vowels("education") << '\n';

    std::string_view whole = owned;
    std::string_view first_word = whole.substr(0, 11);   // no allocation, unlike std::string::substr
    std::cout << "  slice            : [" << first_word << "] vowels " << count_vowels(first_word) << '\n';

    whole.remove_prefix(15);   // only moves the view's start pointer
    std::cout << "  after remove_prefix(15): [" << whole << "]\n";
}

// ------------------------------------------------------------
// Trap 1: Not Null-Terminated
// ------------------------------------------------------------
// A view of part of a string has no '\0' at its end. Passing
// .data() to a C function that expects a C string reads past the
// view, up to whatever '\0' comes next.

void not_null_terminated()
{
    std::cout << "\n=== Trap: No Terminator ===\n";
    std::string_view hello = std::string_view{"hello world"}.substr(0, 5);

    // std::printf("%s\n", hello.data());   // prints "hello world": reads beyond the view
    std::printf("  printf with %%.*s and the length: %.*s\n", static_cast<int>(hello.size()), hello.data());

    std::string terminated{hello};          // or make a std::string when a C API needs one
    std::cout << "  as std::string for a C API: " << terminated.c_str() << '\n';
}

// ------------------------------------------------------------
// Trap 2: Views of Temporaries
// ------------------------------------------------------------
// Each of these compiles. Each leaves a view pointing at freed
// memory (Clang warns about some with -Wdangling; GCC 13+ has
// -Wdangling-reference for related cases):
//
//   std::string_view sv = std::string{"temporary"};          // string dies at the ;
//
//   std::string_view name() { std::string s = "x"; return s; }   // s dies at return
//
//   std::string_view first = get_names()[0];                 // the vector<string> temporary dies
//
//   std::string s = "short";
//   std::string_view v = s;
//   s += " and now much longer than the small buffer";       // reallocates: v dangles
//
//   std::map<std::string_view, int> index;
//   index[std::to_string(42)] = 1;                           // key views a dead temporary
//
// Rules of thumb:
//   - string_view as a PARAMETER: almost always fine
//   - string_view as a RETURN type or MEMBER: only when it points
//     at something that clearly outlives it (literals, a member of
//     *this, a buffer owned by the caller)
//   - never store a view of a std::string you then modify

std::string_view safe_status(int code)
{
    // Returning views of string LITERALS is safe: literals live for
    // the whole program.
    switch (code) {
    case 200: return "OK";
    case 404: return "Not Found";
    default:  return "Unknown";
    }
}

class Config {
public:
    explicit Config(std::string text) : text_{std::move(text)} {}
    // Safe while the Config object is alive and text_ is unchanged.
    std::string_view section() const { return std::string_view{text_}.substr(0, text_.find('.')); }

private:
    std::string text_;
};

int main()
{
    good_uses();
    not_null_terminated();

    std::cout << "\n=== Safe Ways to Return a View ===\n";
    std::cout << "  status 404: " << safe_status(404) << '\n';
    Config config{"network.timeout=30"};
    std::cout << "  config section: " << config.section() << '\n';
}
