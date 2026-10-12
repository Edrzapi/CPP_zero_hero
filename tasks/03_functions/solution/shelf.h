// ============================================================
// shelf.h: Every Task 01 and 02 Feature as a Function
// ============================================================
// Stretch 1: declarations here, definitions in shelf.cpp, and
// main.cpp includes this header (see compilation/02_multi_file).
// Nothing in shelf.cpp prints: every function returns a value or
// the text to print, and the menu in main.cpp decides.
// ============================================================

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

// ------------------------------------------------------------
// The Shelf (constants, so sharing them is safe)
// ------------------------------------------------------------
// inline constexpr: one definition shared by every .cpp file
// that includes this header (C++17 inline variables).

inline constexpr std::size_t SIZE = 10;
inline constexpr int OLD_BEFORE = 1950;

inline constexpr std::array<int, SIZE> IDS{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
inline constexpr std::array<std::string_view, SIZE> TITLES{
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
inline constexpr std::array<int, SIZE> YEARS{1813, 1949, 1937, 1965, 1719, 1895, 1945, 1815, 1969, 1934};
inline constexpr std::array<int, SIZE> COPIES{3, 2, 0, 4, 1, 2, 5, 1, 0, 2};

// ------------------------------------------------------------
// Small Helpers (task 01)
// ------------------------------------------------------------

std::string_view era_of(int year);
std::string copies_text(int count);
std::string plural(std::size_t count, std::string_view singular, std::string_view many);

// ------------------------------------------------------------
// Features (tasks 01 and 02). Each returns the text to print,
// one line per '\n', or a single value.
// ------------------------------------------------------------

std::string catalogue_line(std::size_t index);
std::string catalogue();
std::string search(std::string_view term);
std::string report();

int total_copies(const std::array<int, SIZE>& copies);
int count_out_of_stock(const std::array<int, SIZE>& copies);
int count_old(const std::array<int, SIZE>& years);
int average_year(const std::array<int, SIZE>& years);
int word_count(const std::array<std::string_view, SIZE>& titles);
std::string_view longest_title(const std::array<std::string_view, SIZE>& titles);

// Stretch 2: two results from one function (indexes into the shelf)
std::pair<std::size_t, std::size_t> oldest_and_newest(const std::array<int, SIZE>& years);

std::string stats(bool include_stretch = false);
