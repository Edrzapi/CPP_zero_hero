// ============================================================
// shelf.cpp: Definitions for shelf.h
// ============================================================

#include "shelf.h"

#include <cctype>
#include <format>
#include <sstream>

namespace {

// Internal linkage (unnamed namespace): only this file can call
// it, so it never clashes with a to_lower somewhere else.
std::string to_lower(std::string_view text)
{
    std::string lower;
    for (char c : text) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

}  // namespace

// ------------------------------------------------------------
// Small Helpers
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

std::string plural(std::size_t count, std::string_view singular, std::string_view many)
{
    return std::to_string(count) + " " + std::string(count == 1 ? singular : many);
}

// ------------------------------------------------------------
// Features
// ------------------------------------------------------------

std::string catalogue_line(std::size_t index)
{
    std::string line = std::format("{} | {} | {} | {} | {}", IDS[index], TITLES[index], YEARS[index],
                                   copies_text(COPIES[index]), era_of(YEARS[index]));
    if (YEARS[index] < OLD_BEFORE) {
        line += " | pre-1950";
    }
    return line;
}

std::string catalogue()
{
    std::string text = std::format("Catalogue ({} books)\n", SIZE);
    for (std::size_t i = 0; i < SIZE; ++i) {
        text += catalogue_line(i) + '\n';
    }
    return text;
}

std::string search(std::string_view term)
{
    const std::string wanted = to_lower(term);
    std::string lines;
    std::size_t count = 0;
    for (std::string_view title : TITLES) {
        if (to_lower(title).find(wanted) != std::string::npos) {
            lines += std::format("  {}\n", title);
            ++count;
        }
    }
    if (count == 0) {
        return std::format("Search \"{}\": no matches\n", term);
    }
    return std::format("Search \"{}\": {}\n", term, plural(count, "match", "matches")) + lines;
}

std::string report()
{
    std::string text = "Report\n";
    text += std::format("{:>2}  {:<30}  {:<4}  {:>6}\n", "ID", "Title", "Year", "Copies");
    text += std::format("{}  {}  {}  {}\n", std::string(2, '-'), std::string(30, '-'), std::string(4, '-'),
                        std::string(6, '-'));
    for (std::size_t i = 0; i < SIZE; ++i) {
        text += std::format("{:>2}  {:<30}  {:<4}  {:>6}\n", IDS[i], TITLES[i], YEARS[i], COPIES[i]);
    }
    return text;
}

int total_copies(const std::array<int, SIZE>& copies)
{
    int total = 0;
    for (int count : copies) total += count;
    return total;
}

int count_out_of_stock(const std::array<int, SIZE>& copies)
{
    int count = 0;
    for (int c : copies) {
        if (c == 0) ++count;
    }
    return count;
}

int count_old(const std::array<int, SIZE>& years)
{
    int count = 0;
    for (int year : years) {
        if (year < OLD_BEFORE) ++count;
    }
    return count;
}

int average_year(const std::array<int, SIZE>& years)
{
    int total = 0;
    for (int year : years) total += year;
    return total / static_cast<int>(years.size());   // integer division
}

int word_count(const std::array<std::string_view, SIZE>& titles)
{
    int words = 0;
    for (std::string_view title : titles) {
        std::istringstream stream{std::string{title}};
        std::string word;
        while (stream >> word) ++words;
    }
    return words;
}

std::string_view longest_title(const std::array<std::string_view, SIZE>& titles)
{
    std::string_view longest = titles[0];
    for (std::string_view title : titles) {
        if (title.size() > longest.size()) longest = title;
    }
    return longest;
}

std::pair<std::size_t, std::size_t> oldest_and_newest(const std::array<int, SIZE>& years)
{
    std::size_t oldest = 0;
    std::size_t newest = 0;
    for (std::size_t i = 1; i < years.size(); ++i) {
        if (years[i] < years[oldest]) oldest = i;
        if (years[i] > years[newest]) newest = i;
    }
    return {oldest, newest};
}

std::string stats(bool include_stretch)
{
    const std::string_view longest = longest_title(TITLES);
    std::string text = "Stats\n";
    text += std::format("books: {}\n", SIZE);
    text += std::format("copies: {}\n", total_copies(COPIES));
    text += std::format("out of stock: {}\n", count_out_of_stock(COPIES));
    text += std::format("pre-1950: {}\n", count_old(YEARS));
    text += std::format("average year: {}\n", average_year(YEARS));
    text += std::format("words in titles: {}\n", word_count(TITLES));
    text += std::format("longest title: {} ({} characters)\n", longest, longest.size());
    if (include_stretch) {
        const auto [oldest, newest] = oldest_and_newest(YEARS);   // structured bindings
        text += std::format("oldest: {} ({})\n", TITLES[oldest], YEARS[oldest]);
        text += std::format("newest: {} ({})\n", TITLES[newest], YEARS[newest]);
    }
    return text;
}
