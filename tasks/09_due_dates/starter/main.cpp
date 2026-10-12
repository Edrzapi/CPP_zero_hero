// ============================================================
// Task 09: Loans with Due Dates (starter)
// ============================================================
// Read ../README.md first. library, storage and json are the
// task 08 solution, unchanged. The new pieces start in loans.h.
//
// Build and run (from the repository root):
//   cmake --build build --target task09_starter
//   ./build/bin/task09_starter tasks/data 2026-10-12
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 09 -- build/bin/task09_starter tasks/data
// ============================================================

#include "library.h"
#include "loans.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

int run(int argc, char* argv[])
{
    const std::filesystem::path data = argc > 1 ? std::filesystem::path{argv[1]} : "tasks/data";
    const std::string today = argc > 2 ? argv[2] : "2026-10-12";   // never the real clock

    LoadResult loaded = load_books_csv(data / "books.csv");
    LoadResult members = load_members_csv(data / "members.csv");
    loaded.members = std::move(members.members);
    Library library = build_library(std::move(loaded));

    std::cout << "Today is " << today << '\n';

    // TODO Core 2: load loans.csv into the library
    // TODO Core 6: the loans table, the overdue report, the fines
    std::cout << "not implemented: Core 6: loans table, overdue report and fines\n";

    // TODO Core 7: 105 borrows 15, 105 borrows 16, 102 returns 14
    std::cout << "not implemented: Core 7: new loans\n";
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
