// ============================================================
// String Streams: Parsing and Building Text
// ============================================================
// std::istringstream reads from a string, std::ostringstream
// writes into one, std::stringstream does both. They are the
// pre-C++20 way to build formatted strings and still the easiest
// way to split and parse text with the stream operators.
//
// Build and run:
//   cmake --build build --target io_02_string_streams
//   ./build/bin/io_02_string_streams
// ============================================================

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Building a String
// ------------------------------------------------------------

std::string make_receipt_line(const std::string& item, int quantity, double price)
{
    std::ostringstream out;
    out << std::left << std::setw(12) << item << " x" << quantity
        << "  " << std::fixed << std::setprecision(2) << quantity * price;
    return out.str();
}

// ------------------------------------------------------------
// Splitting on a Delimiter
// ------------------------------------------------------------
// std::getline takes an optional delimiter character, which makes
// a simple CSV splitter. (Real CSV with quoted fields needs a
// proper parser.)

std::vector<std::string> split(const std::string& text, char delimiter)
{
    std::vector<std::string> parts;
    std::istringstream in{text};
    std::string part;
    while (std::getline(in, part, delimiter)) {
        parts.push_back(part);
    }
    return parts;
}

// ------------------------------------------------------------
// Parsing Typed Fields
// ------------------------------------------------------------
// >> converts as it reads. Check the stream afterwards: a failed
// conversion leaves it in a fail state.

struct Reading {
    std::string sensor;
    double value;
    int timestamp;
};

bool parse_reading(const std::string& line, Reading& out)
{
    std::istringstream in{line};
    if (!(in >> out.sensor >> out.value >> out.timestamp)) {
        return false;
    }
    std::string extra;
    return !(in >> extra);   // reject trailing junk
}

int main()
{
    std::cout << "=== ostringstream ===\n";
    std::cout << "  " << make_receipt_line("Coffee", 2, 2.5) << '\n';
    std::cout << "  " << make_receipt_line("Croissant", 1, 1.85) << '\n';

    std::cout << "\n=== Splitting CSV ===\n";
    for (const auto& field : split("Ada,Lovelace,1815,London", ',')) {
        std::cout << "  [" << field << "]\n";
    }

    std::cout << "\n=== Parsing Typed Fields ===\n";
    for (const char* line : {"temp 21.5 1700000000", "humidity abc 17", "temp 20 5 extra"}) {
        Reading r{};
        if (parse_reading(line, r)) {
            std::cout << "  ok:  " << r.sensor << " = " << r.value << " at " << r.timestamp << '\n';
        } else {
            std::cout << "  bad: \"" << line << "\"\n";
        }
    }
}
