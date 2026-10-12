// ============================================================
// Task 02: Search and Report (solution, Core + Stretch)
// ============================================================
// Session 2: loops and strings (see iteration/ and
// basics/08_strings.cpp). Builds on task 01: the same shelf.
//
// Build and run:
//   cmake --build build --target task02_solution
//   ./build/bin/task02_solution              Core output (checked)
//   ./build/bin/task02_solution --stretch    Core, then the Stretch
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 02 -- build/bin/task02_solution
// ============================================================

#include <array>
#include <cctype>
#include <cstddef>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

// ------------------------------------------------------------
// The Shelf from Task 01 (parallel arrays, never resized)
// ------------------------------------------------------------

constexpr std::size_t SIZE = 10;

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

constexpr std::array<std::string_view, 3> search_terms{"the", "TIME", "zzz"};

// ------------------------------------------------------------
// String Helpers
// ------------------------------------------------------------

// std::tolower takes an int that must fit in unsigned char, so
// cast first: a plain char can be negative for non-ASCII bytes.
std::string to_lower(std::string_view text)
{
    std::string lower;
    for (char c : text) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

// >> skips any run of spaces, so "Eighty-Four" stays one word
int count_words(std::string_view text)
{
    std::istringstream words{std::string{text}};
    std::string word;
    int count = 0;
    while (words >> word) {
        ++count;
    }
    return count;
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
    // Core 1: a formatted report table
    // --------------------------------------------------------
    // {:>2} pads on the left (right-aligned), {:<30} pads on the
    // right (left-aligned). Two spaces between the columns.

    std::cout << "Report\n";
    std::cout << std::format("{:>2}  {:<30}  {:<4}  {:>6}\n", "ID", "Title", "Year", "Copies");
    std::cout << std::format("{}  {}  {}  {}\n", std::string(2, '-'), std::string(30, '-'),
                             std::string(4, '-'), std::string(6, '-'));
    for (std::size_t i = 0; i < SIZE; ++i) {
        std::cout << std::format("{:>2}  {:<30}  {:<4}  {:>6}\n", ids[i], titles[i], years[i], copies[i]);
    }

    // --------------------------------------------------------
    // Core 2: case-insensitive search with a while loop
    // --------------------------------------------------------
    // Lower-case both sides, then find() the term anywhere in
    // the title. Matches go into a fixed array plus a count.

    for (std::string_view term : search_terms) {
        const std::string wanted = to_lower(term);
        std::array<std::string_view, SIZE> found{};
        std::size_t count = 0;
        std::size_t i = 0;
        while (i < SIZE) {
            if (to_lower(titles[i]).find(wanted) != std::string::npos) {
                found[count] = titles[i];
                ++count;
            }
            ++i;
        }

        if (count == 0) {
            std::cout << "Search \"" << term << "\": no matches\n";
            continue;   // nothing to list, go to the next term
        }
        std::cout << "Search \"" << term << "\": " << count << (count == 1 ? " match" : " matches") << '\n';
        for (std::size_t j = 0; j < count; ++j) {
            std::cout << "  " << found[j] << '\n';
        }
    }

    // --------------------------------------------------------
    // Core 3 and 4: words in all titles, and the longest title
    // --------------------------------------------------------
    int words = 0;
    for (std::string_view title : titles) {
        words += count_words(title);
    }
    std::cout << "Words in titles: " << words << '\n';

    std::size_t longest = 0;
    for (std::size_t i = 1; i < SIZE; ++i) {
        if (titles[i].size() > titles[longest].size()) {   // > not >=, so a tie keeps the first
            longest = i;
        }
    }
    std::cout << "Longest title: " << titles[longest] << " (" << titles[longest].size() << " characters)\n";

    // --------------------------------------------------------
    // Stretch: initials, the shortest title, a compact list
    // --------------------------------------------------------
    if (has_flag(argc, argv, "--stretch")) {
        std::cout << "Stretch\n";
        for (std::string_view title : titles) {
            std::istringstream title_words{std::string{title}};
            std::string word;
            std::string initials;
            while (title_words >> word) {
                initials += static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
            }
            std::cout << std::format("{:<6}{}\n", initials, title);
        }

        std::size_t shortest = 0;
        for (std::size_t i = 1; i < SIZE; ++i) {
            if (titles[i].size() < titles[shortest].size()) shortest = i;
        }
        std::cout << "Shortest title: " << titles[shortest] << " (" << titles[shortest].size() << " characters)\n";

        // Titles longer than 20 characters are cut to 17 plus "..."
        for (std::size_t i = 0; i < SIZE; ++i) {
            std::string shown{titles[i]};
            if (shown.size() > 20) {
                shown = shown.substr(0, 17) + "...";
            }
            std::cout << std::format("{:>2}  {:<20}  {}\n", ids[i], shown, years[i]);
        }
    }
}
