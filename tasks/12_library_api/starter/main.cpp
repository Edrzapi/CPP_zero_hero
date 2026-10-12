// ============================================================
// Task 12: Expose the Library (starter)
// ============================================================
// Read ../README.md first. library, storage and json come from
// the earlier solutions. server.cpp and api.cpp are stubs.
//
// Build and run (from the repository root):
//   cmake --build build --target task12_starter
//   ./build/bin/task12_starter tasks/data 8080
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 12 -- build/bin/task12_starter tasks/data {port}
// ============================================================

#include "api.h"
#include "library.h"
#include "server.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

int run(int argc, char* argv[])
{
    const std::filesystem::path data = argc > 1 ? std::filesystem::path{argv[1]} : "tasks/data";
    const int port = argc > 2 ? std::stoi(argv[2]) : 8080;
    const Date today = parse_iso_date("2026-10-12").value();   // fixed: never the real clock

    // A fresh library from the CSV files on every start
    LibraryApi api{load_all(data), today};
    serve(port, [&api](const Request& request) { return api.handle(request); },
          [port] { std::cout << "Library API on http://localhost:" << port << std::endl; });
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
