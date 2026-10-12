// ============================================================
// iostreams: Console Output, Input and Formatting
// ============================================================
// std::cout / std::cerr / std::clog for output, std::cin for
// input. The same stream interface works for files (fstream) and
// strings (stringstream), so everything here carries over.
//
// The input examples read from an std::istringstream holding
// sample text, so the program runs unattended (and under ctest).
// Swap "input" for std::cin and the code is identical.
//
// Build and run:
//   cmake --build build --target io_01_iostream
//   ./build/bin/io_01_iostream
// ============================================================

#include <iomanip>     // std::setw, std::setprecision, std::setfill
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

// ------------------------------------------------------------
// The Standard Streams
// ------------------------------------------------------------
//   std::cout  standard output, buffered
//   std::cerr  standard error, unbuffered (appears immediately)
//   std::clog  standard error, buffered (for logging)
// Redirect with the shell:  program > out.txt 2> err.txt

void standard_streams()
{
    std::cout << "=== Standard Streams ===\n";
    std::cout << "  to stdout\n";
    std::clog << "  (to stderr via clog)\n";
}

// ------------------------------------------------------------
// Formatting with Manipulators
// ------------------------------------------------------------
// Most manipulators are "sticky": they stay in effect until
// changed. std::setw is the exception: it applies to the next
// output only. Sticky state leaking into later output is a
// common annoyance, and one reason std::format exists (05_format).

void manipulators()
{
    std::cout << "\n=== Manipulators ===\n";

    double pi = 3.14159265358979;
    std::cout << "  default        : " << pi << '\n';                               // 3.14159
    std::cout << "  setprecision(3): " << std::setprecision(3) << pi << '\n';       // 3.14 (significant digits)
    std::cout << "  fixed + 3      : " << std::fixed << pi << '\n';                 // 3.142 (decimal places)
    std::cout << std::defaultfloat << std::setprecision(6);                          // reset

    std::cout << "  hex 255        : " << std::hex << 255 << std::dec << '\n';      // ff
    std::cout << "  showbase hex   : " << std::showbase << std::hex << 255 << std::dec << std::noshowbase << '\n';
    std::cout << "  boolalpha      : " << std::boolalpha << true << std::noboolalpha << '\n';

    // A small table: setw for width, left/right for alignment, setfill for padding.
    std::cout << "\n  " << std::left << std::setw(10) << "Item" << std::right << std::setw(8) << "Price" << '\n';
    std::cout << "  " << std::setfill('-') << std::setw(18) << "" << std::setfill(' ') << '\n';
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  " << std::left << std::setw(10) << "Coffee" << std::right << std::setw(8) << 2.5 << '\n';
    std::cout << "  " << std::left << std::setw(10) << "Sandwich" << std::right << std::setw(8) << 4.25 << '\n';
    std::cout << std::defaultfloat << std::setprecision(6);
}

// ------------------------------------------------------------
// Reading Input
// ------------------------------------------------------------
// >> skips leading whitespace and reads one "word" or number.
// std::getline reads a whole line (including spaces).
// Mixing them is the classic trap: >> leaves the '\n' behind, so
// the next getline returns an empty line. Skip it with ignore().

void reading(std::istream& input)
{
    std::cout << "\n=== Reading with >> and getline ===\n";

    int age = 0;
    input >> age;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');   // discard the rest of the line

    std::string full_name;
    std::getline(input, full_name);

    std::cout << "  age = " << age << ", name = \"" << full_name << "\"\n";
}

// ------------------------------------------------------------
// Stream State and Validation
// ------------------------------------------------------------
// A failed read sets failbit and leaves the bad characters in the
// stream. Until you clear() the state, every later read fails too.
// Test the stream itself: "if (input >> x)" is true on success.

void validation(std::istream& input)
{
    std::cout << "\n=== Validating Numeric Input ===\n";

    int value = 0;
    int attempts = 0;
    while (++attempts <= 5) {
        if (input >> value) {
            std::cout << "  read the number " << value << '\n';
            break;
        }
        if (input.eof()) {
            std::cout << "  no more input\n";
            break;
        }
        std::cout << "  not a number, skipping a token\n";
        input.clear();                 // reset failbit
        std::string junk;
        input >> junk;                 // throw away the offending token
    }
}

// Reading until end of input: the loop condition is the read itself.
void sum_all(std::istream& input)
{
    std::cout << "\n=== Reading Until End of Input ===\n";
    double total = 0;
    double x = 0;
    int count = 0;
    while (input >> x) {
        total += x;
        ++count;
    }
    std::cout << "  " << count << " numbers, total " << total << '\n';
}

// ------------------------------------------------------------
// Performance Note
// ------------------------------------------------------------
// By default C++ streams stay synchronised with C's stdio so that
// printf and cout can be mixed. Competitive programmers turn that
// off for speed:
//     std::ios::sync_with_stdio(false);
//     std::cin.tie(nullptr);
// After that, do not mix printf and cout.

int main()
{
    standard_streams();
    manipulators();

    std::istringstream sample1{"42\nAda Lovelace\n"};
    reading(sample1);

    std::istringstream sample2{"abc x9 17"};
    validation(sample2);

    std::istringstream sample3{"1.5 2.5 3 4"};
    sum_all(sample3);
}
