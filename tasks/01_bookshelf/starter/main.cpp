// ============================================================
// Task 01: Bookshelf (starter)
// ============================================================
// Read ../README.md first. This file builds and runs as it is:
// every unfinished part prints "not implemented". Replace each
// not_implemented(...) call with your own code, one Core step
// at a time, and run the check after each one.
//
// Build and run:
//   cmake --build build --target task01_starter
//   ./build/bin/task01_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 01 -- build/bin/task01_starter
// ============================================================

#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

// Prints a placeholder so the program runs before it is finished.
void not_implemented(std::string_view what)
{
    std::cout << "not implemented: " << what << '\n';
}

// ------------------------------------------------------------
// Core 1: the shelf, four parallel arrays (typed in for you)
// ------------------------------------------------------------
// Position i in every array describes the same book.

constexpr std::size_t SIZE = 10;

// [[maybe_unused]] stops Clang warning about these constants
// until your code uses them.
[[maybe_unused]] constexpr std::array<int, SIZE> ids{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
[[maybe_unused]] constexpr std::array<std::string_view, SIZE> titles{
    "Pride and Prejudice",
    "Nineteen Eighty-Four",
    "The Hobbit",
    "Dune",
    "Robinson Crusoe",
    "The Time Machine",
    "Animal Farm",
    "Emma",
    "The Left Hand of Darkness",
    "Murder on the Orient Express",
};
[[maybe_unused]] constexpr std::array<int, SIZE> years{1813, 1949, 1937, 1965, 1719, 1895, 1945, 1815, 1969, 1934};
[[maybe_unused]] constexpr std::array<int, SIZE> copies{3, 2, 0, 4, 1, 2, 5, 1, 0, 2};

// ------------------------------------------------------------
// Core 3 and 4: two helpers
// ------------------------------------------------------------

// TODO Core 4: switch on year / 100 (17 -> "18th century", ...,
// anything else -> "unknown era").
std::string_view era_of([[maybe_unused]] int year)
{
    return "not implemented";
}

// TODO Core 3: "out of stock", "1 copy" or "N copies".
std::string copies_text([[maybe_unused]] int count)
{
    return "not implemented";
}

int main()
{
    // TODO Core 2 and 5: "Catalogue (10 books)", then one line per
    // book: id | title | year | copies text | century, plus
    // " | pre-1950" when the year is before 1950.
    not_implemented("Core 2: the catalogue");

    // TODO Core 6: copy the odd-id books into kept_ids and
    // kept_titles, count them, then print "Odd ids only (N books)"
    // and "id | title" for each one.
    not_implemented("Core 6: odd ids only");

    // TODO Core 7: "Totals", then books, copies, out of stock,
    // pre-1950 and the average year (integer division).
    not_implemented("Core 7: totals");
}
