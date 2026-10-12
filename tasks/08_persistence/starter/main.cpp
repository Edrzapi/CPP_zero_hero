// ============================================================
// Task 08: Persistence (starter)
// ============================================================
// Read ../README.md first. library.h and library.cpp are the
// task 07 solution, unchanged. storage.h / storage.cpp hold the
// stubs to fill in; this file runs the Core steps in order.
//
// Build and run (from the repository root):
//   cmake --build build --target task08_starter
//   ./build/bin/task08_starter tasks/data
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 08 -- build/bin/task08_starter tasks/data
// ============================================================

#include "library.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <utility>

namespace fs = std::filesystem;

int run(int argc, char* argv[])
{
    const fs::path data = argc > 1 ? fs::path{argv[1]} : fs::path{"tasks/data"};
    const fs::path out = "out";

    // Core 1: load the real data from disk
    LoadResult loaded = load_books_csv(data / "books.csv");
    LoadResult members = load_members_csv(data / "members.csv");
    std::cout << "Loaded " << loaded.books.size() << " books from books.csv\n";
    std::cout << "Loaded " << members.members.size() << " members from members.csv\n";
    loaded.members = std::move(members.members);
    Library library = build_library(std::move(loaded));

    // Core 2: save as CSV and JSON
    save_books_csv(library, out / "books.csv");
    save_json(library, out / "library.json");

    // TODO Core 2 and 3: print the "Saved ..." lines, load
    // out/books.csv back and compare it with the library.
    std::cout << "not implemented: Core 3: reload and compare\n";

    // TODO Core 4 and 5: "Loading missing.csv" and
    // "Loading books_corrupt.csv", each followed by what happened.
    std::cout << "not implemented: Core 4 and 5: the missing and corrupt files\n";
    return 0;
}

int main(int argc, char* argv[])
{
    // A missing data folder or a folder that cannot be written ends
    // the program with a message, not an unhandled exception
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
