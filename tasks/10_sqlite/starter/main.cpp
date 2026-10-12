// ============================================================
// Task 10: SQLite Storage (starter)
// ============================================================
// Read ../README.md first. library and storage are the task 09
// solution, unchanged. db.h / db.cpp start the database layer.
//
// Build and run (from the repository root):
//   cmake --build build --target task10_starter
//   ./build/bin/task10_starter tasks/data 2026-10-12
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 10 -- build/bin/task10_starter tasks/data
// ============================================================

#include "db.h"
#include "library.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int run(int argc, char* argv[])
{
    const fs::path data = argc > 1 ? fs::path{argv[1]} : "tasks/data";

    // Core 2: a fresh database (the folder first, then delete the old file)
    const fs::path db_path = fs::path{"out"} / "library.db";
    fs::create_directories(db_path.parent_path());
    fs::remove(db_path);
    Database db{db_path};
    db.exec("PRAGMA foreign_keys = ON");

    // TODO Core 2: run data / "schema.sql" and print
    // "Created out/library.db from schema.sql"
    std::cout << "not implemented: Core 2: run " << (data / "schema.sql").generic_string() << '\n';

    // TODO Core 3: import the CSV files in one transaction
    std::cout << "not implemented: Core 3: import\n";

    // TODO Core 4: the four queries
    std::cout << "not implemented: Core 4: queries\n";

    // TODO Core 5 and 6: the task 09 actions, saved to the database,
    // then reload and print the overdue report
    std::cout << "not implemented: Core 5 and 6: new loans and reload\n";
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
