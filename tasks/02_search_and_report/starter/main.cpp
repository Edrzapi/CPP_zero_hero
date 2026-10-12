// ============================================================
// Task 02: Search and Report (starter)
// ============================================================
// Read ../README.md first. The shelf from task 01 is below;
// everything new prints "not implemented" until you write it.
// Task 01's full solution is in ../../01_bookshelf/solution/.
//
// Build and run:
//   cmake --build build --target task02_starter
//   ./build/bin/task02_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 02 -- build/bin/task02_starter
// ============================================================

#include <array>
#include <cstddef>
#include <iostream>
#include <string_view>

// Prints a placeholder so the program runs before it is finished.
void not_implemented(std::string_view what)
{
    std::cout << "not implemented: " << what << '\n';
}

// ------------------------------------------------------------
// The Shelf from Task 01 (parallel arrays, never resized)
// ------------------------------------------------------------

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

[[maybe_unused]] constexpr std::array<std::string_view, 3> search_terms{"the", "TIME", "zzz"};

int main()
{
    // TODO Core 1: "Report", the header row, a row of dashes, then
    // one row per book. std::format("{:>2}  {:<30}  {:<4}  {:>6}", ...)
    not_implemented("Core 1: the report table");

    // TODO Core 2: for each term in search_terms, collect the
    // matching titles (ignoring case) with a while loop into a
    // fixed array plus a count, then print them.
    not_implemented("Core 2: search");

    // TODO Core 3: "Words in titles: N"
    not_implemented("Core 3: words in titles");

    // TODO Core 4: "Longest title: TITLE (N characters)"
    not_implemented("Core 4: longest title");
}
