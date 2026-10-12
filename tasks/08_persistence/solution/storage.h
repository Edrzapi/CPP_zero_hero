// ============================================================
// storage.h: CSV and JSON In and Out
// ============================================================
// Loading never crashes on bad data. A bad line is reported as
// "line N: reason" and skipped; the good lines still load.
// A missing file throws FileNotFoundError; a file that cannot be
// understood at all throws StorageError.
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

class StorageError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// What a load produced: the good rows and the problems.
// It owns the books until build_library moves them into a Library.
struct LoadResult {
    std::vector<std::unique_ptr<Book>> books;
    std::vector<Member> members;
    std::vector<std::pair<int, int>> loans;   // (member id, book id), from JSON only
    std::vector<std::string> problems;
    int skipped = 0;

    void skip(int line_number, const std::string& reason);
};

// Reading
LoadResult load_books_csv(const std::filesystem::path& path);
LoadResult load_members_csv(const std::filesystem::path& path);
LoadResult load_json(const std::filesystem::path& path);   // Stretch

// Writing (the folder is created if needed)
void save_books_csv(const Library& library, const std::filesystem::path& path);
void save_json(const Library& library, const std::filesystem::path& path);

// Moves the loaded rows into a new Library. Saved loans go back
// through borrow(), so copies and limits are checked again.
Library build_library(LoadResult&& result);
