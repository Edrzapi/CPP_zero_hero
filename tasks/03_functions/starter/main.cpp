// ============================================================
// Task 03: Functions and a Menu (starter)
// ============================================================
// Read ../README.md first. era_of and copies_text come from the
// task 01 solution; the rest are stubs. Move each task 01 and
// 02 feature into its function (the code is in
// ../../01_bookshelf/solution/ and ../../02_search_and_report/
// solution/), then write the menu loop in run_menu.
//
// Build and run:
//   cmake --build build --target task03_starter
//   ./build/bin/task03_starter < tasks/data/input_03.txt
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 03 -- build/bin/task03_starter
// ============================================================

#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

// ------------------------------------------------------------
// The Shelf (constants, so a global is safe)
// ------------------------------------------------------------

constexpr std::size_t SIZE = 10;
// [[maybe_unused]] stops Clang warning about these constants
// until your code uses them.
[[maybe_unused]] constexpr int OLD_BEFORE = 1950;

[[maybe_unused]] constexpr std::array<int, SIZE> IDS{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
[[maybe_unused]] constexpr std::array<std::string_view, SIZE> TITLES{
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
[[maybe_unused]] constexpr std::array<int, SIZE> YEARS{1813, 1949, 1937, 1965, 1719, 1895, 1945, 1815, 1969, 1934};
[[maybe_unused]] constexpr std::array<int, SIZE> COPIES{3, 2, 0, 4, 1, 2, 5, 1, 0, 2};

[[maybe_unused]] constexpr std::string_view MENU = "Menu: 1 catalogue | 2 search | 3 report | 4 stats | 0 quit";

// ------------------------------------------------------------
// From Task 01 (done)
// ------------------------------------------------------------

std::string_view era_of(int year)
{
    switch (year / 100) {
    case 17: return "18th century";
    case 18: return "19th century";
    case 19: return "20th century";
    case 20: return "21st century";
    default: return "unknown era";
    }
}

std::string copies_text(int count)
{
    if (count == 0) return "out of stock";
    if (count == 1) return "1 copy";
    return std::to_string(count) + " copies";
}

// ------------------------------------------------------------
// TODO Core 1: one function per feature. Each RETURNS its
// result (a value, or the text to print); only the menu prints.
// ------------------------------------------------------------

// "id | title | year | copies text | century" (+ " | pre-1950")
std::string catalogue_line([[maybe_unused]] std::size_t index)
{
    return "not implemented: catalogue_line";
}

// "Search \"TERM\": N matches" and the matching titles, or "no matches"
std::string search([[maybe_unused]] std::string_view term)
{
    return "not implemented: search";
}

// The task 02 report table, starting with "Report"
std::string report()
{
    return "not implemented: report";
}

int total_copies([[maybe_unused]] const std::array<int, SIZE>& copies)
{
    return 0;   // not implemented
}

int average_year([[maybe_unused]] const std::array<int, SIZE>& years)
{
    return 0;   // not implemented
}

int word_count([[maybe_unused]] const std::array<std::string_view, SIZE>& titles)
{
    return 0;   // not implemented
}

std::string_view longest_title([[maybe_unused]] const std::array<std::string_view, SIZE>& titles)
{
    return "not implemented";
}

// ------------------------------------------------------------
// TODO Core 2 to 5: the menu loop
// ------------------------------------------------------------
// Print "Library Manager", then loop: print MENU, read a line
// with std::getline(std::cin, line) (false at the end of the
// input), trim it, echo it as "> TEXT", and act on it. Compare
// the choice as text. At the end print "Goodbye".

void run_menu()
{
    std::cout << "Library Manager\n";
    std::cout << "not implemented: the menu loop\n";
    std::cout << "Goodbye\n";
}

int main()
{
    run_menu();
}
