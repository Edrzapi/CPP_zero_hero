// ============================================================
// storage.cpp: Definitions for storage.h (starter)
// ============================================================
// LoadResult::skip and build_library are done. The four stubs
// print "not implemented" until you write them.
// ============================================================

#include "storage.h"

#include <fstream>
#include <iostream>

void LoadResult::skip(int line_number, const std::string& reason)
{
    problems.push_back("line " + std::to_string(line_number) + ": " + reason);
    ++skipped;
}

// TODO Core 1 and 5: open the file with std::ifstream (throw
// FileNotFoundError if std::filesystem::exists says it is not
// there), skip the header, and for each line run the checks in
// the README in order, calling skip() for a bad line.
LoadResult load_books_csv(const std::filesystem::path& path)
{
    std::cout << "not implemented: load_books_csv(" << path.generic_string() << ")\n";
    return {};
}

LoadResult load_members_csv(const std::filesystem::path& path)
{
    std::cout << "not implemented: load_members_csv(" << path.generic_string() << ")\n";
    return {};
}

// TODO Core 2: the same header and columns as data/books.csv.
// Open with std::ios::binary so "\n" stays "\n" on Windows.
void save_books_csv([[maybe_unused]] const Library& library, const std::filesystem::path& path)
{
    std::cout << "not implemented: save_books_csv(" << path.generic_string() << ")\n";
}

// TODO Core 2: the shape in the README, written by hand. Escape
// " and \ inside text.
void save_json([[maybe_unused]] const Library& library, const std::filesystem::path& path)
{
    std::cout << "not implemented: save_json(" << path.generic_string() << ")\n";
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
    return library;
}
