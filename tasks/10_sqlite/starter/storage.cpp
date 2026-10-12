// ============================================================
// storage.cpp: Definitions for storage.h
// ============================================================

#include "storage.h"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <fstream>
#include <string_view>
#include <system_error>
#include <utility>

namespace {

constexpr std::size_t BOOK_FIELDS = 8;
constexpr std::size_t MEMBER_FIELDS = 5;
constexpr std::size_t LOAN_FIELDS = 5;

// Every line of a text file, without the line endings
std::vector<std::string> read_lines(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw FileNotFoundError("file not found: " + path.generic_string());
    }
    std::ifstream file{path, std::ios::binary};
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

// Splits on commas and keeps empty fields: "3,4,103,2026-09-05,"
// is 5 fields, the last one empty (a loan still out)
std::vector<std::string> split_fields(const std::string& line)
{
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
}

// (line number, fields) for every data row with the right width.
// Rows with the wrong width are reported and skipped.
std::vector<std::pair<int, std::vector<std::string>>> read_rows(const std::filesystem::path& path,
                                                                std::size_t width, LoadResult& result)
{
    const std::vector<std::string> lines = read_lines(path);
    std::vector<std::pair<int, std::vector<std::string>>> rows;
    for (std::size_t i = 1; i < lines.size(); ++i) {   // lines[0] is the header
        const int line_number = static_cast<int>(i) + 1;
        if (lines[i].empty()) continue;
        std::vector<std::string> row = split_fields(lines[i]);
        if (row.size() != width) {
            result.skip(line_number, "expected " + std::to_string(width) + " fields, found " +
                                         std::to_string(row.size()));
            continue;
        }
        rows.emplace_back(line_number, std::move(row));
    }
    return rows;
}

int whole_number(const std::string& text, std::string_view name)
{
    int value = 0;
    const char* last = text.data() + text.size();
    const auto [end, error] = std::from_chars(text.data(), last, value);
    if (text.empty() || error != std::errc{} || end != last) {
        throw std::invalid_argument(std::string(name) + " must be a whole number, got '" + text + "'");
    }
    return value;
}

Date parse_date(const std::string& text, std::string_view name)
{
    const std::optional<Date> day = parse_iso_date(text);
    if (!day) {
        throw std::invalid_argument(std::string(name) + " must be a date YYYY-MM-DD, got '" + text + "'");
    }
    return *day;
}

}  // namespace

void LoadResult::skip(int line_number, const std::string& reason)
{
    problems.push_back("line " + std::to_string(line_number) + ": " + reason);
    ++skipped;
}

void load_books_csv(const std::filesystem::path& path, LoadResult& result)
{
    std::vector<int> seen;
    for (const auto& [line_number, row] : read_rows(path, BOOK_FIELDS, result)) {
        std::unique_ptr<Book> book;
        try {
            whole_number(row[0], "id");
            whole_number(row[3], "year");
            whole_number(row[5], "copies");
            whole_number(row[7], "length");
            book = make_book(row);
        } catch (const std::invalid_argument& error) {
            result.skip(line_number, error.what());
            continue;
        }
        if (std::find(seen.begin(), seen.end(), book->id()) != seen.end()) {
            result.skip(line_number, "duplicate book id " + std::to_string(book->id()));
            continue;
        }
        seen.push_back(book->id());
        result.books.push_back(std::move(book));
    }
}

void load_members_csv(const std::filesystem::path& path, LoadResult& result)
{
    for (const auto& [line_number, row] : read_rows(path, MEMBER_FIELDS, result)) {
        try {
            parse_date(row[4], "joined");
            result.members.emplace_back(whole_number(row[0], "id"), row[1], row[2], row[3], row[4]);
        } catch (const std::invalid_argument& error) {
            result.skip(line_number, error.what());
        }
    }
}

void load_loans_csv(const std::filesystem::path& path, LoadResult& result)
{
    for (const auto& [line_number, row] : read_rows(path, LOAN_FIELDS, result)) {
        try {
            Loan loan;
            loan.id = whole_number(row[0], "id");
            loan.book_id = whole_number(row[1], "book_id");
            loan.member_id = whole_number(row[2], "member_id");
            loan.borrowed = parse_date(row[3], "borrowed");
            if (!row[4].empty()) {   // an empty field means still on loan
                loan.returned = parse_date(row[4], "returned");
            }
            result.loans.push_back(loan);
        } catch (const std::invalid_argument& error) {
            result.skip(line_number, error.what());
        }
    }
}

Library build_library(LoadResult&& result)
{
    Library library;
    for (auto& book : result.books) {
        library.add_book(std::move(book));
    }
    for (auto& member : result.members) {
        library.add_member(std::move(member));
    }
    for (const Loan& loan : result.loans) {
        library.add_loan(loan);
    }
    return library;
}

Library load_all(const std::filesystem::path& data)
{
    LoadResult result;
    load_books_csv(data / "books.csv", result);
    load_members_csv(data / "members.csv", result);
    load_loans_csv(data / "loans.csv", result);
    return build_library(std::move(result));
}
