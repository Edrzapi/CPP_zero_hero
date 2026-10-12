// ============================================================
// storage.h: CSV and JSON In and Out (starter)
// ============================================================
// Loading never crashes on bad data. A bad line is reported as
// "line N: reason" and skipped; the good lines still load.
// A missing file throws FileNotFoundError.
// ============================================================

#pragma once

#include "library.h"

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class FileNotFoundError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// What a load produced: the good rows and the problems.
// It owns the books until build_library moves them into a Library.
struct LoadResult {
    std::vector<std::unique_ptr<Book>> books;
    std::vector<Member> members;
    std::vector<std::string> problems;
    int skipped = 0;

    void skip(int line_number, const std::string& reason);
};

// Reading
LoadResult load_books_csv(const std::filesystem::path& path);
LoadResult load_members_csv(const std::filesystem::path& path);

// Writing (create the folder if needed)
void save_books_csv(const Library& library, const std::filesystem::path& path);
void save_json(const Library& library, const std::filesystem::path& path);

// Moves the loaded rows into a new Library
Library build_library(LoadResult&& result);
