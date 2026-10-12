// ============================================================
// storage.h: CSV Files (unchanged from task 09)
// ============================================================
// Used by the Stretch test of the corrupt CSV loader.
// ============================================================

#pragma once

#include "library.h"

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class FileNotFoundError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// What a load produced: the good rows and the problems
struct LoadResult {
    std::vector<std::unique_ptr<Book>> books;
    std::vector<Member> members;
    std::vector<Loan> loans;
    std::vector<std::string> problems;
    int skipped = 0;

    void skip(int line_number, const std::string& reason);
};

// Each loader adds to result, so one LoadResult can collect all three files
void load_books_csv(const std::filesystem::path& path, LoadResult& result);
void load_members_csv(const std::filesystem::path& path, LoadResult& result);
void load_loans_csv(const std::filesystem::path& path, LoadResult& result);

// Moves the loaded rows into a new Library (open loans take a copy each)
Library build_library(LoadResult&& result);

// books.csv, members.csv and loans.csv from one folder
Library load_all(const std::filesystem::path& data);
