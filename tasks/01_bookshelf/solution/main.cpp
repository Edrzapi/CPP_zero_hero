// ============================================================
// Task 01: Bookshelf (solution, Core + Stretch)
// ============================================================
// Session 1: types, operators, if/else, switch and fixed-size
// arrays (see basics/ and control_flow/).
//
// Build and run:
//   cmake --build build --target task01_solution
//   ./build/bin/task01_solution              Core output (checked)
//   ./build/bin/task01_solution --stretch    Core, then the Stretch
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 01 -- build/bin/task01_solution
// ============================================================

#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

// ------------------------------------------------------------
// The Shelf: Four Parallel Arrays, One per Field
// ------------------------------------------------------------
// std::array has its size in its type, so it can never grow or
// shrink. Position i in every array describes the same book.

constexpr std::size_t SIZE = 10;
constexpr int OLD_BEFORE = 1950;

constexpr std::array<int, SIZE> ids{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
constexpr std::array<std::string_view, SIZE> titles{
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
constexpr std::array<int, SIZE> years{1813, 1949, 1937, 1965, 1719, 1895, 1945, 1815, 1969, 1934};
constexpr std::array<int, SIZE> copies{3, 2, 0, 4, 1, 2, 5, 1, 0, 2};

// ------------------------------------------------------------
// Helpers: switch on the century, if/else on the copies
// ------------------------------------------------------------

std::string_view era_of(int year)
{
    // / on two ints is integer division: 1813 / 100 is 18
    switch (year / 100) {
    case 17: return "18th century";   // return leaves the switch, so no break needed
    case 18: return "19th century";
    case 19: return "20th century";
    case 20: return "21st century";
    default: return "unknown era";
    }
}

std::string copies_text(int count)
{
    if (count == 0) {
        return "out of stock";
    } else if (count == 1) {
        return "1 copy";
    } else {
        return std::to_string(count) + " copies";
    }
}

bool has_flag(int argc, char* argv[], std::string_view flag)
{
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    // --------------------------------------------------------
    // Core 1 to 3, 5: the catalogue with copies, era and flag
    // --------------------------------------------------------
    std::cout << "Catalogue (" << SIZE << " books)\n";
    for (std::size_t i = 0; i < SIZE; ++i) {
        std::cout << ids[i] << " | " << titles[i] << " | " << years[i] << " | "
                  << copies_text(copies[i]) << " | " << era_of(years[i]);
        if (years[i] < OLD_BEFORE) {
            std::cout << " | pre-1950";
        }
        std::cout << '\n';
    }

    // --------------------------------------------------------
    // Core 6: "remove" the even ids
    // --------------------------------------------------------
    // A fixed array cannot shrink, so the books we keep are
    // copied into a second pair of arrays and counted.

    std::array<int, SIZE> kept_ids{};                // {} fills with zeros
    std::array<std::string_view, SIZE> kept_titles{};
    std::size_t kept = 0;
    for (std::size_t i = 0; i < SIZE; ++i) {
        if (ids[i] % 2 != 0) {                       // % is the remainder: odd ids leave 1
            kept_ids[kept] = ids[i];
            kept_titles[kept] = titles[i];
            ++kept;
        }
    }

    std::cout << "Odd ids only (" << kept << " books)\n";
    for (std::size_t i = 0; i < kept; ++i) {
        std::cout << kept_ids[i] << " | " << kept_titles[i] << '\n';
    }

    // --------------------------------------------------------
    // Core 7: totals with arithmetic and comparison operators
    // --------------------------------------------------------
    int total_copies = 0;
    int out_of_stock = 0;
    int old_books = 0;
    int year_sum = 0;
    for (std::size_t i = 0; i < SIZE; ++i) {
        total_copies += copies[i];
        year_sum += years[i];
        if (copies[i] == 0) ++out_of_stock;
        if (years[i] < OLD_BEFORE) ++old_books;
    }

    std::cout << "Totals\n";
    std::cout << "books: " << SIZE << '\n';
    std::cout << "copies: " << total_copies << '\n';
    std::cout << "out of stock: " << out_of_stock << '\n';
    std::cout << "pre-1950: " << old_books << '\n';
    // Both sides int, so this is integer division and rounds down.
    // static_cast stops SIZE (unsigned) turning the sum unsigned.
    std::cout << "average year: " << year_sum / static_cast<int>(SIZE) << '\n';

    // --------------------------------------------------------
    // Stretch: oldest and newest, and a count per century
    // --------------------------------------------------------
    if (has_flag(argc, argv, "--stretch")) {
        std::size_t oldest = 0;
        std::size_t newest = 0;
        for (std::size_t i = 1; i < SIZE; ++i) {
            if (years[i] < years[oldest]) oldest = i;
            if (years[i] > years[newest]) newest = i;
        }
        std::cout << "Stretch\n";
        std::cout << "oldest: " << titles[oldest] << " (" << years[oldest] << ")\n";
        std::cout << "newest: " << titles[newest] << " (" << years[newest] << ")\n";

        // One counter per century, in a fixed order
        constexpr std::array<std::string_view, 4> era_names{
            "18th century", "19th century", "20th century", "21st century"};
        std::array<int, 4> era_counts{};
        for (std::size_t i = 0; i < SIZE; ++i) {
            for (std::size_t e = 0; e < era_names.size(); ++e) {
                if (era_of(years[i]) == era_names[e]) ++era_counts[e];
            }
        }
        for (std::size_t e = 0; e < era_names.size(); ++e) {
            std::cout << era_names[e] << ": " << era_counts[e] << '\n';
        }
    }
}
