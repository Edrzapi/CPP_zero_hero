// ============================================================
// Strings: C Strings and std::string
// ============================================================
// A C string is an array of char ending in '\0'. std::string
// owns its characters, knows its length and grows as needed.
// Unlike Java and C#, std::string is MUTABLE.
//
// Build and run:
//   cmake --build build --target basics_08_strings
//   ./build/bin/basics_08_strings
// ============================================================

#include <algorithm>
#include <cctype>
#include <cstring>   // std::strlen, std::strcmp
#include <iostream>
#include <string>

// ------------------------------------------------------------
// C Strings
// ------------------------------------------------------------
// Length is found by scanning for '\0' (O(n)). Comparison with
// == compares ADDRESSES, not text. Buffers do not grow, so
// strcpy/strcat into a too-small array overruns memory.

void c_strings()
{
    std::cout << "=== C Strings ===\n";

    const char* greeting = "hello";
    char buffer[] = "hello";          // a modifiable copy: 6 chars including '\0'

    std::cout << "strlen(greeting) = " << std::strlen(greeting) << '\n';  // 5
    std::cout << "sizeof(buffer)   = " << sizeof(buffer) << '\n';         // 6

    // Compare text with strcmp (0 means equal), not ==.
    std::cout << "strcmp equal?    = " << std::boolalpha
              << (std::strcmp(greeting, buffer) == 0) << '\n';            // true

    buffer[0] = 'J';
    std::cout << "modified buffer  = " << buffer << '\n';                 // Jello
    // greeting[0] = 'J';   // error: string literals are const in C++
}

// ------------------------------------------------------------
// std::string Basics
// ------------------------------------------------------------

void string_basics()
{
    std::cout << "\n=== std::string ===\n";

    std::string name = "Bjarne";
    std::string full = name + " Stroustrup";   // + concatenates
    full += "!";

    std::cout << full << '\n';
    std::cout << "length       : " << full.size() << '\n';
    std::cout << "first char   : " << full[0] << '\n';        // no bounds check
    std::cout << "at(1)        : " << full.at(1) << '\n';     // bounds checked, throws std::out_of_range
    std::cout << "front/back   : " << full.front() << full.back() << '\n';

    // == compares text for std::string.
    std::string other = "Bjarne";
    std::cout << "name == other: " << (name == other) << '\n';            // true

    // Mutable in place:
    name[0] = 'b';
    std::cout << "mutated      : " << name << '\n';                       // bjarne
}

// ------------------------------------------------------------
// Searching and Slicing
// ------------------------------------------------------------
// find returns an index or std::string::npos when not found.
// substr(pos, count) copies a piece out.

void searching()
{
    std::cout << "\n=== Searching ===\n";

    std::string sentence = "the quick brown fox";

    auto pos = sentence.find("quick");
    if (pos != std::string::npos) {
        std::cout << "'quick' found at " << pos << '\n';                  // 4
    }
    if (sentence.find("cat") == std::string::npos) {
        std::cout << "'cat' not found\n";
    }

    std::cout << "substr(4, 5) : " << sentence.substr(4, 5) << '\n';     // quick

    // C++20 helpers, long overdue:
    std::cout << "starts_with  : " << sentence.starts_with("the") << '\n';
    std::cout << "ends_with    : " << sentence.ends_with("fox") << '\n';
    // C++23 adds contains(); before that use find() != npos.

    sentence.replace(4, 5, "slow");
    std::cout << "replaced     : " << sentence << '\n';
}

// ------------------------------------------------------------
// Conversions
// ------------------------------------------------------------
// std::to_string for numbers to text, std::stoi / std::stod for
// text to numbers (they throw std::invalid_argument on bad input).
// .c_str() hands a C API a const char* view of the string.

void conversions()
{
    std::cout << "\n=== Conversions ===\n";

    std::string text = std::to_string(42) + " and " + std::to_string(2.5);
    std::cout << text << '\n';

    int n = std::stoi("123");
    double d = std::stod("3.75");
    std::cout << n + 1 << ' ' << d * 2 << '\n';

    std::string upper = "make me loud";
    // std::toupper takes an int that must be representable as unsigned char.
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    std::cout << upper << '\n';

    std::cout << "c_str() length via strlen: " << std::strlen(upper.c_str()) << '\n';
}

int main()
{
    c_strings();
    string_basics();
    searching();
    conversions();
}
